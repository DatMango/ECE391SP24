// #include "types.h"
#include "keyboard.h"
#include "rtc.h"
#include "filesystem.h"
#include "paging.h"
#include "paging_modifier.h"
// #define ENTRIES     1024 
// #define shiftbytwelve   12
// #define MASK_NUMBER     0x003FF000
// #define VIDEO           0xB8000
#define IRQ_0 0

#define eightmb (1024*8)*(1024)
#define eightkb (1024*8)
#define fourmb (1024*4)*(1024)
#define onetwoeightmb (1024 * 1024 * 128)
#define onethreetwomb (1024 * 1024 * 132)
#define programstartvirtualmem 0x8048000
#define shiftbytwentytwo       22
#define USER_CS     0x0023
#define USER_DS     0x002B
#define page_dir_index 32 
#define page_dir_index_extra 33
#define num_processes 6
#define files_per_task 8
#define executable_bytes 4
#define max_filename_size 32
#define file_after_terminerw 2
#define stdin_fd 0 
#define stdout_fd 1
#define rtc_filetype 0 
#define directory_filetype 1
#define file_filetype 2
#define max_fd_array_elem 7 
#define max_pcb_array_elem 5
#define bytes_to_be_read 4
#define eip_bytes_offset 24
#define esp_offset 4
#define each_block 4
#define executable0_bytes 0x7f
#define executable1_bytes 0x45
#define executable2_bytes 0x4c
#define executable3_bytes 0x46
#define thirtythreemb   (33*1024*1024)
#define pit_100hz_upperbyte 0x2E
#define pit_100hz_lowerbyte 0x9B
#define pit_command_port 0x43
#define pit_channel_0 0x40

// dentry for sys calls 
//dentry_t dentry_sys; 

/* halt gets the ebp saved in execute and it restores the context to the parent process  */
extern int32_t halt (uint8_t status);

/* executes the files, takes care of paging (maps the virtual memory to a physical page), 
saves current EBP, and creates context switch*/
extern int32_t execute (const uint8_t* command);

/* syscall read that calls appropriate read (rtc, directory, or file) */
extern int32_t read (int32_t fd, void* buf, int32_t nbytes);

/* syscall write that calls appropriate write (rtc, directory, or file) */
extern int32_t write (int32_t fd, const void* buf, int32_t nbytes);

/* syscall open, checks each dentry filetype for rtc, file, or directory and sets corresponding 
read, write, open, close functions  */
extern int32_t open (const uint8_t* filename);

/* syscall close, sets flags and ensures close is being called on valid fd */
extern int32_t close (int32_t fd);

/*  strncpy to get info from command buffer based on pid from pcb array into buffer  */
extern int32_t getargs (uint8_t* buf, int32_t nbytes);

/*  map 4kb page to video mem and sets virt address to screen start */
extern int32_t vidmap (uint8_t** screen_start);

/* will be updated */
extern int32_t set_handler (int32_t signum, void* handler);

/* will be updated */
extern int32_t sigreturn (void);

/* maps pcb in kernel memory and initializes fd 0 and 1 of each pcb's fd array to be stdin/out*/
extern void init_syscall(); 

/* Helper function, gets num_pids */
extern int get_numpids();

/* Helper function, gets the active terminal */
extern int get_active_terminal();

/* Helper function, updates the active terminal */
extern void set_active_terminal();
