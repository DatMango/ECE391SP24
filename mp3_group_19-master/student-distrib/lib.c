/* lib.c - Some basic library functions (printf, strlen, etc.)
 * vim:ts=4 noexpandtab */

#include "lib.h"

#define VIDEO       0xB8000
#define TERMINAL1_VIDEO 0xB9000
#define TERMINAL2_VIDEO 0xBA000
#define TERMINAL3_VIDEO 0xBB000
#define PERM_VIDEO  0xBC000
#define NUM_COLS    80
#define NUM_ROWS    25
#define ATTRIB      0x7
#define four_kb     (4 * 1024)

static int screen_x;
static int screen_y;

static int vid_mem_screen_x[3] = {0,0,0};
static int vid_mem_screen_y[3] = {0,0,0};

static char* video_mem = (char *)VIDEO;
static char* video_mem1 = (char *)TERMINAL1_VIDEO;
static char* video_mem2 = (char *)TERMINAL2_VIDEO;
static char* video_mem3 = (char *)TERMINAL3_VIDEO;
static char* perm_vid = (char *)PERM_VIDEO;

/* void clear(void);
 * Inputs: void
 * Return Value: none
 * Function: Clears video memory, sets cursor to upper left corner
 */
void clear(void) {
    int32_t i;
    for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
        *(uint8_t *)(video_mem + (i << 1)) = ' ';
        *(uint8_t *)(video_mem + (i << 1) + 1) = ATTRIB;
    }
    screen_x = 0;
    screen_y = 0;
    update_cursor(screen_x, screen_y);
}

void display_terminal(int current_terminal, int num_characters){
    // modifiy video memory calculation later to account for 3 terminals
    // char* vid_mem_calc;
    int i;
    switch(current_terminal){
        // Not supposed to do this, only supposed to display Video (don't display the other 3 parts of vid map)
        case 0:
            memcpy(perm_vid, video_mem1, four_kb);
            // vid_mem_calc = video_mem1;
            break;
        case 1:
            memcpy(perm_vid, video_mem2, four_kb);
            // vid_mem_calc = video_mem2;
            break;
        case 2:
            memcpy(perm_vid, video_mem3, four_kb);
            // vid_mem_calc = video_mem3;
            break;
        default:
            break;
    }
    for(i = 0; i < num_characters; i++){
        putc(*(uint8_t *)((0xB9000 + current_terminal * 0x1000) + ((i) << 1)));
        // putc(*(uint8_t *)((0xB8000) + ((i) << 1)));
    }
}

void save_terminal(int current_terminal, int num_characters){
    // char* vid_mem_calc;
    // int i;
    switch(current_terminal){
        case 0:
            memcpy(video_mem1, perm_vid, four_kb);
            // vid_mem_calc = video_mem1;
            break;
        case 1:
            memcpy(video_mem2, perm_vid, four_kb);
            // vid_mem_calc = video_mem2;
            break;
        case 2:
            memcpy(video_mem3, perm_vid, four_kb);
            // vid_mem_calc = video_mem3;
            break;
        default:
            break;
    }
    // flush_TLB();
    // for(i = 0; i < num_characters; i++){
    //     *(uint8_t *)(vid_mem_calc + ((i) << 1)) = *(uint8_t *)(video_mem + ((i) << 1));
    //     *(uint8_t *)(vid_mem_calc + ((i) << 1) + 1) = *(uint8_t *)(video_mem + ((i) << 1) + 1);
    // }
}

/*
 * update_cursor()
 *  DESCRIPTION: moves the cursor to a new location
 *  INPUTS: x - the column number to move the cursor to
 *          y - the row number to move the cursor to
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: moves the cursor to the specified x and y location
 * 
 */
void update_cursor(int x, int y)
{
	uint16_t pos = y * NUM_COLS + x;    // not sure if NUM_COLS is correct.. used to be VGA_WIDTH osdev
 
	outb(0x0F,0x3D4);
	outb((uint8_t) (pos & 0xFF),0x3D5);
	outb(0x0E,0x3D4);
	outb((uint8_t) ((pos >> 8) & 0xFF),0x3D5);
}

/*
 *
 *
 * 
 * 
 * */
uint32_t get_cursor_location(){
    return NUM_COLS * screen_y + screen_x;
}

