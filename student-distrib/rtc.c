#include "rtc.h"
#include "i8259.h"
#include "lib.h"
#include "syscall.h"
#include "terminal.h"
#include "scheduling.h"
#define RTC_PORT_DATA      0x70
#define RTC_PORT_COMMAND   0x71
#define RTC_IRQ_NUM    8
#define max_limit_freq     1024
#define min_limit_freq     2
#define mask_register      0x0F
#define mask_max_rate_register 15
#define mask_min_rate_register 3
#define bytes_limit        4
#define lower_bound        1
#define RTC_COUNT          3
uint32_t rtc_int_flag[3] = {0, 0, 0};
uint32_t rtc_count[3] = {0, 0, 0};
uint32_t rtc_active[3] = {0, 0, 0};
int32_t rtc_reset_count = 0;
extern int active_term;

void rtc_register(int32_t freq);

/* void rtc_init()
 * 
 * Initialize the rtc 
 * Inputs: None
 * Outputs: None
 * Side Effects: helps initializing the rtc and enables IRQ 8
 * Files: rtc.h
 */
void rtc_init()
{
    char readVal;
    outb(0x8B, RTC_PORT_DATA);      // disable NMIs and selects register B
    readVal = inb(RTC_PORT_COMMAND); //read the current value of register B
    outb(0x8B, RTC_PORT_DATA);  //set the index
    outb(readVal | 0x40, RTC_PORT_COMMAND); //write the previous value OR with 0x40

    // when the rtc is opend, used the initialized settings
    rtc_count[active_term] = max_limit_freq/min_limit_freq;
    //save the value to reuse
    rtc_reset_count = rtc_count[active_term];

    rtc_register(max_limit_freq);

    //enable IRQ 8 on the PIC
    enable_irq(RTC_IRQ_NUM); 
    
}

/* void rtc_handler()
 * 
 * Defines the rtc handler
 * Inputs: None
 * Outputs: None
 * Side Effects: Calls the test_interrupts function and reads from register C
 * Files: rtc.h
 */
void rtc_handler()
{
    int l;
    outb(0x0C, RTC_PORT_DATA); //select register C
    inb(RTC_PORT_COMMAND); //throw away contents
    send_eoi(RTC_IRQ_NUM); //send EOI signal
    // test_interrupts(); 
    for(l=0; l< RTC_COUNT; l++)
    {
        if(rtc_active[l] == 1)
        {
            rtc_count[l] --; 
            if (!rtc_count[l])
            {
                rtc_count[l] = rtc_reset_count;
                
                rtc_int_flag[l] = 1;
            }
        }
    }
    
}

/* int32_t rtc_write()
 * Write the RTC interrupt rate
 * Inputs: fd     - File descriptor number
 *         buf    - The pointer to the passed data
 *         nbytes - The number of bytes the data is
 * Outputs:  0 - if succeed
 *          -1 - if fail
 * Side Effects: Changes the RTC interrupt rate accordingly
 * Files: rtc.h
 */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes)
{
    uint32_t freq;
    // check if the buf exist or not
    if (buf == NULL && nbytes != bytes_limit)
    {
        return -1;
    }
    
    // getting the frequency
    freq = *((uint32_t*)buf);
 
    // check if the freq passed is a power of 2 and within the allowed range
    if (freq  > max_limit_freq || freq < min_limit_freq || (freq & (freq-lower_bound)) != 0)
    {
        return -1;
    }
    // if pass all of the, change the frequency
    rtc_reset_count = max_limit_freq/freq;
    return 0;
}

/* int32_t rtc_read()
 * Wait for interrupt to occur
 * Inputs: fd     - File descriptor number
 *         buf    - The pointer to the passed data
 *         nbytes - The number of bytes of the passed data
 * Outputs:  0 - if succeed
 *          -1 - if fail
 * Side Effects: Block until the interrupt flag is true
 * Files: rtc.h
 */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes)
{
    if(rtc_active[active_term])
        rtc_int_flag[active_term] = 0; 
    // wait until the interrupt flag is changed to 1 in write
    while(!rtc_int_flag[active_term]);
    return 0;
}

/* int32_t rtc_open()
 * Sets the rtc counter
 * Inputs: filename - a string of the filename
 * Outputs:  0 - if succeed
 *          -1 - if fail
 * Side Effects: Opens and initialize the RTC device
 * Files: rtc.h
 */
int32_t rtc_open(const uint8_t* filename)
{
    rtc_active[active_term] = 1;
    rtc_init();
    return 0;
}
 
 /*int32_t rtc_close()
 * Closes the RTC
 * Inputs: fd - File descriptor number 
 * Outputs:  0 - if succeed
 *          -1 - if fail
 * Side Effects: Calls the test_interrupts function and reads from register C
 * Files: rtc.h
 */
int32_t rtc_close(int32_t fd)
{
    rtc_active[active_term] = 0;
    return 0;
}

/* void rtc_registor()
 * Change the frequency
 * Inputs: freq - the frequency that is wanted to be changed
 * Outputs: None
 * Side Effects: Finds the rate based on freq and changes the rtc accordingly
 * Files: rtc.h
 */
void rtc_register(int32_t freq)
{
    char readVal2;
    char rate = mask_max_rate_register + lower_bound;
    int32_t power = 0;
    // finds what the freq is in terms of 2 to the N
    while(freq != lower_bound)
    {
        freq /= min_limit_freq;
        power ++;
    }
    // once the power is found, find the right code for register A
    rate -= power;
    rate &= mask_register;
    // check if the rate is too small
    if(rate < mask_min_rate_register)
    {
        return;
    }
    
    cli();
    outb(0x8A, RTC_PORT_DATA);      // disable NMIs and set index to register A
    readVal2 = inb(RTC_PORT_COMMAND); //get initial value of register A
    outb(0x8A, RTC_PORT_DATA);  //reset index to register A
    outb((readVal2 & 0xF0) | rate, RTC_PORT_COMMAND); //write only bottom 4 bits of the rate to A using 0xF0
    sti();
}
