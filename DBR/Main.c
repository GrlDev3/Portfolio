#include <CPU_init.h>
#include <SysTick.h>
#include <Util.h>
#include <UART.h>
#include <SMBus.h>
#include "FlashPrimitives.h"

/*
Version: 1.0.4.5
*/

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
	SPI0_Init();
	//Free_I2C();
	//SysTimer_Init ();
	Init_Values();


	EA = 1;
	while(1)
	{
		
		if(_testbit_(measure))
		{
		
			READ_INPUT();

			if(PROG_MODE == 2 || PROG_MODE == 3)
			{
				if(_testbit_(process))
					Process_Input();
			}
			else
				Process_Input();

		}
		
		if(_testbit_(blink))
		{
			if(color != YELLOW && sectimer < blink_value)
			{
				en_disp_out = !en_disp_out;
				P3 = 0x00;
				_nop_();
				DISPLAY_OFF ();
			}
		}
		
		if(_testbit_(disable_out))
		{
			P3 = 0x00;
			en_disp_out = 0;
			DISPLAY_OFF ();
			//if(PROG_MODE == 1) sectimer = 0;
			//Create_Packet1(0x46);
		}

		if(en_disp_out)
		{
			if (_testbit_(enable_out))
				write_DISPLAY();

		}
		

			if(r_color_cnt == 1)//?
			{
				P3 = 0x00;
				_nop_();
				DISPLAY_OFF ();
				r_color_cnt = 0;
			}

			if(g_color_cnt == 1)
			{
				if(yellow_cnt)
				{
					Display_Green_OFF();
				}
				else
				{
					P3 = 0x00;
					DISPLAY_OFF ();
				}
				g_color_cnt = 0;
			}


		if (_testbit_(b_rd_U0))
		{
			UART0_Check();
		}


	}//END OF WHILE
}
