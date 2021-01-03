extern void PUT32 ( unsigned int, unsigned int );
extern unsigned int GET32 ( unsigned int );
extern void dummy ( unsigned int );

#define ARM_TIMER_LOD 0x2000B400
#define ARM_TIMER_VAL 0x2000B404
#define ARM_TIMER_CTL 0x2000B408
#define ARM_TIMER_DIV 0x2000B41C
#define ARM_TIMER_CNT 0x2000B420

#define PI_IOBASE_ADDR 0x20000000
#define SYSTIMERCLO 0x20003004
#define GPFSEL1 0x20200004
#define GPSET0  0x2020001C
#define GPCLR0  0x20200028
#define GPFSEL3 0x2020000C
#define GPFSEL4 0x20200010
#define GPSET1  0x20200020
#define GPCLR1  0x2020002C

#define TIMEOUT 4000000

#define BITDEPTH 2

struct DAC {
	unsigned int pins[BITDEPTH];
	unsigned int output;
};

struct DAC dac = {
	{ 47, 35 },
	0
};

typedef enum {
	GPIO_INPUT = 0b000,									// 0
	GPIO_OUTPUT = 0b001,								// 1
	GPIO_ALTFUNC5 = 0b010,								// 2
	GPIO_ALTFUNC4 = 0b011,								// 3
	GPIO_ALTFUNC0 = 0b100,								// 4
	GPIO_ALTFUNC1 = 0b101,								// 5
	GPIO_ALTFUNC2 = 0b110,								// 6
	GPIO_ALTFUNC3 = 0b111,								// 7
} GPIOMODE;

int gpio_setup (unsigned int gpio, GPIOMODE mode) 
{
    unsigned int* GPFSEL  = (unsigned int*) (PI_IOBASE_ADDR + 0x200000 + 0x0);
	if (gpio > 54) return -1;						// Check GPIO pin number valid, return false if invalid
	if (mode < 0 || mode > GPIO_ALTFUNC3) return -1;	// Check requested mode is valid, return false if invalid
	unsigned int bit = ((gpio % 10) * 3);				// Create bit mask
	unsigned int mem = GPFSEL[gpio / 10];				// Read register
	mem &= ~(7 << bit);									// Clear GPIO mode bits for that port
	mem |= (mode << bit);								// Logical OR GPIO mode bits
	GPFSEL[gpio / 10] = mem;							// Write value to register
	return 0;
}

void init_DAC() {
	int i;
	for(i = BITDEPTH; i > 0; i--) {
		gpio_setup(dac.pins[i-1],GPIO_OUTPUT);
	}
}


void output_DAC() {
	int i;
	unsigned int output;
	unsigned int mask;
	
	mask = 1;
	for(i = BITDEPTH; i > 0; i--) {
		output = (dac.output & mask) ? GPSET1 : GPCLR1;
		PUT32(output,1<<(dac.pins[i-1]-32));
		mask <<= 1;
	}
}

//-------------------------------------------------------------------------
int notmain ( void )
{
    unsigned int ra;
    unsigned int rb;
	
	init_DAC();
	
    PUT32(ARM_TIMER_CTL,0x00F90000);
    PUT32(ARM_TIMER_CTL,0x00F90200);

    rb=GET32(ARM_TIMER_CNT);
    while(1)
    {
		output_DAC();
		dac.output++;
		if(dac.output > 0b11) dac.output = 0;
        while(1)
        {
            ra=GET32(ARM_TIMER_CNT);
            if((ra-rb)>=TIMEOUT) break;
        }
        rb+=TIMEOUT;
    }
    return(0);
}