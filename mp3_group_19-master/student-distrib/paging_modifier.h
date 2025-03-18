#define VIDEO           0xB8000
#define TERMINAL1_VIDEO 0xB9000
#define TERMINAL2_VIDEO 0xBA000
#define TERMINAL3_VIDEO 0xBB000
#define KERNEL_ADDR     0x400000  
#define MASK_NUMBER     0x003FF000
#define shiftbytwelve          12
#define shiftbytwentytwo       22

// paging initialization sets the bits for 4MB and 4KB, that is kernel and video memory 
extern void paging_init(); 
