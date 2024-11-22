#include "x86_desc.h"
#include "syscall.h"
#include "paging.h"
#include "pit.h"
#include "terminal.h"
#include "scheduling.h"

extern int active_term;
extern int curr_term;
extern void flush_tlb();


/* void context_switch(int switch_from_pid, int switch_to_pid)
 * 
 * Helps to context switch between two active processes 
 * given their process IDs
 * Inputs: The process IDs of the two processes 
 * during context switch 
 * Outputs: None
 * Files:pit.h

 */
void context_switch(int switch_from_pid, int switch_to_pid)
{

    pcb_t*  pcb_val_old; 
    pcb_val_old = (pcb_t*) (MB_8 - (switch_from_pid+1)*KB_8);

    //saving the old process's esp and ebp 
    register uint32_t schedule_ebp asm("ebp"); 
    register uint32_t schedule_esp asm("esp"); 
    pcb_val_old->schedule_ebp = schedule_ebp; 
    pcb_val_old->schedule_esp = schedule_esp; 
    
    //changing the program image address to point to new process
    page_directory[USER_LEVEL_INDEX].pde_4mb.addr = (uint32_t) (((2 + switch_to_pid) * MB_4) >> ADDR_SHIFT_4MB);
    flush_tlb(); 

    //getting the pointer to the new process 
    pcb_t* pcb_val_new; 
    pcb_val_new = (pcb_t*) (MB_8 - (switch_to_pid+1)*KB_8);

    //switching kernel stack to new stack 
    tss.ss0 = KERNEL_DS; 
    tss.esp0 = (MB_8 - (switch_to_pid)*KB_8) - SIZEOFUINT_32; 

    
    //restoring the ebp and esp values of the new process switching into

    asm volatile ("                                                     \n\
        mov  %1, %%ebp                                                  \n\
        mov  %0, %%esp                                                  \n\
        leave                                                           \n\
        ret                                                             \n\
        "
        :
        : "r"(pcb_val_new->schedule_esp), "r"(pcb_val_new->schedule_ebp)
        : "memory"
    );

    return; 

}