/* Standard printf().
 * Only supports the following format strings:
 * %%  - print a literal '%' character
 * %x  - print a number in hexadecimal
 * %u  - print a number as an unsigned integer
 * %d  - print a number as a signed integer
 * %c  - print a character
 * %s  - print a string
 * %#x - print a number in 32-bit aligned hexadecimal, i.e.
 *       print 8 hexadecimal digits, zero-padded on the left.
 *       For example, the hex number "E" would be printed as
 *       "0000000E".
 *       Note: This is slightly different than the libc specification
 *       for the "#" modifier (this implementation doesn't add a "0x" at
 *       the beginning), but I think it's more flexible this way.
 *       Also note: %x is the only conversion specifier that can use
 *       the "#" modifier to alter output. */
int32_t printf(int8_t *format, ...) {

    /* Pointer to the format string */
    int8_t* buf = format;

    /* Stack pointer for the other parameters */
    int32_t* esp = (void *)&format;
    esp++;

    while (*buf != '\0') {
        switch (*buf) {
            case '%':
                {
                    int32_t alternate = 0;
                    buf++;

format_char_switch:
                    /* Conversion specifiers */
                    switch (*buf) {
                        /* Print a literal '%' character */
                        case '%':
                            putc('%');
                            break;

                        /* Use alternate formatting */
                        case '#':
                            alternate = 1;
                            buf++;
                            /* Yes, I know gotos are bad.  This is the
                             * most elegant and general way to do this,
                             * IMHO. */
                            goto format_char_switch;

                        /* Print a number in hexadecimal form */
                        case 'x':
                            {
                                int8_t conv_buf[64];
                                if (alternate == 0) {
                                    itoa(*((uint32_t *)esp), conv_buf, 16);
                                    puts(conv_buf);
                                } else {
                                    int32_t starting_index;
                                    int32_t i;
                                    itoa(*((uint32_t *)esp), &conv_buf[8], 16);
                                    i = starting_index = strlen(&conv_buf[8]);
                                    while(i < 8) {
                                        conv_buf[i] = '0';
                                        i++;
                                    }
                                    puts(&conv_buf[starting_index]);
                                }
                                esp++;
                            }
                            break;

                        /* Print a number in unsigned int form */
                        case 'u':
                            {
                                int8_t conv_buf[36];
                                itoa(*((uint32_t *)esp), conv_buf, 10);
                                puts(conv_buf);
                                esp++;
                            }
                            break;

                        /* Print a number in signed int form */
                        case 'd':
                            {
                                int8_t conv_buf[36];
                                int32_t value = *((int32_t *)esp);
                                if(value < 0) {
                                    conv_buf[0] = '-';
                                    itoa(-value, &conv_buf[1], 10);
                                } else {
                                    itoa(value, conv_buf, 10);
                                }
                                puts(conv_buf);
                                esp++;
                            }
                            break;

                        /* Print a single character */
                        case 'c':
                            putc((uint8_t) *((int32_t *)esp));
                            esp++;
                            break;

                        /* Print a NULL-terminated string */
                        case 's':
                            puts(*((int8_t **)esp));
                            esp++;
                            break;

                        default:
                            break;
                    }

                }
                break;

            default:
                putc(*buf);
                break;
        }
        buf++;
    }
    return (buf - format);
}

/* int32_t puts(int8_t* s);
 *   Inputs: int_8* s = pointer to a string of characters
 *   Return Value: Number of bytes written
 *    Function: Output a string to the console */
int32_t puts(int8_t* s) {
    register int32_t index = 0;
    while (s[index] != '\0') {
        putc(s[index]);
        index++;
    }
    return index;
}
//need to modify putc because shell uses puts which calls putc, if putc is not modified will never see the starting shell in other terminals 
/* void putc(uint8_t c);
 * Inputs: uint_8* c = character to print
 * Return Value: void
 *  Function: Output a character to the console. Function was also
 *          modified to support scrolling if at bottom row of terminal,
 *          as well as updating the cursor to where the next character
 *          would be. 
 */
