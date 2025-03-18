#include "types.h"
#include "lib.h"
#define DATAENTRYBLOCKS     1023 // 4096/4 – 1 = 1023 => at most 1023 data blocks 
#define DENTRY_RESERVE      24    
#define BOOT_BLOCK_RESERVE  52    
#define FILENAME_SIZE       32   
#define NUM_DIR_ENTRIES     63
#define FOURKB              4096    // in bytes
#define buffer_size         3000
#define MAX_PROCESS 8
#define array_num_pcbs 6

/* dentry struct with bytes allocated based on spec, filename of 32 bytes, 
filetype of 4 bytes, inode num of 4 bytes, and 24 reserved bytes */
typedef struct{
    unsigned char filename[FILENAME_SIZE]; 
    int32_t filetype; 
    int32_t inode_num; 
    unsigned char reserved[DENTRY_RESERVE];
} dentry_t;

/* boot block struct with allocated bytes based on spec (4kB), 4 bytes 
for num direct entries, 4 bytes for num inodes, 4 bytes for num data 
blocks, 52 reserved bytes and 64 bytes of 64 byte dentry */
typedef struct{
    int32_t num_dentry; 
    int32_t num_inodes; 
    int32_t num_datablocks; 
    unsigned char reserved[BOOT_BLOCK_RESERVE];
    dentry_t direct_entry[NUM_DIR_ENTRIES]; 
} boot_block_t; 

/* inode struct with allocated bytes based on spec (4kB), 4 bytes for length 
in bytes, and 1023 entries with data block numbers that point to corresponding 
data block */

typedef struct{
    int32_t length_B; 
    int32_t blocks[DATAENTRYBLOCKS]; 
} inode_t;


/*---------------------------------------system call structs----------------------------------------------------------*/
/* general function pointers for read, write, open, close that are 
later assigned to appropriate (rtc, directory, or file) functions 
depending on dentry filetype */
typedef struct{
    int32_t (*s_open)(const uint8_t*);
    int32_t (*s_read)(int32_t, void*, int32_t);
    int32_t (*s_write)(int32_t, const void*, int32_t);
    int32_t (*s_close)(int32_t); 
} gen_functions_t;

/* file descriptor struct with file operation that maps to appropriate  
general function pointer that eventually maps to the specific function 
pointer, inode from dentry, updated file position, and in use flag */
typedef struct{
    gen_functions_t file_operation; 
    int32_t inode; 
    int32_t file_pos;
    int32_t dir_index;
    int32_t file_size;
    int32_t flags;
    dentry_t fd_dentry;     
} file_descriptor_t; 

/*  pcb struct with an fd array for each, ebp, eip, esp, 
parent pid, pid, a flag to indicate usage, and a saved ebp */
typedef struct{
    file_descriptor_t fd_array[MAX_PROCESS];
    uint32_t EIP;
    uint32_t EBP;
    uint32_t ESP;
    uint32_t terminal_EBP;
    uint32_t terminal_ESP;
    uint32_t parent_pid;
    uint32_t child_pid;
    uint32_t pid;
    uint32_t in_use_flag;
    uint32_t argument;

    uint32_t num_terminal;
    // Buffer from keyboard_buffer
    unsigned char command_buffer[32];

} pcb_t;

// array for pcbs for max number of processes 
pcb_t* pcb_arr[array_num_pcbs]; 

pcb_t* cur_pcb;

pcb_t* term1_process;
pcb_t* term2_process;
pcb_t* term3_process;
/*---------------------------------------------------------------------------------------------------------------------*/

/* bootblock pointer to make it point to address given in kernel.c (mod->mod_start)*/
boot_block_t* bootblock;

/* declaring variables that store dentry structs for file and directory */
dentry_t dentry_glo;
dentry_t dentry_dir;


/* finds right dentry struct based on filename */
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);

/* find right dentry struct based on index into bootblock */
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);

/* reads data based on inode number, offset, and length. mem copies read data into buffer */
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

/* calls read_dentry_by_name to find right dentry struct based on filename */
int32_t file_open(const uint8_t* filename);

/* calls read_data, passes in bytes to be read, buffer, and inode num from right dentry */
int32_t file_read(int32_t fd, void* buf, int32_t nbytes);

/* read-only filesystem so just returns -1 */
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes);

/* closes file, returns 0 */
int32_t file_close(int32_t fd);

/* calls read_dentry_by_name to find right dentry struct based on filename, directory open */
int32_t directory_open(const uint8_t* filename);

/* reads one file name at a time */
int32_t directory_read(int32_t fd, void* buf, int32_t nbytes);

/* read-only so just returns -1 */
int32_t directory_write(int32_t fd, const void* buf, int32_t nbytes);

/* closes file, returns 0*/
int32_t directory_close(int32_t fd);
