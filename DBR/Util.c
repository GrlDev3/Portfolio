#include <CPU_init.h>
#include <SysTick.h>
#include <Util.h>
#include <UART.h>
#include <SMBus.h>
#include <FlashPrimitives.h>
#include <stdlib.h>

bit SB_BUSY;
bit INPUT_1;
bit INPUT_2;
bit INPUT_3;

xdata unsigned int error_cnt = 0;

bit RED_IN_PREV = 0;
bit INT_IN_PREV = 0;
bit TAST_IN_PREV = 0;

xdata unsigned char PROG_MODE;

xdata unsigned char change_on_input = 0;
/*
^7 - red_in value: off & tast_in value:on	(0x80 | 0x7F)	//hasznalatban
^6 - int_in value:off						(0x40 | 0xBF)
^5 - int_in value:on						(0x20 | 0xDF)	//hasznalatban
^4 - tast_in value:off						(0x10 | 0xEF)
^3 - tast_in value:on						(0x08 | 0xF7)	//hasznalatban; de nem igy
^2 - red_in value:off						(0x04 | 0xFB)	//hasznalatban
^1 - red_in value:on						(0x02 | 0xFD)	//hasznalatban
^0 - change on red_in						(0x01 | 0xFE)	//hasznalatban
*/
xdata volatile unsigned char out_equal;
xdata volatile unsigned char red_equal = 0;
xdata volatile unsigned char int_equal = 0;
xdata unsigned char protect_time;
xdata unsigned char allowed_green_diff;
xdata unsigned char allowed_red_diff;

xdata unsigned char init_time;	//time until green
xdata unsigned char init_offset;	//vozac stop
xdata unsigned char red_max_time;
xdata unsigned char green_time;
xdata unsigned char offset_cnt;
xdata unsigned char stop_time;
xdata unsigned char blink_value;
xdata unsigned char r_g_depend;

bit RED_IN = 0;
bit TAST_IN;
bit INT_IN = 0;

bit tast_in_value = 0;

bit en_disp_out = 0;
bit disable_out = 0;
bit enable_out;
bit red_data_num;
bit int_data_num;
bit after_green = 0;
bit blink_en = 0;
bit cnt_enable = 0;
bit offset_en = 0;
bit clear_tast_in = 0;
bit yellow_cnt;

xdata unsigned char brightness;
xdata unsigned char color;

xdata volatile unsigned char RED_IN_VALUES[2];//[4];	//2 adat x (sec, min)
xdata volatile unsigned char INT_IN_VALUES[2];//[4];
xdata volatile unsigned int input1;
xdata volatile unsigned int input2;
xdata volatile unsigned int input3;

void READ_INPUT(void)
{
	RED_IN_PREV = RED_IN;
	INT_IN_PREV = INT_IN;
	TAST_IN_PREV = TAST_IN;
	OE_INT = 0;
	_nop_();
	_nop_();
		
	P3=0xFF;
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();

	OE_INT = 0;
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	
	INPUT_2=IN2;
	INPUT_3=IN3;
	INPUT_1=IN1;
	_nop_();
	OE_INT = 1;

	if (_testbit_(INPUT_1))
	{
		if (input1<=1000)
			input1++;
	}
	else 
	{
		if (input1>0)
			input1--;
	}


	if (_testbit_(INPUT_2))
	{
		if (input2<=1000)
			input2++;
	}
	else 
	{
		if (input2>0)
			input2--;
	}

	if (_testbit_(INPUT_3))
	{
		if (input3<=1000)
			input3++;
	}
	else 
	{
		if (input3>0)
			input3--;
	}


	if (input1>300)
		RED_IN=1;	//crvena
	else
		RED_IN=0;

	if (input2>300)
		TAST_IN=1;	//najava
	else
		TAST_IN=0;

	if (input3>300)
		INT_IN=1;	//init/zelena
	else
		INT_IN=0;


	if(RED_IN_PREV != RED_IN)
	{
		change_on_input |= 0x01;
		if (RED_IN)
		{
			if(PROG_MODE != 1)
				change_on_input &= 0xFB; 
			change_on_input |= 0x02;	//on
		}
		else
			change_on_input |= 0x04;   	//off

		
	}

	if(RED_IN)
	{
		if(	TAST_IN_PREV != TAST_IN)
		{
			if (TAST_IN)
			{
				change_on_input &= 0xEF;
				tast_in_value = 1;//change_on_input |= 0x08;	//on
			}
			else
				change_on_input |= 0x10;   	//off

		
		}
	}

	if(	INT_IN_PREV != INT_IN)
	{
		if (INT_IN)
		{
			change_on_input &= 0xBF;
			change_on_input |= 0x20;	//on
		}
		else
			change_on_input |= 0x40;
		
	}
	
	if((tast_in_value) && (!RED_IN))
	{
		change_on_input |= 0x80;
	}

	//LED=!tast_in_value;

}

