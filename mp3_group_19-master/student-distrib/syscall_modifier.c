#include "syscall_modifier.h"
// #include "paging.h"
#include "x86_desc.h"

static int command_counter2;
static int num_pids = -1;
int active_terminal = 2;
int iter_buffer[3] = {1,1,1};

//eightmb 8388608
//eightkb 8192
//fourmb 4194304
// 83ffffc = 138412028  --> 132 mb - 4 
// 128 mb = 134217728
//
//file_descriptor_t file_array[MAX_PROCESS];
// pcb_t* pcb_arr[6] = {&pcb1, &pcb2, &pcb3, &pcb4, &pcb5, &pcb6};

/* 
 * 
 *   DESCRIPTION: An initialization function that maps the pcb onto kernel 
 *                  memory (in this case, 6 processes). Also sets 
 *                  terminal read, write, open, close for file descriptor 0
 *                  and 1 of each pcb's fd array according to spec. 
 *   INPUTS: none 
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: maps pcb in kernel memory  
 * 
 */
void init_syscall(){
    int i;
    for(i = 0; i < num_processes; i++){
        // maps pcb in kernel memory
        pcb_arr[i] = (pcb_t*)(eightmb - (i + 1)*eightkb);

        // initializes fd 0 and 1 of each pcb's fd array to be stdin/out
        pcb_arr[i]->fd_array[stdin_fd].file_operation.s_open = &terminal_open;
        pcb_arr[i]->fd_array[stdin_fd].file_operation.s_read = &terminal_read;
        pcb_arr[i]->fd_array[stdin_fd].file_operation.s_write = NULL;
        pcb_arr[i]->fd_array[stdin_fd].file_operation.s_close = &terminal_close;

        pcb_arr[i]->fd_array[stdout_fd].file_operation.s_open = &terminal_open;
        pcb_arr[i]->fd_array[stdout_fd].file_operation.s_read = NULL;
        pcb_arr[i]->fd_array[stdout_fd].file_operation.s_write = &terminal_write;
        pcb_arr[i]->fd_array[stdout_fd].file_operation.s_close = &terminal_close;
    }
}

// define CON_SWIT(ESP_POINTER, EIP_POINTER, USERCS, USERDS)   

/* 
 * 
 *   DESCRIPTION: Flushes the translation lookaside buffer. Essentially loads 
 * 
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
 * 
 *   DESCRIPTION: halt gets the ebp saved in execute and it restores the  
 *                  context to the parent process  
 *   INPUTS:  status - return code 
 *   OUTPUTS: none
 *   RETURN VALUE: returns status/return code using inline and returns 0 on 
 *                  successfully halting 
 *   SIDE EFFECTS: restores parent data, parent paging, writes parent info 
 *                  back to TSS, and jumps to execute return 
 * 
 */