void putc(uint8_t c) {
    if(c != '\0'){
        if(c == '\n' || c == '\r') {    // newline
            if(screen_y < NUM_ROWS - 1){
                screen_y++;
            }
            else{
                scroll();               //scroll if screen_y is at line 24
                screen_y = NUM_ROWS - 1; //sets screen_y to 24
            }
            screen_x = 0;
        } 
        else {
            if(screen_y > NUM_ROWS - 1){    //scroll if screen_y is greater than 24
                scroll();                       
                screen_y = NUM_ROWS - 1;    //sets screen_y back to 24
            }
            *(uint8_t *)(perm_vid + ((NUM_COLS * screen_y + screen_x) << 1)) = c;
            *(uint8_t *)(perm_vid + ((NUM_COLS * screen_y + screen_x) << 1) + 1) = ATTRIB;
            if(screen_x > NUM_COLS - 1){
                screen_x = 0;          //wraps screen_x back to col 0, and increments screen_y by 1
                screen_y++;
                if(screen_y > NUM_ROWS - 1){
                    scroll();                //scrolls if screen_y is greater than 24(goes below the terminal dimension)
                    screen_y = NUM_ROWS - 1;
                }
            }
            else{
                screen_x++;
                if(screen_x > NUM_COLS - 1){
                    screen_x = 0;          //wraps screen_x back to col 0, and increments screen_y by 1
                    screen_y++;
                    if(screen_y > NUM_ROWS - 1){
                        scroll();                //scrolls if screen_y is greater than 24(goes below the terminal dimension)
                        screen_y = NUM_ROWS - 1;
                    }
                }
            }
        }
        update_cursor(screen_x, screen_y);
    }
}

void putcvmem(uint8_t c, int active_terminal) {
    char* vid_mem_calc;

    switch(active_terminal){
        case 0:
            vid_mem_calc = video_mem1;
            break;
        case 1:
            vid_mem_calc = video_mem2;
            break;
        case 2:
            vid_mem_calc = video_mem3;
            break;
        default:
            break;
    }
    // if(current_terminal == active_terminal){
    //     vid_mem_calc = VIDEO;
    // }

    if(c != '\0'){
        if(c == '\n' || c == '\r') {    // newline
            if(vid_mem_screen_y[active_terminal] < NUM_ROWS - 1){
                vid_mem_screen_y[active_terminal]++;
            }
            else{
                scroll_mem(active_terminal);               //scroll if screen_y is at line 24
                vid_mem_screen_y[active_terminal] = NUM_ROWS - 1; //sets screen_y to 24
            }
            vid_mem_screen_x[active_terminal] = 0;
        } 
        else {
            if(vid_mem_screen_y[active_terminal] > NUM_ROWS - 1){    //scroll if screen_y is greater than 24
                scroll_mem(active_terminal);                       
                vid_mem_screen_y[active_terminal] = NUM_ROWS - 1;    //sets screen_y back to 24
            }
            *(uint8_t *)(vid_mem_calc + ((NUM_COLS * vid_mem_screen_y[active_terminal] + vid_mem_screen_x[active_terminal]) << 1)) = c;
            *(uint8_t *)(vid_mem_calc + ((NUM_COLS * vid_mem_screen_y[active_terminal] + vid_mem_screen_x[active_terminal]) << 1) + 1) = ATTRIB;
            if(vid_mem_screen_x[active_terminal] > NUM_COLS - 1){
                vid_mem_screen_x[active_terminal] = 0;          //wraps screen_x back to col 0, and increments screen_y by 1
                vid_mem_screen_y[active_terminal]++;
                if(vid_mem_screen_y[active_terminal] > NUM_ROWS - 1){
                    scroll_mem(active_terminal);                //scrolls if screen_y is greater than 24(goes below the terminal dimension)
                    vid_mem_screen_y[active_terminal] = NUM_ROWS - 1;
                }
            }
            else{
                vid_mem_screen_x[active_terminal]++;
                if(vid_mem_screen_x[active_terminal] > NUM_COLS - 1){
                    vid_mem_screen_x[active_terminal] = 0;          //wraps screen_x back to col 0, and increments screen_y by 1
                    vid_mem_screen_y[active_terminal]++;
                    if(vid_mem_screen_y[active_terminal] > NUM_ROWS - 1){
                        scroll_mem(active_terminal);                //scrolls if screen_y is greater than 24(goes below the terminal dimension)
                        vid_mem_screen_y[active_terminal] = NUM_ROWS - 1;
                    }
                }
            }
        }
        cursor_location[active_terminal] = vid_mem_screen_y[active_terminal] * NUM_COLS + vid_mem_screen_x[active_terminal];
    }
}