void Process_Input(void)
{

int tmp_1 = 0, tmp_2 = 0;	//secben tarolja az red_in jel idejet
char debug = 0;
unsigned char tmp_init = 0;

	switch (PROG_MODE)
	{
		case 1:
			if(RED_IN && INT_IN) 	//ha egyszerre van piros es zold
			{
				error_cnt++;
				P3 = 0x00;
				_nop_();
				DISPLAY_OFF ();
			}
			else
			{	
				error_cnt = 0;
				if(change_on_input & 0x01)
				{
					change_on_input &= 0xFE;
					if(change_on_input & 0x02) //r:01 > g:00|10|11
					{
						change_on_input &= 0xFD;

							//green
						if (int_data_num)
						{
							if(int_in_time.mSEC > 600)
								INT_IN_VALUES[0]=int_in_time.SEC+1;
							else
								INT_IN_VALUES[0]=int_in_time.SEC;
							//INT_IN_VALUES[1]=int_in_time.MIN;
						}
						else
						{
							if(int_in_time.mSEC > 600)
								INT_IN_VALUES[1]=int_in_time.SEC+1;//2
							else
								INT_IN_VALUES[1]=int_in_time.SEC;//2
							//INT_IN_VALUES[3]=int_in_time.MIN;
						}

						int_data_num = !int_data_num;

							//osszehasonlitas:
						tmp_1 = INT_IN_VALUES[0];// + (INT_IN_VALUES[1]*60);
						tmp_2 = INT_IN_VALUES[1];//[2] + (INT_IN_VALUES[3]*60);

						if(tmp_1 > 0 && tmp_2 > 0)
						{
							if((tmp_1 == tmp_2) || (abs(tmp_1 - tmp_2) <= allowed_green_diff))
							{
									//out ok
								if(int_equal < 150)
									int_equal++;
							}
							else
							{
								int_equal = 0;
							}
						}
						else
						{
							int_equal = 0;
						}


							//red
						Clock_reset(1);	//red_in_time
						Clock_reset(3);	//clock
		//				if(out_equal >= 1)
						
						if(r_g_depend) //bovites eseten maszkolni
						{
							if(red_equal > 0 && int_equal > 0)	//int_equal lehet 1-nel nagyobbnak kell mar lennie
							{
								en_disp_out = 1;
								sectimer = protect_time;
								color = YELLOW;
							}
							else
							{
								disable_out = 1;
							}
						}
						else
						{
							if(red_equal > 0)
							{
								en_disp_out = 1;
								sectimer = protect_time;
								color = YELLOW;
							}
							else
							{
								disable_out = 1;
							}
						}
					}
					else if(change_on_input & 0x04)	//r:10 > g:00|01|11
					{
						change_on_input &= 0xFB;

							//red
						if (red_data_num)
						{
							if(red_in_time.mSEC > 600)
								RED_IN_VALUES[0]=red_in_time.SEC+1;
							else
								RED_IN_VALUES[0]=red_in_time.SEC;
							//RED_IN_VALUES[1]=red_in_time.MIN;
						}
						else
						{
							if(red_in_time.mSEC > 600)
								RED_IN_VALUES[1]=red_in_time.SEC+1;//2
							else
								RED_IN_VALUES[1]=red_in_time.SEC;//2
							//RED_IN_VALUES[3]=red_in_time.MIN;
						}

						red_data_num = !red_data_num;

							//osszehasonlitas:
						tmp_1 = RED_IN_VALUES[0];// + (RED_IN_VALUES[1]*60);
						tmp_2 = RED_IN_VALUES[1];//[2] + (RED_IN_VALUES[3]*60);

						if(tmp_1 > 0 && tmp_2 > 0)
						{
							if((tmp_1 == tmp_2) || (abs(tmp_1 - tmp_2) <= allowed_red_diff))
							{
									//out ok
								if(red_equal < 150)
									red_equal++;
								//Create_Packet1(0xD2);
							}
							else
							{
								red_equal = 0;
								//Create_Packet1(0xD3);
							}
						}
						else
						{
							red_equal = 0;
						}


						//green
						Clock_reset(2);	//int_in_time
						Clock_reset(3);	//clock
		//				if(out_equal >= 1)

						if(r_g_depend) //bovites eseten maszkolni
						{
							if(red_equal > 0 && int_equal > 0)	//red_equal lehet 1-nel nagyobbnak kell mar lennie
							{
								en_disp_out = 1;
								if(int_data_num)
								{
									sectimer = INT_IN_VALUES[1];//[2] + (INT_IN_VALUES[3]*60);
								}
								else
								{
									sectimer = INT_IN_VALUES[0];// + (INT_IN_VALUES[1]*60);
								}
								color = GREEN;
								//Create_Packet1(0xD4);
							}
							else
							{
								disable_out = 1;
							//	DISPLAY_OFF ();
							} 
						}
						else
						{
							if(int_equal > 0)
							{
								en_disp_out = 1;
								if(int_data_num)
								{
									sectimer = INT_IN_VALUES[1];//[2] + (INT_IN_VALUES[3]*60);
								}
								else
								{
									sectimer = INT_IN_VALUES[0];// + (INT_IN_VALUES[1]*60);
								}
								color = GREEN;
								//Create_Packet1(0xD4);
							}
							else
							{
								disable_out = 1;
							//	DISPLAY_OFF ();
							} 
						}
					}
				}
		

			}


			if(error_cnt > 2000)
			{
				LED=ON;
			}
		break;
		//--------------------------------------------------------------------------------------------------
		case 2:
			if(change_on_input & 0x20)	//(INT_IN = 1 && INT_IN_PREV != INT_IN)
			{
				debug = change_on_input;
				change_on_input &= 0xDF;

				if(init_offset == init_time)
				{
					if(tast_in_value == 1)//change_on_input & 0x08)	//(TAST_IN = 1 && TAST_IN_PREV != TAST_IN)
					{
						//tast_in_value = 0;
						if(sectimer != init_time)
						{
							sectimer = init_time;
							Clock_reset(3);
							//en_disp_out = 1;
						}

					}
					else
					{
						sectimer = init_time + green_time + red_max_time;
						en_disp_out = 0;	//ez majd: 0
						Clock_reset(3);
					}

					offset_cnt = 0;
					offset_en = 0;
					color = RED;
					//Create_Packet1(debug);
				}
				else if(init_offset < init_time)
				{

					if(init_time <= red_max_time)
					{
						if(tast_in_value == 0)
						{
							en_disp_out = 0;
						}

						if(sectimer != init_time)
						{
							sectimer = init_time;
							Clock_reset(3);
						}

						color = RED;
						Clock_reset(3);
					}
					else
					{
						if((after_green ==  1) || (color == GREEN))
						{
							tmp_init = 0;
							tmp_init = green_time - (red_max_time + green_time - init_time);
							if (sectimer != tmp_init)
							{
								sectimer = tmp_init;
								Clock_reset(3);
							}
						}
						else
						{
							if(tast_in_value == 0)
							{
								en_disp_out = 0;
							}

							if(sectimer != init_time)
							{
								sectimer = init_time;
								Clock_reset(3);
							}

							color = RED;
						}

					}
					offset_cnt = init_time - init_offset;	//offset parancsok ezutan - de nem itt: if(offset_en == 1)
					Clock_reset(2);
					//Create_Packet1(debug);
				}
				else	//init_offset > init_time
				{
					if(tast_in_value == 0)
					{
						tmp_init = 0;
						tmp_init = green_time + red_max_time + init_time;
						if(sectimer != tmp_init)
						{
							sectimer = tmp_init; 
							Clock_reset(3);
						}
						en_disp_out = 0;
						color = RED;
					}
					else
					{
						if(_testbit_(clear_tast_in))
						{
							//tast_in_value = 0;
							if(sectimer != init_time)
							{
								sectimer = init_time;
								Clock_reset(3);
							}
						}
					}
					offset_cnt = green_time + red_max_time + init_time - init_offset;
					Clock_reset(2);
					//Create_Packet1(debug);
				}
			}


			if(_testbit_(offset_en) && (init_time != init_offset))
			{
				if(tast_in_value == 1)
				{
					if(sectimer != init_offset)
					{
						if(color != RED)
							color = RED;
						sectimer = init_offset;
						Clock_reset(3);
						//Create_Packet1(0x01);
					}
					clear_tast_in = 1;
				}
				else
				{
					tmp_init = 0;
					tmp_init = green_time + red_max_time + init_offset - 1;
					if(sectimer != tmp_init)
					{
						sectimer = tmp_init;
						Clock_reset(3);
						color = RED;
						//Create_Packet1(0x02);
					}
					en_disp_out = 0;
				}
			}

			if((tast_in_value == 1) && (sectimer > 4))//change_on_input & 0x08)	//(TAST_IN = 1 && TAST_IN_PREV != TAST_IN)
			{
				if(!en_disp_out)
				{
					en_disp_out = 1;	
				}

			}

			if(change_on_input & 0x01)
			{
				change_on_input &= 0xFE;
				if(change_on_input & 0x02)
				{
					change_on_input &= 0xFD;
					if(change_on_input & 0x80)
					{
						change_on_input &= 0x7F; //clear tast_in after green if it was set
						tast_in_value = 0;//change_on_input &= 0xF7;
					}

					if(after_green)
					{
						sectimer = protect_time;
						color = YELLOW;
						en_disp_out = 1;
						after_green = 0;
						Clock_reset(3);
						//Create_Packet1(0x90);
					}
				}
				else if(change_on_input & 0x04)
				{
					change_on_input &= 0xFB;
					tast_in_value = 0;//change_on_input &= 0xF7;
					if(!after_green)
					{
						sectimer = green_time;
						color = GREEN;
						en_disp_out = 1;
						after_green = 1;
						Clock_reset(3);
						//Create_Packet1(0x40);
					}
				}
			}

		break;
		//--------------------------------------------------------------------------------------------------
		case 3:
			if((sectimer == stop_time) && (color == RED))//!tast_in_value
				cnt_enable = 0;
	
			if(tast_in_value == 1)//change_on_input & 0x08)	//(TAST_IN = 1 && TAST_IN_PREV != TAST_IN)
			{
				if(!en_disp_out)
				{
					if (sectimer > 4)
					{
						en_disp_out = 1;	
					}
					else if (!cnt_enable)
					{
						en_disp_out = 1;	
					}
				}			
				cnt_enable = 1;
			}
	

			if(change_on_input & 0x01)
			{
				
				if(change_on_input & 0x02)
				{
					change_on_input &= 0xFD;
					if(change_on_input & 0x80)
					{
						change_on_input &= 0x7F; //clear tast_in after green if it was set
						tast_in_value = 0;//change_on_input &= 0xF7;
					}
	
					if(after_green)
					{
						sectimer = protect_time;
						color = YELLOW;
						en_disp_out = 1;
						after_green = 0;
						Clock_reset(3);
						cnt_enable = 1;
						//Create_Packet1(0x90);
					}
					change_on_input &= 0xFE;
				}
				else if((change_on_input & 0x04) && (change_on_input & 0x20))
				{
					change_on_input &= 0xDB;
					tast_in_value = 0;//change_on_input &= 0xF7;
					if(!after_green)
					{
						sectimer = green_time;
						color = GREEN;
						en_disp_out = 1;
						after_green = 1;
						Clock_reset(3);
						cnt_enable = 1;
						//Create_Packet1(0x40);
					}
					change_on_input &= 0xFE;
				}
			}

		break;
	}
	

}


