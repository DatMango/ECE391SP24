#if !defined(SYSCALLHANDLER_H)
#define SYSCALLHANDLER_H

#include "types.h"

/* syscall handler used by idt at int 0x80 
    comments for assembly written here, that is 
    for syscall_handler.S because comments mess 
    with assembly and cause issues. A jumptable is 
    created with all the system call functions which 
    just make the appropriate call to the C function. 
    Also takes care of indexing on eax and returns -1 
    on a bad return or pops off stack and irets 
*/
extern void syscall_handler();

#endif /* SYSCALLHANDLER_H */

