#include <CPU_init.h>
#include <SysTick.h>
#include <Util.h>
#include <UART.h>

U16 flash_enable_timer;				
U16 delay_cntr;
U16 mstimer;
U8 ctr;	
//U8 hiba;


void Timer0_int(void) interrupt 1
{

	TR0 = 0;          //stop timer0

	//TL0       = 0x77; 	//200Hz/ch
    //TH0       = 0x47;
	//TL0       = 0xC9; 	//500Hz/ch
    //TH0       = 0x4F;
    //TL0       = 0x3F; 	// 1ms
    //TH0       = 0x3F;
	//TL0       = 0xD5; 	// 278us
    //TH0       = 0xCA;	
	//TL0       = 0x14; 		// 250us
    //TH0       = 0xD0;
	//TL0       = 0xEF; 		// 4900Hz (700Hz/ch)
    //TH0       = 0xD8;
	TL0       = 0x8B; 		// 5250Hz (750Hz/ch)
    TH0       = 0xD8;
	//TL0       = 0xC0; 		// 5600Hz (800Hz/ch) NEM MEGY!!!!
    //TH0       = 0xDD;


	if (SB_STATUS==NONE)
	{
		SB_STATUS=SENDREQ;
	}
	else
	{
		if (BUSY)
		{
			SMBTimedOut=0;
			//ctr=0;
		}
		/*hiba++;
		if (hiba>=10)
		{
			hiba=0;
		}*/
		//SB_STATUS=SENDREQ;
	}
	

	if ((UART1_Buffer_Size>0)&&(Pack_ID_Ok==0))
	{
		thisVSGtimeout++;
		if (thisVSGtimeout>=THIS_VSG_TIMEOUT)
		{
			UART1_Buffer_Size=0;
			thisVSGtimeout=0;
		}
	}

	if (waitingNextVSG==1)
	{
		nextVSGtimeout++;
		if (nextVSGtimeout>=NEXT_VSG_TIMEOUT)
		{
			nextVSGtimeout=0;
			waitingNextVSG=0;
		}
	}
	
	delay_cntr++;
	if (delay_cntr > 380) 
	{
		delay_cntr = 0;
	}

	if(flash_enable_timer > 0)
		flash_enable_timer--;

	if (++mstimer>5000)
	{
		mstimer=0;
		if ((err_blink_ok==0)&&((~LED_REQUESTED) & 0x80))
		{
			if (LED_REQUESTED & 0x80)
			{
				LED_REQUESTED &= 0x7F;
			}
			else
			{
				LED_REQUESTED |= 0x80;	
			}
			LEDs_Off();
		}
		err_blink_ok=0;
	}
	
	TF0 = 0;	  //Timer 0 Overflow Flag
	TR0 = 1;
}

