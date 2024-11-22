
#ifndef FILE_SYSTEM_H
#define  FILE_SYSTEM_H
#include "lib.h"

#define MAX_FILENAME        32
#define TOTAL_NUM_DIR       63
#define DATA_BLOCK_SIZE     4096
#define RESERVED_DIR        24
#define RESERVED_BOOT_SIZE  52
#define TOTAL_NUM_BLOCK     1023


typedef struct {
    uint8_t file_name[MAX_FILENAME]; 
    uint32_t file_type; 
    uint32_t inode_idx; 
    uint8_t reserved_dentry[RESERVED_DIR]; 
} dentry_t; 

typedef struct {
    uint32_t dir_num; 
    uint32_t inode_size; 
    uint32_t data_block_size; 
    uint8_t reserved_boot[RESERVED_BOOT_SIZE]; 
    dentry_t dir_entries[TOTAL_NUM_DIR]; 
} boot_block_t; 

typedef struct {
    uint32_t file_length; 
    uint32_t num_of_blocks[TOTAL_NUM_BLOCK]; 
} inode_desc_t; 

extern boot_block_t* boot_block_addr; 
extern inode_desc_t* inode_addr; 
extern uint8_t* data_block_addr;
extern dentry_t dentry_temp;  

void file_system_init(unsigned int base_address); 
int32_t read_dentry_by_name (const int8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);
uint32_t file_open(const int8_t* fname); 
uint32_t file_close(int32_t fd); 
uint32_t file_read(int32_t fd, void* buf, int32_t nbytes);
uint32_t file_write(int32_t fd, const void* buf, int32_t nbytes); 
uint32_t dir_open(const int8_t* fname); 
uint32_t dir_read(int32_t fd, void* buf, int32_t nbytes);
uint32_t dir_write(int32_t fd, const void* buf, int32_t nbytes); 
uint32_t dir_close(int32_t fd); 


#endif 
