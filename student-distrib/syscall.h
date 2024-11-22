#ifndef _SYSCALL_H
#define _SYSCALL_H
#include "types.h"
#include "lib.h"
#include "file_system.h"

#define MAX_OPEN_FILES 8
#define PROG_IMAGE_ADDR 0x8048000
#define SIZEOFUINT_32 4
#define FD_START    2
#define EXCEPTION_VALUE 256
#define PID_OFFSET 1
#define PID_MAX 6
#define VIDMAP_ADDR 0x8400000

//struct defined for the file operations table 
typedef struct {
    int32_t(*read)(int32_t fd, void* buf, int32_t nbytes); 
    int32_t(*write)(int32_t fd, const void* buf, int32_t nbytes); 
    int32_t (*open)(const uint8_t* fname); 
    int32_t (*close)(int32_t fd); 
}fop_t;

//struct defined for the file descriptor array 
typedef struct {
    fop_t fop_table; 
    uint32_t inode; 
    uint32_t position_in_file; 
    uint32_t flag; 
}file_desc_table_t; 

// pcb struct
typedef struct {
    uint32_t pid;
    uint32_t parent_pid;
    file_desc_table_t file_desc_arr[8]; 
    uint32_t saved_esp;
    uint32_t saved_ebp;
    uint32_t schedule_esp;
    uint32_t schedule_ebp;
    uint32_t active_flag;
    uint8_t args_name[MAX_FILENAME+1]; 
} pcb_t;

//declared fop variables for each of the type of files
extern pcb_t* pcb_val_read;
fop_t rtc_operations; 
fop_t dir_operations; 
fop_t file_operations; 
fop_t stdin_operations; 
fop_t stdout_operations; 



// call attempts to load and execute a new program
int32_t execute (const uint8_t* command);
int32_t halt(uint8_t status);
void file_desc_arr_initial(pcb_t* pcb_val);
void fop_init_rtc();
void fop_init_dir(); 
void fop_init_file();
void fop_init_terminal();  
// do nothing only use for stdin write and stdout read
int32_t null_work(int32_t fd, void* buf, int32_t nbytes);
// Populates the file descriptor array for the indexes 2 to 8  
int32_t open(const uint8_t* file_name); 
// Closes the specific file in the file descriptor array 
int32_t close(int32_t fd); 
// Populates the buffer with the number of bytes specified  
int32_t read(int32_t fd, void* buf, int32_t nbytes); 
// Writing the buffer to the specific file accessed from the file descriptor array 
int32_t write(int32_t fd, void* buf, int32_t nbytes); 
// maps the text-mode video memory into user space at a pre-set virtual address.
int32_t vidmap(uint8_t** start_screen);
// parse filename and argument in inputs
int32_t parse_cmd(const uint8_t* command, uint8_t* file_name, uint8_t* command_name);
int32_t set_handler(int32_t signum, void* handler_address); 
//populate the given buffer
int32_t getargs(uint8_t* buf, int32_t nbytes); 
int32_t sigreturn(void); 
int check_avai_pid();
void store_reg_pid(int curr_pid);
void restore_ebp(int curr_pid);
void change_tss(int curr_pid);
#endif
 
