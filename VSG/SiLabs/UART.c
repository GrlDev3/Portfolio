#include <CPU_init.h>
#include <SysTick.h>
#include <Util.h>
#include <UART.h>
#include <SMBus.h>

xdata volatile U8 UART0_Buffer_IN[UART0_BUFFERSIZE]; 
xdata volatile U8 UART0_TMP[UART0_BUFFERSIZE]; 
xdata volatile U8 UART0_Buffer_OUT[PACKET0_SIZE];
xdata volatile U8 UART1_Buffer_IN[UART1_BUFFERSIZE];
xdata volatile U8 UART1_Buffer_OUT[PACKET1_OUT_SIZE];
xdata volatile U8 Packet1[PACKET1_SIZE];
xdata volatile U8 *p_UART0vez = &UART0_Buffer_OUT; 
xdata volatile U8 *p_UART0kov = &UART0_Buffer_OUT;
xdata volatile U8 *p_UART1vez = &UART1_Buffer_OUT;
xdata volatile U8 *p_UART1kov = &UART1_Buffer_OUT;
xdata volatile U8 err_blink;
xdata volatile U8 LAMP_REQUESTED;
xdata volatile U8 LAMP_STATE;
xdata volatile U8 LED_REQUESTED;
xdata volatile U8 LED_OUT;
xdata volatile U8 LAMP_SORTEDSTATE;
xdata volatile U8 LED_REQSTATE;
xdata volatile U8 LED_OUTSTATE;
xdata volatile U8 UART0_RI_ptr = 0;
xdata volatile U8 UART1_Buffer_Size = 0;
xdata volatile U8 thisVSGtimeout;
xdata volatile U8 nextVSGtimeout;
xdata volatile U8 waitingNextVSG;
bit TX0_Ready = 0;
bit send0_end;
bit TX1_Ready = 0;
bit CPSsend;
bit send1_end;
bit Pack_ID_Ok;
bit probav2;
bit err_blink_ok;
bit b_rd_U0 = 0;
bit b_debug = 0;
bit b_write = 0;
		

void UART0_int (void) interrupt 4	
{	
	U8 tmpChar;

	if (_testbit_(RI0))
    {	
		tmpChar = SBUF0;
		if (UART0_RI_ptr == 0)
		{
			if (tmpChar == START)
			{
				UART0_Buffer_IN[UART0_RI_ptr] = tmpChar;
				UART0_RI_ptr++;
			}
		}
		else if (UART0_RI_ptr <= UART0_BUFFERSIZE)
		{
			UART0_Buffer_IN[UART0_RI_ptr] = tmpChar;
			UART0_RI_ptr++;
			if (UART0_RI_ptr == UART0_BUFFERSIZE)
			{
				if (tmpChar == STOP)
				 	b_rd_U0 = 1;
				else
				{
					b_rd_U0 = 0;
					UART0_RI_ptr = 0;
				}
			}
		}
	};
 
	if (_testbit_(TI0)) 
	{
		if (TX0_Ready)			
		{

			if (p_UART0vez == &UART0_Buffer_OUT)
			{
				TX0_Ready = 0;
			}
			else
			{	
				if (p_UART0kov == p_UART0vez)				
				{										
					p_UART0vez = UART0_Buffer_OUT;
					p_UART0kov = UART0_Buffer_OUT;
					TX0_Ready = 0;
					send0_end = 1;
				}
				else
				{
					if (p_UART0kov != p_UART0vez)    
					{	
						send0_end = 0;
						SBUF0 = *p_UART0kov++;
					}	
				};
			};		
		};
	};
}


