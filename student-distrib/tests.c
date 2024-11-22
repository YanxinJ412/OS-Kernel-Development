#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "rtc.h"
#include "file_system.h"
#include "terminal.h"
#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;
	
	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}

		if(idt[i].present == 0)
		{
			assertion_failure(); 
			result = FAIL; 
		}
	}
	return result;
}

/* IDT TEST: idt_divison_test
 * 
 * Throws the division error exception when 
 * this test in called
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Files: x86_desc.h/S, idt.c/h, interrupts_wrapper.S
 */
int idt_divison_test()
{	
	TEST_HEADER;
	
	int result = PASS; 
	int i; 
	int j; 
	int k; 
	i = 1; 
	j = 0; 
	k = i/j; 
	
	result = FAIL; 
	return result; 
}

/* System Call Test: syscall_test
 * 
 * acknowledges that a system call has been invoked 
 * when the vector number 0x80 is used
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Files: x86_desc.h/S, idt.c/h, interrupts_wrapper.S
 */
int syscall_test(){
	TEST_HEADER;
	asm volatile("int $0x80");
	return FAIL;
}

/* PAGING Test: paging_test_kernel_and_vm
 * 
 * Check the memory of kernel and virtual memory
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load cr3 and initialize page
 * Files: x86_desc.h/S, paging.c/h, page.S
 */
int paging_test_kernel_and_vm(){
	TEST_HEADER;
	unsigned int *ptr;
	unsigned int res;
	// check the pagging of kernel
	ptr = (unsigned int*) (0x400000);
	res = *ptr;
	ptr = (unsigned int*) (0x7FFFF5);
	res = *ptr;
	// check the pagging of video memory
	ptr = (unsigned int*) (0xB8000);
	res = *ptr;
	ptr = (unsigned int*) (0xB8FF5);
	res = *ptr;

	// int addr;
	// check the pagging of video memory
	// for(addr = 0x400000; addr < 0x7FFFFD; addr++){
	// 	ptr = (unsigned int*) addr;
	// 	res = *ptr;
	// }
	// // check the pagging of kernel
	// for(addr = 0xB8000; addr < 0xB8FFD; addr++){
	// 	ptr = (unsigned int*) addr;
	// 	res = *ptr;
	// }
	return PASS;
}



/* PAGING Test: paging_test_not_pres1
 * 
 * Check the upper bound of video memory
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load cr3 and initialize page
 * Files: x86_desc.h/S, paging.c/h, page.S
 */
int paging_test_not_pres1(){
	TEST_HEADER;
	unsigned int *ptr;
	unsigned int res;
	// upper bound of video memory
	ptr = (unsigned int*) (0xB7FFF);
	res = *ptr;
	return PASS;
}

/* PAGING Test: paging_test_not_pres2
 * 
 * Check the lower bound of video memory
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load cr3 and initialize page
 * Files: x86_desc.h/S, paging.c/h, page.S
 */
int paging_test_not_pres2(){
	TEST_HEADER;
	unsigned int *ptr;
	unsigned int res;
	//lower bound of video memory
	ptr = (unsigned int*) (0xB9000);
	res = *ptr;
	ptr = (unsigned int*) (0xB9002);
	res = *ptr;
	return PASS; 
}

/* PAGING Test: paging_test_not_pres3
 * 
 * Check the upper bound of kernel
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load cr3 and initialize page
 * Files: x86_desc.h/S, paging.c/h, page.S
 */
int paging_test_not_pres3(){
	TEST_HEADER;
	unsigned int *ptr;
	unsigned int res;
	//upper bound of kernel
	ptr = (unsigned int*) (0x3FFFFF);
	res = *ptr;
	return PASS; 
}

/* PAGING Test: paging_test_not_pres4
 * 
 * Check the lower bound of kernel
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load cr3 and initialize page
 * Files: x86_desc.h/S, paging.c/h, page.S
 */
int paging_test_not_pres4(){
	TEST_HEADER;
	unsigned int *ptr;
	unsigned int res;
	//lower bound of kernel
	ptr = (unsigned int*) (0x800000);
	res = *ptr;
	ptr = (unsigned int*) (0x800002);
	res = *ptr;
	return PASS; 
}




