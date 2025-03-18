#include "syscall_modifier.h"

// #define PERM_VIDEO  0xBC000
#define VIDEO_CHANGER   0x1000
// static char* perm_vid = (char *)PERM_VIDEO;


/* Initializes PIT */
void init_pit();

/* Handler for PIT */
void pit_handler();

/* Scheduler for 3 terminals, called from PIT */
void scheduler();

/* Helper function to change video memory */
void change_vidmem_mapping(int terminal);

/* Helper function for fish to change video memory */
void vidmap_mapping(int terminal);
