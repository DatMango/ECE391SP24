#define ASM     1

syscall_jumptable:
    .long   halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn

.global syscall_handler
syscall_handler:
    cmpl    $1, %eax 
    jb      syscall_bad_return
    cmpl    $10, %eax
    ja      syscall_bad_return
    addl    $-1, %eax

    pushl   %edx
    pushl   %ecx
    pushl   %ebx
    call    *syscall_jumptable(,%eax,4)
    jmp     syscall_return

syscall_bad_return:
    movl    $-1, %eax  
    iret       

syscall_return:
    addl    $12, %esp
    iret 
