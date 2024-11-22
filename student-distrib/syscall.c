#include "syscall.h"
#include "rtc.h"
#include "terminal.h"
#include "file_system.h"
#include "paging.h"
#include "idt.h"
#include "x86_desc.h"
#include "sys_call_handler.h"
#include "lib.h"

//pcb_t* pcb_val;
int vidmap_flag = 0; 
pcb_t* pcb_val_read;
// pid array to check active, only have 6 process in this cp
uint32_t pid[PID_MAX] = {0, 0, 0, 0, 0, 0};
// init current and previous pid
uint32_t curr_pid = -1;
uint32_t prev_pid = -1;
extern void setup_mem(uint32_t curr_pid);
extern void reset_vidmap();
extern void flush_tlb();

extern int term_proc[TERM_COUNT];
extern int curr_term;
extern int active_term;

/* int32_t execute (const uint8_t* command)
 * 
 * call attempts to load and execute a new program
 * Inputs: command - include filename and argument
 * Outputs: -1 - command cannot be executed
 *          256 - program dies by an exception
 *          0-255 - that given by the program’s call to halt.
 * Files: syscall.h, sys_call_handler.h
 */
int32_t execute (const uint8_t* command){
    // init variables
    uint8_t filename[MAX_FILENAME+1];
    // 3 is the maximum number of argument 
    // 128 is the max buffer to store each argument
    uint8_t command_name[MAX_FILENAME];
    int num_args;
    dentry_t dentry;
    // buffer to store 4 ELF magic num
    uint8_t buf[4];
    // store 4 byte eip
    uint8_t eip_buf[4];
    uint32_t prog_eip;
    uint32_t user_esp;
    pcb_t* pcb;
    int i;
    // parse cmd
    if(command == NULL) return -1;
    num_args = parse_cmd(command, filename, command_name);
    if(num_args == -1) return -1; 

    // file checks
    if(read_dentry_by_name((int8_t*)command_name, &dentry) == -1) 
        return -1;
    if(read_data(dentry.inode_idx, 0, buf, 4) == -1)        // reading 4 bytes
        return -1;
    // the numbers below are magic number. If not present, the execute system call call
    // should fail
    if(!(buf[0] == 0x7f && buf[1] == 0x45 && buf[2] == 0x4c && buf[3] == 0x46))
        return -1; 

    // create new PCB
    // check available pcb, only two process used in this cp
    for(i = 0; i < PID_MAX; i++){
        if(pid[i] == 0){
            pid[i] = 1;
            terms[active_term].prev_pid = terms[active_term].curr_pid;
            terms[active_term].curr_pid = i;
            // prev_pid = curr_pid;
            // curr_pid = i;
            
            break;
        }
    }
    if(i >= PID_MAX){
        return -1;
    }
    // give pcb memory and set up new pcb
    pcb = (pcb_t*) (MB_8 - (terms[active_term].curr_pid+PID_OFFSET)*KB_8);
    pcb->pid = terms[active_term].curr_pid;
    pcb->parent_pid = terms[active_term].prev_pid;
    // set active
    pcb->active_flag = 1;
    file_desc_arr_initial(pcb);
    strncpy((int8_t*)pcb->args_name, (int8_t*)(filename), MAX_FILENAME+1);
    // setup memory(paging)
    setup_mem(terms[active_term].curr_pid);
    // read exe data
    read_data(dentry.inode_idx, 0, (uint8_t*) PROG_IMAGE_ADDR, inode_addr[dentry.inode_idx].file_length);
    // setup old stack & eip
    // get eip from bytes 24-27 of the executable just loaded (4 byte)
    read_data(dentry.inode_idx, 24, eip_buf, 4);
    prog_eip = *((uint32_t*) eip_buf);
    user_esp = USER_LEVEL_PROG + MB_4 - SIZEOFUINT_32;
    // store saved_ebp and saved_esp
    register uint32_t saved_ebp asm("ebp");
    register uint32_t saved_esp asm("esp");
    pcb->saved_ebp = saved_ebp;
    pcb->saved_esp = saved_esp;
    // goto usermode
    tss.ss0 = KERNEL_DS;
    tss.esp0 = (MB_8 - (terms[active_term].curr_pid)*KB_8) - SIZEOFUINT_32;
    term_proc[active_term] = terms[active_term].curr_pid;
    sti();
    // Executing User-level Code
    // push correct user-level EIP, CS, EFLAGS, ESP, and SS registers on the kernel-mode stack,
    // then execute an IRET instruction. 
    // context_switch(prog_eip, user_esp);
    asm volatile ("                                                     \n\
        pushl %3                                                        \n\
        pushl %2                                                        \n\
        pushfl                                                          \n\
        pushl %1                                                        \n\
        pushl %0                                                        \n\
        iret                                                            \n\
        "
        :
        : "r"(prog_eip), "r"(USER_CS), "r"(user_esp), "r"(USER_DS)
        : "memory"
    );

    return 0;
}

