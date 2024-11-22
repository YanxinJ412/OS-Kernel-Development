#include "pit.h"
#include "i8259.h"
#include "terminal.h"
#include "paging.h"
#include "scheduling.h"
#include "x86_desc.h"
#include "lib.h"
#include "syscall.h"



extern int curr_term; 
int active_term = 0; 
extern int term_proc[TERM_COUNT];
extern int switch_flag;

/* void pit_init()
 * 
 * Initializes the PIT
 * Inputs: None 
 * Outputs: None
 * Side Effects: helps to initalize the PIT that 
 * is set to execute every 10 ms
 * Files:pit.h

 */
void pit_init()
{
    //set the freq initial value for the pit 
    int frequency_value = (INIT_FREQ/FREQ_PIT_NUM) + 1; 

    //send it to the output register 
    outb(PIT_MODE_3, PIT_CMD); 
    outb(frequency_value & LOW_BIT_MASK, PIT_DATA); 
    //send the frequency data value to the PIT_DATA port
    frequency_value = frequency_value >> MASK_PIT_NUM; 
    outb(frequency_value, PIT_DATA); 

    //enable irq pin for the PIT
    enable_irq(PIT_IRQ); 
}


/* void pit_handler()
 * 
 * Initializes the handler for the PIT
 * Inputs: None 
 * Outputs: None  
 * Files:pit.h

 */
void pit_handler()
{
    int old_term = active_term;
    //send the end of interrupt signal 
    send_eoi(PIT_IRQ);
    //0, 1 and 2 represent the terminal numbers 
    if(term_proc[0] == -1 && term_proc[1] == -1 && term_proc[2] == -1) return;
    //initalize a shell if no processes have been initalized on terminal 1
    if(active_term == 0 && term_proc[0] == 0 && term_proc[1] == -1){
        pcb_t*  pcb_val_old; 
        pcb_val_old = (pcb_t*) (MB_8 - (old_term+1)*KB_8);
        active_term++;
        register uint32_t schedule_ebp asm("ebp"); 
        register uint32_t schedule_esp asm("esp"); 
        pcb_val_old->schedule_ebp = schedule_ebp; 
        pcb_val_old->schedule_esp = schedule_esp; 
        execute((uint8_t*)"shell");
        
        return;
    }
     //initalize a shell if no processes have been initalized on terminal 2
    if(active_term == 1 && term_proc[0] == 0 && term_proc[1] != -1 && term_proc[2] == -1){
        pcb_t*  pcb_val_old; 
        pcb_val_old = (pcb_t*) (MB_8 - (old_term+1)*KB_8);
        //increment the terminal numbers for the round robin process
        active_term++;
        //initalize the ebp and esp values for the initial process
        register uint32_t schedule_ebp asm("ebp"); 
        register uint32_t schedule_esp asm("esp"); 
        pcb_val_old->schedule_ebp = schedule_ebp; 
        pcb_val_old->schedule_esp = schedule_esp; 
        execute((uint8_t*)"shell");
        return;
    }

    active_term++;
    //mod by 3 to keep the terminal count from 0-2
    active_term %= 3;
    //calling the context switch function with the old and the new process ID 
    context_switch(term_proc[old_term], term_proc[active_term]);
}

