#define ASM     1
#include "sys_call_handler.h"

#define USER_CS     0x0023
#define USER_DS     0x002B

# sys_call_handler()
 # 
 # acts as linkage between the C and x86 code for system calls
 # Inputs: None
 # Outputs: None
 # Side Effects: It is the assmebly linkage, 
 #              calls the system calls
 # Files: interrupts_wrapper.h
 #
.globl sys_call_handler
sys_call_handler:
    pushl   %ecx
    pushl   %ebx
    pushl   %edx
    pushl   %esi
    pushl   %edi
    pushl   %ebp
    pushl   %esp

    pushfl

    pushl   %edx
    pushl   %ecx
    pushl   %ebx

    cmpl    $10, %eax
    jg sys_call_fail
    cmpl    $1, %eax
    jl sys_call_fail
    call *system_call_jump_table(,%eax,4)
    jmp sys_call_success

//jumps to this label if wrong value is passed in eax
//pops the stack and flags and returns 
sys_call_fail:
    movl    $-1, %eax
    addl    $12, %esp
    popfl
    popl    %esp
    popl    %ebp
    popl    %edi
    popl    %esi
    popl    %edx
    popl    %ebx
    popl    %ecx
    iret

//jumps to this label if system call is successful
//pops the stack and flags and returns
sys_call_success:
    addl    $12, %esp
    popfl
    popl    %esp
    popl    %ebp
    popl    %edi
    popl    %esi
    popl    %edx
    popl    %ebx
    popl    %ecx
    iret


system_call_jump_table: 
    .long 0x0, halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn
