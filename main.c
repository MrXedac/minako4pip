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
#include <pip/fpinfo.h>
#include <pip/paging.h>
#include <pip/vidt.h>
#include <pip/api.h>
#include <pip/compat.h>
#include <pip/debug.h>

#include "minako.h"
#include "stdlib.h"

#ifndef NULL
#define NULL ((void*)0)
#endif


task_t jsDescriptor;
uint32_t curPart;

#define PANIC() {vcli(); for(;;);}
#define MIN(a,b) ((a)<(b) ? (a) : (b))

/**
 * Page fault irq handler
 */
INTERRUPT_HANDLER(pfAsm, pfHandler)
log("Page fault :(\n");
for(;;);
END_OF_INTERRUPT

uint32_t counter = 0;

/* 
 * init_memory_mapping: [mem 0x00700000-0x011fffff]
 * init_memory_mapping: [mem 0x01202000-0x0fbfffff]
 */
INTERRUPT_HANDLER(signalAsm, signalHandler)
    log("Received count() call from JS interpreter, processing.\n");
    counter++;
    int_ctx_t *intctx = (int_ctx_t*)(((uint32_t)jsDescriptor.vidt) + 0xF0C);
    // log("VIDT at "); puthex((uint32_t)jsDescriptor.vidt); puts("\n");
    // log("Context buffer at "); puthex((uint32_t)intctx); puts("\n");
    intctx->regs.eax = counter;
    Pip_Resume((uint32_t)jsDescriptor.part, 0x1);
END_OF_INTERRUPT

/**
 * General protection failure irq handler
 */
INTERRUPT_HANDLER(gpfAsm, gpfHandler)

END_OF_INTERRUPT

INTERRUPT_HANDLER(timerAsm, timerHandler)
	if(jsDescriptor.state == RUNNING) {
		Pip_Resume((uint32_t)jsDescriptor.part, 1);
	} else {
		jsDescriptor.state = RUNNING;
		Pip_Notify((uint32_t)jsDescriptor.part, 0, 0, 0);
	}
END_OF_INTERRUPT

/*
 * Prepares the fake interrupt vector to receive new interrupts
 */
void initInterrupts()
{
	//registerInterrupt(33, &timerAsm, (uint32_t*)0x2020000); // We can use the same stack for both interrupts, or choose different stacks, let's play a bit
	registerInterrupt(33, &timerAsm, (uint32_t*)0x2020000); // We can use the same stack for both interrupts, or choose different stacks, let's play a bit
	registerInterrupt(14, &gpfAsm, (uint32_t*)0x2030000); /* General Protection Fault */
	registerInterrupt(15, &pfAsm, (uint32_t*)0x2040000); /* Page Fault */
	registerInterrupt(0x88, &signalAsm, (uint32_t*)0x2050000); /* Generic guest-to-host signal handler */
	return;
}

void Minako_Init(pip_fpinfo* bootinfo)
{
	if(bootinfo->magic == FPINFO_MAGIC)
	log("\tBootinfo seems to be correct.\n");
	else {
	log("\tBootinfo is invalid. Aborting.\n");
	PANIC();
	}

	log("\tAvailable memory starts at ");
	puthex((uint32_t)bootinfo->membegin);
	puts(" and ends at ");
	puthex((uint32_t)bootinfo->memend);
	puts("\n");

	log("\tPip revision ");
	puts(bootinfo->revision);
	puts("\n");

    vgaputs("Pip proto-kernel, revision ");
    vgaputs(bootinfo->revision);
    vgaputs("\n");

	return;
}

void Minako_Banner()
{
    vgaputs("  __  __ _             _         \n");
    vgaputs(" |  \\/  (_)_ __   __ _| | _____  \n");
    vgaputs(" | |\\/| | | '_ \\ / _` | |/ / _ \\ \n");
    vgaputs(" | |  | | | | | | (_| |   < (_) |\n");
    vgaputs(" |_|  |_|_|_| |_|\\__,_|_|\\_\\___/ \n");
    vgaputs("                                 \n");
    vgaputs("\nMinako loader for Pip - Target ");
    vgaputs(MINAKO_TARGET);
    vgaputs("\nVersion ");
    vgaputs(MINAKO_VERSION);
    vgaputs(", codename ");
    vgaputs(MINAKO_CODENAME);
    vgaputs("\n");
}

void main(pip_fpinfo* bootinfo)
{
    init_video();
    Minako_Banner();

    log("Loading Minako...\n");

	log("Pip BootInfo: \n");
    Minako_Init(bootinfo);

	log("Initializing paging.\n");
	initPaging((void*)bootinfo->membegin, (void*)bootinfo->memend);

	log("Initializing interrupts... ");
	initInterrupts();
	puts("done.\n");

	/* log("Image info: \n");
	log("\tEmbedded image address : "); puthex(js.start); puts("\n");
	log("\tEmbedded image end : "); puthex(js.end); puts("\n");
	log("\tEmbedded image size : "); puthex(js.end - js.start); puts(" bytes\n"); */

    /* Get minako.cfg from rootfs */
    unsigned char* cfg_addr = 0;
    int cfgsize = rootfs_lookup("minako.cfg", &cfg_addr);

    /* Execute Minako configuration */
    Minako_ExecConfig((unsigned char*)cfg_addr, cfgsize);
	
    for(;;);
	PANIC();
}
