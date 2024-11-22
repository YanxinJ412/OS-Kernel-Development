#include "x86_desc.h"
#include "types.h"

// VIDEO is already defined in lib.c
#define KB_8 0x2000
#define MB_4 0x400000
#define MB_8 0x800000
#define VM_  0xB7
#define VM_T0 0xB9
#define VM_T1 0xBA
#define VM_T2 0xBB

#define VIDEO 0xB8000
#define VM_INDEX VIDEO/KB_4
#define KERNEL_ADDR 0x400000
#define ADDR_SHIFT_4KB 12
#define ADDR_SHIFT_4MB 22
#define USER_LEVEL_PROG 0x8000000
#define USER_LEVEL_INDEX 32
#define USER_VM_INDEX 33



/* initialize paging */
extern void page_init();

