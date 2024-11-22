 /* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask = 0xFF; /* IRQs 0-7  */
uint8_t slave_mask = 0xFF;  /* IRQs 8-15 */
#define MASTER_8259_DATA   (MASTER_8259_PORT+1)
#define SLAVE_8259_DATA    (SLAVE_8259_PORT+1)
#define SLAVE_IRQ_NUM       2 

/* Initialize the 8259 PIC */

/* void i8259_init(void)
 * 
 * Initializes the PIC 
 * Inputs: None
 * Outputs: None
 * Side Effects: helps initializing the PIC and enables the SLAVE IRQ on pin 2 
 * Files: i8259.h
 */
void i8259_init(void) {
    
    // masks all the irq pins for both the Master and Slave data ports
    // using 0xFF
    outb(0xFF, MASTER_8259_DATA);          
    outb(0xFF, SLAVE_8259_DATA); 

    //initalizes the 8259 master PIC
    outb(ICW1, MASTER_8259_PORT);
     //initalizes the 8259 slave PIC 
    outb(ICW1, SLAVE_8259_PORT);

    // sets the vector offset for the master and slave PICS
    outb(ICW2_MASTER, MASTER_8259_DATA); 
    outb(ICW2_SLAVE, SLAVE_8259_DATA); 

    // tell the masrer PIC that there is a slave PIC at IRQ2
    outb(ICW3_MASTER, MASTER_8259_DATA); 
    //tells the slave PIC its cascade identity 
    outb(ICW3_SLAVE, SLAVE_8259_DATA); 

    //provides additional information about the environment 
    outb(ICW4, MASTER_8259_DATA); 
    outb(ICW4, SLAVE_8259_DATA); 

    //restores the masks for the master and slave PICs
    outb(master_mask, MASTER_8259_DATA); 
    outb(slave_mask, SLAVE_8259_DATA); 

    // unmasks the irq pin 2 where the slave PIC is connected to the master PIC
    enable_irq(SLAVE_IRQ_NUM); 
    
}


/* void enable_irq(uint32_t irq_num)
 * 
 * Enable (unmask) the specified IRQ
 * Inputs: irq number 
 * Outputs: None
 * Side Effects: helps to unmask the specified irq pin number 
 * to be able to send interrupts through  
 * Files: i8259.h
 */
void enable_irq(uint32_t irq_num) {
    
    // if the irq number is less than 8, then we are enabling 
    // an IRQ on the master PIC
    if(irq_num < 8)
    {
        master_mask = master_mask & ~(1 << irq_num); 
        // updating the master mask to the data port 
        outb(master_mask, MASTER_8259_DATA); 
    }
    // if the irq number is greater than 8, then we are enabling 
    // an IRQ on the slave PIC
    else
    {
        irq_num -= 8; 
        slave_mask = slave_mask & ~(1 << irq_num); 
        //updating the slave mask to the data port 
        outb(slave_mask, SLAVE_8259_DATA); 
    }
}

/* void disable_irq(uint32_t irq_num)
 * 
 * Disable (mask) the specified IRQ
 * Inputs: irq number
 * Outputs: None
 * Side Effects: helps to mask the specified irq pin number 
 * to not be able to send interrupts through  
 * Files: i8259.h
 */
void disable_irq(uint32_t irq_num) {

    // if the irq number is less than 8, then we are enabling 
    // an IRQ on the master PIC
     if(irq_num < 8)
    {
        master_mask = master_mask | (1 << irq_num); 
        // updating the master mask to the data port 
        outb(master_mask, MASTER_8259_DATA); 
    }
      // if the irq number is greater than 8, then we are enabling 
    // an IRQ on the slave PIC
    else
    {
        irq_num -= 8; 
        slave_mask = slave_mask | (1 << irq_num); 
        // updating the slave mask to the data port 
        outb(slave_mask, SLAVE_8259_DATA); 
    }
}


/* void send_eoi(uint32_t irq_num)
 * 
 * Send end-of-interrupt signal for the specified IRQ
 * Inputs: irq number
 * Outputs: None
 * Side Effects: Sends the end of interrupt signal 
 * which represents the end of the interrupt sent through an IRQ pin 
 * to not be able to send interrupts through  
 * Files: i8259.h
 */
void send_eoi(uint32_t irq_num) {
    int slaveIrqNum; 
    // if the irq number is greater than 8, then we are enabling 
    // an IRQ on the slave PIC
    if(irq_num >= 8)
    {   
        slaveIrqNum = irq_num - 8; 
        //This gets OR'd with the interrupt number and sent out to the PIC
         //to declare the interrupt finished
        outb(EOI|slaveIrqNum, SLAVE_8259_PORT); 
        outb(EOI | 2, MASTER_8259_PORT); 
    }
    else 
    {
        outb(EOI | irq_num, MASTER_8259_PORT); 
    }
    
}
