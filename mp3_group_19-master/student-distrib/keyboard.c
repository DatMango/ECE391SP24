#include "keyboard.h"
#include "syscall_modifier.h"

/*
		0x01	escape pressed	0x02	1 pressed	0x03	2 pressed
0x04	3 pressed	0x05	4 pressed	0x06	5 pressed	0x07	6 pressed
0x08	7 pressed	0x09	8 pressed	0x0A	9 pressed	0x0B	0 (zero) pressed
0x0C	- pressed	0x0D	= pressed	0x0E	backspace pressed	0x0F	tab pressed
0x10	Q pressed	0x11	W pressed	0x12	E pressed	0x13	R pressed
0x14	T pressed	0x15	Y pressed	0x16	U pressed	0x17	I pressed
0x18	O pressed	0x19	P pressed	0x1A	[ pressed	0x1B	] pressed
0x1C	enter pressed	0x1D	left control pressed	0x1E	A pressed	0x1F	S pressed
0x20	D pressed	0x21	F pressed	0x22	G pressed	0x23	H pressed
0x24	J pressed	0x25	K pressed	0x26	L pressed	0x27	 ; pressed
0x28	' (single quote) pressed	0x29	` (back tick) pressed	0x2A	left shift pressed	0x2B	\ pressed
0x2C	Z pressed	0x2D	X pressed	0x2E	C pressed	0x2F	V pressed
0x30	B pressed	0x31	N pressed	0x32	M pressed	0x33	, pressed
0x34	. pressed	0x35	/ pressed	0x36	right shift pressed	0x37	(keypad) * pressed
0x38	left alt pressed	0x39	space pressed	0x3A	CapsLock pressed	0x3B	F1 pressed
0x3C	F2 pressed	0x3D	F3 pressed	0x3E	F4 pressed	0x3F	F5 pressed
0x40	F6 pressed	0x41	F7 pressed	0x42	F8 pressed	0x43	F9 pressed
0x44	F10 pressed	0x45	NumberLock pressed	0x46	ScrollLock pressed	0x47	(keypad) 7 pressed
0x48	(keypad) 8 pressed	0x49	(keypad) 9 pressed	0x4A	(keypad) - pressed	0x4B	(keypad) 4 pressed
0x4C	(keypad) 5 pressed	0x4D	(keypad) 6 pressed	0x4E	(keypad) + pressed	0x4F	(keypad) 1 pressed
0x50	(keypad) 2 pressed	0x51	(keypad) 3 pressed	0x52	(keypad) 0 pressed	0x53	(keypad) . pressed
0x57	F11 pressed
0x58	F12 pressed						
*/

/*
0x81	escape released	0x82	1 released	0x83	2 released
0x84	3 released	0x85	4 released	0x86	5 released	0x87	6 released
0x88	7 released	0x89	8 released	0x8A	9 released	0x8B	0 (zero) released
0x8C	- released	0x8D	= released	0x8E	backspace released	0x8F	tab released
0x90	Q released	0x91	W released	0x92	E released	0x93	R released
0x94	T released	0x95	Y released	0x96	U released	0x97	I released
0x98	O released	0x99	P released	0x9A	[ released	0x9B	] released
0x9C	enter released	0x9D	left control released	0x9E	A released	0x9F	S released
0xA0	D released	0xA1	F released	0xA2	G released	0xA3	H released
0xA4	J released	0xA5	K released	0xA6	L released	0xA7	 ; released
0xA8	' (single quote) released	0xA9	` (back tick) released	0xAA	left shift released	0xAB	\ released
0xAC	Z released	0xAD	X released	0xAE	C released	0xAF	V released
0xB0	B released	0xB1	N released	0xB2	M released	0xB3	, released
0xB4	. released	0xB5	/ released	0xB6	right shift released	0xB7	(keypad) * released
0xB8	left alt released	0xB9	space released	0xBA	CapsLock released	0xBB	F1 released
0xBC	F2 released	0xBD	F3 released	0xBE	F4 released	0xBF	F5 released
0xC0	F6 released	0xC1	F7 released	0xC2	F8 released	0xC3	F9 released
0xC4	F10 released	0xC5	NumberLock released	0xC6	ScrollLock released	0xC7	(keypad) 7 released
0xC8	(keypad) 8 released	0xC9	(keypad) 9 released	0xCA	(keypad) - released	0xCB	(keypad) 4 released
0xCC	(keypad) 5 released	0xCD	(keypad) 6 released	0xCE	(keypad) + released	0xCF	(keypad) 1 released
0xD0	(keypad) 2 released	0xD1	(keypad) 3 released	0xD2	(keypad) 0 released	0xD3	(keypad) . released
0xD7	F11 released
0xD8	F12 released
*/

