#include "idt.h"
#include "lib.h"
#include "x86_desc.h"
#include "idt_handler.h"
#include "syscall_handler.h"
#include "rtc.h"
#include "syscall_modifier.h"

// array of all the exception names to be printed 
char* arr[21] = {
    "Division Error Exception",
    "Debug Exception",
    "Non-Maskable Interrupt Exception",
    "Breakpoint Exception",
    "Overflow Exception",
    "Bound Range Exceeded Exception",
    "Invalid Opcode Exception",
    "Device Not Available Exception",
    "Double Fault Exception",
    "Coprocessor Segment Overrun Exception",
    "Invalid TSS Exception",
    "Segment Not Present Exception",
    "Stack-Segment Fault Exception",
    "General Protection Fault Exception",
    "Page Fault Exception",
    "",
    "x87 Floating-Point Exception",
    "Alignment Check Exception",
    "Machine Check Exception",
    "SIMD Floating-Point Exception",
    "Virtualization Exception"
};

/* 
 * 
 *   DESCRIPTION: Sets the idt bits which is a struct declared in x86_desc.h 
 *                  and raises flags when exceptions occur 
 *   INPUTS: none 
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: raises exceptions for given cases, sets interrupt gates
 *                  and trap gates appropriately. Interrupts, in this case  
 *                  include keyboard and RTC 
 * 
 */
