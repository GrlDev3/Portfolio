/*
 * i2c.c

 */
#include "i2c.h"

//uint8_t i2c_err_type;

void i2c_ClearBusyFlagErratum(struct i2c_module *i2c){
	 GPIO_InitTypeDef GPIO_InitStructure;

	  int timeout_cnt = 0;

	  // 1. Clear PE bit.
	  i2c->instance->Instance->CR1 &= ~(0x0001);

	  //  2. Configure the SCL and SDA I/Os as General Purpose Output Open-Drain, High level (Write 1 to GPIOx_ODR).
	  GPIO_InitStructure.Mode         = GPIO_MODE_OUTPUT_OD;
	  GPIO_InitStructure.Alternate    = I2C_PIN_MAP;
	  GPIO_InitStructure.Pull         = GPIO_PULLUP;
	  GPIO_InitStructure.Speed        = GPIO_SPEED_FREQ_VERY_HIGH;//GPIO_SPEED_FREQ_HIGH;

	  GPIO_InitStructure.Pin          = i2c->sclPin;
	  HAL_GPIO_Init(i2c->sclPort, &GPIO_InitStructure);
	  HAL_GPIO_WritePin(i2c->sclPort, i2c->sclPin, GPIO_PIN_SET);

	  GPIO_InitStructure.Pin          = i2c->sdaPin;
	  HAL_GPIO_Init(i2c->sdaPort, &GPIO_InitStructure);
	  HAL_GPIO_WritePin(i2c->sdaPort, i2c->sdaPin, GPIO_PIN_SET);

	  // 3. Check SCL and SDA High level in GPIOx_IDR.
	  while (GPIO_PIN_SET != HAL_GPIO_ReadPin(i2c->sclPort, i2c->sclPin))
	  {
	    //asm("nop");
		if(++timeout_cnt > 100)
			return;
	  }

	  while (GPIO_PIN_SET != HAL_GPIO_ReadPin(i2c->sdaPort, i2c->sdaPin))
	  {
		  //Move clock to release I2C
		HAL_GPIO_WritePin(i2c->sclPort, i2c->sclPin, GPIO_PIN_RESET);
		asm("nop");
		HAL_GPIO_WritePin(i2c->sclPort, i2c->sclPin, GPIO_PIN_SET);

		if(++timeout_cnt > 100)
			return;
	  }

	  // 4. Configure the SDA I/O as General Purpose Output Open-Drain, Low level (Write 0 to GPIOx_ODR).
	  HAL_GPIO_WritePin(i2c->sdaPort, i2c->sdaPin, GPIO_PIN_RESET);

	  //  5. Check SDA Low level in GPIOx_IDR.
	  while (GPIO_PIN_RESET != HAL_GPIO_ReadPin(i2c->sdaPort, i2c->sdaPin))
	  {
		//asm("nop");
		if(++timeout_cnt > 100)
			return;
	  }

	  // 6. Configure the SCL I/O as General Purpose Output Open-Drain, Low level (Write 0 to GPIOx_ODR).
	  HAL_GPIO_WritePin(i2c->sclPort, i2c->sclPin, GPIO_PIN_RESET);

	  //  7. Check SCL Low level in GPIOx_IDR.
	  while (GPIO_PIN_RESET != HAL_GPIO_ReadPin(i2c->sclPort, i2c->sclPin))
	  {
		//asm("nop");
		if(++timeout_cnt > 100)
			return;
	  }

	  // 8. Configure the SCL I/O as General Purpose Output Open-Drain, High level (Write 1 to GPIOx_ODR).
	  HAL_GPIO_WritePin(i2c->sclPort, i2c->sclPin, GPIO_PIN_SET);

	  // 9. Check SCL High level in GPIOx_IDR.
	  while (GPIO_PIN_SET != HAL_GPIO_ReadPin(i2c->sclPort, i2c->sclPin))
	  {
		//asm("nop");
		if(++timeout_cnt > 100)
			return;
	  }

	  // 10. Configure the SDA I/O as General Purpose Output Open-Drain , High level (Write 1 to GPIOx_ODR).
	  HAL_GPIO_WritePin(i2c->sdaPort, i2c->sdaPin, GPIO_PIN_SET);

	  // 11. Check SDA High level in GPIOx_IDR.
	  while (GPIO_PIN_SET != HAL_GPIO_ReadPin(i2c->sdaPort, i2c->sdaPin))
	  {
		//asm("nop");
		if(++timeout_cnt > 100)
			return;
	  }

	  // 12. Configure the SCL and SDA I/Os as Alternate function Open-Drain.
	  GPIO_InitStructure.Mode         = GPIO_MODE_AF_OD;
	  GPIO_InitStructure.Alternate    = I2C_PIN_MAP;

	  GPIO_InitStructure.Pin          = i2c->sclPin;
	  HAL_GPIO_Init(i2c->sclPort, &GPIO_InitStructure);

	  GPIO_InitStructure.Pin          = i2c->sdaPin;
	  HAL_GPIO_Init(i2c->sdaPort, &GPIO_InitStructure);

	  // 13. Set SWRST bit in I2Cx_CR1 register.
	  i2c->instance->Instance->CR1 |= 0x8000;

	  asm("nop");

	  // 14. Clear SWRST bit in I2Cx_CR1 register.
	  i2c->instance->Instance->CR1 &= ~0x8000;

	  asm("nop");

	  // 15. Enable the I2C peripheral by setting the PE bit in I2Cx_CR1 register
	  i2c->instance->Instance->CR1 |= 0x0001;

	  // Call initialization function.
	  HAL_I2C_Init(i2c->instance);
	  //printf("I2C ClearedBusyFlag\n");
}

HAL_StatusTypeDef i2c1_TX(uint8_t address, uint8_t *channel){
	HAL_StatusTypeDef ret;
	// Send configuration register data
	ret = HAL_I2C_Master_Transmit_DMA(&hi2c1, (uint16_t)(address<<1), channel, 1);
	if(ret == HAL_BUSY)
	{
		//i2c_status |= I2CerrorType_TxBusyErr;
		i2c_ClearBusyFlagErratum(&i2c_mod);
		return ret;
	}
	else if(ret != HAL_OK)
	{
		i2c_ClearBusyFlagErratum(&i2c_mod);
		return ret;
	}
	//i2c_status &= ~I2CerrorType_TxBusyErr;
	return HAL_OK;

}

HAL_StatusTypeDef i2c1_RX(uint8_t address, uint8_t *data, uint8_t length){
	HAL_StatusTypeDef ret;
	// Send configuration register data
	ret = HAL_I2C_Master_Receive_DMA(&hi2c1, (uint16_t)(address<<1)|0x01, data, length);
	if(ret == HAL_BUSY)
	{
		//i2c_status |= I2CerrorType_RxBusyErr;
		i2c_ClearBusyFlagErratum(&i2c_mod);
		i2c_status |= I2Cstate_NotReady;
      	return ret;
	}
    else if(ret != HAL_OK)
    {
    	i2c_ClearBusyFlagErratum(&i2c_mod);
    	i2c_status |= I2Cstate_NotReady;
    	return ret;
    }
	//i2c_status &= ~I2CerrorType_RxBusyErr;
	return HAL_OK;
}


