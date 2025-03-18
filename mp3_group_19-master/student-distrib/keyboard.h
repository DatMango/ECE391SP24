#include "lib.h"
#include "i8259.h"
#include "types.h"

#define IRQ_1               1

#define keyboard_port 0x60
#define keypress_limit_max 0x3B
#define keypress_limit_min 0x01
#define caps_key 0x3A
#define left_shift_key 0x2A
#define left_shift_released 0xAA
#define right_shift_key_press 0x36
#define right_shift_key_release 0xB6
#define left_alt_key_press 0x38
#define left_alt_key_release 0xB8
#define left_ctr_key_press 0x1D
#define left_ctr_key_release 0x9D
#define e_right_ctr_key_press 0x1D
#define e_right_ctr_key_release 0x9D
#define tab_press 0x0F
#define backspace 0x0E
#define enter   0x1C

#define f1_press 0x3B
#define f2_press 0x3C
#define f3_press 0x3D
#define f1_release 0xBB
#define f2_release 0xBC
#define f3_release 0xBD

#define extended_key 0xE0
#define e_right_alt_key_press 0x38
#define e_right_alt_key_release 0xB8

#define NUM_COLS        80
#define NUM_ROWS        25
#define curs_start      0
#define curs_end        NUM_ROWS
#define tab_spacing     4
#define keyboard_buffer_size 128
#define number_of_buffers 3
#define space_array_index 0

#define L_key 0x26

#define low_cursor_reg 0x0A
#define low_cursor_reg1 0x0B
#define cursor_reg1 0x3D4
#define cursor_reg2 0x3D5
#define cursor_reg3 0xC0
#define cursor_reg4 0xE0
#define cursor_reg5 0x20 

#define screen_size (80*25)

#define PERM_VIDEO  0xBC000

char* keyboard_buffer[number_of_buffers][keyboard_buffer_size];    //keyboard buffer to store letters printed to terminal
// char* screen_buffer[number_of_buffers][screen_size];


// initalizes keyboard and enables appropriate IRQ 
void init_keyboard();

// maps the keypresses and prints them from the vector. Also takes care of caps lock flag
void keyboard_handler();

//enables cursor in terminal
void enable_cursor(uint8_t cursor_start, uint8_t cursor_end);

//disables cursor
void disable_cursor();

//does nothing
int32_t terminal_open(const uint8_t* filename);

//does nothing 
int32_t terminal_close(int32_t fd);

//reads from keyboard buffer into buf
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);

//writes from buf to screen
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);

int get_curr_terminal();