int32_t halt (uint8_t status){
    // resets in use flag and pid
    // printf("num processes halt begin: %d\n", num_pids);
    cli();
    int x;
    // NEED TO FIX TO ACCOUNT FOR 3 BASE SHELLS INSTEAD
    // if(num_pids == 0 && pcb_arr[num_pids]->parent_pid == 0){
    int temp_pid;
    pcb_t* temp_process;
    
    switch(get_active_terminal()){
        case 0:
            temp_pid = term1_process->pid;
            break;
        case 1:
            temp_pid = term2_process->pid;
            break;
        case 2:
            temp_pid = term3_process->pid;
            break;
    }

    if(temp_pid == get_active_terminal()){  // if base shell pid# == parent pid
        pcb_arr[temp_pid]->in_use_flag = 0;
        // num_pids = 0;
        // printf("halt hello");
        sti();
        execute((uint8_t*)"shell");
    }
    // CHECK PCBs 
    uint32_t temp_ebp, temp_esp;
    switch(get_active_terminal()){ //need to change to account for which program is calling halt
        case 0:
            term1_process->in_use_flag = 0;
            temp_ebp = term1_process->EBP;
            temp_esp = term1_process->ESP;
            term1_process = pcb_arr[term1_process->parent_pid];
            cur_pcb = term1_process;
            temp_process = term1_process;
            break;
        case 1:
            term2_process->in_use_flag = 0;
            temp_ebp = term2_process->EBP;
            temp_esp = term2_process->ESP;
            term2_process = pcb_arr[term2_process->parent_pid];
            cur_pcb = term2_process;
            temp_process = term2_process;
            break;
        case 2:
            term3_process->in_use_flag = 0;
            temp_ebp = term3_process->EBP;
            temp_esp = term3_process->ESP;
            term3_process = pcb_arr[term3_process->parent_pid];
            cur_pcb = term3_process;
            temp_process = term3_process;
            break;
    }
    // pcb_arr[pcb_arr[num_pids]->parent_pid]->child_pid = 10;

    for(x = 2; x < 8; x++){
        close(x);
        // pcb_arr[temp_pid]->fd_array[x].flags = 0;
        // pcb_arr[temp_pid]->fd_array[x].file_pos = 0;
    }
    // pcb_arr[num_pids]->in_use_flag = 0;
    //set page directory address back to parent pcb page 
    uint32_t program_physaddr = eightmb + (temp_process->pid * fourmb);
    page_directory[page_dir_index].four_mb.pt_base_addr = program_physaddr >> shiftbytwentytwo;  // to get 10 bits 
    page_directory[page_dir_index].four_mb.reserved = 0; 
    page_directory[page_dir_index].four_mb.page_table_attr_index = 0;    
    page_directory[page_dir_index].four_mb.avail = 0;                    
    page_directory[page_dir_index].four_mb.global_page = 0; 
    page_directory[page_dir_index].four_mb.page_size = 1;                // 1 indicates 4 MBytes 
    page_directory[page_dir_index].four_mb.dirty = 0;                    // 0 when initially loaded 
    page_directory[page_dir_index].four_mb.accessed = 0;                 // 0 when initially loaded 
    page_directory[page_dir_index].four_mb.cache_disabled = 0; 
    page_directory[page_dir_index].four_mb.write_through = 0;
    page_directory[page_dir_index].four_mb.user_supervisor = 1;          // user so 1
    page_directory[page_dir_index].four_mb.read_write = 1;               // want both read and write
    page_directory[page_dir_index].four_mb.present = 1;

    flush_TLB();  

    // printf("num processes halt end: %d\n", temp_process->pid);
    int esp_point =  eightmb - (temp_process->pid)*eightkb;
    tss.esp0 = esp_point;
    // uses inline for ebp and esp and also loads the return code/status to eax for return 
    sti();
    asm volatile ("             \n\
        movl %0, %%ebp          \n\
        movl %1, %%esp          \n\
        xor  %%eax, %%eax       \n\
        movb %2, %%al           \n\
        leave                   \n\
        ret                     \n\
        "
        : 
        : "r" (temp_ebp), "r" (temp_esp), "r" (status) \
        : "memory", "cc"           \
    );   

    return 0;
}

/* 
 * 
 *   DESCRIPTION: executes the files, takes care of paging (maps the virtual memory to 
 *                  a physical page), saves current EBP, and creates context switch 
 *   INPUTS:  command - ultimately provides filename after parsing  
 *   OUTPUTS: none
 *   RETURN VALUE: pops the necessary info into the correct registers, returns 0 on 
 *                  success 
 *   SIDE EFFECTS: saves EBP, maps virtual memory, and context switches 
 * 
 */
