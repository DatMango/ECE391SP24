// All necessary declarations for the Tux Controller driver must be in this file

#ifndef TUXCTL_H
#define TUXCTL_H

#define TUX_SET_LED _IOR('E', 0x10, unsigned long)
#define TUX_READ_LED _IOW('E', 0x11, unsigned long*)
#define TUX_BUTTONS _IOW('E', 0x12, unsigned long*)
#define TUX_INIT _IO('E', 0x13)
#define TUX_LED_REQUEST _IO('E', 0x14)
#define TUX_LED_ACK _IO('E', 0x15)


/* tuxctl_ioctl_tux_initialize()
 * Helper function to initialize tux values (anything related 
 * to the driver). Returns 0. Writes bytes to device using 
 * tuxctl_ldisc_put.
 */
int tuxctl_ioctl_tux_initialize(struct tty_struct* tty);

/* tuxctl_ioctl_tux_set_led()
 * Helper function to set LED values using "dynamic" array 
 * (what is wrote to device is changed based on how many LEDS are on).
 * Returns 0. Writes bytes to device using tuxctl_ldisc_put.
 */
int tuxctl_ioctl_tux_set_led(struct tty_struct* tty, unsigned long arg);

/*
 * tuxctl_ioctl_buttons()
 * Helper function to copy to user space the button values.
 * Returns -EINVAL if pointer isn't valid, else returns 0. 
 */
int tuxctl_ioctl_buttons(unsigned long arg);

int tuxctl_ioctl_tux_reset(struct tty_struct* tty);

#endif