/* int32_t halt (uint8_t status)
 * 
 * changes the current pcb to the parent pcb before unmapping the current page and mapping the parent's page
 * Inputs: status - return value of halt
 * Outputs: -1 - command cannot be executed
 *          255 - program dies by an exception
 *          0-254 - that given by the program when the program halts.
 * Files: syscall.h, sys_call_handler.h
 */
int32_t halt(uint8_t status)
{
    // for loop variable
    int i;
    // the return value
    uint32_t curr_status;
    //retore parent Data
    // pointer 
    //cli()
    pcb_t* curr_pcb = (pcb_t*)(MB_8 - KB_8 * (terms[active_term].curr_pid + 1));
    if(terms[active_term].curr_pid == active_term){
        curr_pcb->active_flag = 0;
        pid[active_term] = 0;
        execute((uint8_t*)"shell");
    }
    // set the curr pcb's pid to 0
    pid[curr_pcb->pid] = 0;
    // set the curr pcb's active flag to 0
    curr_pcb->active_flag = 0;
    // change the curr pid to the parent's pid
    terms[active_term].curr_pid = curr_pcb->parent_pid;
    term_proc[active_term] = terms[active_term].curr_pid;
    // get the parent's pcb pointer
    pcb_t* parent_pcb = (pcb_t*)(MB_8 - KB_8 * (terms[active_term].curr_pid + PID_OFFSET));
    // set the prev pid to the parent's parent's pid
    terms[active_term].prev_pid = parent_pcb -> parent_pid;
    // restore parent paging
    page_directory[USER_LEVEL_INDEX].pde_4mb.addr = (uint32_t) (((2 + terms[active_term].curr_pid) * MB_4) >> ADDR_SHIFT_4MB);
    flush_tlb();
    // clear FD
    for(i = 0; i< MAX_OPEN_FILES; i++)
    {
        //sets the file descriptor to 0
       // curr_pcb->file_desc_arr[i].flag = 0;
        close(i); 
    }
    // write parent process info back to TSS
    tss.ss0 = KERNEL_DS;
    tss.esp0 = MB_8 - (KB_8*parent_pcb->pid) - 4;
    // jump to execute return

    // check if there is an exception
    if(exception_status == 1)
    {
        // when there is an exception
        exception_status = 0;
        curr_status = EXCEPTION_VALUE;
    }
    else
    {
        // when there is no exception
        curr_status = (uint32_t) status;
    }
    
    // set the ebp and esp values as well as putting status as the return value
    asm volatile ("                                                     \n\
        mov  %2, %%eax                                                  \n\
        mov  %1, %%ebp                                                  \n\
        mov  %0, %%esp                                                  \n\
        leave                                                           \n\
        ret                                                             \n\
        "
        :
        : "r"(curr_pcb->saved_esp), "r"(curr_pcb->saved_ebp), "r"(curr_status)
        : "%eax", "memory"
    );
    return 0;

}

/* int32_t parse_cmd (const uint8_t* command, uint8_t* filename, uint8_t command_name)
 * 
 * parse filename and argument in inputs
 * Inputs: command - include filename and argument
 *         filename, command_name
 * Outputs: -1 - invalid filename or argument
 *          args_num - num of argument
 * Files: syscall.h
 */
int32_t parse_cmd(const uint8_t* command, uint8_t* file_name, uint8_t* command_name){
    // init var
    int i, j, k;
    int length = strlen((char *)command);
    int32_t space_begin = 0;
    int command_count = 0;  
    int file_name_count = 0; 
    // init filename and argument buffer
    for(j = 0; j < MAX_FILENAME+1; j++){
        file_name[j] = '\0';
    }

    for(j = 0; j < MAX_FILENAME; j++){
        command_name[j] = '\0'; 
    }
    for(k = 0; k < length; k++)
    {
        if(command[k] != ' ')
        {
            break; 
        }
    }
    // store file and command in buffer
    for(i = k; i < length; i++)
    {
        //skip any spaces in the given argument
        if(command[i] == ' ')
        {
            space_begin = 1;  
            continue;   
        }
        //populate command buffer
        if(command_count < MAX_FILENAME && !space_begin)
        {
            command_name[command_count++] = command[i];  
        }
        //populate file buffer
        else if(file_name_count <= MAX_FILENAME && space_begin)
        {
            //check for no spaces
            if(command[i] != ' ')
            {
                file_name[file_name_count++] = command[i]; 
            }
            
        }
        
    }

    return 0;    
}



