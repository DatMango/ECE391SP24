# Load into registers
# --> cr3
# some bits in cr0, cr1/4(?)

# Page 84: Paging Options
# PG (paging) flag. Bit 31 of CR0 (available in all IA-32 processors beginning with the Intel386 processor)
# PSE (page size extensions) flag. Bit 4 of CR4 (introduced in the Pentium processor).
# PAE (physical address extension) flag. Bit 5 of CR4 (introduced in the Pentium Pro processors).

# enable paging 
# kernel (in docs) and video memory (in lib.c)

# are we supposed to iret or ret here?????
# aligning???? page 6 of mp spec????
# when do we populate pte bits?? do we need that??? 
# type for page_directory
# kernel address is 24 bits and not 20 how do we deal with that???
# how to test??

#define ASM     1
#include "paging.h"

# loads the address of page directory to cr3 by taking it in as a parameter 
# sets the correct bits for CR0 and CR4 -- referred to OSdev
# is this it for flushing TLB???? where do you call flush_TLB???? 

.text
.globl page_enable ;\
    page_enable: ;\
        pushl %ebp ;\
        movl %esp, %ebp ;\
        movl 8(%esp), %eax ;\
        movl %eax, %cr3 ;\

        movl %cr4, %eax ;\
        orl $0x00000010, %eax ;\
        movl %eax, %cr4 ;\

        movl %cr0, %eax ;\
        orl $0x80000001, %eax ;\
        movl %eax, %cr0 ;\

        movl %ebp, %esp ;\
        popl %ebp ;\
        ret

    .end

