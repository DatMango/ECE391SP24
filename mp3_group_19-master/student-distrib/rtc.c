#include "rtc.h"
#include "syscall_modifier.h"

#define FREQUENCY           1024
#define DEFAULT_FREQUENCY   2
#define FOURBYTEINTEGER     4
#define LIMIT2TO15          0x0F
#define BITS4TO8            0xF0
#define BIT6                0x40
#define GIVENNUMFROMRTCEQN  32768
#define LOWER_BOUND_RATE    2
#define UPPER_BOUND_RATE    15

int counter[3] = {1, 1, 1}; 
int counter2[3] = {1, 1, 1}; // Clone of regular counter
int interrupt_flag[3] = {1, 1, 1};
int opened_rtcs[3] = {0, 0, 0};
/* initializes rtc */
// referred to osdev
/* 
 * 
 *   DESCRIPTION: Initializes rtc. Here we enable and disable interrupts to be safe 
 *                  and call the change freq rate function and also enable the IRQ 
 *                  for rtc 
 *   INPUTS: none 
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: enables IRQ for rtc
 * 
 */
void init_rtc(int freq){
    /* enabling IRQ 8*/
    char prev;
    char rate = 3;
    disable_int();                      // disable interrupts
    outb(STATUS_REG_B, REG_NUM_INDEX);  // select reg b, disable nmi
    prev = inb(CMOS_CONFIG);            // read the current value of register b
    outb(STATUS_REG_B, REG_NUM_INDEX);  // set index again (read resets index to reg d)
    outb(prev | BIT6, CMOS_CONFIG);     // write the previous value ORed with 0x40. This turns on bit 6 of register B
    
    // int temp;
    // temp = change_rtc_rate(freq);                 // Will change later in cp 2 or 5 

    outb(STATUS_REG_A, REG_NUM_INDEX);
    prev = inb(CMOS_CONFIG);
    outb(STATUS_REG_A, REG_NUM_INDEX);
    outb(((prev * BITS4TO8) | rate), CMOS_CONFIG);

    // // Re-enable NMI? --> OSDEV: outb(0x70, inb(0x70) & 0x7F);
    outb(inb(0x70) & 0x7F, 0x70);

    outb(STATUS_REG_C, REG_NUM_INDEX);     // Register C values

    enable_irq(IRQ_8);                     // rtc interrupt
    enable_int();                          // enables interrupts
}   

/* 
 * 
 *   DESCRIPTION: This is the handler for when interrupts are called. 
 *                  Increments a counter to show the interrupt at the frequency 
 *   INPUTS: none 
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: enables IRQ for rtc
 * 
 */
void rtc_handler(){

    // counter++;
    // int one = 1; 
    // printf("%d", one); // To test RTC interrupts using our own version
    //test_interrupts(); // To test interrupts using test case provided
    int i = 0;
    outb(STATUS_REG_C, REG_NUM_INDEX);
    inb(CMOS_CONFIG);
    //send eoi 
    send_eoi(IRQ_8); 
    
    for(i = 0; i < 3; i++){
        counter[i] -= 1;
        if(counter[i] == 0){
            counter[i] = 1;
            interrupt_flag[i] = 0;
        }
    }
}

/* 
 * 
 *   DESCRIPTION: This function sets the rtc rate and ensures
 *                  that it does not go below 2 or above 15 
 *   INPUTS: none 
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: sets frequency rate 
 * 
 */
int32_t change_rtc_rate(int freq){
    // calculate freq: frequency =  32768 >> (rate-1);
    // 1000000000000000 <-- binary of 32768
    int rate = 1;
    while(freq < (GIVENNUMFROMRTCEQN >> (rate-1))){
        rate++;
    }

    // Hard set rate if it is less than 2 because of rate bounds
    if(rate <= LOWER_BOUND_RATE){
        // rate = 3; 
        return -1;
    }
    else if(rate > UPPER_BOUND_RATE){
        // rate = 15;
        return -1;
    }

    // rate = 2;               // For testing purposes
    rate &= LIMIT2TO15;			// rate must be above 2 and not over 15
    char prev;
    disable_int();          // disable interrupts
    outb(STATUS_REG_A, REG_NUM_INDEX);
    prev = inb(CMOS_CONFIG);
    outb(STATUS_REG_A, REG_NUM_INDEX);
    outb(((prev * BITS4TO8) | rate), CMOS_CONFIG);
    enable_int();           // enables interrupts
    return 0;
}