/* void bputc(uint8_t c);
 * Inputs: uint_8* c = character to print
 * Return Value: void
 * Function: Output a character to the console */
void bputc(uint8_t c) {
    screen_x--;
    if(screen_x < 0 && screen_y == 0){      //checks to see if screen_x and screen_y are at 0,0 (top left corner)
        screen_x = 0;                       //prevents screen_x and y from going back
        screen_y = 0;
    }
    if(screen_x < 0){
        screen_x = NUM_COLS - 1;           //wraps screen_x and screen_y when appropriate
        if(screen_y > 0){
        screen_y--;
        }
    }
    update_cursor(screen_x, screen_y);      //update cursor location
    *(uint8_t *)(perm_vid + ((NUM_COLS * screen_y + screen_x) << 1)) = c;          //copies character and attribute to video memory
    *(uint8_t *)(perm_vid + ((NUM_COLS * screen_y + screen_x) << 1) + 1) = ATTRIB;
    update_cursor(screen_x, screen_y);      //update cursor location
}

/*
 *  void scroll()
 *  Inputs: none
 *  Outputs: none
 *  Return Value: none
 *  Function: helper function which 'scrolls' the terminal up. Copies
 *          each row of the video memory up a row. 
 */
void scroll(){
    int i = 0;
    int j = 0;
    for(i = 0; i < NUM_ROWS; i++){
        for(j = 0; j < NUM_COLS; j++){
            if(i == (NUM_ROWS-1)){
                *(uint8_t *)(perm_vid + ((NUM_COLS * i + j) << 1)) =  ' ';
                *(uint8_t *)(perm_vid + ((NUM_COLS * i + j) << 1) + 1) = ATTRIB;
            }
            else{
                *(uint8_t *)(perm_vid + ((NUM_COLS * i + j) << 1)) =  *(uint8_t *)(perm_vid + ((NUM_COLS * (i+1) + (j)) << 1));
                *(uint8_t *)(perm_vid + ((NUM_COLS * i + j) << 1) + 1) = ATTRIB;
            }
        }   
    }
}

/*
 *  void scroll()
 *  Inputs: none
 *  Outputs: none
 *  Return Value: none
 *  Function: helper function which 'scrolls' the terminal up. Copies
 *          each row of the video memory up a row. 
 */
void scroll_mem(int active_terminal){
    int i = 0;
    int j = 0;
    for(i = 0; i < NUM_ROWS; i++){
        for(j = 0; j < NUM_COLS; j++){
            if(i == (NUM_ROWS-1)){
                *(uint8_t *)(((active_terminal+1) * 0x1000 + video_mem) + ((NUM_COLS * i + j) << 1)) =  ' ';
                *(uint8_t *)(((active_terminal+1) * 0x1000 + video_mem) + ((NUM_COLS * i + j) << 1) + 1) = ATTRIB;
            }
            else{
                *(uint8_t *)(((active_terminal+1) * 0x1000 + video_mem) + ((NUM_COLS * i + j) << 1)) =  *(uint8_t *)(((active_terminal+1) * 0x1000 + video_mem) + ((NUM_COLS * (i+1) + (j)) << 1));
                *(uint8_t *)(((active_terminal+1) * 0x1000 + video_mem) + ((NUM_COLS * i + j) << 1) + 1) = ATTRIB;
            }
        }   
    }
}

/* int8_t* itoa(uint32_t value, int8_t* buf, int32_t radix);
 * Inputs: uint32_t value = number to convert
 *            int8_t* buf = allocated buffer to place string in
 *          int32_t radix = base system. hex, oct, dec, etc.
 * Return Value: number of bytes written
 * Function: Convert a number to its ASCII representation, with base "radix" */
int8_t* itoa(uint32_t value, int8_t* buf, int32_t radix) {
    static int8_t lookup[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int8_t *newbuf = buf;
    int32_t i;
    uint32_t newval = value;

    /* Special case for zero */
    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return buf;
    }

    /* Go through the number one place value at a time, and add the
     * correct digit to "newbuf".  We actually add characters to the
     * ASCII string from lowest place value to highest, which is the
     * opposite of how the number should be printed.  We'll reverse the
     * characters later. */
    while (newval > 0) {
        i = newval % radix;
        *newbuf = lookup[i];
        newbuf++;
        newval /= radix;
    }

    /* Add a terminating NULL */
    *newbuf = '\0';

    /* Reverse the string and return */
    return strrev(buf);
}

