#include "lib.h"
#include "paging.h"
#include "terminal.h"
// #include "keyboard.h"


int curr_term = 0;
// store current pid of each terminal
int term_proc[TERM_COUNT] = {-1, -1, -1};
int term_buf_length[TERM_COUNT] = {0, 0, 0};
// set up variables
volatile int enter_flag[3] = {0, 0, 0};
int read_flag = 0;
unsigned char keyboard_buffer[KB_BUFFER_SIZE];
unsigned char input_buffer[TERM_COUNT][KB_BUFFER_SIZE];
extern void flush_tlb();

extern int32_t execute (const uint8_t* command);
extern void setup_mem(uint32_t curr_pid);
extern int check_avai_pid();
extern void store_reg_pid(int curr_pid);
extern void restore_ebp(int curr_pid);
extern void change_tss(int curr_pid);
extern int active_term;


/* int32_t terminal_init()
 * 
 * Initializes terminal structure 
 * Inputs: None
 * Outputs: Sets a terminal structure for each 
 * of the 3 terminals 
 * Files: terminal.h
 */
int32_t terminal_init(){
    int i, j;
    for(i = 0; i < TERM_COUNT; i++){
        terms[i].screen_x = 0;
        terms[i].screen_y = 0;
        terms[i].vidmem = START_VM_T + i * KB_4;
        terms[i].prev_pid = -1;
        terms[i].curr_pid = -1;
        terms[i].buf_idx = 0;
        terms[i].num_char = 0;
        for(j = 0; j < NUM_ROWS; j++){
            terms[i].prev_x[j] = 0;
        }
        for(j = 0; j < KB_BUFFER_SIZE; j++){
            terms[i].key_buffer[j] = NULL;
        }
    }
    return 0;
}


/* int32_t terminal_open (const uint8_t* filename)
 * 
 * Used to open file on terminal 
 * Inputs: filename
 * Outputs: return 0 
 * Files: terminal.h
 */
int32_t terminal_open (const uint8_t* filename){
    if(filename == NULL) return -1;
    return 0;
}

/* int32_t terminal_close (int32_t fd)
 * 
 * (Clear any terminal specific variables) and return 0
 * Inputs: fd
 * Outputs: return 0 
 * Files: terminal.h
 */
int32_t terminal_close (int32_t fd){
    return -1;
}

/* int32_t terminal_read (int32_t fd, void* buf, int32_t nbytes)
 * 
 * Read from the keyboard buffer into buf
 * Inputs: fd - file descriptor
 *         buf - the pointer to the buffer 
 *         nbytes - the max number of keys we want to enter in buffer
 * Outputs: return number of bytes read 
 * Files: terminal.h
 */
int32_t terminal_read (int32_t fd, void* buf, int32_t nbytes){
    int i;
    int32_t byte_read = 0;
    // if(curr_term != active_term)
    // if buffer is null, return -1
    if(buf == NULL) return -1;
    read_flag = 1;
    // wait until pressing enter
    while(!enter_flag[active_term]) {}
    cli();
    read_flag = 0;
    // read buffer
    for(i = 0; i < nbytes && i < KB_BUFFER_SIZE; i++){
        ((char*)buf)[i] = terms[active_term].key_buffer[i];
        byte_read++;
        if(terms[active_term].key_buffer[i] == '\n') break;
        // if(i == nbytes - 1)  ((char*)buf)[i] = '\n';
    }
    enter_flag[active_term] = 0;
    terms[active_term].buf_idx = 0;
    // clear keyboard buffer
    for(i = 0; i < nbytes && i < KB_BUFFER_SIZE; i++){
        terms[active_term].key_buffer[i] = NULL;
    }

    sti(); 
    return byte_read;
    


}

/* int32_t terminal_write (int32_t fd, const void* buf, int32_t nbytes)
 * 
 * Print the characters in the buffer to the console
 * Inputs: fd - file descriptor
 *         buf - the pointer to the buffer 
 *         nbytes - the number of keys we print from the buffer
 * Files: terminal.h
 */
int32_t terminal_write (int32_t fd, const void* buf, int32_t nbytes){
    int i;
    char c;
    int32_t byte_read = 0;
    // if buffer is null, return -1
    if(buf == NULL) return -1;
    // cli();
    // print chars in buffers on the console
    for(i = 0; i < nbytes; i++){
        if(i == nbytes) break;
        c = ((char*)buf)[i];
        if(c != '\0'){
            // print 4 spaces if char is tab
            if(c == '\t'){
                putc(ASCII_SPACE);
                putc(ASCII_SPACE);
                putc(ASCII_SPACE);
                putc(ASCII_SPACE);
            }
            else{
                putc(c);
            }
            byte_read++;
        }
        
    }
    return byte_read; 
}

/* void set_enter_flag(int set_cmd)
 * 
 * set enter flag
 * Inputs: set_cmd - 1 : set enter flag; 0: set enter flag to 0
 * Files: terminal.h
 */
void set_enter_flag(int set_cmd){
    if(set_cmd == 1) enter_flag[curr_term] = 1;
    else enter_flag[curr_term] = 0;
}


/* uint8_t terminal_switch(int term_id)
 * 
 * Helps switch the terminal to the terminal with 
 * ID: term_id 
 * Inputs: ID of the terminal we are switching to 
 * Files: terminal.h
 */
uint8_t terminal_switch(int term_id){
    if(term_id == curr_term) return 0;
    // todo: video mem
    memcpy((uint8_t*) VIDEO, (uint8_t*) (START_VM_T + curr_term * VM_SIZE), NUM_BYTE_COPY);
    //The pages tables used to map the terminal we are switching to, to video on the physical memory 
    page_table[VM_T0 + term_id].pte.addr = VM_INDEX;
    // Used to map the terminal we are switching from to itself in the corresponding physical memory
    page_table[VM_T0 + curr_term].pte.addr = VM_T0 + curr_term;
    //the video on the virtual memory would point to the terminal we are switching to on the 
    //physical memory
    page_table[VM_INDEX].pte.addr = VM_T0 + term_id;
    page_vm_table[term_id].pte.addr = VM_INDEX;
    page_vm_table[curr_term].pte.addr = VM_T0 + curr_term;
    flush_tlb();
    memcpy((uint8_t*) (START_VM_T + term_id * VM_SIZE), (uint8_t*) VIDEO, NUM_BYTE_COPY);
    // store cursor pos for current terminal
    curr_term = term_id;
    // update the position of cursor
    update_cursor_key();
    return 0;

}
