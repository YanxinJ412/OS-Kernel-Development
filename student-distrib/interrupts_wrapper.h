
#ifndef ASM

#include "keyboard.h"
#include "pit.h"

/*Function definitions of the handlers for the exception, system calls,
keyboard and rtc interrupts*/
void excpHandler(int vecNum); 
void sysCallHandler();  
void KEYBOARD_INTR();
void RTC_INTR();
void PIT_INTR(); 
//void SysCall();

/*Function definitions of the exception handlers*/
void divide_error_exception(); 
void debug_error_excepction(); 
void nmi_exception(); 
void breakpoint_exception(); 
void overflow_exception();
void bound_exceeded_exception(); 
void invalid_opcode_exception(); 
void device_unavailable_exception(); 
void double_fault_exception(); 
void segment_run_exception(); 
void invalid_tss_exception(); 
void segment_not_present_exception(); 
void stack_fault_exception(); 
void general_protection_exception(); 
void page_fault_exception(); 
void FPU_Floating_point_exception(); 
void alignment_check_exception(); 
void machine_check_exception(); 
void smid_floating_point_exception(); 
#endif 
