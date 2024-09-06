/*
 * flash.c
 */
#include "flash.h"

//call ex: erase_flash(FLASH_SECTOR_3);
void erase_flash(uint16_t sector)
{
	HAL_FLASH_Unlock();
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGSERR | FLASH_FLAG_PGPERR);

	FLASH_Erase_Sector(sector, FLASH_VOLTAGE_RANGE_2);

	HAL_FLASH_Lock();
}


void write_flash_16(uint16_t *data, uint32_t address)
{
	HAL_FLASH_Unlock();
	HAL_FLASH_OB_Unlock();

	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGSERR | FLASH_FLAG_PGPERR);
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, address, *data);

	HAL_FLASH_OB_Lock();
	HAL_FLASH_Lock();
}

uint16_t read_flash_16(uint32_t address)
{
	return *(uint16_t*)address;
}

void WriteValuetoFlash()
{
	uint8_t i;
	if((ADC_MIN[0] < ADC_MAX) && (ADC_MIN[1] < ADC_MAX) && (ADC_MIN[2] < ADC_MAX) &&
			(ADC_MIN[3] < ADC_MAX) && (ADC_MIN[4] < ADC_MAX) && (ADC_MIN[5] < ADC_MAX))
	{
		erase_flash(FLASH_SECTOR_3);
		write_flash_16(&ADC_MAX, ADC_MAX_ADDR);
		for(i = 0; i < 6; i++)
		{
			write_flash_16(&ADC_MIN[i], ADC_MIN_ADDR + (i*2));
		}
	}
}

void InitValuefromFlash(void)
{
	uint8_t i, rewrite;
	uint16_t adc_max_temp, adc_min_temp[6];
	for(i = 0; i < 6; i++)
	{
		adc_min_temp[i] = read_flash_16(ADC_MIN_ADDR + (i*2));
	}
	adc_max_temp = read_flash_16(ADC_MAX_ADDR);

	for(i = 0; i < 6; i++)
	{
		if(adc_min_temp[i] == 0xFFFF)
		{
			ADC_MIN[i] = 0x0078;
			rewrite |= 0x01 << i;
		}
		else
		{
			ADC_MIN[i] = adc_min_temp[i];
		}
	}

	if(adc_max_temp == 0xFFFF)
	{
		ADC_MAX = 0x06F0;
		rewrite |= 0x01 << 7;
	}
	else
	{
		ADC_MAX = adc_max_temp;
	}

	if(rewrite > 0)
	{
		WriteValuetoFlash();
		rewrite = 0;
	}
}