/*void file_desc_arr_initial(pcb_t* pcb_val)
 * 
 * Initializes the file descriptor array 
 * Inputs: pcb_t* pcb_val 
 * Outputs: None
 * Side Effects: helps to initalize the file descriptor array 
 * indexes 0 to 8 
 * Files:syscall.h

 */
void file_desc_arr_initial(pcb_t* pcb_val)
{     
    int i; 
    for(i = 0; i < MAX_OPEN_FILES; i++)
    {
        if(i == 0)
        {
            //0th index of the file descriptor array is populated with the 
            // file operation functions of the stdin 
            //Initalize the other attributes of the file descriptor array 
            pcb_val->file_desc_arr[i].fop_table = stdin_operations;
            pcb_val->file_desc_arr[i].position_in_file = 0; 
            pcb_val->file_desc_arr[i].inode = 0; 
            pcb_val->file_desc_arr[i].flag = 1;  
        }
        else if(i == 1)
        {
            //1st index of the file descriptor array is populated with the 
            // file operation functions of the stdin 
            //Initalize the other attributes of the file descriptor array 
            pcb_val->file_desc_arr[i].fop_table = stdout_operations;
            pcb_val->file_desc_arr[i].position_in_file = 0; 
            pcb_val->file_desc_arr[i].inode = 0;
            pcb_val->file_desc_arr[i].flag = 1;  
        }
        else{
            //for all the other indexes, set the flag to FREE(0)
            pcb_val->file_desc_arr[i].flag = 0; 
        }
        
    }
    
}


/*void fop_init_rtc()
 * 
 * Initializes the file operations for the rtc 
 * Inputs: None
 * Outputs: None
 * Side Effects: initalizes the rtc read, 
 * write, open and close functions in the fop
 * Files:syscall.h

 */
void fop_init_rtc()
{
    //initializing the specific rtc operation functions
    // for the file operations table
    rtc_operations.read = (void *)&rtc_read; 
    rtc_operations.open = (void *)&rtc_open; 
    rtc_operations.write = (void *)&rtc_write; 
    rtc_operations.close = (void *)&rtc_close; 
}

/*void fop_init_dir()
 * 
 * Initializes the file operations for the dir 
 * Inputs: None
 * Outputs: None
 * Side Effects: initalizes the rtc read, 
 * write, open and close functions in the fop
 * Files:syscall.h

 */
void fop_init_dir()
{
     //initializing the specific dir operation functions
    // for the file operations table
    dir_operations.read = (void *)&dir_read; 
    dir_operations.open = (void *)&dir_open; 
    dir_operations.write =(void *)&dir_write; 
    dir_operations.close = (void *)&dir_close; 
}

/*void fop_init_file()
 * 
 * Initializes the file operations for the file 
 * Inputs: None
 * Outputs: None
 * Side Effects: initalizes the rtc read, 
 * write, open and close functions in the fop
 * Files:syscall.h

 */
void fop_init_file()
{
     //initializing the specific file operation functions
    // for the file operations table
    file_operations.read =(void *) &file_read; 
    file_operations.open = (void *)&file_open; 
    file_operations.write =(void *)&file_write; 
    file_operations.close = (void *)&file_close; 
}

/*void fop_init_terminal()
 * 
 * Initializes the file operations for the terminal 
 * Inputs: None
 * Outputs: None
 * Side Effects: initalizes the terminal read, 
 * write, open and close functions in the fop
 * Files:syscall.h

 */
void fop_init_terminal()
{
     //initializing the specific terminal operation functions
    // for the stdin operations table
    stdin_operations.read = (void *)&terminal_read; 
    stdin_operations.write = (void *)&null_work; 
    stdin_operations.open = (void *)&terminal_open;  
    stdin_operations.close = (void *)&terminal_close; 

     //initializing the specific terminal operation functions
    // for the stdout operations table
    stdout_operations.open =(void *) &terminal_open; 
    stdout_operations.read = (void *)&null_work; 
    stdout_operations.write = (void *)&terminal_write; 
    stdout_operations.close = (void *)&terminal_close; 
} 

