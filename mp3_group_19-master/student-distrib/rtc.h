#include "lib.h"
#include "i8259.h"
#include "types.h"

/* defining constants for data */
#define REG_NUM_INDEX       0x70        // disables NMI
#define CMOS_CONFIG         0x71
#define DV1_ON              0x20

/* defining constants for ports */
#define STATUS_REG_A        0x8A
#define STATUS_REG_B        0x8B
#define STATUS_REG_C        0x0C

#define IRQ_8               8

/* defining constants for rtc rates */
#define MAX_RATE            15
#define MIN_RATE            3

uint32_t flags; // Used to save flags

/* initializing rtc*/
extern void init_rtc(int freq);

/* Changing rtc rate function */
extern int32_t change_rtc_rate(int freq);

/* Handler for RTC (for interrupts)*/
void rtc_handler();

/* Enables interrupts */
void enable_int();

/* Disables interrupts */
void disable_int();


// RTC Drivers
/* RTC Open driver, sets to 2Hz */
int32_t rtc_open(const uint8_t* filename);

/* Reads and only returns after an RTC interrupt occurs */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);

/* Changes the RTC rate based upon based through buffer */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);

/* Closing RTC driver */
int32_t rtc_close(int32_t fd);
