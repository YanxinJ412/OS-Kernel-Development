#include "i8259.h"
#ifndef RTC_H
#define RTC_H

/*Initialize the RTC*/
void rtc_init(); 
/*Define the RTC handler*/
void rtc_handler(); 

/* writes rtc frequency when the interrupt occurs */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);

/* blocks the rtc until a interrupt is read */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);

/* opens the rtc device */
int32_t rtc_open(const uint8_t* filename);

/* closes the rtc device */
int32_t rtc_close(int32_t fd);

#endif 