void DISPLAY_OFF ()
{
	char test;
//	OE_DISP = 1;
	test=0x00;
	OE_INT = 1;
	_nop_();
	_nop_();

	P3 = test;
	_nop_();
	_nop_();

	
	CP_DISP1R = 1;
	CP_DISP1G = 1;
	CP_DISP2R = 1;
	CP_DISP2G = 1;

	_nop_();
	_nop_();

	CP_DISP1R = 0;
	CP_DISP1G = 0;
	CP_DISP2R = 0;
	CP_DISP2G = 0;
	_nop_();

	OE_DISP = 1;
	_nop_();

}

void write_DISPLAY ()	//color => 1=Red; 2=Yelow; 3=Green
{
	unsigned char seg1, seg2, seg3, seg_tmp, num = 0;	
	num = sectimer;
//	P3 = 0x00;
//	DISPLAY_OFF ();
	if ((num>0)&&(num<200))
	{
		if ((num>99)&(num<200)) 
		{
			seg3=0x80;
			num=num-100;

		}
		else seg3=0x00;

		if ((num>9) || (seg3==0x80))
		{
			//seg_tmp=num;
			seg_tmp=num/10;
			seg_tmp=NUM_to_SEG (seg_tmp);

			seg2 = seg_tmp + seg3;
		}
		else seg2=0;

		if(num<10) seg1=num;
		else
		{
			seg1=num%10;
		}
		seg1=NUM_to_SEG (seg1);

		OE_INT = 1;
		_nop_();

		switch (color)
		{
			case 1:
				r_color_cnt = RG4DISP;
				Display_Red (seg1, seg2);
			break;

			case 2:			
				r_color_cnt = RG4DISP;				
				Display_Red(seg1, seg2);
				g_color_cnt = Y4DISP;
				yellow_cnt = 1;
				Display_Green (seg1,seg2);
			break;

			case 3:
				g_color_cnt = RG4DISP;
				yellow_cnt = 0;
				Display_Green (seg1,seg2);
			break;

		}	
	
		_nop_();
		_nop_();

		CP_DISP1R = 0; 
		CP_DISP1G = 0;
		CP_DISP2R = 0;
		CP_DISP2G = 0;
		OE_DISP = 0;
	}
}


void Display_Red (unsigned char seg1_val, unsigned char seg2_val)
{
	P3 = 0xFF;
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();

	P3 = seg2_val;
	_nop_();
	_nop_();
	_nop_();

	CP_DISP1G = 0;
	CP_DISP2G = 0;
	CP_DISP2R = 0;
	_nop_();
	CP_DISP1R = 1;
	
	_nop_();
	_nop_();
	_nop_();
	CP_DISP1R = 0;
	_nop_();

	P3 = 0xFF;
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();

	P3 = seg1_val;
	_nop_();
	_nop_();
	_nop_();

	CP_DISP1G = 0;
	CP_DISP2G = 0;
	CP_DISP1R = 0;
	_nop_();
	CP_DISP2R = 1;

	_nop_();
	_nop_();
	_nop_();
	CP_DISP2R = 0;
		_nop_();
}

void Display_Green (unsigned char seg1_val, unsigned char seg2_val)
{
	P3 = 0xFF;
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();

	P3 = seg2_val;
	_nop_();
	_nop_();
	_nop_();

	CP_DISP1R = 0;
	CP_DISP2R = 0;
	CP_DISP2G = 0;
	_nop_();
	CP_DISP1G = 1;

	_nop_();
	_nop_();
	_nop_();
	CP_DISP1G = 0;
	_nop_();

	P3 = 0xFF;
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();

	P3 = seg1_val;
	_nop_();
	_nop_();
	_nop_();

	CP_DISP1R = 0;
	CP_DISP2R = 0;
	CP_DISP1G = 0;
	_nop_();
	CP_DISP2G = 1;

	_nop_();
	_nop_();
	_nop_();

	CP_DISP2G = 0;
		_nop_();
}

