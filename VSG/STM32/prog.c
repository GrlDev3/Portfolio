/*
 * prog.c
 *
 *  Created on: Feb 8, 2023
 *      Author: Selma
 */

#include "prog.h"

uint16_t ADC_MAX = 0x06F0;
uint16_t ADC_MIN[6] = {0x0078, 0x0078, 0x0078, 0x0078, 0x0078, 0x0078};

uint8_t VSG_ID_OUT;
uint8_t VSG_ID;
uint8_t VSG_ID_NEXT;

uint8_t cps_out_buff[PacketLongOut_CPS];
uint8_t user_out_buff[PacketOut_USER];

TimeoutTimer delaycntr = {false, false, 0, 380};
TimeoutTimer flashtimer = {false, false, 0, 300};
bool errorLed;//ezt az erteket valtoztatni attol fuggoen hogy van e error illetve kell e pislogni


volatile uint8_t lamp_requested; 	//  ,  ,R2,Y2,G2,R1,Y1,G1
volatile uint8_t lamp_state;		//  ,  ,R2,Y2,G2,R1,Y1,G1
volatile uint8_t lamp_state_n;		//cnt
volatile uint8_t lamp_state_a;		//avg
//volatile uint8_t lamp_sorted_req;	//R1,Y1,G1,  ,Y2,G2,R2,

volatile uint8_t led_requested;		//ER,NA,R2,Y2,G2,R1,Y1,G1
volatile uint8_t led_state;			//ER,NA,R2,Y2,G2,R1,Y1,G1
volatile uint8_t led_state_n;
volatile uint8_t led_state_a;
//volatile uint8_t led_sorted_req;	//R1,Y1,G1,R2,Y2,G2,NA,ER
//volatile uint8_t led_sorted_out;	//R1,Y1,G1,R2,Y2,G2,NA,ER


uint16_t C_AvgValue[ADC_CHNUM];//(C_M)
int16_t C_C[ADC_CHNUM][ADC_BUF_SIZE];//[40];//
int32_t CMsum[ADC_CHNUM]; //uintbol hatha ez a baj
uint8_t measCnt[ADC_CHNUM];
uint8_t i2cErrorCnt[ADC_CHNUM];
//int16_t test_i2c_curr [ADC_CHNUM];
//int16_t test_i2c_bigger [ADC_CHNUM];
uint16_t MEASURE[6];

//phase
uint8_t PH_M[7]; 			//   NA,R2,Y2,G2,R1,Y1,G1	//Phase Measure
uint8_t PH_C[8]; 			//R1,  ,Y2,G2,G1,Y1,R2,NA	//Phase Count
uint16_t ph_measPtr =0;
uint16_t ph_measMask = 0;
uint8_t ph_mask;
uint8_t PH_IN = 0;
uint16_t PH_meas[8];

