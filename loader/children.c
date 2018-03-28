/*******************************************************************************/
/*  © Université Lille 1, The Pip Development Team (2015-2016)                 */
/*                                                                             */
/*  This software is a computer program whose purpose is to run a minimal,     */
/*  hypervisor relying on proven properties such as memory isolation.          */
/*                                                                             */
/*  This software is governed by the CeCILL license under French law and       */
/*  abiding by the rules of distribution of free software.  You can  use,      */
/*  modify and/ or redistribute the software under the terms of the CeCILL     */
/*  license as circulated by CEA, CNRS and INRIA at the following URL          */
/*  "http://www.cecill.info".                                                  */
/*                                                                             */
/*  As a counterpart to the access to the source code and  rights to copy,     */
/*  modify and redistribute granted by the license, users are provided only    */
/*  with a limited warranty  and the software's author,  the holder of the     */
/*  economic rights,  and the successive licensors  have only  limited         */
/*  liability.                                                                 */
/*                                                                             */
/*  In this respect, the user's attention is drawn to the risks associated     */
/*  with loading,  using,  modifying and/or developing or reproducing the      */
/*  software by the user in light of its specific status of free software,     */
/*  that may mean  that it is complicated to manipulate,  and  that  also      */
/*  therefore means  that it is reserved for developers  and  experienced      */
/*  professionals having in-depth computer knowledge. Users are therefore      */
/*  encouraged to load and test the software's suitability as regards their    */
/*  requirements in conditions enabling the security of their systems and/or   */
/*  data to be ensured and,  more generally, to use and operate it in the      */
/*  same conditions as regards security.                                       */
/*                                                                             */
/*  The fact that you are presently reading this means that you have had       */
/*  knowledge of the CeCILL license and that you accept its terms.             */
/*******************************************************************************/

#include <stdint.h>
#include <pip/api.h>
#include <pip/vidt.h>
#include <pip/paging.h>
#include <pip/compat.h>
#include <pip/debug.h>
#include <string.h>

#include "minako.h"

#define LOAD_LOG(a) puts("[MINAKO:LOAD] "); puts(a)

int BootstrapPartition(uint32_t base, uint32_t length, uint32_t load_addr, task_t* part)
{
	uint32_t offset;
	void *jspd, *jssh1, *jssh2, *jssh3;
	
	uint32_t laddr = load_addr;
	
	LOAD_LOG("Allocating pages for partition... ");
	part->part = allocPage();
	jspd = allocPage();
	jssh1 = allocPage();
	jssh2 = allocPage();
	jssh3 = allocPage();
	puts("done.\n");
	LOAD_LOG("\tPartition descriptor : ");	puthex((uint32_t)part->part); puts("\n");
	LOAD_LOG("\tpd : "); puthex((uint32_t)jspd); puts("\n");
	
	LOAD_LOG("Creating partition... ");
	memset(jspd, 0x0, 0x1000);
	if(createPartition((uint32_t)part->part, (uint32_t)jspd, (uint32_t)jssh1, (uint32_t)jssh2, (uint32_t)jssh3))
		puts("done.\n");
	else
		goto fail;
	
	/* Now, map the whole kernel thing */
	LOAD_LOG("Mapping partition... ");
	for(offset = 0; offset < length; offset+=0x1000){
		if (mapPageWrapper((uint32_t)(base + offset), (uint32_t)part->part, (uint32_t)(laddr + offset))){
			LOAD_LOG("Failed to map source ");
			puthex(base + offset);
			puts(" to destination ");
			puthex(laddr + offset);
			puts("\n");
			goto fail;
		}
		
		if(offset % 0x10000 == 0)
			puts(".");
	}
	
	uint32_t lastPage = laddr + offset;
	puts("done, mapped until "); puthex(lastPage); puts("\n");
	
	LOAD_LOG("Mapping stack... ");
	uint32_t stack_off = 0;
    for(stack_off = 0; stack_off <= 0x100000; stack_off+=0x1000)
    {
        uint32_t stack_addr = (uint32_t)allocPage();
	    if(mapPageWrapper((uint32_t)stack_addr, (uint32_t)part->part, (uint32_t)0x2000000 + (stack_off)))
	    {
		    LOAD_LOG("Couldn't map stack.\n");
		    goto fail;
	    } else {
	    }
    }
    puts("\n");

	LOAD_LOG("Mapping some memory for JS heap... ");
    uint32_t heapoff = 0;
    for(heapoff = 0; heapoff < 0x100000; heapoff += 0x1000)
    {
    	uint32_t heap_addr = (uint32_t)allocPage();
    	if(mapPageWrapper((uint32_t)heap_addr, (uint32_t)part->part, (uint32_t)(0x1000000 + heapoff)))
    	{
    		LOAD_LOG("Couldn't map heap.\n");
    		goto fail;
    	} else {
    	}
    }
    puts("\n");

    LOAD_LOG("Mapping interrupt stack... ");
	uint32_t istack_addr = (uint32_t)allocPage();
	if(mapPageWrapper((uint32_t)istack_addr, (uint32_t)part->part, (uint32_t)0x804000))
	{
		LOAD_LOG("Couldn't map stack.\n");
		goto fail;
	} else {
		puts("done.\n");
	}
	
	/* Prepare the virtual interrupt vector to allow the partition to boot. */
	part->vidt = (vidt_t*)allocPage();
	puts("vidt at ");puthex((uint32_t)part->vidt);puts("\n");
	part->vidt->vint[0].eip = laddr;
	part->vidt->vint[0].esp = 0x2100000 + 0x1000 - sizeof(uint32_t);
	part->vidt->flags = 0x1;

	
	/* Finally, map VIDT */
	if (mapPageWrapper((uint32_t)part->vidt, (uint32_t)part->part, (uint32_t)0xFFFFF000))
		goto fail;
	
	part->state = BOOTSTRAPPED;
	
	return 0;
fail:
	puts("fail\n");
	return -1;
}
