/*
 * global.h
 */

#ifndef INC_GLOBAL_H_
#define INC_GLOBAL_H_

#include "stm32f4xx_hal.h"
#include "stdbool.h"
#include "stdio.h"
#include "math.h"
#include <string.h>
#include <stdlib.h>

#define JUMPER_Pin GPIO_PIN_13
#define JUMPER_GPIO_Port GPIOC
#define WRLED_Pin GPIO_PIN_4
#define WRLED_GPIO_Port GPIOA
#define WRHSEC_Pin GPIO_PIN_5
#define WRHSEC_GPIO_Port GPIOA
#define AD7_Pin GPIO_PIN_6
#define AD7_GPIO_Port GPIOA
#define AD6_Pin GPIO_PIN_7
#define AD6_GPIO_Port GPIOA
#define AD5_Pin GPIO_PIN_4
#define AD5_GPIO_Port GPIOC
#define AD4_Pin GPIO_PIN_5
#define AD4_GPIO_Port GPIOC
#define AD3_Pin GPIO_PIN_0
#define AD3_GPIO_Port GPIOB
#define AD2_Pin GPIO_PIN_1
#define AD2_GPIO_Port GPIOB
#define AD1_Pin GPIO_PIN_2
#define AD1_GPIO_Port GPIOB
#define AD0_Pin GPIO_PIN_10
#define AD0_GPIO_Port GPIOB
#define OE_OUT_CN_Pin GPIO_PIN_12
#define OE_OUT_CN_GPIO_Port GPIOB
#define OE_VLTG_CHK_Pin GPIO_PIN_13
#define OE_VLTG_CHK_GPIO_Port GPIOB
#define MID1_Pin GPIO_PIN_14
#define MID1_GPIO_Port GPIOB
#define MID0_Pin GPIO_PIN_15
#define MID0_GPIO_Port GPIOB
#define MID3_Pin GPIO_PIN_6
#define MID3_GPIO_Port GPIOC
#define MID2_Pin GPIO_PIN_7
#define MID2_GPIO_Port GPIOC



typedef enum I2Cstate{
	I2Cstate_NotReady = 0x00,
	I2Cstate_TransmitStarted = 0x01,
	I2Cstate_ReadReady = 0x02,
	I2Cstate_ReadPending = 0x04,
	I2Cstate_ReadDone = 0x08,
	I2Cstate_ResetDone = 0x10//,
	//I2CerrorType_TxBusyErr = 0x20,
	//I2CerrorType_RxBusyErr = 0x40
}I2Cstate;

typedef struct TimeoutTimer{
	bool enabled;
	bool interrupt_flag;
	uint32_t counter;
	uint32_t interrupt_time;

}TimeoutTimer;

typedef enum UartState{
	UartState_NoSend,
	UartState_Ready2Send,
	UartState_Sending
}UartState;

typedef enum CPSType{
	CPSType_old = 0,
	CPSType_STM32F413ZHTX_short_packet = 1,
	CPSType_STM32F413ZHTX_long_packet = 2
}CPStype;

#define CPS_VERSION 2
#define BYTE 1
#define BUFF_Size_User 17
#define BUFF_Size_CPS 4 //4 Byte a csomag hossz (vsg_id, lampa, inverz lampa, ures v checksum de nem ellenorzott)
#define PacketLongOut_CPS 41 //18 ha a uj cps tipusu
#define PacketOut_CPS 8 //18 ha a uj cps tipusu

#define PacketOut_USER 28
#define PacketIn_USER 17

extern uint16_t ADC_MAX;
extern uint16_t ADC_MIN[6];


extern uint8_t i2c_status;
extern TimeoutTimer delaycntr;
extern TimeoutTimer flashtimer;
extern bool errorLed;
extern uint8_t cps_out_buff[PacketLongOut_CPS];
extern uint8_t user_out_buff[PacketOut_USER];
extern uint32_t tx_uart_err_cnt;
extern uint32_t rx_uart_err_cnt;

extern I2C_HandleTypeDef hi2c1;

#endif /* INC_GLOBAL_H_ */
