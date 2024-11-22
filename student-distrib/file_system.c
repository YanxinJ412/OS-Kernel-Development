#include "file_system.h"
#include "syscall.h"
#define MAX_FILENAME    32
#define TOTAL_NUM_DIR   63
#define DATA_BLOCK_SIZE 4096

//declared global variables
boot_block_t* boot_block_addr; 
inode_desc_t* inode_addr; 
uint8_t* data_block_addr;
dentry_t dentry_temp;  
uint32_t file_count = 0;



/* void file_system_init(unsigned int base_address)
 * 
 * Initializes the starting addresses of the 
 * boot block, inode and the data block 
 * Inputs: the base address of the boot block  
 * Outputs: None
 * Side Effects: helps to initalize the file system addresses
 * Files:filesystem.h

 */
void file_system_init(unsigned int base_address){
    //initializing the boot block address 
    boot_block_addr = (boot_block_t*)(base_address);     

    //initializing the inode start address 
    inode_addr = (inode_desc_t*)(boot_block_addr + 1); 

    //initializing the data block start address
    data_block_addr = (uint8_t*)(boot_block_addr->inode_size + inode_addr);
}


/* int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry)
 * 
 * finds the filename from the dir entries and populates 
 * the information in the dentry variable  
 * Inputs: the file name to be found, the dentry variable that 
 * has to be populated  
 * Outputs: return 0 for success, -1 for failure
 * Files:filesystem.h
 */
int32_t read_dentry_by_name (const int8_t* fname, dentry_t* dentry)
{
    int i; 
    int j;
    dentry_t* start_of_dentry =  boot_block_addr->dir_entries; 
    int8_t dentry_name[MAX_FILENAME+1]; 
    uint32_t dentry_name_size; 
    uint32_t fname_size = strlen(fname); 
    if(fname_size > MAX_FILENAME)           
    {
        return -1;
    }

    //iterate throguh the dir entries 
    for(i = 0; i < TOTAL_NUM_DIR; i++)
    {
        for (j = 0; j < MAX_FILENAME; j++)
        {
             dentry_name[j] = start_of_dentry[i].file_name[j]; 
        }
        dentry_name[j] = '\0'; 

        dentry_name_size = strlen(dentry_name); 
        if(dentry_name_size == fname_size)
        {
            // compare the file and dentry with only 32 bytes 
            if(strncmp(dentry_name, fname, MAX_FILENAME) == 0)
            {
                for (j = 0; j < MAX_FILENAME; j++)
                {   
                    //if the fname and the filename found in the dentry 
                    // are equal then, populate the dentry variable with the filename 
                    dentry->file_name[j] = dentry_name[j]; 
                } 
                 
                //populate the dentry variable with the information such as 
                // file type and the inode index
                dentry->file_type = start_of_dentry[i].file_type;       
                dentry->inode_idx = start_of_dentry[i].inode_idx;
                return 0; 
            }
        }

        // if the length of filename is mot equal to dentry then continue
    }

    //return -1 for failure 
    return -1; 

}

/* int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry)
 * 
 * finds the dentry struct that is used to populate the temporary dentry variable  
 * the information in the dentry variable  
 * Inputs: the index to the dentry strucy, the dentry variable that 
 * has to be populated  
 * Outputs: return 0 for success, -1 for failure
 * Files:filesystem.h
 */
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry)
{
    dentry_t* start_of_dentry =  (boot_block_addr->dir_entries); 
    int j; 

    //if the index value is larger than the number of total dentry structs exists 
    // or if the index is less than 0, return failure
    if(index >= TOTAL_NUM_DIR || index < 0)
    {
        return -1; 
    }
    
    //populate the dentry variable given with the filename
    for (j = 0; j < MAX_FILENAME; j++)
    {
        dentry->file_name[j] = start_of_dentry[index].file_name[j];
    } 

    // populate the dentry variable given with the file type 
    // and the inode index
    dentry->file_type = start_of_dentry[index].file_type; 
    dentry->inode_idx = start_of_dentry[index].inode_idx;

    //return 0 for success
    return 0; 
}