int32_t execute (const uint8_t* command){
    // printf("process number execute begin: %d\n", num_pids);
    /* steps from discussion slides 
    1. Parse the arguments passed in.
    2. Check if the filename from the argument is valid and also an executable file.
    3. Check if we reached the maximum supported PCB’s (Max programs).
    4. Map the virtual memory correctly, to a physical page to the common virtual
    page.
    5. Load the program from the file system into memory directly.
    6. Save current EBP
    7. Create Context Switch.
    8. IRET*/

    // returns -1 if command can't be executed, (program doesnt exist/filename not an executable)
    // return 256 if program dies by exception
    // return 0-255 if program executes halt sys call

    // Parse "command" --> Space separated
    // First word before space is filename
    int counter;
    int command_counter = 0;
    // int command_counter2 = 1;
    command_counter2 = 1;
    unsigned char filename[max_filename_size] = {0};
    int rdentry_val;
    int EIP_val = 0;
    unsigned int buffer_EIP[1] = {0};
    unsigned char buffer_ELFs[executable_bytes]; 
    int i;
    // uint32_t curr_pid_iterator;
    inode_t* inodeptr;
    dentry_t dentry_sys_exec;

    // checks for executable files 
    int32_t magic_numbers[executable_bytes] = {executable0_bytes, executable1_bytes, executable2_bytes, executable3_bytes}; // ELFS
    if(command == NULL){
        return -1;
    }
    cli();
    //parses arguments
    while(1){
        // Parse the command in front and breaks if it is only the first command, else move onto the second command
        if(*(command + command_counter) ==  '\n' || *(command + command_counter) ==  '\0' || *(command + command_counter) == NULL || command_counter == max_filename_size){
            break;
        }
        else if(*(command + command_counter) ==  ' '){
            // Checks if there is an argument after the first command, increments command_counter2 if there is an argument after
            while(1){
                if(*(command + command_counter + command_counter2) ==  ' ' || *(command + command_counter + command_counter2) ==  '\0' || *(command + command_counter + command_counter2) ==  '\n' || *(command + command_counter) ==  NULL){
                    break;
                }
                command_counter2++; // Increments based on the second command / arg (ex. cat frame0.txt would increase this)
            }
            break;
        }
        command_counter++; // Increments command_counter for the first part of the command
    }

    // Copy the front command into the filename
    if(command_counter2 == max_filename_size + 1){
        command_counter2 = max_filename_size + 1;
    }
    
    for(i = 0; i < command_counter; i++){
        filename[i] = command[i];
    }
    // printf("filename: %s\n", filename);
    for(i = 0; i < num_processes; i++){
        if((*(pcb_arr[i])).in_use_flag != 1){
            num_pids = i;
            pcb_arr[num_pids]->in_use_flag = 1;
            break;
        }
        else if(((*(pcb_arr[i])).in_use_flag == 1) && (i == max_pcb_array_elem)){
            sti();
            return -1;
        }
    }

    // checks for valid filename and finds corresponding dentry 
    rdentry_val = read_dentry_by_name(filename, &dentry_sys_exec);
    if(rdentry_val == -1){
        pcb_arr[num_pids]->in_use_flag = 0;
        sti();
        return -1;
    }

    // EIP is bytes 24-27 uses offset to grab the bytes 
    if(read_data((uint32_t)dentry_sys_exec.inode_num, eip_bytes_offset, (uint8_t*)buffer_EIP, bytes_to_be_read) == -1){   
        pcb_arr[num_pids]->in_use_flag = 0;
        sti();
        return -1;
    }

    EIP_val = buffer_EIP[0]; 

    // gets first 4 bytes 
    if(read_data(dentry_sys_exec.inode_num, 0, buffer_ELFs, bytes_to_be_read) == -1){
        pcb_arr[num_pids]->in_use_flag = 0;
        sti();
        return -1;
    }

    // checks to see if they match the magic numbers representing the excutable and if not 
    for(counter = 0; counter < executable_bytes; counter++){
        if(magic_numbers[counter] != buffer_ELFs[counter]){
            pcb_arr[num_pids]->in_use_flag = 0;
            sti();
            return -1;  
        }
    }

    // check if we've reached the max number of pcbs 
    if(num_pids >= num_processes){ 
        pcb_arr[num_pids]->in_use_flag = 0;
        sti();
        return -1;
    }

    // checks for next available on pcb array by using the in use flag and updates num 
    // pids accordingly 
    //num_pids++;
    for(i = 0; i < max_filename_size; i++){
            pcb_arr[num_pids]->command_buffer[i] = NULL;
        }

    // More getargs here
    if(command_counter2 <= 1){
        pcb_arr[num_pids]->argument = 0;
    }
    else{
        // Copy the argument of the command if exists into the command_buffer
        for(i = 0; i < command_counter2 - 1; i++){
            if(command_counter2 > max_filename_size + 1){
                command_counter2 = 1;
                pcb_arr[num_pids]->command_buffer[i] = 'f';
            }
            else{
                pcb_arr[num_pids]->command_buffer[i] = command[i + command_counter + 1];
            }
        }
        pcb_arr[num_pids]->argument = 1;
    }


    // setting paging, that is, maps virtual memory to a page 
    uint32_t program_physaddr = eightmb + (num_pids * fourmb);
    page_directory[page_dir_index].four_mb.pt_base_addr = program_physaddr >> shiftbytwentytwo;  // to get 10 bits 
    page_directory[page_dir_index].four_mb.reserved = 0; 
    page_directory[page_dir_index].four_mb.page_table_attr_index = 0;    
    page_directory[page_dir_index].four_mb.avail = 0;                    
    page_directory[page_dir_index].four_mb.global_page = 0; 
    page_directory[page_dir_index].four_mb.page_size = 1;                // 1 indicates 4 MBytes 
    page_directory[page_dir_index].four_mb.dirty = 0;                    // 0 when initially loaded 
    page_directory[page_dir_index].four_mb.accessed = 0;                 // 0 when initially loaded 
    page_directory[page_dir_index].four_mb.cache_disabled = 0; 
    page_directory[page_dir_index].four_mb.write_through = 0;
    page_directory[page_dir_index].four_mb.user_supervisor = 1;          // user so 1
    page_directory[page_dir_index].four_mb.read_write = 1;               // want both read and write
    page_directory[page_dir_index].four_mb.present = 1;

    flush_TLB(); // every time page table entry is changed is when you want to flush_TLB 

    //load file into memory
    uint32_t* program_virtaddr = (uint32_t*)programstartvirtualmem;
    inodeptr = (inode_t*)(bootblock + dentry_sys_exec.inode_num + 1);
    read_data(dentry_sys_exec.inode_num, 0, (uint8_t*)program_virtaddr, inodeptr->length_B);
    
    //create PCB/Open fds 
    // pcb_arr[num_pids] = (pcb_t*)(eightmb - (num_pids + 1)*eightkb); //pcb pointer location math (start at 8 MB - (pid# + 1) *8 KB)
    pcb_arr[num_pids]->pid = num_pids;

    // setting parent pids (modify for later cp)
    // Figure out else statement for setting children_pid

    /* case of base shells 0, 1, 2:
        set parent equal to its own process number 
        also update the pointer to the most recent respective terminal process */
    // if(get_curr_terminal() == num_pids){
    cur_pcb = pcb_arr[num_pids];
    if(get_active_terminal() == num_pids){
        pcb_arr[num_pids]->parent_pid = (uint32_t)num_pids;
        switch(get_active_terminal()){
            case 0:
                term1_process = pcb_arr[num_pids];
                break;
            case 1:
                term2_process = pcb_arr[num_pids];
                break;
            case 2:
                term3_process = pcb_arr[num_pids];
                break;
        }
    }
    /* case of non base shells: process numbers 3, 4, 5 
        set parent to ptr to most recent respective terminal process 
        then update the ptr to most recent respective process with the process to be added */
    else{
        switch(get_active_terminal()){
            case 0:
                pcb_arr[num_pids]->parent_pid = (uint32_t)(term1_process->pid);
                term1_process = pcb_arr[num_pids];
                break;
            case 1:
                pcb_arr[num_pids]->parent_pid = (uint32_t)(term2_process->pid);
                term2_process = pcb_arr[num_pids];
                break;
            case 2:
                pcb_arr[num_pids]->parent_pid = (uint32_t)(term3_process->pid);
                term3_process = pcb_arr[num_pids];
                break;
        }
    }

    /* set the terminal of either case to be the active terminal */
    pcb_arr[num_pids]->num_terminal = get_active_terminal();
    
    pcb_arr[num_pids]->EIP = EIP_val;  //set eip value calculated from earlier
    for(i = 0; i < files_per_task; i++){
        if(i == 0 || i == 1){
            pcb_arr[num_pids]->fd_array[i].flags = 1; // set fd0 and fd1 to 1 always
        }
        else{
            pcb_arr[num_pids]->fd_array[i].flags = 0;  //new pcb set all fd flags to 0 bc no files have been opened in the process
            pcb_arr[num_pids]->fd_array[i].file_pos = 0;
        }
        
    }
    // printf("num processes execute end: %d\n", num_pids);
    //context switch    
    int esp_point;
    esp_point =  eightmb - (num_pids)*eightkb;

    // updates the TSS esp0 with the appropriate esp 
    tss.esp0 = esp_point; 
    
    uint32_t temp_ebp, temp_esp;
    
    asm volatile ("             \n\
        movl %%ebp, %0          \n\
        movl %%esp, %1          \n\
        "
        : "=r" (temp_ebp), "=r" (temp_esp) \
        : 
        : "memory", "cc"           \
    );

    pcb_arr[num_pids]->EBP = temp_ebp;
    pcb_arr[num_pids]->ESP = temp_esp; 

    //a - eax; b - ebx; c - ecx; d - edx
    // pushes all the necessary info to stack, that is USER_DS, esp, eflags, USER_CS, and EIP
    sti();
    asm volatile ("             \n\
        pushl   %%edx           \n\
        pushl   %%eax           \n\
        pushfl                  \n\
        popl    %%edi           \n\
        orl     $0x200, %%edi   \n\
        pushl   %%edi           \n\
        pushl   %%ecx           \n\
        pushl   %%ebx           \n\
        iret                    \n\
        "
        :
        : "a"(onetwoeightmb + fourmb - esp_offset), "b"(pcb_arr[num_pids]->EIP), "c"(USER_CS), "d"(USER_DS) \
        : "memory", "cc"    \
    );
        // inline to push things to stack, then iret to pop into correct registers 
        // update TSS.esp0 to new esp in increments 

    return 0;
}


