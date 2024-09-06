#include <CPU_init.h>
#include <SysTick.h>
#include <Util.h>
#include <UART.h>
#include <SMBus.h>

xdata volatile U8 UART0_Buffer_IN[UART0_BUFFERSIZE], UART0_Buffer_OUT[UART0_BUFFERSIZE]; 
xdata U8 UART0_RI_ptr = 0;
xdata volatile U8 UART0_TMP[UART0_BUFFERSIZE];
xdata volatile U8 *p_U0vez = &UART0_Buffer_OUT, *p_U0kov = &UART0_Buffer_OUT;
xdata volatile data_err = 0;
bit TX0_Ready = 0;
bit send0_end;

bit b_rd_U0 = 0;


void UART0_int (void) interrupt 4	
{	
	U8 tmpChar;
	char SFRPAGE_SAVE = SFRPAGE;	
	SFRPAGE   = UART0_PAGE;

	if (_testbit_(RI0))
    {	
		tmpChar = SBUF0;
		if (UART0_RI_ptr > 0)
		{
			if (UART0_RI_ptr < UART0_BUFFERSIZE && packTimer < 20)
			{
				UART0_Buffer_IN[UART0_RI_ptr] = tmpChar;
				UART0_RI_ptr++;
				if ((UART0_RI_ptr == PACKET_SIZE) && (tmpChar == STOP))
				{
					b_rd_U0 = 1;
					UART0_RI_ptr = 0;
				}
				else if (UART0_RI_ptr > PACKET_SIZE)
				{
					UART0_RI_ptr = 0;
					packTimer = 0;
					memset(&UART0_Buffer_IN, 0, UART0_BUFFERSIZE);
					Stat_for_U0(PACKLENERR);
				}
				
			}
			else
			{
				UART0_RI_ptr = 0;
				packTimer = 0;
				memset(&UART0_Buffer_IN, 0, UART0_BUFFERSIZE);
				Stat_for_U0(PACKLENERR);
			}
		}
		else 
		{
			if (tmpChar == START)
			{
				UART0_Buffer_IN[0] = tmpChar;
				UART0_RI_ptr = 1;
				packTimer = 0;
			}
		}
		tmpChar = 0;
	};
 
	if (_testbit_(TI0)) 
	{
		if (TX0_Ready)			
		{

			if (p_U0vez == &UART0_Buffer_OUT)
			{
				TX0_Ready = 0;
			}
			else
			{	
				if (p_U0kov == p_U0vez)		//Reset variables after last sent byte 		
				{										
					p_U0vez = UART0_Buffer_OUT;
					p_U0kov = UART0_Buffer_OUT;
					TX0_Ready = 0;
					send0_end = 1;
				}
				else
				{
					if (p_U0kov != p_U0vez)    //Send packet byte by byte
					{	
						send0_end = 0;
						SBUF0 = *p_U0kov++;
					}	
				};
			};		
		};
	
	};

	SFRPAGE = SFRPAGE_SAVE;
}


void UART1_int(void) interrupt 20 	
{	
	U8 tmpChar;

	SFRPAGE = UART1_PAGE;
	if (_testbit_(RI1))
    {
		tmpChar=SBUF1;
		SBUF1 = tmpChar;
	};
 
	if (_testbit_(TI1)) 
	{
		TI1=0;
		
	};

}


void UART0_Check (void)
{
	U8 i, chkSUM;
//	ES0 = 0;									//disable UART0 int

	memset(&UART0_TMP, 0, UART0_BUFFERSIZE);	//empty temporary buffer
	
	for (i = 0; i < PACKET_SIZE; i++)		//read buffer
	{
		UART0_TMP[i] = UART0_Buffer_IN[i];
	}

	chkSUM = 0;
	i = 1;
	while (i < (PACKET_SIZE-2))		//calculate checksum
	{
		chkSUM = chkSUM + UART0_TMP[i];
		i++;
	}
	if(UART0_TMP[(PACKET_SIZE-2)] == chkSUM)
	{
		Check_In_Data ();
		if (data_err == 0)
		{
			WriteValuesToFlash();
			Init_Values();
			Stat_for_U0(SENDDATA);
		}
		else
		{
			Stat_for_U0(INVALIDPACKET);
			Init_Values();
	
		}
	}//jo checksum
	else
	{
		Stat_for_U0(CHKERR);
	}
	
//	ES0 = 1;
}


void UART1_Check(void)	
{
//	U8 i, res = 0;


	EIE2      &=~ 0x40;    //disable UART1 int



	EIE2      |= 0x40;		//enable UART1 int
}

void Check_In_Data (void)
{
	data_err = 0;
			
	if(UART0_TMP[1] > 0 && UART0_TMP[1] <= 3)
	{
		PROG_MODE = UART0_TMP[1];
		switch (PROG_MODE)
		{
		case 1:
			if(UART0_TMP[3] > 0 && UART0_TMP[3] < 199)
				protect_time = UART0_TMP[3];
			else
				data_err |= 0x20;

			if(UART0_TMP[4] > 0 && UART0_TMP[4] < 199)
				allowed_green_diff = UART0_TMP[4];
			else
				data_err |= 0x10;

			if(UART0_TMP[5] > 0 && UART0_TMP[5] < 199)
				allowed_red_diff = UART0_TMP[5];
			else
				data_err |= 0x08;
			
			if(UART0_TMP[6] == 1)
			{
				r_g_depend = 1;
			}
			else
			{
				r_g_depend = 0;
				if (UART0_TMP[6] > 1)
					data_err |= 0x04;
			}

		break;
		case 2:
			if(UART0_TMP[4] > 0 && UART0_TMP[4] < 199)
				red_max_time = UART0_TMP[4];
			else
				data_err |= 0x10;

			if(UART0_TMP[5] > 0 && UART0_TMP[5] < 199)
				green_time = UART0_TMP[5];
			else
				data_err |= 0x08;

			if(!(data_err & 0x10) || !(data_err & 0x08))
			{
				if((UART0_TMP[6] >= 0 && UART0_TMP[6] < 199) && (UART0_TMP[6] < (UART0_TMP[4]+UART0_TMP[5])))
					init_time = UART0_TMP[6];
				else
					data_err |= 0x04;

				if((UART0_TMP[7] >= 0 && UART0_TMP[7] < 199) && (UART0_TMP[7] < (UART0_TMP[4]+UART0_TMP[5])))
					init_offset = UART0_TMP[7];
				else
					data_err |= 0x02;

				if((UART0_TMP[3] > 0 && UART0_TMP[3] < 199) && (UART0_TMP[3] <= UART0_TMP[4]))
					protect_time = UART0_TMP[3];
				else
					data_err |= 0x20;
			}			
		break;
		case 3:
			if(UART0_TMP[4] > 0 && UART0_TMP[4] < 199)
				red_max_time = UART0_TMP[4];
			else
				data_err |= 0x10;

			if(UART0_TMP[5] > 0 && UART0_TMP[5] < 199)
				green_time = UART0_TMP[5];
			else
				data_err |= 0x08;

			if(!(data_err & 0x10) || !(data_err & 0x08))
			{
				if((UART0_TMP[6] >= 0 && UART0_TMP[6] < 199) && (UART0_TMP[6] < (UART0_TMP[4]+UART0_TMP[5])))
					stop_time = UART0_TMP[6];
				else
					data_err |= 0x04;
	
				if((UART0_TMP[3] > 0 && UART0_TMP[3] < 199) && (UART0_TMP[3] <= UART0_TMP[4]))
					protect_time = UART0_TMP[3];
				else
					data_err |= 0x20;
			}
		break;
		}

		if(UART0_TMP[2] > 1 && UART0_TMP[2] < 51)
			brightness = UART0_TMP[2];
		else
			data_err |= 0x80;

		if(UART0_TMP[8] > 0 && UART0_TMP[8] < 199)
		{
			blink_en = 1;
			blink_value = UART0_TMP[8];
		}
		else
		{
			blink_en = 0;
			blink_value = 0;
			if(UART0_TMP[8] != 0)
				data_err |= 0x01;
		}

	}
	else if (UART0_TMP[1] == 4)	//read flash - talan emiatt kellene hogy a brightness is benne legyen
	{
		data_err = 0;
	}
	else
	{
		data_err |= 0x40;
	}
			
}