void init_no_output()
{
	HAL_GPIO_WritePin(OE_VLTG_CHK_GPIO_Port, OE_VLTG_CHK_Pin, GPIO_PIN_SET);

	HAL_GPIO_WritePin(AD0_GPIO_Port, AD0_Pin, GPIO_PIN_RESET); //GRS
	HAL_GPIO_WritePin(AD1_GPIO_Port, AD1_Pin, GPIO_PIN_RESET); //NJV
	HAL_GPIO_WritePin(AD2_GPIO_Port, AD2_Pin, GPIO_PIN_RESET); //G2G
	HAL_GPIO_WritePin(AD3_GPIO_Port, AD3_Pin, GPIO_PIN_RESET); //G2A
	HAL_GPIO_WritePin(AD4_GPIO_Port, AD4_Pin, GPIO_PIN_RESET); //G2R
	HAL_GPIO_WritePin(AD5_GPIO_Port, AD5_Pin, GPIO_PIN_RESET); //G1G
	HAL_GPIO_WritePin(AD6_GPIO_Port, AD6_Pin, GPIO_PIN_RESET); //G1A
	HAL_GPIO_WritePin(AD7_GPIO_Port, AD7_Pin, GPIO_PIN_RESET); //G1R

	HAL_GPIO_WritePin(OE_OUT_CN_GPIO_Port, OE_OUT_CN_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(WRLED_GPIO_Port, WRLED_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(WRLED_GPIO_Port, WRLED_Pin, GPIO_PIN_SET);

	HAL_GPIO_WritePin(WRHSEC_GPIO_Port, WRHSEC_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(WRHSEC_GPIO_Port, WRHSEC_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(OE_OUT_CN_GPIO_Port, OE_OUT_CN_Pin, GPIO_PIN_SET);

	led_requested = 0;
	lamp_requested = 0;
}

void Change_LEDs (uint8_t ledReq, uint8_t ledState)
{
	uint8_t i, tmp_in, tmp_out, state, tmp_maszk = 0x04;
	if(!errorLed)
	{
		for(i = 0; i < 6; i++)
		{
			tmp_in = ledReq & tmp_maszk;
			tmp_out = ledState & tmp_maszk;

			if((tmp_out != tmp_in) && (tmp_in != 0))
			{
				if(delaycntr.counter <= 190)
				{
					ledReq &= ~tmp_in;
				}
				else
				{
					ledReq |= tmp_in;
				}
			}
			tmp_maszk <<= 1;
		}
	}

	state = ledState;
	state |= ledReq;

	HAL_GPIO_WritePin(OE_VLTG_CHK_GPIO_Port, OE_VLTG_CHK_Pin, GPIO_PIN_SET);
	asm("nop");
	HAL_GPIO_WritePin(AD0_GPIO_Port, AD0_Pin, state & 0x01); //GRS
	HAL_GPIO_WritePin(AD1_GPIO_Port, AD1_Pin, state & 0x02); //NJV
	HAL_GPIO_WritePin(AD2_GPIO_Port, AD2_Pin, state & 0x04); //G2G
	HAL_GPIO_WritePin(AD3_GPIO_Port, AD3_Pin, state & 0x08); //G2A
	HAL_GPIO_WritePin(AD4_GPIO_Port, AD4_Pin, state & 0x10); //G2R
	HAL_GPIO_WritePin(AD5_GPIO_Port, AD5_Pin, state & 0x20); //G1G
	HAL_GPIO_WritePin(AD6_GPIO_Port, AD6_Pin, state & 0x40); //G1A
	HAL_GPIO_WritePin(AD7_GPIO_Port, AD7_Pin, state & 0x80); //G1R

	HAL_GPIO_WritePin(WRLED_GPIO_Port, WRLED_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(OE_OUT_CN_GPIO_Port, OE_OUT_CN_Pin, GPIO_PIN_RESET);
	asm("nop");
	asm("nop");
	HAL_GPIO_WritePin(WRLED_GPIO_Port, WRLED_Pin, GPIO_PIN_SET);
}


void Change_LAMPs (uint8_t lampstate)
{
	HAL_GPIO_WritePin(OE_VLTG_CHK_GPIO_Port, OE_VLTG_CHK_Pin, GPIO_PIN_SET);
	asm("nop");
	//HAL_GPIO_WritePin(AD0_GPIO_Port, AD0_Pin, lampstate & 0x01); //ez nem csinal semmit
	HAL_GPIO_WritePin(AD1_GPIO_Port, AD1_Pin, lampstate & 0x02); //G2R
	HAL_GPIO_WritePin(AD2_GPIO_Port, AD2_Pin, lampstate & 0x04); //G2G
	HAL_GPIO_WritePin(AD3_GPIO_Port, AD3_Pin, lampstate & 0x08); //G2A
	//HAL_GPIO_WritePin(AD4_GPIO_Port, AD4_Pin, lampstate & 0x10); //nem csinal semmit
	HAL_GPIO_WritePin(AD5_GPIO_Port, AD5_Pin, lampstate & 0x20); //G1G
	HAL_GPIO_WritePin(AD6_GPIO_Port, AD6_Pin, lampstate & 0x40); //G1A
	HAL_GPIO_WritePin(AD7_GPIO_Port, AD7_Pin, lampstate & 0x80); //G1R

	HAL_GPIO_WritePin(WRHSEC_GPIO_Port, WRHSEC_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(OE_OUT_CN_GPIO_Port, OE_OUT_CN_Pin, GPIO_PIN_RESET);
	asm("nop");
	asm("nop");
	HAL_GPIO_WritePin(WRHSEC_GPIO_Port, WRHSEC_Pin, GPIO_PIN_SET);
}

uint8_t convert_LEDs(uint8_t ledstate)
{
	return (0|(ledstate & 0x07) << 5) | ((ledstate & 0x38) >> 1) | ((ledstate & 0x40) >> 5) | ((ledstate & 0x80) >> 7);
}

uint8_t convert_LAMPs(uint8_t lampstate)
{
	return (0|(lampstate & 0x07) << 5) | ((lampstate & 0x18) >> 1) | ((lampstate & 0x20) >> 4);
}

uint8_t Check_Output ()
{
	uint8_t outputvalues = 0;
	HAL_GPIO_WritePin(WRHSEC_GPIO_Port, WRHSEC_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(WRLED_GPIO_Port, WRLED_Pin, GPIO_PIN_SET);
	//HAL_GPIO_WritePin(OE_OUT_CN_GPIO_Port, OE_OUT_CN_Pin, GPIO_PIN_SET);
	asm("nop");
	AD_INIT_INPUT ();


	HAL_GPIO_WritePin(OE_VLTG_CHK_GPIO_Port, OE_VLTG_CHK_Pin, GPIO_PIN_RESET);
	asm("nop");
	asm("nop");
	outputvalues = HAL_GPIO_ReadPin(AD0_GPIO_Port, AD0_Pin); //<<6
	outputvalues += HAL_GPIO_ReadPin(AD1_GPIO_Port, AD1_Pin) << 1; //<<5
	outputvalues += HAL_GPIO_ReadPin(AD2_GPIO_Port, AD2_Pin) << 2; //<<1
	outputvalues += HAL_GPIO_ReadPin(AD3_GPIO_Port, AD3_Pin) << 3; //0
	outputvalues += HAL_GPIO_ReadPin(AD4_GPIO_Port, AD4_Pin) << 4; //<<3
	outputvalues += HAL_GPIO_ReadPin(AD5_GPIO_Port, AD5_Pin) << 5; //<<4
	outputvalues += HAL_GPIO_ReadPin(AD6_GPIO_Port, AD6_Pin) << 6; //not used
	outputvalues += HAL_GPIO_ReadPin(AD7_GPIO_Port, AD7_Pin) << 7; //<<2

	asm("nop");
	HAL_GPIO_WritePin(OE_VLTG_CHK_GPIO_Port, OE_VLTG_CHK_Pin, GPIO_PIN_SET);

	AD_INIT_OUTPUT ();
	asm("nop");
	asm("nop");
	asm("nop");
	return outputvalues;
}

void Check_ID ()
{
	uint8_t temp;
	//uint8_t buff[5] = {'!',0,0,0,'+'};
	VSG_ID = 0;

	VSG_ID += HAL_GPIO_ReadPin(MID0_GPIO_Port, MID0_Pin);
	VSG_ID += HAL_GPIO_ReadPin(MID1_GPIO_Port, MID1_Pin) << 1;
	VSG_ID += HAL_GPIO_ReadPin(MID2_GPIO_Port, MID2_Pin) << 2;
	VSG_ID += HAL_GPIO_ReadPin(MID3_GPIO_Port, MID3_Pin) << 3;

	VSG_ID++;
	VSG_ID_OUT = VSG_ID;

	temp = ~(VSG_ID + (VSG_ID << 4)) & 0xF0;
	VSG_ID += temp;

	if(VSG_ID_OUT == 12)
		VSG_ID_NEXT=1;
	else
		VSG_ID_NEXT=VSG_ID_OUT+1;

	temp = ~(VSG_ID_NEXT + (VSG_ID_NEXT << 4)) & 0xF0;
	VSG_ID_NEXT += temp;
}

void RestartTimer(TimeoutTimer* timer)
{
	timer->counter = 0;
	timer->enabled = true;
	timer->interrupt_flag = false;
}
void DisableTimer(TimeoutTimer* timer)
{
	timer->enabled = false;
}

void EnableTimer(TimeoutTimer* timer)
{
	timer->enabled = true;
}

void TickTimer(TimeoutTimer* timer){
	if(timer->enabled)
	{
		if(timer->counter < timer->interrupt_time)
		{
			timer->counter++;
		}
		else
		{
			timer->counter = 0;
			timer->interrupt_flag = true;
		}
	}
}

void ReplyPacket4CPS()
{
	cps_out_buff[0] = VSG_ID_OUT;
	cps_out_buff[1] = lamp_state; //-_n
	cps_out_buff[2] = lamp_requested & 0x3F;
	cps_out_buff[3] = 0x00;
	cps_out_buff[5] = 0xFF;
	cps_out_buff[6] = 0xFF;
	cps_out_buff[7] = 0xFF;

	uint8_t checksum = 0;
	for(int j=0; j < PacketOut_CPS-4; j++)
	{
		checksum += cps_out_buff[j];
	}
	cps_out_buff[PacketOut_CPS-4] = checksum;

//#if CPS_VERSION == 2
	/*//if(HAL_GPIO_ReadPin(JUMPER_GPIO_Port, JUMPER_Pin) == GPIO_PIN_SET){	//nincs rajta = hosszu csomag
		cps_out_buff[3] = 0x01;
		PrepForSend(false);
		int k = 8, i;
		for( i=0; i<6; i++)
		{
			cps_out_buff[k++] = (uint8_t) MEASURE[i] >> 8;
			cps_out_buff[k++] = (uint8_t) MEASURE[i];
		}
		cps_out_buff[20] = (uint8_t) ADC_MAX >> 8;
		cps_out_buff[21] = (uint8_t) ADC_MAX;
		k=22;
		for(i=0; i<6; i++)
		{
			cps_out_buff[k++] = (uint8_t) ADC_MIN[i] >> 8;
			cps_out_buff[k++] = (uint8_t) ADC_MIN[i];
		}
		cps_out_buff[34] = (uint8_t) hi2c1.ErrorCode >> 8;//error
		cps_out_buff[35] = (uint8_t) hi2c1.ErrorCode;
		cps_out_buff[36] = i2c_status;
		cps_out_buff[38] = 0xFF;
		cps_out_buff[39] = 0xFF;
		cps_out_buff[40] = 0xFF;


		checksum = 0;
		for(int j=0; j < PacketLongOut_CPS-4; j++)
		{
			checksum += cps_out_buff[j];
		}

		cps_out_buff[PacketLongOut_CPS-4] = checksum;

		//	HAL_UART_Transmit_IT(&huart2, &cps_out_buff[0], PacketLongOut_CPS);
		if(HAL_UART_Transmit_DMA(&huart2, &cps_out_buff[0], PacketLongOut_CPS)!= HAL_OK){
			tx_uart_err_cnt++;
		}
		else
		  {
			  tx_uart_err_cnt = 0;
		  }
	}
	else
	{*/
//#else
	//HAL_UART_Transmit_IT(&huart2, &cps_out_buff[0], PacketOut_CPS);
		if(HAL_UART_Transmit_DMA(&huart2, &cps_out_buff[0], PacketOut_CPS)!= HAL_OK){
			tx_uart_err_cnt++;
		}else
		  {
			  tx_uart_err_cnt = 0;
		  }
	//}
//#endif
	//HAL_Delay(10);
}

void ReplyPacket4USER()
{
	uint8_t k, i;
	user_out_buff[0] = Start_Byte;
	k = 1;
	for(i=0; i<6; i++)
	{
		user_out_buff[k++] = MEASURE[i] >> 8;
		user_out_buff[k++] = MEASURE[i];
	}
	user_out_buff[13] = ADC_MAX >> 8;
	user_out_buff[14] = (uint8_t) ADC_MAX;
	k=15;
	for(i=0; i<6; i++)
	{
		user_out_buff[k++] = ADC_MIN[i] >> 8;
		user_out_buff[k++] = ADC_MIN[i];
	}
	user_out_buff[PacketOut_USER - 1] = Stop_Byte;
	HAL_UART_Transmit_IT(&huart1, &user_out_buff[0], PacketOut_USER);
}



void AD_INIT_INPUT ()
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/*Configure GPIO pins : AD7_Pin AD6_Pin */
	GPIO_InitStruct.Pin = AD7_Pin|AD6_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : AD5_Pin AD4_Pin */
	GPIO_InitStruct.Pin = AD5_Pin|AD4_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pins : AD3_Pin AD2_Pin AD1_Pin AD0_Pin */
	GPIO_InitStruct.Pin = AD3_Pin|AD2_Pin|AD1_Pin|AD0_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void AD_INIT_OUTPUT ()
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/*Configure GPIO pins : AD7_Pin AD6_Pin */
	GPIO_InitStruct.Pin = AD7_Pin|AD6_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : AD5_Pin AD4_Pin */
	GPIO_InitStruct.Pin = AD5_Pin|AD4_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pins : AD3_Pin AD2_Pin AD1_Pin AD0_Pin */
	GPIO_InitStruct.Pin = AD3_Pin|AD2_Pin|AD1_Pin|AD0_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}


void MeasureDecoder(int16_t Current_Measure,  uint8_t ChannelPtr, uint8_t adc_measPtr)
{
	uint8_t maszk = 0x01 << ChannelPtr;
	bool avg_stat, cnt_stat;
	//if(CMsum[ChannelPtr] > C_C[ChannelPtr][adc_measPtr])
	CMsum[ChannelPtr] -= C_C[ChannelPtr][adc_measPtr]; //c_c miert buffer ha csak az elozo ertek eltarolasara hasznaljuk?
	//else
		//CMsum[ChannelPtr] = 0;

	CMsum[ChannelPtr] += Current_Measure;
	C_C[ChannelPtr][adc_measPtr] = Current_Measure;

	/*if(adc_measPtr == ADC_BUF_SIZE-2)
	{
		int32_t sum = 0;
		for(int i = 0; i < ADC_BUF_SIZE; i++)
		{
			sum += C_C[ChannelPtr][i];
		}

		if((sum - 100) > CMsum[ChannelPtr] || (sum + 100) < CMsum[ChannelPtr])
		{
			i2cErrorCnt[ChannelPtr] = 250;
			test_i2c_bigger[ChannelPtr] = sum;
		}
	}*/

	C_AvgValue[ChannelPtr] = (uint16_t) (CMsum[ChannelPtr]/ADC_BUF_SIZE)*1;

	if((C_AvgValue[ChannelPtr] < ADC_MAX) && (C_AvgValue[ChannelPtr] > ADC_MIN[ChannelPtr])) //&& (PH_M[ChannelPtr] >= 6))//fazis is ide?
	{
		avg_stat = true;
		lamp_state_a |= maszk;
	}
	else if(((lamp_requested & maszk) == 0x00) && ((C_AvgValue[ChannelPtr] > ADC_MIN[ChannelPtr])))// || ((PH_M[ChannelPtr] >= 6)))) //fazis is ide?
	{
		avg_stat = true;
		lamp_state_a |= maszk;
	}
	else
	{
		avg_stat = false;
		lamp_state_a &= ~maszk;
	}

	if(Current_Measure < ADC_MAX)
	{
		if(Current_Measure > ADC_MIN[ChannelPtr])
		{
			if(measCnt[ChannelPtr] < 10)
			{
				measCnt[ChannelPtr]++;
			}
		}else
		{
			if(measCnt[ChannelPtr] > 0)
			{
				measCnt[ChannelPtr]--;
			}
		}
		/*if(i2cErrorCnt[ChannelPtr] > 0)
		{
			i2cErrorCnt[ChannelPtr]--;
		}*/
	}
	/*else
	{
		if(i2cErrorCnt[ChannelPtr] < 200 )
		{
			i2cErrorCnt[ChannelPtr]++;
		}
	}*/

	if(measCnt[ChannelPtr] > 5)
	{
		cnt_stat = true;
		lamp_state_n |= maszk;
	}
	else
	{
		cnt_stat = false;
		lamp_state_n &= ~maszk;
	}

	if((avg_stat | cnt_stat) & (PH_M[ChannelPtr] >= 6))
	{
		lamp_state |= maszk;
		led_state |= maszk;
	}
	else
	{
		lamp_state &= ~maszk;
		led_state &= ~maszk;
	}

}

void VoltageDecoder()
{
	int i, j;
	uint8_t ph_mask = 0x01;

	//check phase
	PH_IN = Check_Output();

	ph_measMask = 0x0001 << ph_measPtr;

	for(i = 0; i < 8; i++)
	{
		if(PH_meas[i] & ph_measMask)
		{
			if(PH_C[i] > 0)
				PH_C[i]--;
		}

		if(PH_IN & ph_mask)
		{
			if(PH_C[i] < PH_BUF_SIZE)
			{
				PH_C[i]++;
			}
			PH_meas[i] |= ph_measMask;
		}
		else
		{
			PH_meas[i] &= (~ph_measMask);
		}

		//sorting
		if(i == 0)
			j = 6;
		else if(i == 1)
			j = 5;
		else if(i == 2 || i == 4 || i == 5)
			j = i-1; //1,3,4
		else if(i == 3)
			j = 0;
		else if(i == 7)
			j = 2;

		if(i != 6)
			PH_M[j] = PH_C[i]; //Phase Measure

		ph_mask <<= 1;

	}

	//check det input
	if(PH_M[6] >= PH_Limit)
	{
		VSG_ID_OUT |= 0x10;
		led_requested |= 0x40;
	}
	else
	{
		VSG_ID_OUT &= ~0x10;
		led_requested &= ~0x40;
	}
	//sprintf((char*)UART_buf, "REF %d %d\r\n", PH_IN, PH_C[1]);
	//HAL_UART_Transmit_IT(&huart1, UART_buf, sizeof(UART_buf));
}

void PrepForSend(bool mix)
{
	uint8_t k, i, maszk;
	maszk = 0x01;
	for(k = 0; k < 6; k++)
	{
		if(mix){
			if(k == 0)
			{
				i = 2;
			}
			else if(k == 2)
			{
				i = 0;
			}
			else if(k == 3)
			{
				i = 5;
			}
			else if(k == 5)
			{
				i = 3;
			}
			else{
				i = k;
			}
		}
		else{
			i = k;
		}

		MEASURE[i] = C_AvgValue[k];
		if(PH_M[k] >= PH_Limit-1)
		{
			MEASURE[i] |= 0x8000;
		}
		if(lamp_requested & maszk)
		{
			MEASURE[i] |= 0x4000;
		}

		if(mix){
			if(lamp_state & maszk)
			{
				MEASURE[i] |= 0x2000;
			}
		}
		else{
			if(lamp_state_a & maszk)
			{
				MEASURE[i] |= 0x2000;
			}
			if(lamp_state_n & maszk)
			{
				MEASURE[i] |= 0x1000;
			}
		}
		maszk <<= 1;

	}
}