// add more tests here

/* Checkpoint 2 tests */

/* Checkpoint 2 tests */
/* RTC Test: RTC_write_read
 * Test the all of the RTC frequencies
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: RTC system call functions
 * Files: rtc.c/h
 */
int RTC_write_read()
{
	int32_t i;
    int32_t result = rtc_open(NULL);
	for(i = 2; i <= 1024; i *= 2)
	{
		int32_t count = 0;
		result += rtc_write(NULL, &i, sizeof(uint32_t));
		printf("Current Freq: %d HZ\n", i);
		while (count < i)
		{
			result += rtc_read(NULL, NULL, NULL);
			printf("1");
			count ++;
		}
		printf("\n");
	}
	result += rtc_close(NULL);
	if(!result)
	{
		return PASS;
	}
	return FAIL;
}



/* keyboard&terminal Test: int terminal_test
 * 
 * Check terminal read and terminal write inside 128 bytes
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: keyboard and terminal driver
 * Files: keyboard.h/c, lib.c/h, terminal.c/h
 */
int terminal_test1(){
	TEST_HEADER;
	char buf[128];
	int byte_read;

	while(1){
		byte_read = terminal_read(0, buf, 5);
		terminal_write(0, buf, byte_read);
		printf("byte_read is %d\n", byte_read);
	}
	return PASS;
}


/* keyboard&terminal Test: int terminal_test
 * 
 * Check terminal read and terminal write with 128 bytes
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: keyboard and terminal driver
 * Files: keyboard.h/c, lib.c/h, terminal.c/h
 */
int terminal_test2(){
	TEST_HEADER;
	char buf[128];
	int byte_read;

	while(1){
		byte_read = terminal_read(0, buf, 128);
		terminal_write(0, buf, byte_read);
		printf("byte_read is %d\n", byte_read);
	}
	return PASS;
}

/* keyboard&terminal Test: int terminal_test
 * 
 * Check terminal read and terminal write outside the 128 bytes boundary
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: keyboard and terminal driver
 * Files: keyboard.h/c, lib.c/h, terminal.c/h
 */
int terminal_test3(){
	TEST_HEADER;
	char buf[128];
	int byte_read;

	while(1){
		byte_read = terminal_read(0, buf, 140);
		terminal_write(0, buf, byte_read);
		printf("byte_read is %d\n", byte_read);
	}
	return PASS;
}


/* void output_dentry_filename(dentry_t dentry_output)
 * 
 * Output the file name, size and type
 * Inputs: dentry with information to output
 * Outputs: Printed file information
 * Side Effects: None
 * Files: file_system.h
 */
void output_dentry_filename(dentry_t dentry_output)
{
	int j;
	uint8_t dentry_name[MAX_FILENAME];
	uint32_t file_tp; 
	uint32_t inode_num;
	inode_desc_t* inode_curr;
	uint32_t file_size;
	int left;
	int right;
	char temp1; 
	
	
	printf("file name: ");
	for (j = MAX_FILENAME-1; j >= 0; j--)
    {
        dentry_name[j] = dentry_output.file_name[MAX_FILENAME-1-j];	
    } 
	for (j = 0; j < MAX_FILENAME; j++)
    {
		if(dentry_name[j] != '\0'){
			left = j;
			right = MAX_FILENAME-1;
			while(left < right){
				temp1 = dentry_name[left];
				dentry_name[left] = dentry_name[right];
				dentry_name[right] = temp1;
				left++;
				right--;
			}
			break;
		}
        
    }
	for(j=0; j < MAX_FILENAME; j++){
		putc(dentry_name[j]);
	} 
	
	file_tp = dentry_output.file_type;
	inode_num = dentry_output.inode_idx;
	inode_curr = &inode_addr[inode_num];
	file_size = inode_curr->file_length;
	printf(", file_type: %d, file_size: %d\n", file_tp, file_size);

	
}

/* int test_file_size()
 * 
 * Test to print file information
 * Inputs: None
 * Outputs: Printed file information
 * Side Effects: None
 * Files: file_system.h
 */
