#include "keyboard.h"
#include "terminal.h"
#include "i8259.h"
#include "lib.h"

#define KEYBOARD_PORT_DATA      0x60
#define KEYBOARD_PORT_COMMAND   0x64
#define KEYBOARD_IRQ_NUM        1
#define KB_BUFFER_SIZE          128
// keybard map constant variables
#define KEY_SIZE                0x3B
#define ESCAPE                  0x01
#define BACKSPACE               0x0E
#define TAB                     0x0F
#define ENTER                   0x1C
#define LEFT_CTRL               0x1D
#define LEFT_CTRL_REL           0x9D  
#define LEFT_SHIFT              0x2A
#define LEFT_SHIFT_REL          0xAA  
#define RIGHT_SHIFT             0x36
#define RIGHT_SHIFT_REL         0xB6
#define LEFT_ALT                0x38
#define LEFT_ALT_REL            0xB8
#define SPACE                   0x39
#define CAPSLOCK                0x3A
#define CAPSLOCK_REL            0xBA
#define L_PRESS                 0x26
#define C_PRESS                 0x2E
#define UNKNOWN                 0x00
#define KEYPAD                  0x37
#define F1                      0x3B
#define F2                      0x3C
#define F3                      0x3D 
// ascii 
#define ASCII_SPACE             0x20
#define ASCII_BS                0x08   
// boundary of letters in keyboard map
#define LETTER_BOUND1           0x10
#define LETTER_BOUND2           0x19
#define LETTER_BOUND3           0x1E
#define LETTER_BOUND4           0x26
#define LETTER_BOUND5           0x2E
#define LETTER_BOUND6           0x32
// function key flag
static int shift_flag = 0;
static int ctrl_flag = 0;
static int alt_flag = 0;
static int capslock_flag = 0;

static int capital = 0;
// static int num_char = 0;

extern int read_flag;
// extern int buf_idx;
extern int curr_term;
extern unsigned char keyboard_buffer[KB_BUFFER_SIZE];
extern int32_t halt(uint8_t status);

//keyboard map represneting lowercase characters and numbers
// unsigned char keyboardMap[56] = 
// {
//     ' ', ' ', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 
//     '-', '=', ' ', ' ', 'q', 'w', 'e', 'r', 't', 
//     'y', 'u', 'i', 'o', 'p', '[', ']', ' ', ' ', 
//     'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 
//     ';', ' ', '`', ' ', ' ', 'z', 'x', 'c', 
//     'v', 'b', 'n', 'm', ',', '.', '/', ' ', '*'

// }; 
// keyboard representing two modes of each key
unsigned char keyboardMap[KEY_SIZE][2] = 
{
    {0x00, 0x00}, {ESCAPE,ESCAPE}, {'1', '!'}, {'2', '@'}, 
    {'3', '#'}, {'4', '$'}, {'5', '%'}, {'6', '^'},
    {'7', '&'}, {'8', '*'}, {'9', '('}, {'0', ')'},
    {'-', '_'}, {'=', '+'}, {ASCII_BS, ASCII_BS}, {ASCII_SPACE, '\t'},
    {'q', 'Q'}, {'w', 'W'}, {'e', 'E'}, {'r', 'R'},
    {'t', 'T'}, {'y', 'Y'}, {'u', 'U'}, {'i', 'I'},
    {'o', 'O'}, {'p', 'P'}, {'[', '{'}, {']', '}'},
    {'\n', '\n'}, {LEFT_CTRL, LEFT_CTRL}, {'a', 'A'}, {'s', 'S'},
    {'d', 'D'}, {'f', 'F'}, {'g', 'G'}, {'h', 'H'},
    {'j', 'J'}, {'k', 'K'}, {'l', 'L'}, {';', ':'},
    {'\'', '"'}, {'`', '~'}, {LEFT_SHIFT, LEFT_SHIFT}, {'\\', '|'},
    {'z', 'Z'}, {'x', 'X'}, {'c', 'C'}, {'v', 'V'},
    {'b', 'B'}, {'n', 'N'}, {'m', 'M'}, {',', '<'},
    {'.', '>'}, {'/', '?'}, {RIGHT_SHIFT, RIGHT_SHIFT}, {0x00, 0x00},
    {LEFT_ALT, LEFT_ALT}, {ASCII_SPACE, ASCII_SPACE}, {CAPSLOCK, CAPSLOCK}
};