void UART1_int(void) interrupt 20 	
{	
	U8 tmpChar;

	SFRPAGE = UART1_PAGE;
	if (_testbit_(RI1))
    {
		tmpChar=SBUF1;
		if (waitingNextVSG==0)
		{
			if (UART1_Buffer_Size == 0)
			{
				if (tmpChar==VSG_ID)
				{
					UART1_Buffer_IN[UART1_Buffer_Size] = tmpChar;
				 	UART1_Buffer_Size++;
				};
			}
			else if (UART1_Buffer_Size == (UART1_BUFFERSIZE-1))
			{
				UART1_Buffer_IN[UART1_Buffer_Size] = tmpChar;
				UART1_Buffer_Size++;	
				err_blink_ok=1;
				Pack_ID_Ok=1;
				thisVSGtimeout=0;
				nextVSGtimeout=0;
				waitingNextVSG=1;
			}
			else if (UART1_Buffer_Size < (UART1_BUFFERSIZE-1))
			{
				UART1_Buffer_IN[UART1_Buffer_Size] = tmpChar;
				UART1_Buffer_Size++;
			}
		}
		else
		{
			if ((UART1_Buffer_Size == 0)&&(waitingNextVSG==1))
			{
				if (tmpChar==VSG_NEXT_ID)
				{
					CPSsend=1;
					waitingNextVSG=0;
					nextVSGtimeout=0;
					err_blink++;
				};
			}
		}
	};
 
	if (_testbit_(TI1)) 
	{
		if (TX1_Ready)			
		{

			if (p_UART1vez == &UART1_Buffer_OUT)
			{
				TX1_Ready = 0;
			}
			else
			{	
				if (p_UART1kov == p_UART1vez)				
				{										
					p_UART1vez = UART1_Buffer_OUT;
					p_UART1kov = UART1_Buffer_OUT;
					TX1_Ready = 0;
					send1_end =1;
					sendingToCPS=0;
					flash_enable_timer = 300;
				}
				else
				{
					if (p_UART1kov != p_UART1vez)  
					{  	
						SBUF1 = *p_UART1kov++;
					}
				};
			};
		};
	};

}


void UART0_Check (void)
{
	U8 i,j, chkSUM;
	ES0 = 0;									//disable UART0 int

	memset(&UART0_TMP, 0, UART0_BUFFERSIZE);	//empty temporary buffer
	
	if(UART0_RI_ptr > (UART0_BUFFERSIZE-1))
	{
		for (i = 0; i < UART0_RI_ptr; i++)		//read buffer
		{
			UART0_TMP[i] = UART0_Buffer_IN[i];
		}

		chkSUM = 0;
		i = 1;
		while (i < (UART0_BUFFERSIZE-2))		//calculate checksum
		{
			chkSUM = chkSUM + UART0_TMP[i];
			i++;
		}
		//remelhetoleg csak az also byteot tartja meg
		if(UART0_TMP[(UART0_BUFFERSIZE-2)] == chkSUM)
		{
			ADC_MAX = (UART0_TMP[1] << 8) + UART0_TMP[2];
			for (i = 3,j = 0; i < UART0_BUFFERSIZE-2; i+=2, j++)
				ADC_MIN[j] = (UART0_TMP[i] << 8) + UART0_TMP[i+1];
			b_write = 1;
		}//jo checksum
	}

	UART0_RI_ptr = 0;
	ES0 = 1;									//enable UART0 int
}


void UART1_Check(void)	
{
	U8 i, res = 0;


	EIE2      &=~ 0x40;    //disable UART1 int

	
	if (UART1_Buffer_Size > (PACKET1_SIZE - 1))		//teljes csomag-e
	{
		for (i = 0; i < UART1_Buffer_Size; i++)		//csomag kezdete keresese
		{
			Packet1[i] = UART1_Buffer_IN[i];
		}
		

		if (((Packet1[1] & 0x3F)==((~Packet1[2])&0x3F))&&((Packet1[1] & 0xC0)==0x40)&&((Packet1[2] & 0xC0)==0x40))
		{		
			LED_REQUESTED&=0x40;	//miert van a najava maszkolva
			LED_REQUESTED|=Packet1[3];

			if ((~LED_REQUESTED) & 0x80)
			{
				if (err_blink >= 8)
				{
					LED_REQUESTED|=0x80;
					err_blink = 0;
				}
				else
				{
					LED_REQUESTED&=0x7F;
				}
				errorByCPS = 0;
			}
			else
			{
				errorByCPS = 1;
			}

			LAMP_STATE=0;
			LAMP_REQUESTED=Packet1[1];
			LAMP_On ();	
			LED_On ();
		}
	}

	UART1_Buffer_Size=0;

	EIE2      |= 0x40;		//enable UART1 int
}


