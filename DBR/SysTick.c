#include <CPU_init.h>
#include <SysTick.h>
#include <Util.h>
#include <UART.h>

struct clock red_in_time;
struct clock int_in_time;
struct clock clock;

bit measure;
bit process;
bit blink;

U8 sectimer = 0;
U8 packTimer;

xdata volatile U8 disp_out_cnt;
xdata volatile U8 r_color_cnt;
xdata volatile U8 g_color_cnt;


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
	TL0       = 0xF0; 			// 5250Hz (750Hz/ch)   0X8B		
    TH0       = 0xEC;							// 0XD9
	//TL0       = 0xC0; 		// 5600Hz (800Hz/ch) NEM MEGY!!!!
    //TH0       = 0xDD;

	measure = 1;
	//LED=!LED;

	//0.1ms | 100us
	if (--disp_out_cnt <= 1 ) 
	{
		disp_out_cnt = brightness;
		enable_out = 1;
		//ide kell egy en_bit ami bekapcsolja a displayt
	}
	else if (disp_out_cnt == 2)
	{
		if(r_color_cnt > 2)
			r_color_cnt = 2;

		if(g_color_cnt > 2)
			g_color_cnt = 2;
	}

	if (r_color_cnt > 1)
		r_color_cnt--;

	if (g_color_cnt > 1)
		g_color_cnt--;

	if(packTimer <= 20)
		packTimer++;
	else
	{
		if(UART0_RI_ptr > 0 && b_rd_U0 == 0)
		{
			UART0_RI_ptr = 0;
		}
	}

	clock.uSEC++;
	if (clock.uSEC >= 10) // -> 1ms
	{
		clock.mSEC++;
		clock.uSEC = 0;

		if (clock.mSEC % 500 == 0)	//every 500 ms
		{
			if(error_cnt <= 2000);
				LED=!LED;
			
			if (blink_en)
			{
				if(PROG_MODE == 3)
				{
					if(cnt_enable)
						blink = 1;
				}
				else
					blink = 1;
			}
		}

		if(clock.mSEC % 50 == 0)	//every 50 ms
		{
			process = 1;
		}

		if (clock.mSEC >= 1000) //-> 1sec	
		{
//			clock.SEC++;
			clock.mSEC = 0;

			if(PROG_MODE == 1)
			{
				if (sectimer > 1)
				{
					sectimer--;
					//Create_Packet1(sectimer);
				}
				else
				{
					P3 = 0x00;
					sectimer = 0;
					DISPLAY_OFF ();


					if(color == 2)
					{
						if(red_data_num)
						{
							sectimer = RED_IN_VALUES[1];//(RED_IN_VALUES[2] + (RED_IN_VALUES[3]*60));
							sectimer -= protect_time;
						}
						else
						{
							sectimer = RED_IN_VALUES[0];//(RED_IN_VALUES[0] + (RED_IN_VALUES[1]*60));
							sectimer -= protect_time;
						}
						color = RED;
					}
				}
			
			}
			else if(PROG_MODE == 2)
			{
				if (sectimer > 1)
				{
					sectimer--;

					//if(en_disp_out)
						//Create_Packet1(sectimer);
				}
				else
				{
					P3 = 0x00;
					sectimer = 0;
					DISPLAY_OFF ();

					if(color == 2)
					{
						sectimer = red_max_time - protect_time;
						disable_out = 1;
						//Create_Packet1(0x47);
						color = RED;
					}
				}

			}
			else if(PROG_MODE == 3)
			{
				if(cnt_enable)
				{
					if (sectimer > 1)
					{
						sectimer--;
						//if(en_disp_out)
							//Create_Packet1(sectimer);
					}
					else
					{
						P3 = 0x00;
						sectimer = 0;
						DISPLAY_OFF ();

						if(color == 2)
						{
							sectimer = red_max_time - protect_time - 1;
							disable_out = 1;
							//Create_Packet1(0x47);
							color = RED;
						}
					}
				}
			}
			

	/*		if(clock.SEC >= 60) //-> 1min
			{
				clock.MIN++;
				clock.SEC = 0;


				if(clock.MIN >= 60)	//-> 1 hour
					clock.MIN = 0;
			}*/
		}
	}

	if(PROG_MODE == 1)
	{
		if(RED_IN)			//RED_IN
		{
			red_in_time.uSEC++;
			if (red_in_time.uSEC >= 10) // -> 1msec
			{
				red_in_time.mSEC++;
				red_in_time.uSEC = 0;

				if (red_in_time.mSEC >= 1000) //-> 1sec
				{
					red_in_time.SEC++;
					red_in_time.mSEC = 0;

					if(red_in_time.SEC >= 240) //-> 4min
					{
//						red_in_time.MIN++;
						red_in_time.SEC = 0;
						error_cnt = 2001;//ezt nem kene hogy elerje
						P3 = 0x00;
						DISPLAY_OFF ();
//						if(red_in_time.MIN >= 60)	//-> 1 hour
//							red_in_time.MIN = 0;
					}
				}
			}
		}
			
			
		if(!RED_IN) //|| INT_IN
		{
			int_in_time.uSEC++;
			if (int_in_time.uSEC >= 10) // -> 1msec
			{
				int_in_time.mSEC++;
				int_in_time.uSEC = 0;

				if (int_in_time.mSEC >= 1000) //-> 1sec
				{
					int_in_time.SEC++;
					int_in_time.mSEC = 0;

					if(int_in_time.SEC >= 240) //-> 4min
					{
//						int_in_time.MIN++;
						int_in_time.SEC = 0;
						error_cnt = 2001;//ezt nem kene hogy elerje
						P3 = 0x00;
						DISPLAY_OFF ();

//						if(int_in_time.MIN >= 60)	//-> 1 hour
//							int_in_time.MIN = 0;
					}
				}
			}
		}

	}//endif(PROG_MODE == 1)
	else if ((PROG_MODE == 2) && (init_time != init_offset))
	{
		int_in_time.uSEC++;
		if (int_in_time.uSEC >= 10) // -> 1msec
		{
			int_in_time.mSEC++;
			int_in_time.uSEC = 0;

			if (int_in_time.mSEC >= 1000) //-> 1sec
			{
				int_in_time.mSEC = 0;

				if(offset_cnt > 1)
				{
					offset_cnt--;
				}
				else
				{
					if(offset_cnt == 1)
					{
						offset_en = 1;
						offset_cnt--;
					}
				}
			}
		}
	}//endif(PROG_MODE == 2)
	




	
	TF0 = 0;	  //Timer 0 Overflow Flag
	TR0 = 1;
}


void Clock_reset(char clk)
{
	switch(clk)
	{
		case 1:
			red_in_time.uSEC = 0;
			red_in_time.mSEC = 0;
			red_in_time.SEC = 0;
//			red_in_time.MIN = 0;
		break;
		case 2:
			int_in_time.uSEC = 0;
			int_in_time.mSEC = 0;
			int_in_time.SEC = 0;
//			int_in_time.MIN = 0;
		break;
		case 3:
			clock.uSEC = 0;
			clock.mSEC = 0;
//			clock.SEC = 0;
//			clock.MIN = 0;
		break;

	}
}