#define ASM     1
#include "interrupts_wrapper.h"
#include "x86_desc.h"

# Initialize the 8259 PIC */

# EXCEPTION(name, index)
 # 
 # acts as linkage between the C and x86 code 
 # Inputs: None
 # Outputs: None
 # Side Effects: It is the assmebly linkage, 
 #              calls the exception handler
 # Files: interrupts_wrapper.h
 #
#define EXCEPTION(name, index)  \
.globl name                     ;\
name:                           ;\
    pushal                      ;\
    pushfl                      ;\
    cli                         ;\
    pushl   $index              ;\
    call    excpHandler         ;\
    addl    $4, %esp            ;\
    popfl                       ;\
    popal                       ;\
    iret            

# KEYBOARD_INTR                                 #ASK HOW TO?
 # 
 # acts as linkage between the C and x86 code 
 # Inputs: None
 # Outputs: None
 # Side Effects: It is the assmebly linkage, 
 #              calls the keyboard handler
 # Files: interrupts_wrapper.h
 #
.globl KEYBOARD_INTR
KEYBOARD_INTR:
    pushal                           ;\
    pushfl                           ;\
    call    keyboard_handler         ;\
    popfl                            ;\
    popal                            ;\
    iret           


# RTC_INTR
 # 
 # acts as linkage between the C and x86 code 
 # Inputs: None
 # Outputs: None
 # Side Effects: It is the assmebly linkage, 
 #              calls the rtc handler
 # Files: interrupts_wrapper.h
 #
.globl RTC_INTR
RTC_INTR:
    pushal                           ;\
    pushfl                           ;\
    call    rtc_handler              ;\
    popfl                            ;\
    popal                            ;\
    iret           

# PIT_INTR
 # 
 # acts as linkage between the C and x86 code 
 # Inputs: None
 # Outputs: None
 # Side Effects: It is the assmebly linkage, 
 #              calls the rtc handler
 # Files: interrupts_wrapper.h
 #
.globl PIT_INTR
PIT_INTR:
    pushal                           ;\
    pushfl                           ;\
    call    pit_handler              ;\
    popfl                            ;\
    popal                            ;\
    iret    


# SysCall
 # 
 # acts as linkage between the C and x86 code 
 # Inputs: None
 # Outputs: None
 # Side Effects: It is the assmebly linkage, 
 #              calls the system call handler
 # Files: interrupts_wrapper.h
 #
# .globl SysCall
# SysCall:                        ;\
    #pushal                      ;\
    #pushfl                      ;\
    #call    sysCallHandler      ;\
    #popfl                       ;\
    #popal                       ;\
    #iret


# links the exception names to their respective vector numbers 
EXCEPTION(divide_error_exception, 0x00); 
EXCEPTION(debug_error_excepction, 0x01); 
EXCEPTION(nmi_exception,  0x02); 
EXCEPTION(breakpoint_exception, 0x03);
EXCEPTION(overflow_exception, 0x04);
EXCEPTION(bound_exceeded_exception, 0x05); 
EXCEPTION(invalid_opcode_exception,  0x06); 
EXCEPTION(device_unavailable_exception, 0x07); 
EXCEPTION(double_fault_exception, 0x08); 
EXCEPTION(segment_run_exception, 0x09); 
EXCEPTION(invalid_tss_exception, 0x0A); 
EXCEPTION(segment_not_present_exception, 0x0B); 
EXCEPTION(stack_fault_exception, 0x0C); 
EXCEPTION(general_protection_exception, 0x0D); 
EXCEPTION(page_fault_exception, 0x0E); 
EXCEPTION(FPU_Floating_point_exception, 0x10); 
EXCEPTION(alignment_check_exception, 0x11); 
EXCEPTION(machine_check_exception, 0x12); 
EXCEPTION(smid_floating_point_exception, 0x13); 


