#include "i8259.h"
#ifndef KEYBOARD_H
#define KEYBOARD_H
/*Initialize the keyboard*/
void keyboard_init(); 
void keyboard_handler(); 
/* Check whether to capitalize the letter */
void check_capital();
/* Check whether to switch the char of the key for non-letter */
void check_switch();
/* check which function key is pressed ana set up the corresponding flag*/
uint32_t check_function_key(uint32_t keyPress);


#endif 
