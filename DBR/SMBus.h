#include <compiler_defs.h>

#define  SMB_IN_SIZE    2

#define  ADC 			0x90		




#define  SMB_MTSTA      0x08           // (MT) start transmitted
#define  SMB_MTDB       0x18           // (MT) data byte transmitted
#define  SMB_MTLD       0x28
#define  SMB_MRAA       0x40
#define  SMB_MRDA       0x50
#define  SMB_MRLD		0x58

#define  NUM_BYTES_WR	0x02
#define  NUM_BYTES_RD	0x02

#define  WRITE          0x00           // SMBus WRITE command
#define  READ           0x01           // SMBus READ command





extern xdata volatile U8 SMB_DATA_IN[SMB_IN_SIZE];
extern xdata volatile U16 CH; 

extern bit SB_OK;


extern void SMBus_ISR(void);
extern void Timer3_ISR(void);
extern void Free_I2C (void);