/* int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
 * 
 * populates the buffer with the contents of the data blcoks
 * Inputs: the inode index, offset, buffer that gets populated and 
 * the number of bytes to be read
 * Outputs: return the total number of bytes read from the data blocks
 * Files:filesystem.h
 */
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)       
{
    int j; 
    int buff_ind = 0;
    inode_desc_t* inode_info; 
    uint32_t idx_val = offset/DATA_BLOCK_SIZE;
    uint32_t idx_rem = offset % DATA_BLOCK_SIZE;             
    uint8_t* data_info; 
    uint32_t data_block_idx; 
    uint32_t file_size; 
    uint32_t bytes_incr; 

    bytes_incr = 0; 
    
    //if the inode index is greater than the number of 
    // total inodes then, return failure
    if(inode >= boot_block_addr->inode_size)
    {
        return -1; 
    }
    inode_info = (inode_desc_t*)&inode_addr[inode]; 
    //if the inode pointer obtained is NULL, return failure
    if(inode_info == NULL)
    {
        return -1; 
    }
    //if offset is greater than the length of the file, 
    //no bytes have been read so return 0 
    file_size = inode_info->file_length;
    if(offset >= file_size)
    {
        return 0;
    }

    
    data_block_idx = inode_info->num_of_blocks[idx_val]; 
    //data_info represents starting address of the specifc data block 
    //from which we have to start reading the data 
    data_info = (uint8_t*)&data_block_addr[data_block_idx*DATA_BLOCK_SIZE]; 
    while(1)
    {
        for(j = idx_rem; j < DATA_BLOCK_SIZE; j++)
        {   
            //if the offset and the number of bytes read has reached the end of the 
            //file then return the number of bytes read 
            if(offset + bytes_incr == file_size || bytes_incr == length)
            {
                return bytes_incr; 
            }
            // populate the buffer with the data contents
            buf[buff_ind] = data_info[j];
            //putc(data_info[j]);
            //incrementing the number of bytes read
            bytes_incr++; 
            buff_ind++;
        }
        //incrementing the current data block to the next
        idx_val++; 
        idx_rem = 0;
        //update the data block index and info
        data_block_idx = inode_info->num_of_blocks[idx_val]; 
        data_info = (uint8_t*)&data_block_addr[data_block_idx*DATA_BLOCK_SIZE]; 
    }
   
    //return the total number of bytes read from 
    //the data blocks
    return bytes_incr; 

}

/* uint32_t file_open(const uint8_t* fname)
 * 
 * opens the given file and populates a global dentry
 * Inputs: the file name
 * Outputs: return 0 on a successful open and -1 on fail
 * Files:filesystem.h
 */
uint32_t file_open(const int8_t* fname){
    //declare a dentry variable 
    uint32_t copyDentry_val; 
    //find the dentry matching the file name
    copyDentry_val = read_dentry_by_name(fname, &dentry_temp); 
    //check if the file is a regular file and the copying worked
    if(dentry_temp.file_type == 2 && copyDentry_val == 0)
    {
        return 0; 
    }
    //opening the file did not work
    return -1; 
}


/* uint32_t file_close(int32_t fd)
 * 
 * closes the file
 * Inputs: file index
 * Outputs: return 0 to close file
 * Files:filesystem.h
 */
uint32_t file_close(int32_t fd)
{
    //close file return 0 
   // printf("file is closed"); 
    return 0; 

}


/* uint32_t file_read(int32_t fd, uint32_t offset, void* buf, int32_t nbytes)
 * 
 * reads the open file by calling read_data to populate the buffer parameter
 * Inputs: file index, offset to start read from, buffer to populate, number of bytes to read
 * Outputs: return the number of bytes read or return -1 on fail read
 * Files:filesystem.h
 */
