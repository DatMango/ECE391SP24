#define MY_ASM_MACRO(function_name, handler, vector) \
    .GLOBL function_name ;\
    function_name: ;\
        pushal ;\
        pushfl ;\
        pushl $vector ;\
        call handler ;\
        addl $4, %esp ;\
        popfl ;\
        popal ;\
        iret

#define MACRO2(function_name, handler, vector) \
    .GLOBL function_name ;\
    function_name: ;\
        addl $4, %esp ;\
        pushal ;\
        pushfl ;\
        pushl $vector ;\
        call handler ;\
        addl $4, %esp ;\
        popfl ;\
        popal ;\
        iret

# the macros above take care of the following: 
# save_registers ;\
# save_eflags ;\
# restore_eflags ;\
# restore_registers  ;\
# referred to discussion slides 

# here we have two macros, one with popping and one without and 
# the interrupts are also taken care of using the first macro in this 
# file 

MY_ASM_MACRO(div_exc, exc_handler, 0)       
MY_ASM_MACRO(debug_exc, exc_handler, 1)
MY_ASM_MACRO(nonmask_int, exc_handler, 2)          # actually an interrupt but in mp3 is an exception
MY_ASM_MACRO(breakpt_exc, exc_handler, 3)
MY_ASM_MACRO(overflow_exc, exc_handler, 4)
MY_ASM_MACRO(exceed_bounds_exc, exc_handler, 5)
MY_ASM_MACRO(inval_op_exc, exc_handler, 6)
MY_ASM_MACRO(device_na_exc, exc_handler, 7)
MACRO2(dbl_fault_exc, exc_handler1, 8)             # error code (zero) pushed onto stack need to take care
MY_ASM_MACRO(cso_exc, exc_handler, 9)                    # this one was crossed out in os dev idk
MACRO2(inval_tss, exc_handler1, 10)                # error code (zero) pushed onto stack need to take care
MACRO2(seg_na_exc, exc_handler1, 11)               # error code (zero) pushed onto stack need to take care
MACRO2(ss_fault_exc, exc_handler1, 12)             # error code (zero) pushed onto stack need to take care
MACRO2(gen_protect_fault_exc, exc_handler1, 13)    # error code (zero) pushed onto stack need to take care
MACRO2(pg_fault_exc, exc_handler1, 14)             # error code (zero) pushed onto stack need to take care
# 15 unneeded
MY_ASM_MACRO(float_exc, exc_handler, 16)
MACRO2(align_check_exc, exc_handler1, 17)          # error code (zero) pushed onto stack need to take care
MY_ASM_MACRO(mach_check_exc, exc_handler, 18)
MY_ASM_MACRO(simd_float_exc, exc_handler, 19)
MY_ASM_MACRO(virt_exc, exc_handler, 20)
MY_ASM_MACRO(pit_handling, pit_handler, 32)
MY_ASM_MACRO(keyboard_handling, keyboard_handler, 33)
MY_ASM_MACRO(rtc_handling, rtc_handler, 40)

