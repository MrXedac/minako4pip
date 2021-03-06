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

#ifndef __JS_LAB__
#define __JS_LAB__

#include <pip/vidt.h>
#include <stdint.h>

typedef unsigned long size_t; 

/**
 * \struct int_stack_s
 * \brief Stack context from interrupt/exception
 */
typedef struct int_stack_s
{
	pushad_regs_t regs;//!< Interrupt handler saved regs
	uint32_t int_no; //!< Interrupt number
	uint32_t err_code; //!< Interrupt error code
	uint32_t eip; //!< Execution pointer
	uint32_t cs; //!< Code segment
	uint32_t eflags; //!< CPU flags
	/* only present when we're coming from userland */
	uint32_t useresp; //!< User-mode ESP
	uint32_t ss; //!< Stack segment
} int_ctx_t ;

typedef enum task_state { BOOTSTRAPPED, RUNNING, READY } task_state_e;

typedef struct task_s
{
	task_state_e state;
	void *part;
	vidt_t *vidt;
} task_t;

/* Serial output */
void log(char* str);

/* VGA controller */
void init_video();
void vgaputs(char*);
void vgaputch(unsigned char);
void vgaputdec(uint32_t);
void vgaputhex(uint32_t);

/* Generic loader */
int BootstrapPartition(uint32_t base, uint32_t length, uint32_t load_addr, task_t* part);

/* Memory functions */
void * memset( void * ptr, int value, unsigned num );
void * memsetw( void * ptr, uint16_t value, unsigned num );
void * memcpy ( void * destination, const void * source, unsigned num );
long strtol(const char *nptr, char **endptr, register int base);
int strcmp(const char* s1, const char* s2);
int memcmp(const void* s1, const void* s2, size_t n);
size_t strlen(const char *s);

/* Minako defines */
#define MINAKO_VERSION  "0.3"
#define MINAKO_CODENAME "aino"
#define MINAKO_TARGET   "x86mp"

/* Parser */
void Minako_ExecConfig();

/* RootFS */
int rootfs_lookup(char *filename, unsigned char **out);

#endif
