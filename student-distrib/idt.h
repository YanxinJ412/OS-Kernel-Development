#include "x86_desc.h"
#include "lib.h"
#ifndef IDT_H
#define IDT_H

// hold the status of exceptions
uint32_t exception_status;
/*initializes the IDT*/
void idt_init(); 
/*Acts as the exception handler, called from the linkage */
void excpHandler(int vecNum); 
/*Acts as the system call handler, called from the linkage */
void sysCallHandler(); 

#endif /* IDT_H */
