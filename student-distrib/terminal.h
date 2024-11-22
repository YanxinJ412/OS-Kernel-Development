#define KB_BUFFER_SIZE          128
#define TERM_COUNT              3
#define NUM_ROWS                25
#define NUM_BYTE_COPY           0x1000
#define START_VM_T              0xb9000
#define VIDEO                   0xB8000
#define VM_SIZE                 0x1000


/* terminal struct*/
typedef struct terminal{
    int screen_x;
    int screen_y;
    int vidmem;
    int prev_pid;
    int curr_pid;
    int prev_x[NUM_ROWS];
    int buf_idx;
    int num_char;
    unsigned char key_buffer[KB_BUFFER_SIZE];
} terminal_t;
terminal_t terms[TERM_COUNT];

// initialize terminal
int32_t terminal_init();
// Initialize terminal stuff and return 0
int32_t terminal_open (const uint8_t* filename);
// (Clear any terminal specific variables) and return 0
int32_t terminal_close (int32_t fd);
// Read from the keyboard buffer into buf
int32_t terminal_read (int32_t fd, void* buf, int32_t nbytes);
// Print the characters in the buffer to the console
int32_t terminal_write (int32_t fd, const void* buf, int32_t nbytes);
// Set enter flag
void set_enter_flag(int set_cmd);
// switch terminal
uint8_t terminal_switch(int term_id);



