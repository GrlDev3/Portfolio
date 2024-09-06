/*
 * flash.h
 */

#ifndef INC_FLASH_H_
#define INC_FLASH_H_

//#include "stm32f4xx_hal.h"
#include "global.h"

#define SECTOR3		0x0800C000
#define SECTOR3_END 0x0800FFFF

#define ADC_MAX_ADDR 0x0800C000
#define ADC_MIN_ADDR 0x0800C002
/*#define ADC_MIN_1_ADDR 0x0800C004
#define ADC_MIN_2_ADDR 0x0800C006
#define ADC_MIN_3_ADDR 0x0800C008
#define ADC_MIN_4_ADDR 0x0800C00A
#define ADC_MIN_5_ADDR 0x0800C00C*/

void erase_flash(uint16_t sector);
void write_flash_ui16(uint16_t data, uint32_t address);
uint16_t read_flash_16(uint32_t address);
void WriteValuetoFlash(void);
void InitValuefromFlash(void);

#endif /* INC_FLASH_H_ */