/* 
 * 
 *   DESCRIPTION: syscall read, for fd 0, calls terminal read and after checking for
 *                  validity, calls appropriate read (that is, rtc, file, or directory)
 *                  based on function pointers and updates position of how much was 
 *                  read
 *   INPUTS:  fd - file descriptor 
 *            buf - where read bytes is copied into 
 *            nbytes - bytes to be read 
 *   OUTPUTS: none
 *   RETURN VALUE: returns bytes read or -1 on failure 
 *   SIDE EFFECTS: calls appropriate read using function pointers 
 * 
 */
int32_t read (int32_t fd, void* buf, int32_t nbytes){
    cli();
    if(fd == stdin_fd){
        int temp = terminal_read(fd, buf, nbytes);  // calls terminal read when stdin
        sti();
        return temp;
        // return 0;
    }
    else if(fd != stdout_fd && fd < files_per_task && fd >= 0 && (cur_pcb->fd_array[fd].flags)){
        int file_size = cur_pcb->fd_array[fd].file_size; // Obtain file size from dentry and inode num
        int temp;
        int file_position = cur_pcb->fd_array[fd].file_pos;
        if(file_size < file_position){ 
            sti();
            return 0;
        }
        temp = (cur_pcb->fd_array[fd].file_operation.s_read)(fd, buf, nbytes);    // calls appropriate read func 
        sti();
        return temp;
        // return pcb_arr[num_pids]->fd_array[fd].file_operation.s_read(fd, buf, nbytes);
        // return 0;
    }
    return -1;
}