void Display_Green_OFF ()
{
	P3 = 0x00;
	_nop_();
	_nop_();


	CP_DISP1R = 0;
	CP_DISP2R = 0;
	_nop_();
	CP_DISP2G = 1;
	CP_DISP1G = 1;

	_nop_();
	_nop_();

	CP_DISP1G = 0;
	CP_DISP2G = 0;

	
	_nop_();
	_nop_();
}

unsigned char NUM_to_SEG (unsigned char br)
{
unsigned char seg;
	
	switch (br)
	{
		case 0:
			seg=0x3F;
		break;
		case 1:
			seg=0x06;
		break;
		case 2:
			seg=0x5B;
		break;
		case 3:
			seg=0x4F;
		break;
		case 4:
			seg=0x66;
		break;
		case 5:
			seg=0x6D;
		break;
		case 6:
			seg=0x7D;
		break;
		case 7:
			seg=0x07;
		break;
		case 8:
			seg=0x7F;
		break;
		case 9:
			seg=0x6F;
		break;
         default:
		 	seg=0x00;
            break;

	}

	return seg;

}





void Init_Values (void)
{
	P3=0x00;
	_nop_();
	DISPLAY_OFF ();

	Clock_reset(1);
	Clock_reset(2);
	Clock_reset(3);

	memset (&RED_IN_VALUES, 0x00, 4);
	memset (&INT_IN_VALUES, 0x00, 4);

	TAST_IN = 0;
	RED_IN = 0;
	INT_IN = 0;

	input1 = 0;
	input2 = 0;
	input3 = 0;

	RED_IN_PREV = 0;
	INT_IN_PREV = 0;
	TAST_IN_PREV = 0;

	out_equal = 0;
	red_equal = 0;
	int_equal = 0;
	
	InitValuesFromFlash();
	en_disp_out = 0;	//0-rol kell indulnia, es csak akkor valtani 1-re ha kimerte ketszer a zoldet
	disable_out = 0;
	sectimer = 0;
	red_data_num = 1;
	int_data_num = 1;
	measure = 0;
	after_green = 0;
	process = 0;
	offset_cnt = 0;
	offset_en = 0;
	cnt_enable = 0;
	data_err = 0;
	enable_out = 0;
	disp_out_cnt = brightness;
	r_color_cnt = 0;
	g_color_cnt = 0;
	yellow_cnt = 0;
	change_on_input = 0;
	tast_in_value = 0;
	clear_tast_in = 0;

	UART0_RI_ptr = 0;
	memset(&UART0_Buffer_IN, 0, UART0_BUFFERSIZE);
	memset(&UART0_TMP, 0, UART0_BUFFERSIZE);
}