/* int8_t* strrev(int8_t* s);
 * Inputs: int8_t* s = string to reverse
 * Return Value: reversed string
 * Function: reverses a string s */
int8_t* strrev(int8_t* s) {
    register int8_t tmp;
    register int32_t beg = 0;
    register int32_t end = strlen(s) - 1;

    while (beg < end) {
        tmp = s[end];
        s[end] = s[beg];
        s[beg] = tmp;
        beg++;
        end--;
    }
    return s;
}

/* uint32_t strlen(const int8_t* s);
 * Inputs: const int8_t* s = string to take length of
 * Return Value: length of string s
 * Function: return length of string s */
uint32_t strlen(const int8_t* s) {
    register uint32_t len = 0;
    while (s[len] != '\0')
        len++;
    return len;
}

/* void* memset(void* s, int32_t c, uint32_t n);
 * Inputs:    void* s = pointer to memory
 *          int32_t c = value to set memory to
 *         uint32_t n = number of bytes to set
 * Return Value: new string
 * Function: set n consecutive bytes of pointer s to value c */
void* memset(void* s, int32_t c, uint32_t n) {
    c &= 0xFF;
    asm volatile ("                 \n\
            .memset_top:            \n\
            testl   %%ecx, %%ecx    \n\
            jz      .memset_done    \n\
            testl   $0x3, %%edi     \n\
            jz      .memset_aligned \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            subl    $1, %%ecx       \n\
            jmp     .memset_top     \n\
            .memset_aligned:        \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            movl    %%ecx, %%edx    \n\
            shrl    $2, %%ecx       \n\
            andl    $0x3, %%edx     \n\
            cld                     \n\
            rep     stosl           \n\
            .memset_bottom:         \n\
            testl   %%edx, %%edx    \n\
            jz      .memset_done    \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            subl    $1, %%edx       \n\
            jmp     .memset_bottom  \n\
            .memset_done:           \n\
            "
            :
            : "a"(c << 24 | c << 16 | c << 8 | c), "D"(s), "c"(n)
            : "edx", "memory", "cc"
    );
    return s;
}

/* void* memset_word(void* s, int32_t c, uint32_t n);
 * Description: Optimized memset_word
 * Inputs:    void* s = pointer to memory
 *          int32_t c = value to set memory to
 *         uint32_t n = number of bytes to set
 * Return Value: new string
 * Function: set lower 16 bits of n consecutive memory locations of pointer s to value c */
void* memset_word(void* s, int32_t c, uint32_t n) {
    asm volatile ("                 \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            cld                     \n\
            rep     stosw           \n\
            "
            :
            : "a"(c), "D"(s), "c"(n)
            : "edx", "memory", "cc"
    );
    return s;
}

/* void* memset_dword(void* s, int32_t c, uint32_t n);
 * Inputs:    void* s = pointer to memory
 *          int32_t c = value to set memory to
 *         uint32_t n = number of bytes to set
 * Return Value: new string
 * Function: set n consecutive memory locations of pointer s to value c */
void* memset_dword(void* s, int32_t c, uint32_t n) {
    asm volatile ("                 \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            cld                     \n\
            rep     stosl           \n\
            "
            :
            : "a"(c), "D"(s), "c"(n)
            : "edx", "memory", "cc"
    );
    return s;
}

/* void* memcpy(void* dest, const void* src, uint32_t n);
 * Inputs:      void* dest = destination of copy
 *         const void* src = source of copy
 *              uint32_t n = number of byets to copy
 * Return Value: pointer to dest
 * Function: copy n bytes of src to dest */
