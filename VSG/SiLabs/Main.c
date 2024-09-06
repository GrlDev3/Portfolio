#include <CPU_init.h>
#include <SysTick.h>
#include <Util.h>
#include <UART.h>
#include <SMBus.h>
#include "FlashPrimitives.h"

void main (void) 
{

	SFRPAGE = CONFIG_PAGE;
	WDTCN = 0xDE;                       // Disable watchdog timer
    WDTCN = 0xAD;
	Free_I2C();

	EA = 0;
	
	
	Oscillator_Init ();
	Timer0_Init ();
	Port_Init();	
	UART0_Init();
	UART1_Init();
	SMBus_Init();
	
	Free_I2C();
	LEDs_Off ();
	//SysTimer_Init ();
	Prg_Init();
	//EA = 1;
	memset(&ADC_MIN, 0x0078, 6);
	InitValueFromFlash();
	SMBTimedOut=1;
	EA = 1;
	while(1)
	{		
		if(~SB_BUSY)
		{
			if (Out_Ptr==6)
			{
				SB_BUSY=1;
				Check_Phase();
				Check_DetInp();	
				if (adc_MeasPtr >= ADC_BUF_SIZE-1)
				{	
					SERVICE_SEND=1;	
					adc_MeasPtr = 0;
				}
				else
				{
					adc_MeasPtr++;
				}
				if (ph_MeasPtr >= PH_BUF_SIZE-1)
				{	
					ph_MeasPtr = 0;
				}
				else
				{
					ph_MeasPtr++;
				}
				Out_Ptr=0x00;
				SB_BUSY=0;
				SB_STATUS=NONE;
			}
			else
			{
				if (SB_STATUS==SENDREQ)
				{
					SB_BUSY=1;
					ADC_RequestSend();
				}
				else if(SB_STATUS==READREQ)
				{
					SB_BUSY=1;
					ADC_RequestGet();
				}
				else if(SB_STATUS==MEAS)
				{
					SB_BUSY=1;
					ADC_ReadMeas();
				}
				else if(SB_STATUS==NONE)
				{
					if (SERV_JMP==0)
					{
						if ((sendingToCPS==0)&&(packet_ctr>=2))
						{
							SERVICE_SEND=0;
							packet_ctr=0;	
							PrepForSend();
							Create_Packet0();
						}
						else if (SERVICE_SEND==1)//(CPSsend == 0) 
						{
							SERVICE_SEND=0;
							packet_ctr++;
						}
					}
				}
			}
		}


		if (_testbit_(Pack_ID_Ok))
		{
			UART1_Check();
		}

		if (CPSsend == 1)
		{
			CPSsend = 0;
			sendingToCPS=1;
			Create_Packet1();
		}

		if (_testbit_(b_rd_U0))
		{
			UART0_Check();
		}
		
		if((flash_enable_timer > 0) && (sendingToCPS == 0))
		{
			if(_testbit_(b_write))
			{
				WriteValueToFlash(1);	//write adc_max
				WriteValueToFlash(0);	//write adc_min
				b_debug = 1;
			}
			else if(_testbit_(b_debug))
			{
				InitValueFromFlash();
			}
		}

	}
}