int test_file_size(){
	
	int8_t filename[16][35] = {".", "sigtest", "shell", "grep", "syserr", "fish", "counter", "pingpong", "cat",
								"frame0.txt", "verylargetextwithverylongname.tx", "ls", "testprint",
								"created.txt", "frame1.txt", "hello"};
	int j;
	for(j=0; j < 16; j++){
		uint8_t buff[6000];
		int32_t bytes_read;  
		
		if(j == 0){
			test_dir();
			continue;
		}
		if(file_open(filename[j]) == -1){
			return FAIL;
		} 
		output_dentry_filename(dentry_temp);

		if(file_write(0, buff, bytes_read) != -1){
			return FAIL;
		}

		if(file_close(0) == 0){
			continue;
		}
		else{
			return FAIL;
		}
	}
	return PASS;
}


/* int test_file_content()
 * 
 * Test to print file content
 * Inputs: None
 * Outputs: Printed file content
 * Side Effects: None
 * Files: file_system.h
 */
int test_file_content(){
	
	int8_t filename[16][35] = {".", "sigtest", "shell", "grep", "syserr", "fish", "counter", "pingpong", "cat",
								"frame0.txt", "verylargetextwithverylongname.tx", "ls", "testprint",
								"created.txt", "frame1.txt", "hello"};
		int j;
		uint8_t buff[6000];
		int32_t bytes_read;  
		int i;

		j = 15;
		if(j == 0){
			test_dir_content();
			return PASS;
		}
		
		if(file_open(filename[j]) == -1){
			return FAIL;
		}
		bytes_read = file_read(0, buff, 100);

		for(i=0; i < bytes_read; i++){
			putc(buff[i]);
		}
		
		if(file_write(0, buff, bytes_read) != -1){
			return FAIL;
		}

		if(file_close(0) == 0){
			return PASS;
		}
	
	return FAIL;
}




/* int test_dir()
 * 
 * Test to print directory information
 * Inputs: None
 * Outputs: Printed directory information
 * Side Effects: None
 * Files: file_system.h
 */
int test_dir(){
	int8_t filename[35] = ".";
	
	uint8_t buff[33];
	int32_t bytes_read;  

	if(dir_open(filename) == -1){
		
		return FAIL;
	}

	output_dentry_filename(dentry_temp);

	if(dir_write(0, buff, bytes_read) != -1){
		return FAIL;
	}

	if(dir_close(0) == 0){
		return PASS;
	}
	return FAIL;

}

/* int test_dir_content()
 * 
 * Test to print directory content
 * Inputs: None
 * Outputs: Printed directory content
 * Side Effects: None
 * Files: file_system.h
 */
int test_dir_content(){
	int8_t filename[35] = ".";
	
	uint8_t buff[33];
	int32_t bytes_read;  
	int i;

	if(dir_open(filename) == -1){
		
		return FAIL;
	}

	for(i=0;i<15;i++){
		bytes_read = dir_read(0, buff, 32);
		printf("%s\n", buff);
	}	
	
	if(dir_write(0, buff, bytes_read) != -1){
		return FAIL;
	}

	if(dir_close(0) == 0){
		return PASS;
	}
	return FAIL;

}


/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	//TEST_OUTPUT("idt_test", idt_test());
	//TEST_OUTPUT("idt_divison_test", idt_divison_test());	
	//TEST_OUTPUT("System Call Test", syscall_test());
	//TEST_OUTPUT("Paging test 0", paging_test_kernel_and_vm());
	//TEST_OUTPUT("Paging test 1", paging_test_not_pres1());
	//TEST_OUTPUT("Paging test 2", paging_test_not_pres2());
	//TEST_OUTPUT("Paging test 3", paging_test_not_pres3());
	//TEST_OUTPUT("Paging test 4", paging_test_not_pres4());
	
	// launch your tests here

	// TEST_OUTPUT("terminal_test 1", terminal_test1());
	TEST_OUTPUT("terminal_test 2", terminal_test2());
	//TEST_OUTPUT("terminal_test 3", terminal_test3());

	//TEST_OUTPUT("RTC_write_read", RTC_write_read());

	//TEST_OUTPUT("Print File Sizes Test", test_file_size());
	// TEST_OUTPUT("Print File Content Test", test_file_content());
}
