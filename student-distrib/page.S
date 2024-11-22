#define ASM     1
; #include "paging.h"


.text
.globl loadPageDirectory
.globl enablePaging
.globl flush_tlb

.align 4

/* 
 * loadPageDirectory
 *   DESCRIPTION: tell the processor where to find our page directory
 *          by ptting it's address into the CR3 register. 
 *   INPUTS: the address of the page director
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: load the address onto the CR3 register
 *   REGISTERS: cr3: store the page directory address in cr3
 */
loadPageDirectory:
pushl %ebp
movl %esp, %ebp
// store the page directory address in cr3
movl 8(%esp), %eax
movl %eax, %cr3
movl %ebp, %esp
popl %ebp
ret

.align 4
/* 
 * enablePaging
 *   DESCRIPTION: enable paging by setting the 32th bit in the
 *          CR0 register and allow mixed page sizes
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS:
 *   REGISTERS: cr0: set the 32th bit, the paging bit and enable
 *              paging
 *              cr4: set the 4th bit, PSE, to allow mixed page 
 *              sizes (4kB and 4MB)
 */
enablePaging:
pushl %ebp
movl  %esp, %ebp
// set PSE bit4 in CR4 to allow mixed page sizes (4kB and 4MB)
movl %cr4, %eax
orl  $0x0010, %eax
movl %eax, %cr4
// set the paging bit to enable paging
movl %cr0, %eax
orl  $0x80000000, %eax
movl %eax, %cr0

movl %ebp, %esp
popl %ebp
ret


/* 
 * flush_tlb
 *   DESCRIPTION: flush the tlb if we modify or create page structure 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: flush tlb
 *   REGISTERS: cr3: flush tlb
 */
flush_tlb:
movl    %cr3,%eax
movl    %eax,%cr3
ret