/*int32_t null_work(int32_t fd, void* buf, int32_t nbytes)
 * do nothing only use for stdin write and stdout read
 * file descriptor array 
 * Inputs: int32_t fd, void* buf, int32_t nbytes
 * Outputs: return -1
 * Side Effects: writes to the specific file   
 * Files:syscall.h

 */
int32_t null_work(int32_t fd, void* buf, int32_t nbytes){
    return -1;
}



/*int32_t open(const uint8_t* file_name)
 * 
 * Populates the file descriptor array for the 
 * indexes 2 to 8  
 * Inputs: None
 * Outputs: None
 * Side Effects: initalizes the terminal read, 
 * write, open and close functions in the fop
 * Files:syscall.h

 */
int32_t open(const uint8_t* file_name)
{
    pcb_t* pcb_val; 
    pcb_val = (pcb_t*) (MB_8 - (terms[active_term].curr_pid+1)*KB_8);
    dentry_t dentry_temp; 
    int i; 
    if(strlen((char*)file_name) == NULL) return -1;  
    //if the file name does not exist or the func returns -1 then, invalid 
    if(file_name == NULL || read_dentry_by_name((int8_t*)file_name, &dentry_temp) == -1)
    {
        return -1; 
    }

    // looping through indexes 2 to 8 to populate the file descriptor array 
    for(i = FD_START; i < MAX_OPEN_FILES; i++)
    {
        // 0 flag represents that the fd index is unused 
        if(pcb_val->file_desc_arr[i].flag == 0)
        {   
            //if the type of file is a directory then use all the dir 
            //operation functions to populate the array 
            if(dentry_temp.file_type == 1)
            {
                pcb_val->file_desc_arr[i].fop_table = dir_operations;
                pcb_val->file_desc_arr[i].position_in_file = 0; 
                pcb_val->file_desc_arr[i].inode = dentry_temp.inode_idx; 
                pcb_val->file_desc_arr[i].flag = 1; 
                pcb_val->file_desc_arr[i].fop_table.open(file_name); 
                return i; 
            }
            //if the type of file is a rtc then use all the rtc 
            //operation functions to populate the array 
            else if(dentry_temp.file_type == 0)
            {
                pcb_val->file_desc_arr[i].fop_table = rtc_operations;
                pcb_val->file_desc_arr[i].position_in_file = 0; 
                pcb_val->file_desc_arr[i].inode = dentry_temp.inode_idx; 
                pcb_val->file_desc_arr[i].flag = 1;
                pcb_val->file_desc_arr[i].fop_table.open(file_name); 
                return i;              
                 
            }
            //if the type is file then use all the file 
            //operation functions to populate the array 
            else if(dentry_temp.file_type == 2)
            {
                pcb_val->file_desc_arr[i].fop_table = file_operations;
                pcb_val->file_desc_arr[i].position_in_file = 0; 
                pcb_val->file_desc_arr[i].inode = dentry_temp.inode_idx; 
                pcb_val->file_desc_arr[i].flag = 1;
                pcb_val->file_desc_arr[i].fop_table.open(file_name); 
                return i;  
            }
        }

    }
    
    //return -1 if function failed 
    return -1; 
}



/*int32_t close(int32_t fd)
 * 
 * Closes the specific file in the file descriptor array 
 * using the fd 
 * Inputs: int32_t fd
 * Outputs: None
 * Side Effects: closes the specfic type of file  
 * Files:syscall.h

 */
int32_t close(int32_t fd)
{
    pcb_t* pcb_val; 
    pcb_val = (pcb_t*) (MB_8 - (terms[active_term].curr_pid+1)*KB_8);
    int32_t ret_close; 

    //if the fd variable is not within the bounds of 2 and 8 or the fd we are accessing has 
    //not been used then, invalid 
    if(fd >=MAX_OPEN_FILES || fd < FD_START || pcb_val->file_desc_arr[fd].flag == 0)
    {
        return -1; 
    }

    //to close, set the flag in the file desc array to 0(FREE)
    pcb_val->file_desc_arr[fd].flag = 0; 
    //use the close function defined in the fop table 
    ret_close = pcb_val->file_desc_arr[fd].fop_table.close(fd); 
    return ret_close; 
}


/*int32_t write(int32_t fd, void* buf, int32_t nbytes)
 * Writing the buffer to the specific file accessed from the 
 * file descriptor array 
 * Inputs: int32_t fd, void* buf, int32_t nbytes
 * Outputs: None
 * Side Effects: writes to the specific file   
 * Files:syscall.h

 */