void Stat_for_U0 (char stat)
{
	U8 i, chkSUM;
	SFRPAGE   = UART0_PAGE;
	
	UART0_Buffer_OUT[0] = START;
	
	switch(stat)
	{

	case CHKERR:
		UART0_Buffer_OUT[1] = 0x45;	//E:CHKSUM
		UART0_Buffer_OUT[2] = 0x3A;
		UART0_Buffer_OUT[3] = 0x43;
		UART0_Buffer_OUT[4] = 0x48;
		UART0_Buffer_OUT[5] = 0x4B;
		UART0_Buffer_OUT[6] = 0x53;
		UART0_Buffer_OUT[7] = 0x55;
		UART0_Buffer_OUT[8] = 0x4D;
	break;

	case PACKLENERR:
		UART0_Buffer_OUT[1] = 0x45;	//E:LENGTH
		UART0_Buffer_OUT[2] = 0x3A;
		UART0_Buffer_OUT[3] = 0x4C;
		UART0_Buffer_OUT[4] = 0x45;
		UART0_Buffer_OUT[5] = 0x4E;
		UART0_Buffer_OUT[6] = 0x47;
		UART0_Buffer_OUT[7] = 0x54;
		UART0_Buffer_OUT[8] = 0x48;
	break;

	case INVALIDPACKET:
		UART0_Buffer_OUT[1] = 0x45;	//E:DATA:data_err
		UART0_Buffer_OUT[2] = 0x3A;
		UART0_Buffer_OUT[3] = 0x44;
		UART0_Buffer_OUT[4] = 0x41;
		UART0_Buffer_OUT[5] = 0x54;
		UART0_Buffer_OUT[6] = 0x41;
		UART0_Buffer_OUT[7] = 0x3A;
		UART0_Buffer_OUT[8] = data_err;
	break;
		
	case SENDDATA:
		UART0_Buffer_OUT[1] = PROG_MODE;
		UART0_Buffer_OUT[2] = brightness;
		UART0_Buffer_OUT[3] = protect_time;	

		switch (PROG_MODE)
		{
		case 1:
			UART0_Buffer_OUT[4] = allowed_green_diff;
			UART0_Buffer_OUT[5] = allowed_red_diff;
			UART0_Buffer_OUT[6] = r_g_depend;
			UART0_Buffer_OUT[7] = 0x00;
		break;
		case 2:
			UART0_Buffer_OUT[4] = red_max_time;
			UART0_Buffer_OUT[5] = green_time;
			UART0_Buffer_OUT[6] = init_time;
			UART0_Buffer_OUT[7] = init_offset;
		break;
		case 3:
			UART0_Buffer_OUT[4] = red_max_time;
			UART0_Buffer_OUT[5] = green_time;
			UART0_Buffer_OUT[6] = stop_time;
			UART0_Buffer_OUT[7] = 0x00;
		break;
		}

		UART0_Buffer_OUT[8] = blink_value;
	break;
	}

	chkSUM = 0;
	i = 1;
	while (i < PACKET_SIZE-2)		//calculate checksum
	{
		chkSUM = chkSUM + UART0_Buffer_OUT[i];
		i++;
	}
	UART0_Buffer_OUT[9] = chkSUM;
	UART0_Buffer_OUT[10] = STOP;
	
	i=0;
	p_U0vez = &UART0_Buffer_OUT[0];         	// Initiatize pointer with UART0 out-buffer
	
	while (i < PACKET_SIZE)
	{
		*p_U0vez++ = UART0_Buffer_OUT[i];   
		i++;
	}; 


	TX0_Ready=1;								// Set bits to start transmission
	TI0=1;

}


void Create_Packet1 (unsigned char kimenet)
{
	SFRPAGE   = UART0_PAGE;
	
	UART0_Buffer_OUT[0] = kimenet;
	p_U0vez = &UART0_Buffer_OUT[0];

	*p_U0vez++ = UART0_Buffer_OUT[0];


	TX0_Ready = 1;
	TI0=1;
}