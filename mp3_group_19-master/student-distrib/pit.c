#include "pit.h"
#include "x86_desc.h"
#include "lib.h"

int execute_buffer[3] = {1,1,1};


/* 
 *   DESCRIPTION: Flushes the translation lookaside buffer. Essentially loads 
 *                  address into CR3. The OS must flush TLB if it changes the 
 *                  page table 
 *   INPUTS:  none  
 *   OUTPUTS: none
 *   RETURN VALUE: ensures that address in cr3 is not stale by using eax
 *   SIDE EFFECTS: flushes TLB 
 * 
 */
#define flush_TLB()            \
    asm volatile ("                   \n\
            movl %cr3, %eax ;\
            movl %eax, %cr3 ;\
            "                           \
    );   


/* 
 *   DESCRIPTION: Helper function to change video memory based on a given
                  terminal.
 *   INPUTS:  terminal - Terminal to change page table
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: changes paging and flushes TLB 
 * 
 */
void change_vidmem_mapping(int terminal){
    // Change page table based on terminal (0xB9000 to 0xBB000)
    if(terminal != 4){
        page_table_entries[((VIDEO & MASK_NUMBER) >> shiftbytwelve)].pt_base_addr = (TERMINAL1_VIDEO + terminal * VIDEO_CHANGER) >> shiftbytwelve;
    }
    else{ // If terminal is 4, means we are changing the extra page table (virtual 0xBC000 points to physical 0xB8000)
        page_table_entries[((VIDEO & MASK_NUMBER) >> shiftbytwelve)].pt_base_addr = VIDEO >> shiftbytwelve;
    }
    flush_TLB();
}

/* 
 *   DESCRIPTION: Helper function to change video memory based on a given
                  terminal for fish.
 *   INPUTS:  terminal - Terminal to change page table
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: changes paging and flushes TLB 
 * 
 */
void vidmap_mapping(int terminal){
    // Change page table based on terminal (0xB9000 to 0xBB000)
    if(terminal != 4){
        syscall_page_table_entries[0].pt_base_addr = (TERMINAL1_VIDEO + terminal * VIDEO_CHANGER) >> shiftbytwelve;
    }
    if(terminal == 4){
        syscall_page_table_entries[0].pt_base_addr = VIDEO >> shiftbytwelve;
    }
    flush_TLB();
}

/* 
 *   Referenced OSDev and listed references 
 *   DESCRIPTION: initialization for pit (enables irq that pit is connected to)
 *                 
 *   INPUTS: none 
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: enables IRQ for pit
 * 
 */
void init_pit(){
    // Causes segmnent not present fault when enabled in kernel.c
    // command 0x36 is channel 0, LSB then MSB (for putting in counter data), square wave mode, 16 bit counters 
    enable_irq(IRQ_0); // Meant for PIT
    outb(0x36, pit_command_port);
    int f = 100; // Frequency
    int x = 1193180 / f;
    outb((uint8_t)x & 0xFF, pit_channel_0);
    outb((uint8_t)(x >> 8) , pit_channel_0);
}   

/* 
 *   DESCRIPTION: PIT Handler, called from IDT for PIT interrupts
 *                 
 *   INPUTS: none 
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: calls scheduler, sends EOI 
 * 
 */
void pit_handler(){
 
    // timer_ticks++;
    // /* Every 18 clocks (approximately 1 second), we will
    // *  display a message on the screen */
    // if (timer_ticks % 18 == 0)
    // {
    //     printf("One second has passed\n");
    // }

    //handler should change which terminal memory is being operated on, call schedule, and then acknowledge
    //switch to correct area of memory,
    
    send_eoi(IRQ_0);
    cli();
    scheduler(); // Call scheduler
    sti();
    // vidmap_mapping(get_active_terminal());

}

/* 
 *   DESCRIPTION: Called from PIT handler
 *                 
 *   INPUTS: none 
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: Initially executes all three shells, updates paging 
 *                 by calling change_vidmem_mapping()
 * 
 */