/* 
 * 
 *   DESCRIPTION: This function enables interrupts 
 *                  and restores flags 
 *   INPUTS: none 
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: enables interrupts and restores flags  
 * 
 */
void enable_int(){
    restore_flags(flags);
}

/* 
 * 
 *   DESCRIPTION: This function disables interrupts 
 *                  and saves flags 
 *   INPUTS: none 
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: disables interrupts and saves flags  
 * 
 */
void disable_int(){
    cli_and_save(flags);
}


// RTC Drivers
/* 
 * 
 *   DESCRIPTION: This function resets the frequency to 2Hz.
 *   INPUTS: filename (not used)
 *   OUTPUTS: none
 *   RETURN VALUE: 0 upon success
 *   SIDE EFFECTS: Changes the RTC frequency to 2Hz. 
 * 
 */
int32_t rtc_open(const uint8_t* filename){
    int curr_term = get_curr_terminal(); // Obtain current terminal for setting rtc values
    opened_rtcs[curr_term] = 1; // Set 1 for Opened RTCs (terminal specific)
    // Set counters
    counter[curr_term] = FREQUENCY / DEFAULT_FREQUENCY;
    counter2[curr_term] = FREQUENCY / DEFAULT_FREQUENCY;

    // temp = change_rtc_rate(DEFAULT_FREQUENCY);
    return 0;
}

/* 
 * 
 *   DESCRIPTION: This function returns only after an RTC
 *                interrupt has occurred.
 *   INPUTS: fd - file descriptor (not used)
 *           buf - buffer with information (not used)
 *           nbytes - number of bytes in the buffer (not used)
 *   OUTPUTS: none
 *   RETURN VALUE: 0 upon success 
 *   SIDE EFFECTS: Sets interrupt flag and only returns after
 *                 an interrupt has occurred.  
 * 
 */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes){
    int curr_term = get_curr_terminal(); // Obtain current terminal
    if(opened_rtcs[curr_term] == 1){
        interrupt_flag[curr_term] = 1; // Update IF if opened
    }
    // counter = 1;
    
    sti();
    while(interrupt_flag){
        // Nothing
    }
    cli();

    return 0;
}

/* 
 * 
 *   DESCRIPTION: This function changes the RTC rate to whatever
 *                is inputted and makes sure the rate is a power
 *                of 2. Also checks if the given value in buffer
 *                is 4 bytes from nbytes.
 *   INPUTS: fd - file descriptor (not used)
 *           buf - buffer in which rate will be based through
 *           nbytes - number of bytes written into the buffer
 *   OUTPUTS: none
 *   RETURN VALUE: 0 upon success, -1 upon failure
 *   SIDE EFFECTS: changes the rtc rate and returns 0 or -1 
 *                 based on if it was changed to that rate  
 * 
 */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes){
    int32_t freq;
    // null check 
    if(buf == NULL){
        return -1;
    }

    // If not given 4 bytes, don't accept
    if(nbytes != FOURBYTEINTEGER){
        return -1;
    }

    freq = *((int32_t *) buf);
    // Check if frequency is a power of 2
    if((freq & (freq - 1)) != 0){
        return -1;
    }

    // Altered change_rtc_rate to return 0 or -1 based on if within range
    return change_rtc_rate(freq);
}


/* 
 * 
 *   DESCRIPTION: This function closes rtc
 *   INPUTS: fd - file descriptor (not used)
 *   OUTPUTS: none
 *   RETURN VALUE: 0 upon success
 *   SIDE EFFECTS: none  
 * 
 */
int32_t rtc_close(int32_t fd){
    opened_rtcs[get_active_terminal()] = 0; // Close the current opened rtc
    return 0;
}
