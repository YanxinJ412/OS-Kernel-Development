

#define PIT_CMD     0x43
#define PIT_DATA    0x40
#define INIT_FREQ   1193180
#define LOW_BIT_MASK 0xFF
#define PIT_IRQ      0 
#define PIT_MODE_3  0x36 
#define MASK_PIT_NUM 8
#define FREQ_PIT_NUM 100




/*initializing the pit functions */
void pit_init(); 
void pit_handler(); 
