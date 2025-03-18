#ifndef IDT_H
#define IDT_H

#ifndef ASM

// idt initializer sets the idt bits and raises interrupts and exceptions appropriately 
extern void idt_initializer();

// exception handler prints the exceptions 
extern void exc_handler(int vector);

// exception handler prints the exceptions for the second MACRO 
extern void exc_handler1(int vector);

// created for testing purposes to print interrupts 
extern void int_handler();

#endif /* ASM */

#endif