/* 
 * 
 *   DESCRIPTION: syscall write, for fd 1, calls terminal write and after checking for
 *                  validity, calls appropriate write (that is, rtc, file, or directory)
 *                  based on function pointers 
 *   INPUTS:  fd - file descriptor 
 *            buf - buffer with bytes to be written  
 *            nbytes - bytes to be written  
 *   OUTPUTS: none
 *   RETURN VALUE: returns the return value from terminal write or appropriate write 
 *                  call (recall read-only filesystem) or -1 on failure 
 *   SIDE EFFECTS: calls appropriate write using function pointers 
 * 
 */
int32_t write (int32_t fd, const void* buf, int32_t nbytes){
    if(fd == stdout_fd){
        int temp;
        temp = terminal_write(fd, buf, nbytes); // calls terminal write when stdout 
        return temp;
        // return 0;
    }
    else if(fd != stdin_fd && fd < files_per_task && fd >= 0 && (cur_pcb->fd_array[fd].flags)){
        return (cur_pcb->fd_array[fd].file_operation.s_write)(fd, buf, nbytes); // calls the appropriate write func 
        // return 0;
    }
    return -1;
}

/* 
 * 
 *   DESCRIPTION: syscall open, checks each dentry filetype for rtc, file, or 
 *                  directory and sets corresponding read, write, open, close   
 *                  functions 
 *   INPUTS:  filename - name of file to open and find corresponding dentry for 
 *   OUTPUTS: none
 *   RETURN VALUE: returns fd or -1 on failure  
 *   SIDE EFFECTS: sets corresponding read, write, open, close functions based on
 *                  dentry filetype which is found by using filename to find 
 *                  appropriate dentry 
 * 
 */
