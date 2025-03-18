#ifndef IDT_HANDLER_H
#define IDT_HANDLER_H

/* Declare all the functions created by the macro */

/* declares function for division by zero exception */
extern void div_exc();

/* declares function for nonmask */
extern void debug_exc();

/* declares function for nonmaskable interrupt */
extern void nonmask_int();

/* declares function for breakpoint exception */
extern void breakpt_exc();

/* declares function for overflow exception */
extern void overflow_exc();

/* declares function for out of bounds/bound range exceeded exception */
extern void exceed_bounds_exc();

/* declares function for invalid opcode exception */
extern void inval_op_exc();

/* declares function for device not available exception */
extern void device_na_exc();

/* declares function for double fault exception */
extern void dbl_fault_exc();

/* declares function for coprocessor segment overrun exception */
extern void cso_exc();

/* declares function for invalid tss exception */
extern void inval_tss();

/* declares function for segment not present exception */
extern void seg_na_exc();

/* declares function for stack segment fault exception */
extern void ss_fault_exc();

/* declares function for general protection fault exception */
extern void gen_protect_fault_exc();

/* declares function for page fault exception */
extern void pg_fault_exc();

/* declares function for x87 float-point exception */
extern void float_exc();

/* declares function for alignment check exception */
extern void align_check_exc();

/* declares function for machine check exception */
extern void mach_check_exc();

/* declares function for SIMD floating-point exception */
extern void simd_float_exc();

/* declares function for virtualization exception */
extern void virt_exc();

/* declares function for keyboard interrupt */
extern void keyboard_handling();

/* declares function for rtc interrupt */
extern void rtc_handling(); 

/* declares function for system calls*/
extern void syscall_handling();

/* declares function for pit interrupt*/
extern void pit_handling(); 

#endif
