#include "paging.h"


/* store PD address in cr3 */
extern void loadPageDirectory(unsigned int*);
/* enable paging and allow mixed page sizes (both 4kB and 4MB) */
extern void enablePaging();
/* flush tlb in cr3 */
extern void flush_tlb();

/* initialize and set pde to not present at the beginning */
void set_init_PDE(int index, int cmd);
/* initialize and set pte to not present at the beginning */
void set_init_PTE(int index);
void set_init_vm_PTE(int index);
void setup_mem(uint32_t curr_pid);
void reset_vidmap();
extern int active_term;
/* 
 * page_init
 *   DESCRIPTION: initialize paging
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: initialize paging
 */
void page_init(){
    //set each entry to not present at the beginning
    int i;
    for(i = 0; i < NUM_PDE; i++){
        // initialize the first entry for 4kB pages 
        if(i == 0){
            set_init_PDE(i, 0);
        }
        // initialize the rest of entries for 4MB pages
        else{
            set_init_PDE(i, 1);
        }  
        // fill all 1024 entries in the table, mapping the first 4MB
        // since only first 4MB require page table with 4 kB pages
        set_init_PTE(i);
        set_init_vm_PTE(i);    
    }
    // set pte of video memory page to present
    page_table[VM_INDEX].pte.present = 1;
    page_table[VM_INDEX].pte.addr = VM_T0;
    page_table[VM_T0].pte.present = 1;
    page_table[VM_T0].pte.addr = VM_INDEX;
    page_table[VM_T1].pte.present = 1;
    page_table[VM_T2].pte.present = 1;
    // set up 2nd pde which is the kernel 4MB page
    page_directory[1].pde_4mb.present = 1;
    page_directory[1].pde_4mb.global_p = 1;
    page_directory[1].pde_4mb.addr = (uint32_t) (KERNEL_ADDR >> ADDR_SHIFT_4MB);
    // user level program paging
    page_directory[USER_LEVEL_INDEX].pde_4mb.present = 1;
    page_directory[USER_LEVEL_INDEX].pde_4mb.U_S = 1;

    //used for the vidmap functionality 
    page_directory[USER_VM_INDEX].pde_4kb.present = 1;
    page_directory[USER_VM_INDEX].pde_4kb.U_S = 1;
    page_directory[USER_VM_INDEX].pde_4kb.p_s = 0;
    page_directory[USER_VM_INDEX].pde_4kb.addr = ((uint32_t) page_vm_table) >> ADDR_SHIFT_4KB;
    page_vm_table[0].pte.present = 1;
    page_vm_table[0].pte.U_S = 1;
    page_vm_table[0].pte.addr = VM_INDEX;
    page_vm_table[1].pte.present = 1;
    page_vm_table[1].pte.U_S = 1;
    page_vm_table[1].pte.addr = VM_T1;
    page_vm_table[2].pte.present = 1;
    page_vm_table[2].pte.U_S = 1;
    page_vm_table[2].pte.addr = VM_T2;


    // load page directory and enable paging
    loadPageDirectory((unsigned int*) page_directory);
    enablePaging();

    return;




}

/* 
 * set_init_PDE
 *   DESCRIPTION: set initialized PDE
 *   INPUTS: pde - page directory entry
 *           cmd - 0 for 1st pde and 1 for rest of the pde 
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: initialize PDE
 */
void set_init_PDE(int index, int cmd){
    // This sets the following flags to the pages:
    //   Supervisor: Only kernel-mode can access them
    //   Write Enabled: It can be both read from and written to
    //   Not Present: The page table is not present
    switch (cmd){
    // if cmd == 0, PDE is the entry for 4kB page
    case 0:
        page_directory[index].pde_4kb.present = 1;
        page_directory[index].pde_4kb.R_W = 1;
        page_directory[index].pde_4kb.U_S = 0;
        page_directory[index].pde_4kb.wr_thru = 0;
        page_directory[index].pde_4kb.cache_dis = 0;
        page_directory[index].pde_4kb.accessed = 0;
        page_directory[index].pde_4kb.reserved = 0;
        page_directory[index].pde_4kb.p_s = 0;
        page_directory[index].pde_4kb.global_p = 0;
        page_directory[index].pde_4kb.available = 0;
        page_directory[index].pde_4kb.addr = ((uint32_t) page_table) >> ADDR_SHIFT_4KB;
        break;
    // if cmd == 1, PDE is the entry for 4MB page
    case 1:
        page_directory[index].pde_4mb.present = 0;
        page_directory[index].pde_4mb.R_W = 1;
        page_directory[index].pde_4mb.U_S = 0;
        page_directory[index].pde_4mb.wr_thru = 0;
        page_directory[index].pde_4mb.cache_dis = 0;
        page_directory[index].pde_4mb.accessed = 0;
        page_directory[index].pde_4mb.dirty = 0;
        page_directory[index].pde_4mb.p_s = 1;
        page_directory[index].pde_4mb.global_p = 0;
        page_directory[index].pde_4mb.available = 0;
        page_directory[index].pde_4mb.pat = 0;
        page_directory[index].pde_4mb.reserved = 0;
        page_directory[index].pde_4mb.addr = 0;
        break;
    
    default:
        break;
    }
    
}

/*  
 * set_init_PTE
 *   DESCRIPTION: set initialized PTE
 *   INPUTS: pte - page table entry
 *           index
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: initialize PTE
 */
void set_init_PTE(int index){
    // This sets the following flags to the pages:
    //   Supervisor: Only kernel-mode can access them
    //   Write Enabled: It can be both read from and written to
    //   Not Present: The page table is not present
    page_table[index].pte.present = 0;
    page_table[index].pte.R_W = 1;
    page_table[index].pte.U_S = 0;
    page_table[index].pte.wr_thru = 0;
    page_table[index].pte.cache_dis = 0;
    page_table[index].pte.accessed = 0;
    page_table[index].pte.dirty = 0;
    page_table[index].pte.pat = 0;
    page_table[index].pte.global_p = 0;
    page_table[index].pte.available = 0;
    page_table[index].pte.addr = index;
}

/*  
 * set_init_PTE
 *   DESCRIPTION: set initialized PTE
 *   INPUTS: pte - page table entry
 *           index
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: initialize PTE
 */
void set_init_vm_PTE(int index){
    // This sets the following flags to the pages:
    //   Supervisor: Only kernel-mode can access them
    //   Write Enabled: It can be both read from and written to
    //   Not Present: The page table is not present
    page_vm_table[index].pte.present = 0;
    page_vm_table[index].pte.R_W = 1;
    page_vm_table[index].pte.U_S = 0;
    page_vm_table[index].pte.wr_thru = 0;
    page_vm_table[index].pte.cache_dis = 0;
    page_vm_table[index].pte.accessed = 0;
    page_vm_table[index].pte.dirty = 0;
    page_vm_table[index].pte.pat = 0;
    page_vm_table[index].pte.global_p = 0;
    page_vm_table[index].pte.available = 0;
    page_vm_table[index].pte.addr = index;
}

/*  
 * setup_mem(uint32_t curr_pid)
 *   DESCRIPTION: set up user level program paging
 *   INPUTS: curr_pid
 *   OUTPUTS: none
 *   RETURN VALUE: none
 */

void setup_mem(uint32_t curr_pid){
    // physical memory starts at 8MB + pid*4MB
    page_directory[USER_LEVEL_INDEX].pde_4mb.addr = (uint32_t) (((2 + curr_pid) * MB_4) >> ADDR_SHIFT_4MB);
    flush_tlb();
}