int32_t open (const uint8_t* filename){
    int read_ret;
    dentry_t dentry_sys_open;
    if(*filename == NULL){
        return -1;
    }
    read_ret = read_dentry_by_name(filename, &dentry_sys_open);
    inode_t* inode_ptr = (inode_t*)(bootblock + dentry_sys_open.inode_num + 1);
    if(read_ret == -1 || &dentry_sys_open == NULL|| filename == NULL){
        return -1;
    }

    /* check each dentry file type for rtc, directory, and file. set corresponding 
        functions if matches. additionally, set the fd to be in use. fd[0] and
        fd[1] are reserved for stdin and stdout (terminal read/write) so looping
        starts from 2 instead of 0. checks <8 since max fd is 7. */

    // dentry filetype = 0: set rtc functions
    if(dentry_sys_open.filetype == rtc_filetype){
        int i;
        for(i = file_after_terminerw; i < files_per_task; i++){
            if(cur_pcb->fd_array[i].flags == 0){
                cur_pcb->fd_array[i].file_operation.s_open = &rtc_open;
                (*cur_pcb->fd_array[i].file_operation.s_open)(filename); 
                cur_pcb->fd_array[i].file_operation.s_read = &rtc_read;
                cur_pcb->fd_array[i].file_operation.s_write = &rtc_write;
                cur_pcb->fd_array[i].file_operation.s_close = &rtc_close;
                cur_pcb->fd_array[i].flags = 1;
                cur_pcb->fd_array[i].inode = dentry_sys_open.inode_num;
                cur_pcb->fd_array[i].file_pos = 0;
                (cur_pcb->fd_array[i].fd_dentry) = dentry_sys_open;
                return i;
            }   
        }
    }
    // dentry filetype = 1: set directory functions
    else if(dentry_sys_open.filetype == directory_filetype){
        int i;
        for(i = file_after_terminerw; i < files_per_task; i++){
            if(cur_pcb->fd_array[i].flags == 0){
                cur_pcb->fd_array[i].file_operation.s_open = &directory_open;
                (*cur_pcb->fd_array[i].file_operation.s_open)(filename); 
                cur_pcb->fd_array[i].file_operation.s_read = &directory_read;
                cur_pcb->fd_array[i].file_operation.s_write = &directory_write;
                cur_pcb->fd_array[i].file_operation.s_close = &directory_close;
                cur_pcb->fd_array[i].flags = 1;
                cur_pcb->fd_array[i].inode = dentry_sys_open.inode_num;
                cur_pcb->fd_array[i].file_pos = 0;
                // pcb_arr[num_pids]->fd_array[i].file_size = inode_ptr->length_B;
                cur_pcb->fd_array[i].file_size = bootblock->num_dentry;
                (cur_pcb->fd_array[i].fd_dentry) = dentry_sys_open;
                return i;
            }   
        }
    } 
    // dentry filetype = 2: set file functions
    else if(dentry_sys_open.filetype == file_filetype){
        int i;
        for(i = file_after_terminerw; i < files_per_task; i++){
            if(cur_pcb->fd_array[i].flags == 0){
                cur_pcb->fd_array[i].file_operation.s_open = &file_open;
                (*cur_pcb->fd_array[i].file_operation.s_open)(filename); 
                cur_pcb->fd_array[i].file_operation.s_read = &file_read;
                cur_pcb->fd_array[i].file_operation.s_write = &file_write;
                cur_pcb->fd_array[i].file_operation.s_close = &file_close;
                cur_pcb->fd_array[i].flags = 1;
                cur_pcb->fd_array[i].inode = dentry_sys_open.inode_num;
                cur_pcb->fd_array[i].file_pos = 0;
                cur_pcb->fd_array[i].file_size = inode_ptr->length_B;
                (cur_pcb->fd_array[i].fd_dentry) = dentry_sys_open;
                return i;
            }   
        }
    }
    return -1;

}

