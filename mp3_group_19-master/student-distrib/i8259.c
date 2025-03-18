/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

#define num_irq 8 
// referred to osdev
/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* Initialize the 8259 PIC */
/* 
 * 
 *   DESCRIPTION: Intiliazes the 8259 PIC. Saves and sets masks. 
 *                 
 *   INPUTS: none 
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: PIC initialized 
 * 
 */
void i8259_init(void) {
    // uint8_t a1, a2;
 
	// a1 = inb(MASTER_DATA_PORT);                  // save masks
	// a2 = inb(SLAVE_DATA_PORT);

    master_mask = PIC_MASTER_MASK;                  // set masks to 0xff initially (mask all interrupts)
    slave_mask = PIC_SLAVE_MASK;                    // masks will change everytime enable_irq is called

    outb(master_mask, MASTER_DATA_PORT);
    outb(slave_mask, SLAVE_DATA_PORT);
 
	outb(ICW1, MASTER_8259_PORT);                   // starts the initialization sequence (in cascade mode)
	outb(ICW1, SLAVE_8259_PORT);

	outb(ICW2_MASTER, MASTER_DATA_PORT);            // ICW2: Master PIC vector offset
	outb(ICW2_SLAVE, SLAVE_DATA_PORT);              // ICW2: Slave PIC vector offset

	outb(ICW3_MASTER ,MASTER_DATA_PORT);            // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	outb(ICW3_SLAVE ,SLAVE_DATA_PORT);              // ICW3: tell Slave PIC its cascade identity (0000 0010)
 
	outb(ICW4,MASTER_DATA_PORT);                    // ICW4: have the PICs use 8086 mode (and not 8080 mode)
	outb(ICW4,SLAVE_DATA_PORT);

    outb(master_mask, MASTER_DATA_PORT);                   // Remask port
    outb(slave_mask, SLAVE_DATA_PORT);

    enable_irq(SLAVE_IRQ_NUMBER);                   // wire IRQ2 to slave PIC
 
	// outb(a1, MASTER_DATA_PORT);   // restore saved masks.
	// outb(a2, SLAVE_DATA_PORT);
}


/* 
 * 
 *   DESCRIPTION: Enable (unmask) the specified IRQ 
 *                 
 *   INPUTS: irq_num - the irq number  
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: Enables IRQ  
 * 
 */
void enable_irq(uint32_t irq_num) {
    uint16_t port;

    if(irq_num < num_irq) {
        port = MASTER_DATA_PORT;
        master_mask = master_mask & ~(1 << irq_num);
        outb(master_mask, port);
    } else {
        port = SLAVE_DATA_PORT;
        irq_num -= num_irq;
        slave_mask = slave_mask & ~(1 << irq_num);
        outb(slave_mask, port);
    }
    
}

/* 
 * 
 *   DESCRIPTION: Disable (mask) the specified IRQ
 *                 
 *   INPUTS: irq_num - the irq number
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: Disables IRQ  
 * 
 */
void disable_irq(uint32_t irq_num) {
    uint16_t port;
 
    if(irq_num < num_irq) {
        port = MASTER_DATA_PORT;
        master_mask = master_mask | (1 << irq_num);
        outb(master_mask, port);
    } else {
        port = SLAVE_DATA_PORT;
        irq_num -= num_irq;
        slave_mask = slave_mask | (1 << irq_num);
        outb(slave_mask, port);
    }
}

/* 
 * 
 *   DESCRIPTION: Send end-of-interrupt signal for the specified IRQ
 *                 
 *   INPUTS: irq_num - the irq number
 *   OUTPUTS: none
 *   RETURN VALUE: none 
 *   SIDE EFFECTS: Sends eoi 
 * 
 */
void send_eoi(uint32_t irq_num) {
    if(irq_num >= num_irq){
        // sending EOI for slave interrupt
        irq_num -= num_irq;
		outb((EOI | irq_num), SLAVE_8259_PORT);
        outb((EOI | SLAVE_IRQ_NUMBER), MASTER_8259_PORT);
    }
    else{
        // sending EOI for master interrupt
	    outb((EOI | irq_num), MASTER_8259_PORT);
    }
}
