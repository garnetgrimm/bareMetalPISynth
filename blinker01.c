extern void PUT32 ( unsigned int, unsigned int );
extern unsigned int GET32 ( unsigned int );
extern void dummy ( unsigned int );

#define ARM_TIMER_LOD 0x2000B400
#define ARM_TIMER_VAL 0x2000B404
#define ARM_TIMER_CTL 0x2000B408
#define ARM_TIMER_DIV 0x2000B41C
#define ARM_TIMER_CNT 0x2000B420

#define SYSTIMERCLO 0x20003004
#define GPFSEL1 0x20200004
#define GPSET0  0x2020001C
#define GPCLR0  0x20200028
#define GPFSEL3 0x2020000C
#define GPFSEL4 0x20200010
#define GPSET1  0x20200020
#define GPCLR1  0x2020002C

//#define TIMEOUT 20000000
#define TIMEOUT 4000000
#define BITDEPTH 2

struct DAC {
	unsigned int pins[BITDEPTH];
	unsigned int output;
};

struct DAC dac = {
	{ 47, 35 },
	0b01
};


void outputDAC() {
	unsigned int i;
	unsigned int output;
	unsigned int mask;
	
	mask = 1;
	for(i = BITDEPTH-1; i > 0; i--) {
		output = (dac.output & mask) ? GPSET0 : GPCLR1;
		PUT32(GPSET0,1<<(dac.pins[i]-32));
		mask <<= 1;
	}
}

//-------------------------------------------------------------------------
int notmain ( void )
{
    unsigned int ra;
    unsigned int rb;

    ra=GET32(GPFSEL4);
    ra&=~(7<<21);
    ra|=1<<21;
    PUT32(GPFSEL4,ra);

    ra=GET32(GPFSEL3);
    ra&=~(7<<15);
    ra|=1<<15;
    PUT32(GPFSEL3,ra);

    PUT32(ARM_TIMER_CTL,0x00F90000);
    PUT32(ARM_TIMER_CTL,0x00F90200);

    rb=GET32(ARM_TIMER_CNT);
    while(1)
    {
        dac.output = ~dac.output;
		outputDAC();
        while(1)
        {
            ra=GET32(ARM_TIMER_CNT);
            if((ra-rb)>=TIMEOUT) break;
        }
        rb+=TIMEOUT;
    }
    return(0);
}