/* 
 * 
 *   DESCRIPTION: syscall close, sets flags and ensures close is being called on 
 *                  valid fd 
 *   INPUTS:  fd - file descriptor 
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure   
 *   SIDE EFFECTS: sets flag 
 * 
 */
int32_t close(int32_t fd){
    if(fd > max_fd_array_elem || fd < file_after_terminerw){
        return -1;                                      // invalid check and return 
    }
    if(cur_pcb->fd_array[fd].flags && (fd != stdin_fd || fd != stdout_fd)){
        cur_pcb->fd_array[fd].flags = 0;      // resets flags 
        return 0;
    }
    cur_pcb->fd_array[fd].file_pos = 0;       // sets file position to 0 
    return -1;
}


/* 
 * 
 *   DESCRIPTION: The arguments are parsed in from command buffer in execute and get
 *                  args uses strncpy to get info from command buffer based on pid from 
 *                  pcb array into buffer 
 *   INPUTS:  buf - to be copied into 
 *            nbytes - bytes/arg 
 *   OUTPUTS: none 
 *   RETURN VALUE: 0 on success, -1 on failure  
 *   SIDE EFFECTS: uses strncpy to get info from command buffer based on pid from pcb array 
 *                  into buffer 
 * 
 */
int32_t getargs (uint8_t* buf, int32_t nbytes){
    if(nbytes == 0 || cur_pcb->command_buffer == NULL || cur_pcb->command_buffer[0] == '\0' || cur_pcb->argument == 0){
        return -1;
    }

    strcpy((int8_t*)buf, (const int8_t*)cur_pcb->command_buffer);
    // printf("command buffer %s", buf);
    return 0;
}

/* 
 * 
 *   DESCRIPTION: Sets up paging, i.e., maps onto video memory and page directory points to the 
 *                  page table entry set up 
 *   INPUTS:  screen_start - virtual address is set to screen start 
 *   OUTPUTS: none 
 *   RETURN VALUE: returns -1 temporarily  
 *   SIDE EFFECTS: flushes TLB 
 * 
 */
