/*
 * i2c.h
 */

#ifndef I2C_H_
#define I2C_H_

//#include "stm32f4xx_hal.h"
#include "global.h"

#define I2C_PIN_MAP GPIO_AF4_I2C1

typedef struct i2c_module{
	I2C_HandleTypeDef*	instance;
	uint16_t			sdaPin;
	GPIO_TypeDef*		sdaPort;
	uint16_t			sclPin;
	GPIO_TypeDef*		sclPort;
}i2c_module;


extern i2c_module i2c_mod;

void i2c_ClearBusyFlagErratum(struct i2c_module *i2c);
HAL_StatusTypeDef i2c1_TX(uint8_t address, uint8_t *channel);
HAL_StatusTypeDef i2c1_RX(uint8_t address, uint8_t *data, uint8_t length);

#endif /* I2C_H_ */