/* void keyboard_init()
 * 
 * Initialize the keyboard 
 * Inputs: None
 * Outputs: None
 * Side Effects: helps initializing the keyboard and enables IRQ 1
 * Files: keyboard.h
 */
void keyboard_init()
{
    //enable IRQ 1 to connect the keyboard to the PIC
    enable_irq(KEYBOARD_IRQ_NUM); 
}


/* void keyboard_handler()
 * 
 * Defines the keyboard handler
 * Inputs: None
 * Outputs: None
 * Side Effects: Reads in the key pressed on the keyboard and 
 * outputs to the screen
 * Files: keyboard.h
 */
void keyboard_handler()
{
    // define variables 
    unsigned char statusKey; 
    uint8_t keyPress; 
    char actualKey;
    int i;
    

    statusKey = inb(KEYBOARD_PORT_COMMAND); 
    if(statusKey & 0x1)
    {
        // get scancode
        keyPress = inb(KEYBOARD_PORT_DATA);
        // do nothing if the pressed key is unknown or in keypad
        if(keyPress == UNKNOWN || keyPress == KEYPAD){
            send_eoi(KEYBOARD_IRQ_NUM);
            return;
        }
        // check function key and set up function flag
        if(check_function_key(keyPress) == 1){
            send_eoi(KEYBOARD_IRQ_NUM); 
            return;
        }
        if(alt_flag == 1 && (keyPress >= F1 && keyPress <= F3)){
            send_eoi(KEYBOARD_IRQ_NUM); 
            if(keyPress == F1) terminal_switch(0);
            if(keyPress == F2) terminal_switch(1);
            if(keyPress == F3) terminal_switch(2);
            return;
        }
        //check if they key pressed is within 0 and 55 to validate the key
        if(keyPress < KEY_SIZE && keyPress > 1 )
        { 
            // clear screen if ctrl + l are pressed  
            if(ctrl_flag == 1 && keyPress == L_PRESS){
                terms[curr_term].num_char = 0;
                if(read_flag == 0) terms[curr_term].buf_idx = 0;
                clear_key();
                reset_pos(0, 0);
                update_cursor_key();
                send_eoi(KEYBOARD_IRQ_NUM);
                return;
            }
            // halt the program
            if(ctrl_flag == 1 && keyPress == C_PRESS){
                send_eoi(KEYBOARD_IRQ_NUM);
                halt(0);
                return;
            }
            // enter is pressed
            else if(keyPress == ENTER){
                actualKey = keyboardMap[keyPress][0];
                putc_keyboard(actualKey);
                // add key in buffer
                if(terms[curr_term].buf_idx < KB_BUFFER_SIZE){
                    terms[curr_term].key_buffer[terms[curr_term].buf_idx] = actualKey; 
                    terms[curr_term].buf_idx++; 
                }
                terms[curr_term].num_char++;
                set_enter_flag(1);
                terms[curr_term].num_char = 0;
            }
            // backspace is pressed
            else if(keyPress == BACKSPACE){
                if(terms[curr_term].num_char > 0){
                    actualKey = keyboardMap[keyPress][0];   
                    // if the preceding key which should be deleted is tab
                    if(terms[curr_term].buf_idx > 0 && keyboard_buffer[terms[curr_term].buf_idx-1] == '\t'){
                        for(i = 0; i < 4; i++) putc_keyboard(actualKey);
                    } 
                    else{
                        putc_keyboard(actualKey); 
                    }
                    // decrement buffer idx
                    terms[curr_term].buf_idx--;
                    terms[curr_term].num_char--;
                }
            }
            // tab is pressed
            else if(keyPress == TAB){
                // tab is 4 spaces
                actualKey = keyboardMap[keyPress][1];
                // print 4 spaces as tab
                for(i = 0; i < 4; i++){
                    putc_keyboard(ASCII_SPACE);
                }
                // inside buffer capacity, increment num_char
                if(terms[curr_term].buf_idx < KB_BUFFER_SIZE - 1){
                    terms[curr_term].key_buffer[terms[curr_term].buf_idx] = actualKey;
                    terms[curr_term].buf_idx++;
                    terms[curr_term].num_char++;
                }
                // exceed the capacity, in order to make the functionality of
                // backspace work, consider tab as 4 chars
                else terms[curr_term].num_char+=4;
            }
            else{
                // pressed key is letters
                if((keyPress >= LETTER_BOUND1 && keyPress <= LETTER_BOUND2) ||
                   (keyPress >= LETTER_BOUND3 && keyPress <= LETTER_BOUND4) ||
                   (keyPress >= LETTER_BOUND5 && keyPress <= LETTER_BOUND6))
                    check_capital();
                // pressed key is not letters
                else check_switch();
                actualKey = keyboardMap[keyPress][capital];
                putc_keyboard(actualKey);
                // add char in buffer
                if(terms[curr_term].buf_idx < KB_BUFFER_SIZE - 1){
                    terms[curr_term].key_buffer[terms[curr_term].buf_idx] = actualKey;
                    terms[curr_term].buf_idx++;
                    
                }
                terms[curr_term].num_char++;
            }          
        } 
    }
    send_eoi(KEYBOARD_IRQ_NUM); 
}

