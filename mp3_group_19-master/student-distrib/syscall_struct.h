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
    int32_t flags;     
} file_descriptor_t; 

/*  pcb struct with an fd array for each, ebp, eip, esp, 
parent pid, pid, a flag to indicate usage, and a saved ebp */
typedef struct{
    file_descriptor_t fd_array[MAX_PROCESS];
    uint32_t EBP;
    uint32_t EIP;
    uint32_t ESP;
    uint32_t parent_pid;
    uint32_t pid;
    uint32_t in_use_flag;
    uint32_t save_ebp;
    uint32_t argument;

    // Buffer from keyboard_buffer
    unsigned char command_buffer[32];

} pcb_t;