void Create_Packet0 (void)
{
	U8 i,j = 1;
	SFRPAGE   = UART0_PAGE;
	TX0_Ready=1;
	UART0_Buffer_OUT[0] = START;
	UART0_Buffer_OUT[1] = MEAUSURE [0]>>(8);
	UART0_Buffer_OUT[2] = MEAUSURE [0];
	UART0_Buffer_OUT[3] = MEAUSURE [1]>>(8);
	UART0_Buffer_OUT[4] = MEAUSURE [1];
	UART0_Buffer_OUT[5] = MEAUSURE [2]>>(8);
	UART0_Buffer_OUT[6] = MEAUSURE [2];
	UART0_Buffer_OUT[7] = MEAUSURE [3]>>(8);
	UART0_Buffer_OUT[8] = MEAUSURE [3];
	UART0_Buffer_OUT[9] = MEAUSURE [4]>>(8);
	UART0_Buffer_OUT[10] = MEAUSURE [4];
	UART0_Buffer_OUT[11] = MEAUSURE [5]>>(8);
	UART0_Buffer_OUT[12] = MEAUSURE [5];
	UART0_Buffer_OUT[13] = ADC_MAX>>(8);
	UART0_Buffer_OUT[14] = ADC_MAX;
	UART0_Buffer_OUT[15] = ADC_MIN[0]>>(8);
	UART0_Buffer_OUT[16] = ADC_MIN[0];
	UART0_Buffer_OUT[17] = ADC_MIN[1]>>(8);
	UART0_Buffer_OUT[18] = ADC_MIN[1];
	UART0_Buffer_OUT[19] = ADC_MIN[2]>>(8);
	UART0_Buffer_OUT[20] = ADC_MIN[2];
	UART0_Buffer_OUT[21] = ADC_MIN[3]>>(8);
	UART0_Buffer_OUT[22] = ADC_MIN[3];
	UART0_Buffer_OUT[23] = ADC_MIN[4]>>(8);
	UART0_Buffer_OUT[24] = ADC_MIN[4];
	UART0_Buffer_OUT[25] = ADC_MIN[5]>>(8);
	UART0_Buffer_OUT[26] = ADC_MIN[5];

	UART0_Buffer_OUT[PACKET0_SIZE-1] = STOP;
		
	i=0;
	p_UART0vez = &UART0_Buffer_OUT[0];         
	while (i < PACKET0_SIZE)
	{
		*p_UART0vez++ = UART0_Buffer_OUT[i];   
		i++;
	}; 

	TI0=1;

}


void Create_Packet1 (void)
{
	U8 i,j = 1;
	U8 checksum1;
	SFRPAGE   = UART1_PAGE;
	SFRPAGE   = UART1_PAGE;
	
	
	TX1_Ready=1;
	UART1_Buffer_OUT[0] = VSG_ID_OUT;
	UART1_Buffer_OUT[1] = LAMP_STATE;				// Ez van a kimeneten
	UART1_Buffer_OUT[2] = LAMP_REQUESTED&0x3F;		// Ez kellene hogy legyen a kimeneten
	UART1_Buffer_OUT[3] = 0x00;
	UART1_Buffer_OUT[5] = 0xFF;
	UART1_Buffer_OUT[6] = 0xFF;
	UART1_Buffer_OUT[7] = 0xFF;

	checksum1=0;
	for(j=0;j<PACKET1_OUT_SIZE-4;j++)
	{
		checksum1 += UART1_Buffer_OUT[j];
	}

	UART1_Buffer_OUT[PACKET1_OUT_SIZE-4] = checksum1;
	i=0;
	p_UART1vez = &UART1_Buffer_OUT[0];         
	while (i < PACKET1_OUT_SIZE)
	{
		*p_UART1vez++ = UART1_Buffer_OUT[i];   
		i++;
	}; 
	TX1_Ready=1;
	send1_end =0;
	TI1=1;
}