uint32_t file_read(int32_t fd, void* buf, int32_t nbytes)
{

    int32_t bytes_read;   
    //call read data to read the given file and store the number of bytes read 
    bytes_read = read_data(pcb_val_read->file_desc_arr[fd].inode, pcb_val_read->file_desc_arr[fd].position_in_file, buf, nbytes); 
    if(bytes_read == -1)
    {
         //reading the file did not work
        return -1; 
    }
    pcb_val_read->file_desc_arr[fd].position_in_file += bytes_read;
    //return the bytes read
    return bytes_read; 

}


/* uint32_t file_write(int32_t fd, const void* buf, int32_t nbytes) 
 * 
 * write to a file
 * Inputs: file descriptor, buffer, number bytes to write
 * Outputs: return -1 since files are read only
 * Files:filesystem.h
 */
uint32_t file_write(int32_t fd, const void* buf, int32_t nbytes)       
{
    //return -1 since files are read only
    return -1; 
}


/* uint32_t dir_open(const uint8_t* fname)
 * 
 * open the directory by calling read_dentry_by_name
 * Inputs: directory name
 * Outputs: return 0 if directory was opened and -1 if not opened
 * Files:filesystem.h
 */
uint32_t dir_open(const int8_t* fname){
    //declare a dentry variable 
    uint32_t copyDentry_val; 
    // use the function read_name_dentry to look for the filename and populate the local defined dentry 
    copyDentry_val = read_dentry_by_name(fname, &dentry_temp); 
    //if the file type is 1 and read_name_dentry returns 0 then return 0 otherwise -1
    //if((dentry_temp.file_type == 1 || dentry_temp.file_type == 2)  && copyDentry_val == 0)
    if(copyDentry_val == 0)
    {
        return 0; 
    }

    return -1; 
}

/* uint32_t dir_close(int32_t fd)
 * 
 * closes the directory
 * Inputs: directory index
 * Outputs: return 0 to close directory
 * Files:filesystem.h
 */
uint32_t dir_close(int32_t fd)
{
    return 0; 

}

/* uint32_t dir_read(int32_t fd, uint32_t offset, void* buf, int32_t nbytes)
 * 
 * reads the open directory by calling read_dentry_by_index to populate the buffer parameter
 * Inputs: directory index, offset to start read from, buffer to populate, number of bytes to read
 * Outputs: return the number of bytes read or return -1 on fail read
 * Files:filesystem.h
 */
uint32_t dir_read(int32_t fd, void* buf, int32_t nbytes)     
{
    int32_t checkVal; 
    int32_t read_num;
    int32_t counterVal = pcb_val_read->file_desc_arr[fd].position_in_file; 
    //check if the count is greater than the number of directories
    //check if the buffer is null
   // if(file_count > boot_block_addr->dir_num || buf == NULL)
   // {
        //return -1 if either cases are met
       // return -1;  
   // }

    //call read_dentry_by_index to populate global dentry
    checkVal = read_dentry_by_index(pcb_val_read->file_desc_arr[fd].position_in_file, &dentry_temp); 
    if(checkVal == -1)
    {
        //if read_dentry_by_index returns -1 then there is a failed read
        return -1; 
    }
    read_num = 32;
    if(strlen((int8_t*)dentry_temp.file_name) < 32){
        read_num = strlen((int8_t*)dentry_temp.file_name);
    }
    //copy the file name into the buffer
    memcpy(buf, dentry_temp.file_name, read_num); 
    //add a null terminating character
    //((unsigned char*)buf)[nbytes+1] = 0;
    //increment everytime dir_read is called
    counterVal++;
    pcb_val_read->file_desc_arr[fd].position_in_file = counterVal; 
    
    
    return read_num;
}


/* uint32_t dir_write(int32_t fd, const void* buf, int32_t nbytes)  
 * 
 * write to a directory
 * Inputs: directory descriptor, buffer, number bytes to write
 * Outputs: return -1 since directories are read only
 * Files:filesystem.h
 */
uint32_t dir_write(int32_t fd, const void* buf, int32_t nbytes)          
{
    //return -1 since directories are read only
    return -1; 
}