void scheduler(){
    int curr_pid_iterator = -1; // Initialize statement
    set_active_terminal((get_active_terminal() + 1) % 3); // Change active terminal (0 to 2)
    
    // int curr_pid_iterator = get_active_terminal();

    // Execute all 3 shells, use execute buffer as flags
    if(execute_buffer[get_active_terminal()]){
        execute_buffer[get_active_terminal()] = 0;
        execute((uint8_t*)"shell");
    }
    // active_terminal = 0; // Debugging statement
    uint32_t temp1_esp;

    // Save EBP and ESP
    register uint32_t saved_ebp asm("ebp");
    register uint32_t saved_esp asm("esp");

    // if(iter_buffer[active_terminal]){

    // Change processes based on active terminal
    /* Save EBP, ESP
     * Obtain PID from the process as well as
     * a pointer to the pcb
     */
    switch((get_active_terminal())){
        case 0:
            // term3_process->EBP = saved_ebp;
            // term3_process->ESP = saved_esp;
            term1_process->terminal_EBP = saved_ebp;
            term1_process->terminal_ESP = saved_esp;
            curr_pid_iterator = term1_process->pid;
            cur_pcb = term1_process;
            break;
        case 1:
            // term1_process->EBP = saved_ebp;
            // term1_process->ESP = saved_esp;
            term2_process->terminal_EBP = saved_ebp;
            term2_process->terminal_ESP = saved_esp;
            curr_pid_iterator = term2_process->pid;
            cur_pcb = term2_process;
            break;
        case 2:
            // term2_process->EBP = saved_ebp;
            // term2_process->ESP = saved_esp;
            term3_process->terminal_EBP = saved_ebp;
            term3_process->terminal_ESP = saved_esp;
            curr_pid_iterator = term3_process->pid;
            cur_pcb = term3_process;
            break;
    }

        // iter_buffer[active_terminal] = 0;
    // }
    // else{
    //     switch((curr_pid_iterator)){
    //         case 0:
    //             term3_process->terminal_EBP = saved_ebp;
    //             term3_process->terminal_ESP = saved_esp;
    //             // term1_process->EBP = saved_ebp;
    //             // term1_process->ESP = saved_esp;
    //             // printf("curr_pid_it 0 b4 %d\n", curr_pid_iterator);
    //             curr_pid_iterator = term1_process->pid;
    //             // printf("curr_pid_it 0 af %d\n", curr_pid_iterator);
    //             break;
    //         case 1:
    //             term1_process->terminal_EBP = saved_ebp;
    //             term1_process->terminal_ESP = saved_esp;
    //             // term2_process->EBP = saved_ebp;
    //             // term2_process->ESP = saved_esp;
    //             // printf("curr_pid_it 1 b4 %d\n", curr_pid_iterator);
    //             curr_pid_iterator = term2_process->pid;
    //             // printf("curr_pid_it 1 af %d\n", curr_pid_iterator);
    //             break;
    //         case 2:
    //             term2_process->terminal_EBP = saved_ebp;
    //             term2_process->terminal_ESP = saved_esp;
    //             // term3_process->EBP = saved_ebp;
    //             // term3_process->ESP = saved_esp;
    //             // printf("curr_pid_it 2 b4 %d\n", curr_pid_iterator);
    //             curr_pid_iterator = term3_process->pid;
    //             // printf("curr_pid_it 2 af %d\n", curr_pid_iterator);
    //             break;
    //     }
    // }    
    
    temp1_esp =  eightmb - (curr_pid_iterator)*eightkb;
    // updates the TSS esp0 with the appropriate esp 
    tss.esp0 = temp1_esp; 

    //update page table for user program mem address
    uint32_t program_physaddr = eightmb + (curr_pid_iterator * fourmb);
    page_directory[page_dir_index].four_mb.pt_base_addr = program_physaddr >> shiftbytwentytwo;

    flush_TLB(); //flushes tlb

    // Change paging based on active terminal. If current = active, use terminal "4"
    if(get_active_terminal() == get_curr_terminal()){
        change_vidmem_mapping(4);
    }
    else{
        change_vidmem_mapping(get_active_terminal());
    }

    flush_TLB();

    // active_terminal = (active_terminal + 1) % 3; //update active terminal to next one
    // if(active_terminal == get_curr_terminal()){
    //     change_vidmem_mapping(4);
    // }
    // else{
    //     change_vidmem_mapping(active_terminal);
    // }

    // flush_TLB();
    

    // printf("%d\n", active_terminal);

    // Updates EBP and ESP
    asm volatile ("             \n\
        movl %%eax, %%ebp       \n\
        movl %%ebx, %%esp       \n\
        leave                   \n\
        ret                     \n\
        "
        :
        : "a"(cur_pcb->terminal_EBP), "b" (cur_pcb->terminal_ESP) \
        : "memory", "cc"    \
    );
}
