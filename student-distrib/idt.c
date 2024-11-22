#include "idt.h"
#include "interrupts_wrapper.h"
#include "x86_desc.h"
#include "lib.h"
#include "keyboard.h"
#include "file_system.h"
#include "sys_call_handler.h"
#include "syscall.h"
#include "pit.h"

// vector numbers for the hardware devices: 
#define SYSTEM_CALL_VEC_NUM     0X80
#define KEYBOARD_VEC_NUM        0X21
#define RTC_VEC_NUM             0X28
#define PIT_VEC_NUM             0x20

// a string array containig the names of all the expections 
static char* exception_strings[] = {
"divide_error", "debug", "nmi", "breakpoint", "overflow", "BOUND_Range_Exceeded", " Invalid_opcode", "Device_Not_Available", 
"double_fault", "Coprocessor_segment_overun", "invalid_tss", "Segment_Not_Present", "stack_fault", 
"general_protection_fault", "page_fault", "RESERVED", "x87_FPU_Floating_Point Error", "alignment_check", 
"machine_check", "SMID_Floating_Point_Error"
}; 


/* void idt_init()
 * 
 * Initializes the interrupt descriptor table
 * Inputs: None 
 * Outputs: None
 * Side Effects: helps to initalize the IDT with sys calls, 
 * exceptions and interrupts  
 * Files:idt.h

 */
void idt_init()
{
    
    int i; 
    // sets the exception status to 0 because no exception 
    exception_status = 0;
    // looping through all the 256 entries of the IDT
    for(i = 0; i < 256; i++)
    {   
        // skipping the 15th entry of the IDT as it is reserved by Intel 
        if(i != 15)
        {    
            // for trap gates, 0b111 according to the osdev 
            idt[i].seg_selector = KERNEL_CS; 
            idt[i].reserved4 = 0;       // always set to 0 
            idt[i].reserved3 = 1;       // 0 for interrupt gates and aet to 1 for trap gates 
            idt[i].reserved2 = 1;       
            idt[i].reserved1 = 1;
            idt[i].size = 1;            // size is always set to 1 
            idt[i].reserved0 = 0;       
            idt[i].dpl = 0;             // set to 0 for kernel as set to 3 for user space 
            idt[i].present = 1;         // initalizing all entries of the table to present 
        }
    }

    
    //THIS IS THE SET UP FOR THE EXCEPTION HANDLERS 
    // populating entries in SET_IDT_ENTRY with the 
    // second parameter representing the exception handler 
    SET_IDT_ENTRY(idt[0], divide_error_exception); 
    SET_IDT_ENTRY(idt[1], debug_error_excepction); 
    SET_IDT_ENTRY(idt[2], nmi_exception); 
    SET_IDT_ENTRY(idt[3], breakpoint_exception);
    SET_IDT_ENTRY(idt[4], overflow_exception);
    SET_IDT_ENTRY(idt[5], bound_exceeded_exception); 
    SET_IDT_ENTRY(idt[6], invalid_opcode_exception); 
    SET_IDT_ENTRY(idt[7], device_unavailable_exception); 
    SET_IDT_ENTRY(idt[8], double_fault_exception); 
    SET_IDT_ENTRY(idt[9], segment_run_exception); 
    SET_IDT_ENTRY(idt[10], invalid_tss_exception); 
    SET_IDT_ENTRY(idt[11], segment_not_present_exception); 
    SET_IDT_ENTRY(idt[12], stack_fault_exception); 
    SET_IDT_ENTRY(idt[13], general_protection_exception); 
    SET_IDT_ENTRY(idt[14], page_fault_exception); 
    SET_IDT_ENTRY(idt[16], FPU_Floating_point_exception); 
    SET_IDT_ENTRY(idt[17], alignment_check_exception); 
    SET_IDT_ENTRY(idt[18], machine_check_exception); 
    SET_IDT_ENTRY(idt[19], smid_floating_point_exception); 

    //THIS IS THE SET UP FOR THE SYSTEM CALLS 
    SET_IDT_ENTRY(idt[SYSTEM_CALL_VEC_NUM], sys_call_handler); 
    idt[SYSTEM_CALL_VEC_NUM].present = 1; 
    idt[SYSTEM_CALL_VEC_NUM].dpl = 3;           // dpl set to 3 for the system calls(user space)

    //THIS IS THE SET UP FOR THE INTERRUPT HANDLERS
   SET_IDT_ENTRY(idt[KEYBOARD_VEC_NUM], KEYBOARD_INTR); 
   idt[KEYBOARD_VEC_NUM].present = 1; 
   idt[KEYBOARD_VEC_NUM].reserved3 = 0;         // for interrupt gates: 0b110 so the reserved3 bit is set to 0 

    SET_IDT_ENTRY(idt[RTC_VEC_NUM], RTC_INTR); 
    idt[RTC_VEC_NUM].present = 1; 
    idt[RTC_VEC_NUM].reserved3 = 0;         //for interrupt gates: 0b110 so the reserved3 bit is set to 0 

    SET_IDT_ENTRY(idt[PIT_VEC_NUM], PIT_INTR); 
    idt[PIT_VEC_NUM].present = 1; 
    idt[PIT_VEC_NUM].reserved3 = 0;  

}



/*void excpHandler(int vecNum)
 * 
 * Exception Handler 
 * Inputs: vector number  
 * Outputs: None
 * Side Effects: prints the name of the exception that is invoked
 * Files:idt.h

 */
void excpHandler(int vecNum)
{  
    // set the exception status to 1 because an exception occured
    exception_status = 1;
    // acknowledges when an exception handler is invoked and prints the exception name
    printf("The exception that was encountered is %s\n", exception_strings[vecNum]); 
    // while(1);
    halt((uint32_t)255); // ?
}


/* void sysCallHandler()
 * 
 * Temporary handler for system calls 
 * Inputs: None 
 * Outputs: None
 * Side Effects: prints that a system call is invoked 
 * Files:idt.h

 */
void sysCallHandler()
{
    // print statement acknowledges when a system call is invoked 
    printf("A system call was called\n");         
    while(1);
}