void* memcpy(void* dest, const void* src, uint32_t n) {
    asm volatile ("                 \n\
            .memcpy_top:            \n\
            testl   %%ecx, %%ecx    \n\
            jz      .memcpy_done    \n\
            testl   $0x3, %%edi     \n\
            jz      .memcpy_aligned \n\
            movb    (%%esi), %%al   \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            addl    $1, %%esi       \n\
            subl    $1, %%ecx       \n\
            jmp     .memcpy_top     \n\
            .memcpy_aligned:        \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            movl    %%ecx, %%edx    \n\
            shrl    $2, %%ecx       \n\
            andl    $0x3, %%edx     \n\
            cld                     \n\
            rep     movsl           \n\
            .memcpy_bottom:         \n\
            testl   %%edx, %%edx    \n\
            jz      .memcpy_done    \n\
            movb    (%%esi), %%al   \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            addl    $1, %%esi       \n\
            subl    $1, %%edx       \n\
            jmp     .memcpy_bottom  \n\
            .memcpy_done:           \n\
            "
            :
            : "S"(src), "D"(dest), "c"(n)
            : "eax", "edx", "memory", "cc"
    );
    return dest;
}

/* void* memmove(void* dest, const void* src, uint32_t n);
 * Description: Optimized memmove (used for overlapping memory areas)
 * Inputs:      void* dest = destination of move
 *         const void* src = source of move
 *              uint32_t n = number of byets to move
 * Return Value: pointer to dest
 * Function: move n bytes of src to dest */
void* memmove(void* dest, const void* src, uint32_t n) {
    asm volatile ("                             \n\
            movw    %%ds, %%dx                  \n\
            movw    %%dx, %%es                  \n\
            cld                                 \n\
            cmp     %%edi, %%esi                \n\
            jae     .memmove_go                 \n\
            leal    -1(%%esi, %%ecx), %%esi     \n\
            leal    -1(%%edi, %%ecx), %%edi     \n\
            std                                 \n\
            .memmove_go:                        \n\
            rep     movsb                       \n\
            "
            :
            : "D"(dest), "S"(src), "c"(n)
            : "edx", "memory", "cc"
    );
    return dest;
}

/* int32_t strncmp(const int8_t* s1, const int8_t* s2, uint32_t n)
 * Inputs: const int8_t* s1 = first string to compare
 *         const int8_t* s2 = second string to compare
 *               uint32_t n = number of bytes to compare
 * Return Value: A zero value indicates that the characters compared
 *               in both strings form the same string.
 *               A value greater than zero indicates that the first
 *               character that does not match has a greater value
 *               in str1 than in str2; And a value less than zero
 *               indicates the opposite.
 * Function: compares string 1 and string 2 for equality */
int32_t strncmp(const int8_t* s1, const int8_t* s2, uint32_t n) {
    int32_t i;
    for (i = 0; i < n; i++) {
        if ((s1[i] != s2[i]) || (s1[i] == '\0') /* || s2[i] == '\0' */) {

            /* The s2[i] == '\0' is unnecessary because of the short-circuit
             * semantics of 'if' expressions in C.  If the first expression
             * (s1[i] != s2[i]) evaluates to false, that is, if s1[i] ==
             * s2[i], then we only need to test either s1[i] or s2[i] for
             * '\0', since we know they are equal. */
            return s1[i] - s2[i];
        }
    }
    return 0;
}

/* int8_t* strcpy(int8_t* dest, const int8_t* src)
 * Inputs:      int8_t* dest = destination string of copy
 *         const int8_t* src = source string of copy
 * Return Value: pointer to dest
 * Function: copy the source string into the destination string */
int8_t* strcpy(int8_t* dest, const int8_t* src) {
    int32_t i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
    return dest;
}

/* int8_t* strcpy(int8_t* dest, const int8_t* src, uint32_t n)
 * Inputs:      int8_t* dest = destination string of copy
 *         const int8_t* src = source string of copy
 *                uint32_t n = number of bytes to copy
 * Return Value: pointer to dest
 * Function: copy n bytes of the source string into the destination string */
int8_t* strncpy(int8_t* dest, const int8_t* src, uint32_t n) {
    int32_t i = 0;
    while (src[i] != '\0' && i < n) {
        dest[i] = src[i];
        i++;
    }
    while (i < n) {
        dest[i] = '\0';
        i++;
    }
    return dest;
}

/* void test_interrupts(void)
 * Inputs: void
 * Return Value: void
 * Function: increments video memory. To be used to test rtc */
void test_interrupts(void) {
    int32_t i;
    for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
        video_mem[i << 1]++;
    }
}