void idt_initializer(){
    int i;
    for(i = 0; i < NUM_VEC; i++){   
        // for exceptions and sys calls    
        idt[i].seg_selector = KERNEL_CS; 
        idt[i].reserved4 = 0;
        idt[i].reserved3 = 1;   // assumes trap gate
        idt[i].reserved2 = 1;
        idt[i].reserved1 = 1;
        idt[i].size = 1;        // 32-bit system

        idt[i].reserved0 = 0; // Always 0

        idt[i].dpl = 0; // What level access
        idt[i].present = 0; // ?

        if(i == 0x02 || (i >= 0x20 && i <= 0x2F)){     // 2: non-maskable interrupt 0x20-0x2F: hardware interrupts
            // interrupt gate
            idt[i].reserved3 = 0;
        }
        switch(i){
            case 0:
                idt[i].present = 1;
                SET_IDT_ENTRY(idt[i], div_exc);
                break;
            
            case 1:
                idt[i].present = 1;
                SET_IDT_ENTRY(idt[i], debug_exc);
                break;

            case 2:
                idt[i].present = 1;
                SET_IDT_ENTRY(idt[i], nonmask_int);
                break;

            case 3:
                idt[i].present = 1;
                SET_IDT_ENTRY(idt[i], breakpt_exc);
                break;
            
            case 4:
                idt[i].present = 1;
                SET_IDT_ENTRY(idt[i], overflow_exc);
                break;

            case 5:
                idt[i].present = 1;
                SET_IDT_ENTRY(idt[i], exceed_bounds_exc);
                break;
            
            case 6:
                idt[i].present = 1;
                SET_IDT_ENTRY(idt[i], inval_op_exc);
                break;
            
            case 7:
                idt[i].present = 1;
                SET_IDT_ENTRY(idt[i], device_na_exc);
                break;
            
            case 8:
                idt[i].present = 1;
                SET_IDT_ENTRY(idt[i], dbl_fault_exc);
                break;
            
            case 9:
                idt[i].present = 1;
                SET_IDT_ENTRY(idt[i], cso_exc);
                break;

            case 10:
                idt[i].present = 1;
                SET_IDT_ENTRY(idt[i], inval_tss);
                break;
            
            case 11:
                idt[i].present = 1;
                SET_IDT_ENTRY(idt[i], seg_na_exc);
                break;

            case 12:
                idt[i].present = 1;
                SET_IDT_ENTRY(idt[i], ss_fault_exc);
                break;

            case 13:
                idt[i].present = 1;
                SET_IDT_ENTRY(idt[i], gen_protect_fault_exc);
                break;
            
            case 14:
                idt[i].present = 1;
                SET_IDT_ENTRY(idt[i], pg_fault_exc);
                break;
            
            case 16:
                idt[i].present = 1;
                SET_IDT_ENTRY(idt[i], float_exc);
                break;
            
            case 17:
                idt[i].present = 1;
                SET_IDT_ENTRY(idt[i], align_check_exc);
                break;
            
            case 18:
                idt[i].present = 1;
                SET_IDT_ENTRY(idt[i], mach_check_exc);
                break;
            
            case 19:
                idt[i].present = 1;
                SET_IDT_ENTRY(idt[i], simd_float_exc);
                break;

            case 20:
                idt[i].present = 1;
                SET_IDT_ENTRY(idt[i], virt_exc);
                break;
            
            case 33: // 0x21 -- keyboard
                idt[i].present = 1;
                idt[i].reserved3 = 0;
                SET_IDT_ENTRY(idt[i], keyboard_handling);
                break;

            case 32: // 0x20 PIT Interrupt
                idt[i].present = 1;
                idt[i].dpl = 3;
                idt[i].reserved3 = 0;
                SET_IDT_ENTRY(idt[i], pit_handling);
                break;

            case 40: // 0x28 -- real time clock (RTC case)
                idt[i].present = 1;
                idt[i].reserved3 = 0;
                SET_IDT_ENTRY(idt[i], rtc_handling); 
                break; 
            
            case 128: // 0x80 -- syscalls
                idt[i].present = 1;
                idt[i].dpl = 3;
                idt[i].reserved3 = 0;
                // SET_IDT_ENTRY(idt[i], syscall_handling);
                SET_IDT_ENTRY(idt[i], syscall_handler);
                break;
            
            default:
                break;
        }
    }
    // FOR DEBUGGING PURPOSES
    // SET_IDT_ENTRY(idt[0], div_exc);
    // SET_IDT_ENTRY(idt[1], debug_exc);
    // SET_IDT_ENTRY(idt[2], nonmask_int);
    // SET_IDT_ENTRY(idt[3], breakpt_exc);
    // SET_IDT_ENTRY(idt[4], overflow_exc);
    // SET_IDT_ENTRY(idt[5], exceed_bounds_exc);
    // SET_IDT_ENTRY(idt[6], inval_op_exc);
    // SET_IDT_ENTRY(idt[7], device_na_exc);
    // SET_IDT_ENTRY(idt[8], dbl_fault_exc);
    // SET_IDT_ENTRY(idt[9], cso_exc);
    // SET_IDT_ENTRY(idt[10], inval_tss);
    // SET_IDT_ENTRY(idt[11], seg_na_exc);
    // SET_IDT_ENTRY(idt[12], ss_fault_exc);
    // SET_IDT_ENTRY(idt[13], gen_protect_fault_exc);
    // SET_IDT_ENTRY(idt[14], pg_fault_exc);
    // SET_IDT_ENTRY(idt[16], float_exc);
    // SET_IDT_ENTRY(idt[17], align_check_exc);
    // SET_IDT_ENTRY(idt[18], mach_check_exc);
    // SET_IDT_ENTRY(idt[19], simd_float_exc);
    // SET_IDT_ENTRY(idt[20], virt_exc);
    // idt[33].dpl = 0;
    // idt[33].reserved3 = 0;
    // SET_IDT_ENTRY(idt[33], keyboard_handling);
    // idt[40].reserved3 = 0;
    // SET_IDT_ENTRY(idt[40], rtc_handling); 

}

/* 
 * 
 *   DESCRIPTION: Prints exceptions according to what is triggered according
 *                  to the idt initializer  
 *   INPUTS: none 
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: grabs the correct names from the vector of names and prints them
 *                  according to which exception is raised 
 * 
 */
void exc_handler(int vector){
    printf("Exception Handler: %s caught \n", arr[vector]);
    while(1){

    }
}

/* 
 * 
 *   DESCRIPTION: Prints exceptions according to what is triggered according
 *                  to the idt initializer. This uses a different MACRO which 
 *                  includes popping  
 *   INPUTS: none 
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: grabs the correct names from the vector of names and prints them
 *                  according to which exception is raised 
 * 
 */
void exc_handler1(int vector){
    printf("Exception Handler: %s caught \n", arr[vector]);
    // Pop off stack in assembly
    while(1){

    }
}

/* 
 * 
 *   DESCRIPTION: Prints interrupts according to what is triggered according
 *                  to the idt initializer. Created for testing purposes. 
 *   INPUTS: none 
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: grabs the correct names from the vector of names and prints them
 *                  according to which interrupt is raised 
 * 
 */
void int_handler(int vector){
    printf("Interrupt Handler %s caught \n", arr[vector]);

}