/* void check_capital()
 * 
 * Defines: check whether to capitalize the letter
 * Inputs: None
 * Outputs: None
 * Files: keyboard.h
 */
void check_capital(){
    if(shift_flag == 0 && capslock_flag == 0){
        capital = 0;
        return;
    }
    if(shift_flag == 0 && capslock_flag == 1){
        capital = 1;
        return;
    } 
    if(shift_flag == 1 && capslock_flag == 0){
        capital = 1;
        return;
    } 
    if(shift_flag == 1 && capslock_flag == 1){
        capital = 0;
        return;
    } 
    
}

/* void check_switch()
 * 
 * Defines: check whether to switch the char of the key for non-letter
 * excluding letter characters
 * Inputs: None
 * Outputs: None
 * Files: keyboard.h
 */
void check_switch(){
    if(shift_flag == 0){
        capital = 0;
        return;
    } 
    if(shift_flag == 1){
        capital = 1;
        return;
    } 
}

/* uint32_t check_function_key(uint32_t keyPress)
 * 
 * Defines Check which function key is pressed ana set up the corresponding flag
 * Inputs: keyPress - the key pressed or released
 * Outputs: return 1 if pressed key is 
 * Side Effects: set up the flag of the function key
 * Files: keyboard.h
 */
uint32_t check_function_key(uint32_t keyPress){
    // set up shift flag
    if(keyPress == LEFT_SHIFT || keyPress == RIGHT_SHIFT){
        shift_flag = 1;
        return 1;
    }
    // mask shift flag
    if(keyPress == LEFT_SHIFT_REL || keyPress == RIGHT_SHIFT_REL){
        shift_flag = 0;
        return 1;
    }
    // set up control flag
    if(keyPress == LEFT_CTRL){
        ctrl_flag = 1;
        return 1;
    }
    // mask control flag
    if(keyPress == LEFT_CTRL_REL){
        ctrl_flag = 0;
        return 1;
    }
    // set up alt flag
    if(keyPress == LEFT_ALT){
        alt_flag = 1;
        return 1;
    }
    // mask alt flag
    if(keyPress == LEFT_ALT_REL){
        alt_flag = 0;
        return 1;
    }
    // set up or mask capslock flag
    if(keyPress == CAPSLOCK){
        if(capslock_flag == 0)
            capslock_flag = 1;
        else
            capslock_flag = 0;
        return 1;
    }
    // escape is pressed
    if(keyPress == ESCAPE){
        return 1;
    }

    return 0;
    
}

void reset_terminal(unsigned char func_key){

}