void InitValuesFromFlash (void)
{
bit overwrite = 0;

	PROG_MODE = FLASH_ByteRead (PGMD_ADDR, 0);
	protect_time = FLASH_ByteRead (PRTM_ADDR, 0);
	allowed_green_diff = FLASH_ByteRead (ALDG_ADDR, 0);
	allowed_red_diff = FLASH_ByteRead (ALDR_ADDR, 0);
	brightness = FLASH_ByteRead (BRNS_ADDR, 0);
	init_time = FLASH_ByteRead (INIT_ADDR, 0);
	red_max_time = FLASH_ByteRead (RMTM_ADDR, 0);
	green_time = FLASH_ByteRead (GRTM_ADDR, 0);
	init_offset = FLASH_ByteRead (OFSI_ADDR, 0);
	stop_time = FLASH_ByteRead (STPR_ADDR, 0);
	blink_value = FLASH_ByteRead (BLNK_ADDR, 0);
	r_g_depend = FLASH_ByteRead (STNG_ADDR, 0);
	
	if((PROG_MODE < 1) || (PROG_MODE > 3))
	{
		PROG_MODE = 3;	//1
		overwrite = 1;
	}

	if(protect_time == 0xFF)
	{
		protect_time = 5;
		overwrite = 1;
	}
	
	if(allowed_green_diff == 0xFF)
	{
		allowed_green_diff = 7;
		overwrite = 1;
	}

	if(allowed_red_diff == 0xFF)
	{
		allowed_red_diff = 14;
		overwrite = 1;
	}

	if(brightness == 0xFF || brightness == 0x00)
	{
		brightness = DIMMER;
		overwrite = 1;
	}

	if(init_time == 0xFF)
	{
		init_time = 9;
		overwrite = 1;
	}

	if(red_max_time == 0xFF)
	{
		red_max_time = 35;//34;
		overwrite = 1;
	}

	if(green_time == 0xFF)
	{
		green_time = 5;//16;
		overwrite = 1;
	}
	
	if(init_offset == 0xFF)
	{
		init_offset = 0;
		overwrite = 1;
	}

	if(stop_time == 0xFF)
	{
		stop_time = 22;
		overwrite = 1;
	}

	if (blink_value == 0)
		blink_en = 0;
	else
		blink_en = 1;

	if(blink_value == 0xFF)
	{
		blink_en = 1;
		blink_value = 5;
		overwrite = 1;
	}

	if(r_g_depend == 0xFF)
	{
		r_g_depend = 0;
		overwrite = 1;
	}


	if(overwrite)
		WriteValuesToFlash();

		//Create_Packet0();	//debug
}


void WriteValuesToFlash (void)
{
	FLASH_PageErase (PGMD_ADDR, 0);
	FLASH_ByteWrite (PGMD_ADDR, PROG_MODE, 0);
	FLASH_ByteWrite (PRTM_ADDR, protect_time, 0);
	FLASH_ByteWrite (ALDG_ADDR, allowed_green_diff, 0);
	FLASH_ByteWrite (ALDR_ADDR, allowed_red_diff, 0);
	FLASH_ByteWrite (BRNS_ADDR, brightness, 0);
	FLASH_ByteWrite (INIT_ADDR, init_time, 0);
	FLASH_ByteWrite (RMTM_ADDR, red_max_time, 0);
	FLASH_ByteWrite (GRTM_ADDR, green_time, 0);
	FLASH_ByteWrite (OFSI_ADDR, init_offset, 0);
	FLASH_ByteWrite (STPR_ADDR, stop_time, 0);
	FLASH_ByteWrite (BLNK_ADDR, blink_value, 0);
	FLASH_ByteWrite (STNG_ADDR, r_g_depend, 0);
}