int32_t write(int32_t fd, void* buf, int32_t nbytes)
{
    pcb_t* pcb_val; 
    pcb_val = (pcb_t*) (MB_8 - (terms[active_term].curr_pid+1)*KB_8);
    int32_t bytes_write;
    
    //if the fd variable is not within the bounds of 0 and 8 or the fd we are accessing has 
    //not been used then, invalid  
    if(fd >=MAX_OPEN_FILES || fd < 0 || buf == NULL || pcb_val->file_desc_arr[fd].flag == 0)
    {
        return -1; 
    }

    //use the write function defined in the fop table to write to specific type of file 
    bytes_write = pcb_val->file_desc_arr[fd].fop_table.write(fd, buf, nbytes); 
    return bytes_write;   
}


/*int32_t read(int32_t fd, void* buf, int32_t nbytes)
 * 
 * Populates the buffer with the number of bytes specified  
 * Inputs: int32_t fd, void* buf, int32_t nbytes
 * Outputs: bytes_read
 * Side Effects: reads the specific file into the bufer   
 * Files:syscall.h

 */
int32_t read(int32_t fd, void* buf, int32_t nbytes)
{
    pcb_val_read = (pcb_t*) (MB_8 - (terms[active_term].curr_pid+1)*KB_8);
    int32_t bytes_read; 

    //if the fd variable is not within the bounds of 0 and 8 or the fd we are accessing has 
    //not been used then, invalid 
    if(fd >=MAX_OPEN_FILES || fd < 0 || pcb_val_read->file_desc_arr[fd].flag == 0)
    {
        return -1; 
    }

    //use the read function defined in the fop table to read to specific type of file
    bytes_read = pcb_val_read->file_desc_arr[fd].fop_table.read(fd, buf, nbytes); 
    return bytes_read;   
}

/*int32_t vidmap(uint8_t** start_screen)
 * 
 * maps the text-mode video memory into user space at a pre-set virtual address. 
 * Inputs: uint8_t** start_screen
 * Outputs: -1 if start_screen invalid
 * Side Effects: map the video memory into user space  
 * Files:syscall.h

 */
int32_t vidmap(uint8_t** start_screen)
{
    // check if start_screen is valid
    if(start_screen == NULL) return -1;
    uint32_t start = (uint32_t) start_screen;
    // check if *start_screen is in user level program
    if(start < USER_LEVEL_PROG || start >= USER_LEVEL_PROG + MB_4) return -1;
    // vidmap_flag = 1;
    // set up addr 
    *start_screen = (uint8_t*) VIDMAP_ADDR + active_term * VM_SIZE;
    // remap the video memory
    // page_directory[USER_VM_INDEX].pde_4kb.U_S = 1;
    // page_vm_table[active_term].pte.U_S = 1;
    // page_vm_table[active_term].pte.addr = page_table[VM_T0 + active_term].pte.addr;
    // flush_tlb();
    return 0;

}

/*int32_t set_handler(int32_t signum, void* handler_address)
 * 
 * Inputs: int32_t signum, void* handler_address
 * Outputs: -1 
 * Files:syscall.h

 */
int32_t set_handler(int32_t signum, void* handler_address)
{
    return -1;
}

/*int32_t sigreturn(void)
 * 
 * Inputs: void
 * Outputs: -1 
 * Files:syscall.h

 */
int32_t sigreturn(void){
    return -1;
}

/*int32_t getargs(uint8_t* buf, int32_t nbytes)
 * 
 * Populates the given buffer with the right file name 
 * Inputs: uint8_t* buf, int32_t nbytes
 * Outputs: 0 for valid population
 * Side Effects:None  
 * Files:syscall.h

 */
int32_t getargs(uint8_t* buf, int32_t nbytes)
{
    //get the updated pcb address
    pcb_val_read = (pcb_t*) (MB_8 - (terms[active_term].curr_pid+1)*KB_8);
    //check valid pcb arguments
    if(pcb_val_read->args_name[0] != NULL && buf != NULL)
    {
        //copy into buffer given the pcb file name
        strncpy((int8_t*)buf, (int8_t*)(pcb_val_read->args_name), nbytes);
        return 0; 
    } 

    return -1;

}

/*int check_avai_pid()
 * 
 * Used to check the available process ID 
 * Inputs: None
 * Outputs: -1 for fail, 0 for success
 * Side Effects:None  
 * Files:syscall.h

 */
int check_avai_pid(){
    int i;
    for(i = 0; i < PID_MAX; i++){
        if(pid[i] == 0){
            return 0;
        }
    }
    return -1;
}