// keyboard connected to 0x60, connected to IR1

// array of all keys (lowercase) with indexes corresponding to their scan codes
char* lowercasearray[] = {" ", "esc", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", " ", "tab", "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]", 
"enter", "ctrl", "a", "s", "d", "f", "g", "h", "j", "k", "l", ";", "'", "`", "left shift", "\\", "z", "x", "c", "v", "b", "n", "m" , ",", ".", "/", "right shft", "*", "left alt"
, " ", "CapsLock", "F1", "F2","F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "NUMLOCK", "SCROLLLOCK", "7", "8", "9", "-", "4", "5", "6", "+", "1", "2", "3", "0", ".", " "," "," ","F11","F12"};
//array of all keys (but uppercase when appropriate)
char* uppercasearray[] = {" ", "esc", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "back", "tab", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "[", "]", 
"enter", "ctrl", "A", "S", "D", "F", "G", "H", "J", "K", "L", ";", "'", "`", "left shift", "\\", "Z", "X", "C", "V", "B", "N", "M" , ",", ".", "/", "right shft", "*", "left alt"
, " ", "CapsLock", "F1", "F2","F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "NUMLOCK", "SCROLLLOCK", "7", "8", "9", "-", "4", "5", "6", "+", "1", "2", "3", "0", ".", " "," "," ","F11","F12"};
// array of all keys (but the shift version when appropriate)
char* shiftcasearray[] = {" ", "esc", "!", "@", "#", "$", "%%", "^", "&", "*", "(", ")", "_", "+", "back", "tab", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "{", "}", 
"enter", "ctrl", "A", "S", "D", "F", "G", "H", "J", "K", "L", ":", "\"", "~", "left shift", "|", "Z", "X", "C", "V", "B", "N", "M" , "<", ">", "?", "right shft", "*", "left alt"
, " ", "CapsLock", "F1", "F2","F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "NUMLOCK", "SCROLLLOCK", "7", "8", "9", "-", "4", "5", "6", "+", "1", "2", "3", "0", ".", " "," "," ","F11","F12"};

int CAPS_LOCK = 0;                              //flags for caps lock, shift, ctrl, enter, alt
int SHIFT_HELD = 0;
int CTRL = 0;
int ENTER[3] = {0,0,0};
int ALT = 0;
int saved_curr_terminal =0;

int CURSOR_X = 0;                               //absolute x location of cursor
int CURSOR_Y = 0;                               //absolute y location of cursor

static int keyboard_count[3] = {0,0,0};                  //counter for current index in the keyboard buffer
static int keyboard_count1[3] = {0,0,0};                 //variable to hold previous value of keyboard_count after enter is pressed
int curr_terminal = 0;

int flag = 1;

// char* keyboard_buffer[keyboard_buffer_size];

/* 
 * 
 *   DESCRIPTION: initialization for keyboard (enables irq that keyboard is connected to)
 *                 
 *   INPUTS: none 
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: enables IRQ for keyboard, enables cursor, clears the screen to blank
 * 
 */
void init_keyboard(){
    enable_irq(IRQ_1);
    enable_cursor(curs_start, curs_end);
    clear();
}   

/* Handler for when interrupts are called */
/* 
 * 
 *   DESCRIPTION: This is the handler for when keyboard interrupts are encountered. Handles
 *              various cases of caps lock, shift, enter, and other function keys.
 *                 
 *   INPUTS: none 
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: recognizes correct keypresses and prints them to terminal
 * 
 */
void keyboard_handler(){
    //cli();
    int keypress;                          //int to hold value of keypress
    keypress = inb(keyboard_port);         //reads info from port

    if(keypress == enter){
        keyboard_count1[curr_terminal] = keyboard_count[curr_terminal];    //saves value of keyboard_count
        keyboard_buffer[curr_terminal][keyboard_buffer_size-1] = "\n";         //inserts new line character to end of keyboard buffer
        keyboard_count[curr_terminal] = 0;                  //resets keyboard counter to 0
        saved_curr_terminal = curr_terminal;
        putc('\n');                          //prints new line
        cursor_location[curr_terminal] = get_cursor_location();
        send_eoi(IRQ_1);                     //send eoi

        ENTER[curr_terminal] = 1;                           //set enter flag to 1
        return;
    }

    if(keypress == caps_key){         //checks to see if caps lock is pressed to update caps lock flag
        CAPS_LOCK = !CAPS_LOCK;
        send_eoi(IRQ_1); 
        return;
    }

    if(keypress == left_shift_key || keypress == right_shift_key_press){  //checks to see if shift is pressed and updates shift flag
        SHIFT_HELD = 1;     
        send_eoi(IRQ_1); 
        return;
    }
    if(keypress == left_shift_released || keypress == right_shift_key_release){ //checks to see if shift is released and updates shift flag
        SHIFT_HELD = 0;
        send_eoi(IRQ_1); 
        return;
    }

    if(keypress == left_alt_key_press || keypress == e_right_alt_key_press){ //checks to see if alt is pressed and sends eoi and updates alt flag
        ALT = 1;
        send_eoi(IRQ_1); 
        return;
    }
    if(keypress == left_alt_key_release || keypress == e_right_alt_key_release){ //checks to see if alt is pressed and sends eoi and updates alt flag
        ALT = 0;
        send_eoi(IRQ_1); 
        return;
    }

    // IMPLEMENT PRINTING EVERYTHING BACK TO SCREEN FOR DIFFERENT TERMINALS

    // If alt + f1, switch to terminal 1
    if(keypress == f1_press && ALT == 1){
        if(curr_terminal != 0){ // Checks if it is already on terminal 1, prevents spamming and breaking
            page_table_entries[((VIDEO & MASK_NUMBER) >> shiftbytwelve)].pt_base_addr = PERM_VIDEO >> shiftbytwelve;
            save_terminal(curr_terminal, cursor_location[curr_terminal]);
            curr_terminal = 0;
            clear();
            display_terminal(curr_terminal, cursor_location[curr_terminal]);
        }
    }

    // If alt + f2, switch to terminal 2
    if(keypress == f2_press && ALT == 1){
        if(curr_terminal != 1){ // Checks if it is already on terminal 2, prevents spamming and breaking
            page_table_entries[((VIDEO & MASK_NUMBER) >> shiftbytwelve)].pt_base_addr = PERM_VIDEO >> shiftbytwelve;
            save_terminal(curr_terminal, cursor_location[curr_terminal]);
            curr_terminal = 1;
            clear();
            display_terminal(curr_terminal, cursor_location[curr_terminal]);
        }
    }

    // If alt + f3, switch to terminal 3
    if(keypress == f3_press && ALT == 1){
        if(curr_terminal != 2){ // Checks if it is already on terminal 3, prevents spamming and breaking
            page_table_entries[((VIDEO & MASK_NUMBER) >> shiftbytwelve)].pt_base_addr = PERM_VIDEO >> shiftbytwelve;
            save_terminal(curr_terminal, cursor_location[curr_terminal]);
            curr_terminal = 2;
            clear();
            display_terminal(curr_terminal, cursor_location[curr_terminal]);
        }
    }

    if(keypress == left_ctr_key_press || keypress == e_right_ctr_key_press){ //checks to see if ctrl is pressed and updates ctrl flag
        CTRL = 1;
        send_eoi(IRQ_1); 
        return;
    }
    if(keypress == left_ctr_key_release || keypress == e_right_ctr_key_release){ //checks to see if ctrl is released and updates ctrl flag
        CTRL = 0;
        send_eoi(IRQ_1); 
        return;
    }
    if(keypress == tab_press && keyboard_count[curr_terminal] < keyboard_buffer_size - 1){ //checks to see if tab is pressed and if keyboard buffer is full
        int i;  
        int total_spaces = 0; //max spaces either 4 or spaces left in keyboard buffer
        total_spaces = (keyboard_buffer_size - 1) - keyboard_count[curr_terminal]; //calculates total spaces left in keyboard buffer
        if(total_spaces >= tab_spacing){                            //if total spaces is greater than 4, set total spaces to 4
            total_spaces = tab_spacing;
        }
        for(i = 0; i < total_spaces ; i++){                         //prints total_spaces number of spaces and adds corresponding number of spaces to keyboard buffer
            putc(*(uppercasearray[space_array_index]));
            cursor_location[curr_terminal] = get_cursor_location();
            keyboard_buffer[curr_terminal][keyboard_count[curr_terminal]] = " ";
            keyboard_count[curr_terminal]++;
        }
        send_eoi(IRQ_1);
        return;
    }
    if(keypress == backspace){                              //checks to see if backspace is pressed
        if(keyboard_count[curr_terminal] > 0){                            
            keyboard_count[curr_terminal]--;                               //decrements keyboard_count
            keyboard_buffer[curr_terminal][keyboard_count[curr_terminal]] = " ";          //replaces previous letter with a space in keyboard buffer
            bputc(*(lowercasearray[space_array_index]));    //prints a space in place of previous letter with modified putc function
            cursor_location[curr_terminal] = get_cursor_location();
            
        }
        send_eoi(IRQ_1);
        return;
   }
    if(CTRL && keypress == L_key){                          //checks to see if ctrl is being held and if keypress is L and clears if both conditions are met
        clear();
        send_eoi(IRQ_1);
        return;
    }

    if((keypress < keypress_limit_max) && (keypress > keypress_limit_min) && (keyboard_count[curr_terminal] < keyboard_buffer_size - 1)){    //checks to see if keypress is in supported key range and if there is space in keyboard buffer
        if(SHIFT_HELD){                                             //if shift flag is 1, then show shifted version of characters
            putc(*(shiftcasearray[keypress]));
            cursor_location[curr_terminal] = get_cursor_location();
            keyboard_buffer[curr_terminal][keyboard_count[curr_terminal]] = shiftcasearray[keypress];
        }
        else if(CAPS_LOCK){                                          //if caps lock flag is 1, then show uppercase letters
            putc(*(uppercasearray[keypress]));
            cursor_location[curr_terminal] = get_cursor_location();
            keyboard_buffer[curr_terminal][keyboard_count[curr_terminal]] = uppercasearray[keypress];  
        }
        else{                                                        //if caps lock flag is 0, then show lowercase letters
            putc(*(lowercasearray[keypress]));
            cursor_location[curr_terminal] = get_cursor_location();
            keyboard_buffer[curr_terminal][keyboard_count[curr_terminal]] = lowercasearray[keypress];
        }
        keyboard_count[curr_terminal]++;                                            //increment keyboard_count
    }    
    send_eoi(IRQ_1); 
    return;    
}

/*
 * enable_cursor()
 *  DESCRIPTION: set the start and end scanlines, the rows where the cursor starts and ends
 *  INPUTS: cursor_start - the start scanline
 *          cursor_end - the end scanline
 *  OUTPUTS: 
 *  RETURN VALUE: 
 *  SIDE EFFECTS: 
 * 
 */
void enable_cursor(uint8_t cursor_start, uint8_t cursor_end){
    // referenced from os dev
    outb(low_cursor_reg,cursor_reg1);
	outb((inb(cursor_reg2) & cursor_reg3) | cursor_start,cursor_reg2);
 
	outb(low_cursor_reg1,cursor_reg1);
	outb((inb(cursor_reg2) & cursor_reg4) | cursor_end,cursor_reg2);
    CURSOR_X = 0;
    CURSOR_Y = 0;
    update_cursor(CURSOR_X,CURSOR_Y); // makes cursor start at 0,0
}

/*
 * disable_cursor()
 *  DESCRIPTION: disables the cursor
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: disables cursor
 * 
 */
void disable_cursor()
{
	outb(low_cursor_reg,cursor_reg1);
	outb(cursor_reg5,cursor_reg2);
}

/*
 * terminal_open()
 *  DESCRIPTION: initializes terminal stuff
 *  INPUTS: filename
 *  OUTPUTS: none
 *  RETURN VALUE: 0 on success, -1 on failure
 *  SIDE EFFECTS: none
 */
int32_t terminal_open(const uint8_t* filename){
    return 0;
}

/*
 * terminal_close()
 *  DESCRIPTION: clears any terminal specific variables
 *  INPUTS: fd
 *  OUTPUTS: none
 *  RETURN VALUE: 0 on success, -1 on failure
 *  SIDE EFFECTS: none
 */
int32_t terminal_close(int32_t fd){
    return 0;
}

/* read FROM keyboard buffer to terminal buffer */
/*
* terminal_read()
 *  DESCRIPTION: read from keyboard buffer to terminal buffer
 *  INPUTS: fd, buf - terminal buffer to read to, nbytes - number of bytes
 *  OUTPUTS: none
 *  RETURN VALUE: number of bytes read on success, -1 on failure
 *  SIDE EFFECTS: terminal buffer contains entries from keyboard buffer
 */
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes){
    int i;
    // printf("current terminal: %d:\n", curr_terminal);
    keyboard_count[get_active_terminal()] = 0;
    sti();
    while(!ENTER[curr_terminal] || (curr_terminal != get_active_terminal())){     //should only return when enter is pressed, so while loop prevents function from returning

    }
    cli();
    for(i = 0; i < keyboard_count1[get_active_terminal()]; i++){         //loads buf with keyboard buffer
        ((uint8_t*)buf)[i] = *(keyboard_buffer[get_active_terminal()][i]);
    }
    // printf("current terminal: %d:\n", curr_terminal);
    ENTER[get_active_terminal()] = 0;                                    //resets enter flag
    // cli();
    return keyboard_count1[get_active_terminal()];                       //returns number of bytes read -> number of chars read -> keyboard_count
}

/* writes to the terminal from terminal buffer */
/*
 * terminal_write()
 *  DESCRIPTION: writes to the terminal from the terminal buffer
 *  INPUTS: fd, buf - terminal buffer to written to terminal, nbytes - number of bytes
 *  OUTPUTS: none
 *  RETURN VALUE: number of bytes written on success, -1 on failure
 *  SIDE EFFECTS: video memory is altered, terminal displays what was in terminal buffer
 */
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes){
    int i;          
    uint8_t* ptr;                       //pointer to hold buf pointer
    int count;
    ptr = ((uint8_t*)buf);              //casts but to uint8_t pointer and sets this as ptr
    pcb_t* pcb_ptr;
    if(ptr == NULL){                    //checks to see if ptr is null
        return -1;
    }
    cli();
    // ENTER[curr_terminal] = 0;
    switch(get_active_terminal()){
        case 0:
            pcb_ptr = term1_process;
            break;
        case 1:
            pcb_ptr = term2_process;
            break;
        case 2:
            pcb_ptr = term3_process;
            break;
    }
    for(i = 0; i < nbytes; i++){        //prints out buf character by character
        if(curr_terminal == pcb_ptr->num_terminal){ //compare it to the pcbs terminal instead of active terminal ?, move num_pid so we have access to it 
            putc(ptr[i]);
        }
        else{
            putcvmem(ptr[i], get_active_terminal());
        }
        cursor_location[curr_terminal] = get_cursor_location();           
        count++;                        //increments count (nnumber of characters/bytes written)
    }
    sti();
    return count;                       //returns number of bytes written
}

/* getter function for terminal being shown to screen */
int get_curr_terminal(){
    return curr_terminal;
}