int32_t vidmap (uint8_t** screen_start){
    if(screen_start == NULL){
        return -1;
    }

    // Check if screen start range is within kernel stack, return -1 if it is
    if((uint32_t)screen_start >= fourmb && (uint32_t)screen_start < eightmb){
        return -1;
    }

    // 128 to 132 --> Program image in stack mb
    // if((uint8_t**)onetwoeightmb > screen_start || (uint8_t**)(onethreetwomb - 1) < screen_start){
    //     printf("HI\n");
    //     return -1;
    // }

    // if(screen_start >= fourmb && screen_start < eightmb || screen_start > onetwoeightmb && screen_start < onethreetwomb){
    //     return -1;
    // }
    
    // map 4kb page to video mem
    // set virt address to screen start

    // make a new page table, make helper function later
    // Update all of paging for fish
    uint32_t program_physaddr = each_block * thirtythreemb;
    page_directory[page_dir_index_extra].four_kb.pt_base_addr = (uint32_t)syscall_page_table_entries >> shiftbytwelve;  // to get 10 bits 
    page_directory[page_dir_index_extra].four_kb.reserved = 0;  
    page_directory[page_dir_index_extra].four_kb.avail = 0;                    
    page_directory[page_dir_index_extra].four_kb.global_page = 0; 
    page_directory[page_dir_index_extra].four_kb.page_size = 0;                // 1 indicates 4 MBytes, 0 not
    page_directory[page_dir_index_extra].four_kb.accessed = 0;                 // 0 when initially loaded 
    page_directory[page_dir_index_extra].four_kb.cache_disabled = 0; 
    page_directory[page_dir_index_extra].four_kb.write_through = 0;
    page_directory[page_dir_index_extra].four_kb.user_supervisor = 1;          // user so 1
    page_directory[page_dir_index_extra].four_kb.read_write = 1;               // want both read and write
    page_directory[page_dir_index_extra].four_kb.present = 1;

    int page_table_index; 
    for(page_table_index = 0; page_table_index < ENTRIES; page_table_index++){
        if(page_table_index == 0){                  // to get next 10 bits, in this case the start  
            syscall_page_table_entries[page_table_index].pt_base_addr = VIDEO >> shiftbytwelve;   // to get 20 bits
            syscall_page_table_entries[page_table_index].avail = 0;
            syscall_page_table_entries[page_table_index].global_page = 0; 
            syscall_page_table_entries[page_table_index].page_size = 0; 
            syscall_page_table_entries[page_table_index].reserved = 0; 
            syscall_page_table_entries[page_table_index].accessed = 0;
            syscall_page_table_entries[page_table_index].cache_disabled = 0; 
            syscall_page_table_entries[page_table_index].write_through = 0;
            syscall_page_table_entries[page_table_index].user_supervisor = 1; 
            syscall_page_table_entries[page_table_index].read_write = 1;  
            syscall_page_table_entries[page_table_index].present = 1; 
        } else{
            syscall_page_table_entries[page_table_index].pt_base_addr = page_table_index; 
            syscall_page_table_entries[page_table_index].avail = 0;
            syscall_page_table_entries[page_table_index].global_page = 0; 
            syscall_page_table_entries[page_table_index].page_size = 0; 
            syscall_page_table_entries[page_table_index].reserved = 0; 
            syscall_page_table_entries[page_table_index].accessed = 0;
            syscall_page_table_entries[page_table_index].cache_disabled = 0; 
            syscall_page_table_entries[page_table_index].write_through = 0;
            syscall_page_table_entries[page_table_index].user_supervisor = 0; 
            syscall_page_table_entries[page_table_index].read_write = 0;  
            syscall_page_table_entries[page_table_index].present = 0; 
        }
    } 

    *screen_start = (uint8_t*) program_physaddr; // set screen start

    flush_TLB(); // because updating paging

    return 0;
}

/* 
 * 
 *   DESCRIPTION: will be updated 
 *   INPUTS:  will be updated 
 *   OUTPUTS: will be updated 
 *   RETURN VALUE: returns -1 temporarily  
 *   SIDE EFFECTS: will be updated 
 * 
 */
int32_t set_handler (int32_t signum, void* handler){
    return -1;
}

/* 
 * 
 *   DESCRIPTION: will be updated 
 *   INPUTS:  will be updated 
 *   OUTPUTS: will be updated 
 *   RETURN VALUE: returns -1 temporarily  
 *   SIDE EFFECTS: will be updated 
 * 
 */
int32_t sigreturn (void){
    return -1;
}

/* 
 *   DESCRIPTION: Helper function that obtains num_pids
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: returns num_pids
 *   SIDE EFFECTS: none
 */
int get_numpids(){
    return num_pids;
}

/* 
 *   DESCRIPTION: Helper function that obtains active_terminal
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: returns active_terminal
 *   SIDE EFFECTS: none
 */
int get_active_terminal(){
    return active_terminal;
}

/* 
 *   DESCRIPTION: Helper function that changes active_terminal
 *   INPUTS: terminal - terminal that we are changing active terminal to
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Updates active terminal based on the input terminal
 */
void set_active_terminal(int terminal){
    active_terminal = terminal;
}
