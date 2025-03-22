/* tuxctl-ioctl.c
 *
 * Driver (skeleton) for the mp2 tuxcontrollers for ECE391 at UIUC.
 *
 * Mark Murphy 2006
 * Andrew Ofisher 2007
 * Steve Lumetta 12-13 Sep 2009
 * Puskar Naha 2013
 */

#include <asm/current.h>
#include <asm/uaccess.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/file.h>
#include <linux/miscdevice.h>
#include <linux/kdev_t.h>
#include <linux/tty.h>
#include <linux/spinlock.h>

#include "tuxctl-ld.h"
#include "tuxctl-ioctl.h"
#include "mtcp.h"

#define debug(str, ...) \
	printk(KERN_DEBUG "%s: " str, __FUNCTION__, ## __VA_ARGS__)

#define ONE_BYTE 8
#define LEDS_AMT 4
#define OFFSET_I_ARRAY 2

unsigned button_a;
unsigned button_b;
unsigned button_c;
unsigned char tux_button_packet;
unsigned char led_on_global[6];
unsigned led_busy;

/************************ Protocol Implementation *************************/

/* tuxctl_handle_packet()
 * IMPORTANT : Read the header for tuxctl_ldisc_data_callback() in 
 * tuxctl-ld.c. It calls this function, so all warnings there apply 
 * here as well.
 */
void tuxctl_handle_packet (struct tty_struct* tty, unsigned char* packet)
{
    // unsigned a, b, c;
	unsigned char temp;

    button_a = packet[0]; /* Avoid printk() sign extending the 8-bit */
    button_b = packet[1]; /* values when printing them. */
    button_c = packet[2];
	// printk("B: %d\n", button_b);
	// printk("C: %d\n", button_c);

	if(button_a == MTCP_RESET){
		tuxctl_ioctl_tux_reset(tty);
	}

	// Check for ACK flag
	if(button_a == MTCP_ACK){
		led_busy = 0;
	}

	// Checks if paket[0] == MTCP_BIOC_EVENT
	// Given right down left up --> change to right left down up c b a start using bitwise operations
	if(button_a == MTCP_BIOC_EVENT){
		// printk("Packet\n");
		temp = ((button_c & 0x09) | ((button_c & 0x04) >> 1) | ((button_c & 0x02) << 1));
		// printk("%d\n", temp);
		tux_button_packet = ~((temp << 4) | (button_b & 0x0F));
		// printk("Packet: %d\n", tux_button_packet);
	}

    // printk("packet : %x %x %x\n", button_a, button_b, button_c);
}

/******** IMPORTANT NOTE: READ THIS BEFORE IMPLEMENTING THE IOCTLS ************
 *                                                                            *
 * The ioctls should not spend any time waiting for responses to the commands *
 * they send to the controller. The data is sent over the serial line at      *
 * 9600 BAUD. At this rate, a byte takes approximately 1 millisecond to       *
 * transmit; this means that there will be about 9 milliseconds between       *
 * the time you request that the low-level serial driver send the             *
 * 6-byte SET_LEDS packet and the time the 3-byte ACK packet finishes         *
 * arriving. This is far too long a time for a system call to take. The       *
 * ioctls should return immediately with success if their parameters are      *
 * valid.                                                                     *
 *                                                                            *
 ******************************************************************************/
int 
tuxctl_ioctl (struct tty_struct* tty, struct file* file, 
	      unsigned cmd, unsigned long arg)
{
    switch (cmd) {
	case TUX_INIT:
		return tuxctl_ioctl_tux_initialize(tty);
	case TUX_BUTTONS:
		return tuxctl_ioctl_buttons(arg);
	case TUX_SET_LED:
		return tuxctl_ioctl_tux_set_led(tty, arg);
	case TUX_LED_ACK:
		return -EINVAL;
	case TUX_LED_REQUEST:
		return -EINVAL;
	case TUX_READ_LED:
		return -EINVAL;
	default:
	    return -EINVAL;
    }
}

/* tuxctl_ioctl_tux_initialize()
 * Helper function to initialize tux values (anything related 
 * to the driver). Returns 0. Writes bytes to device using 
 * tuxctl_ldisc_put.
 */
int tuxctl_ioctl_tux_initialize(struct tty_struct* tty){
	unsigned char tux_init_buf[2];
	tux_init_buf[0] = MTCP_BIOC_ON;
	tux_init_buf[1] = MTCP_LED_USR;
	tuxctl_ldisc_put(tty, tux_init_buf, 2); // Write to device
	return 0;
}

/* tuxctl_ioctl_tux_set_led()
 * Helper function to set LED values using "dynamic" array 
 * (what is wrote to device is changed based on how many LEDS are on).
 * Returns 0. Writes bytes to device using tuxctl_ldisc_put.
 */
int tuxctl_ioctl_tux_set_led(struct tty_struct* tty, unsigned long arg){
	// 7 Segment display array
	unsigned char segments_displayed_arr[16];

	// LED Buffer, one is helper and other is for returning
	unsigned char led_buf[6];

	// Initialized values
	int i; // Loop
	int leds_to_be_on;

	unsigned long segment_displays = (arg << (2 * ONE_BYTE)) >> (2 * ONE_BYTE); // Obtain lower 16 bits, shifts 2*8 bits left and then right
	unsigned long leds_turn_on = (arg & 0x000F0000) >> (2 * ONE_BYTE); // Obtain low 4 bits of third byte
	unsigned long decimal_on_off = (arg & 0x0F000000) >> (3 * ONE_BYTE); // Obtain low 4 bits of highest byte
	
	if(led_busy == 1){
		return 0;
	}
	led_busy = 1;

	segments_displayed_arr[0] = 0xE7, // 11100111 --> 0
	segments_displayed_arr[1] = 0x06, // 00000110 --> 1
	segments_displayed_arr[2] = 0xCB, // 11001011 --> 2
	segments_displayed_arr[3] = 0x8F; // 10011111 --> 3
	segments_displayed_arr[4] = 0x2E; // 00111110 --> 4
	segments_displayed_arr[5] = 0xAD; // 10111101 --> 5
	segments_displayed_arr[6] = 0xED; // 11111101 --> 6
	segments_displayed_arr[7] = 0x86; // 10010110 --> 7
	segments_displayed_arr[8] = 0xEF; // 11111111 --> 8
	segments_displayed_arr[9] = 0xAE; // 10111110 --> 9

	// In case of full hexadecimal, account for A-F as well
	segments_displayed_arr[10] = 0xEE; // 11101110 --> A
	segments_displayed_arr[11] = 0x6D; // 01101101 --> B (lowercase)
	segments_displayed_arr[12] = 0xE1; // 11100001 --> C
	segments_displayed_arr[13] = 0x4F; // 01001111 --> D (lowercase)
	segments_displayed_arr[14] = 0xE9; // 11101001 --> E
	segments_displayed_arr[15] = 0xE8; // 11101000 --> F


	// Op Code for Setting LEDs
	led_buf[0] = MTCP_LED_SET;

	// // LEDs that should be on
	led_buf[1] = 0x0F;


	// For each of the 4 LEDs, get values. OFFSET_I_ARRAY ensures led_buf[2] to led_buf[5] are filled
	for(i = 0; i < LEDS_AMT; i++){
		led_buf[i + OFFSET_I_ARRAY] = (segment_displays >> (i * LEDS_AMT)) & 0x0F; // Reverses LEDs order as well
	}

	leds_to_be_on = LEDS_AMT;
	// Checks if needs decimal display, makes displays onto tux
	for(i = 0; i < LEDS_AMT; i++){
		if(((leds_turn_on >> i) & 0x01) == 1){
			if((decimal_on_off >> i) & 0x01){ // Checks proper LED decimal point as is reversed already
				led_buf[i + OFFSET_I_ARRAY] = segments_displayed_arr[led_buf[i + OFFSET_I_ARRAY]] | 0x10; // Enables dp (5th bit)
			}
			else{
				led_buf[i + OFFSET_I_ARRAY] = segments_displayed_arr[led_buf[i + OFFSET_I_ARRAY]]; // Regular value (no dp)
			}
		}
		else{
			led_buf[i + OFFSET_I_ARRAY] = 0x00; // Set to off
		}
	}

	for(i = 0; i < LEDS_AMT + OFFSET_I_ARRAY; i++){
		led_on_global[i] = led_buf[i];
	}
	// Writes bytes to device
	tuxctl_ldisc_put(tty, led_buf, (OFFSET_I_ARRAY + leds_to_be_on)); // 4 bytes for 32 bits (LED) (dynamic), 2 bytes before for set led and which leds on
	return 0;
}


/*
 * tuxctl_ioctl_buttons()
 * Helper function to copy to user space the button values in
 * a given format of (7) right left down up c b a start (0).
 * Returns -EINVAL if pointer isn't valid, else returns 0. 
 */
int tuxctl_ioctl_buttons(unsigned long arg){
	// void *p;
	// unsigned long temp;
	int temp_pass;
	if(arg == 0){
		return -EINVAL;	
	}
	else{
		// p = &arg;
		// temp = p & (unsigned long)tux_button_packet;
		temp_pass = 0;
		temp_pass += tux_button_packet;
		copy_to_user((int*)arg, &temp_pass, 4); // 8 bits in 1 byte
		return 0;
	}
}

int tuxctl_ioctl_tux_reset(struct tty_struct* tty){
	unsigned char tux_init_buf[2];
	tux_init_buf[0] = MTCP_BIOC_ON;
	tux_init_buf[1] = MTCP_LED_USR;
	tuxctl_ldisc_put(tty, tux_init_buf, 2); // Write to device

	if(led_busy == 0){
		led_busy = 1;
		tuxctl_ldisc_put(tty, led_on_global, (LEDS_AMT + OFFSET_I_ARRAY)); // 4 bytes for 32 bits (LED) (dynamic), 2 bytes before for set led and which leds on
	}
	return 0;
}
