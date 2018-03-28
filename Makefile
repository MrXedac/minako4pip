###############################################################################
#  © Université Lille 1, The Pip Development Team (2015-2016)                 #
#                                                                             #
#  This software is a computer program whose purpose is to run a minimal,     #
#  hypervisor relying on proven properties such as memory isolation.          #
#                                                                             #
#  This software is governed by the CeCILL license under French law and       #
#  abiding by the rules of distribution of free software.  You can  use,      #
#  modify and/ or redistribute the software under the terms of the CeCILL     #
#  license as circulated by CEA, CNRS and INRIA at the following URL          #
#  "http://www.cecill.info".                                                  #
#                                                                             #
#  As a counterpart to the access to the source code and  rights to copy,     #
#  modify and redistribute granted by the license, users are provided only    #
#  with a limited warranty  and the software's author,  the holder of the     #
#  economic rights,  and the successive licensors  have only  limited         #
#  liability.                                                                 #
#                                                                             #
#  In this respect, the user's attention is drawn to the risks associated     #
#  with loading,  using,  modifying and/or developing or reproducing the      #
#  software by the user in light of its specific status of free software,     #
#  that may mean  that it is complicated to manipulate,  and  that  also      #
#  therefore means  that it is reserved for developers  and  experienced      #
#  professionals having in-depth computer knowledge. Users are therefore      #
#  encouraged to load and test the software's suitability as regards their    #
#  requirements in conditions enabling the security of their systems and/or   #
#  data to be ensured and,  more generally, to use and operate it in the      #
#  same conditions as regards security.                                       #
#                                                                             #
#  The fact that you are presently reading this means that you have had       #
#  knowledge of the CeCILL license and that you accept its terms.             #
###############################################################################

include ../toolchain.mk

ASFLAGS=-m32 -c -I. -I./include --freestanding -O0 -nostdlib
CFLAGS= -m32 -c -I. -I./include --freestanding -O0 -nostdlib -Wall -Werror
LDFLAGS=-L$(LIBPIP)/lib -melf_i386

# Include Libpip
CFLAGS+=-I$(LIBPIP)/include/ -I$(LIBPIP)/arch/x86/include/
CFLAGS+= $(MKARGS)
ASFLAGS+=-I$(LIBPIP)/include/ -I$(LIBPIP)/arch/x86/include/

ASSOURCES=$(wildcard *.S)
CSOURCES=$(wildcard *.c)
CSOURCES+=$(wildcard loader/*.c)
CSOURCES+=$(wildcard util/*.c)

ASOBJ=$(ASSOURCES:.S=.o)
COBJ=$(CSOURCES:.c=.o)

EXEC=$(shell basename `pwd`).bin

all: dep $(EXEC)
	@echo Done.

clean:
	rm -f $(ASOBJ) $(COBJ) $(EXEC) rootfs.img

$(EXEC): $(ASOBJ) $(COBJ)
	$(LD) $(LDFLAGS) $^ -Tlink.ld -o $@ -lpip

dep:
	#make -C interpreter clean all
	# RootFS creation
	cd images && tar -cvf rootfs.tar --format ustar * && mv rootfs.tar ../rootfs.img

partition.o: partition.S images/minako.cfg images/source.bin

%.o: %.S
	$(AS) $(ASFLAGS) $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

.PHONY: dep
