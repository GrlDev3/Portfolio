/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ADC_ADDR 0x48<<1 // =0x90 Use 8-bit address
#define ADC_ADDR_N 0x48 //not shifted

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
DMA_HandleTypeDef hdma_i2c1_rx;
DMA_HandleTypeDef hdma_i2c1_tx;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_rx;
DMA_HandleTypeDef hdma_usart2_tx;

/* USER CODE BEGIN PV */

//timer
TimeoutTimer thisVSGtimeout = {false, false, 0, 15}; //3ms //min 8 * 200us
TimeoutTimer nextVSGtimeout = {false, false, 0, 55}; //11ms
TimeoutTimer commTimeout = {false, false, 0, 3000}; //600ms //min 350-400 * 200us //not enabled
TimeoutTimer i2cTimeout = {false, false, 0, 5000};	//1s
uint16_t mstimer;
//bool toogleLed;
uint8_t service_send;
uint8_t err_blink;

//timer2
uint32_t mstimer2;
bool sendComm4ADC;

//i2c
uint8_t i2c_status;

//bool newADCdata;
uint8_t ChannelPtr;
uint8_t adc_measPtr;
uint8_t ADC_measure_raw[2];
int16_t ADC_value;
int16_t Current_Measure; //ABS(ADC_value - 2600)

/*
 * [6] = {
 * 0xA4,	GRN_1
 * 0xE4,	AMB_1
 * 0xB4,	RED_1
 * 0xD4,	GRN_2
 * 0x94,	AMB_2
 * 0xC4,	RED_2
 * };
 * 0x84,	CH_1//not used
 * 0xF4		CH_8//not used
 * adc channel addresses
 * */
uint8_t ADC_COMMAND[ADC_CHNUM] = {0xA4, 0xE4, 0xB4, 0xD4, 0x94, 0xC4};
uint32_t tx_i2c_err_cnt;
uint32_t rx_i2c_err_cnt;

i2c_module i2c_mod;

//uart
bool thisIDreceived = false;
bool waitingNextVSG = false;
bool pack_in_cps_buffer = false;
bool pack_in_user_buffer = false;
bool header_received_user = false;
uint8_t reply4CPS;
uint8_t cps_in_buff[BUFF_Size_CPS];//50
uint8_t user_in_buff[BUFF_Size_User];
bool flash_write;
bool flash_read;
//uint8_t in_byte;
//uint8_t buff_ptr;
uint32_t tx_uart_err_cnt;
uint32_t rx_uart_err_cnt;
HAL_StatusTypeDef uartrx;






/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_TIM_PeriodElapsedCallback (TIM_HandleTypeDef* htim);
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart);
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */


	uint8_t UART_buf[25];
	HAL_StatusTypeDef ret;
	uint8_t index, k, chSum;


  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  	i2c_status = I2Cstate_NotReady;
  	uartrx = HAL_OK;
	//toogleLed = false;
	mstimer2 = 0;
	sendComm4ADC = false;
	tx_i2c_err_cnt = 0;
	rx_i2c_err_cnt = 0;
	tx_uart_err_cnt = 0;
	rx_uart_err_cnt = 0;
	errorLed = false;
	reply4CPS = UartState_NoSend;
	service_send = 0;
	ChannelPtr = 0;
	adc_measPtr = 0;
	ph_measPtr =0;
	ph_measMask = 0;
	flash_write = false;
	flash_read = false;
	memset(UART_buf, 0, sizeof(UART_buf));
	memset(user_in_buff, 0, sizeof(user_in_buff));
	memset(cps_in_buff, 0, sizeof(cps_in_buff));
	memset(cps_out_buff, 0, sizeof(cps_out_buff));
	memset(CMsum, 0, sizeof(CMsum));
	memset(C_C, 0 , sizeof(C_C));
	memset(C_AvgValue, 0, sizeof(C_AvgValue));
	i2c_mod.instance = &hi2c1;
	i2c_mod.sclPin = GPIO_PIN_6;
	i2c_mod.sclPort = GPIOB;
	i2c_mod.sdaPin = GPIO_PIN_7;
	i2c_mod.sdaPort = GPIOB;


  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */



  Check_ID();
  InitValuefromFlash();

  HAL_UART_Receive_IT(&huart1, &user_in_buff[0], BYTE);
  //HAL_UART_Receive_IT(&huart2, &cps_in_buff[0], BYTE);
  uartrx = HAL_UART_Receive_DMA(&huart2, &cps_in_buff[0], BYTE);
  if(uartrx != HAL_OK){
	  rx_uart_err_cnt++;
  }
  else
  {
	  rx_uart_err_cnt = 0;
  }

  HAL_TIM_Base_Start_IT(&htim1);
  HAL_TIM_Base_Start_IT(&htim2);

  EnableTimer(&delaycntr);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */



	  if(i2c_status & I2Cstate_ReadDone)
	  {
		  i2c_status = I2Cstate_NotReady;
		  ADC_value = (ADC_measure_raw[0] << 8) | ADC_measure_raw[1];
		  Current_Measure = abs(ADC_value - 2600);
		  //test_i2c_curr[ChannelPtr] = Current_Measure;

		  //previous value calculation
		  //MeasureDecoderO(Current_Measure, ChannelPtr, adc_measPtr);

		  //new value calculation
		  //MeasureDecoderN(Current_Measure, ChannelPtr);

		  MeasureDecoder(Current_Measure, ChannelPtr, adc_measPtr);


		  //cnt change
		  if(ChannelPtr < ADC_CHNUM-1)
		  {
			  ChannelPtr++;
		  }
		  else
		  {
			  ChannelPtr = 0;
			  VoltageDecoder();


			  if(adc_measPtr < ADC_BUF_SIZE-2)
				  adc_measPtr++;
			  else
			  {
				  adc_measPtr = 0;
				  service_send++;
			  }

			  if(ph_measPtr < PH_BUF_SIZE)
				  ph_measPtr++;
			  else
				  ph_measPtr = 0;


		  }
	  }

	  if(i2c_status == I2Cstate_NotReady && sendComm4ADC)
	  {
		  sendComm4ADC = 0;
		  RestartTimer(&i2cTimeout);
		  ret = i2c1_TX(ADC_ADDR_N, &ADC_COMMAND[ChannelPtr]);
		  if(ret != HAL_OK)
		  {
			  tx_i2c_err_cnt++;
		  }
		  else
		  {

			  i2c_status |= I2Cstate_TransmitStarted;
			  tx_i2c_err_cnt = 0;
		  }
	  }


	  if(i2c_status & I2Cstate_ReadReady)
	  {
		  i2c_status &= ~I2Cstate_ReadReady;
		  if(i2c1_RX(ADC_ADDR_N, ADC_measure_raw, 2) != HAL_OK)
		  {
			  rx_i2c_err_cnt++;
		  }
		  else
		  {
			  i2c_status |= I2Cstate_ReadPending;
			  rx_i2c_err_cnt = 0;
		  }
	  }

	 /* if((tx_i2c_err_cnt > 10) || (rx_i2c_err_cnt > 10))
	  {
		  tx_i2c_err_cnt = 0;
		  rx_i2c_err_cnt = 0;

		  __HAL_RCC_I2C1_FORCE_RESET();
		  __HAL_RCC_I2C1_RELEASE_RESET();

		  i2c_status = I2Cstate_ResetDone;
	  }*/

	  if(HAL_GPIO_ReadPin(JUMPER_GPIO_Port, JUMPER_Pin) == GPIO_PIN_RESET)	// nem cps csomaghosszra van kihasznalva
	  {
		  if(service_send >= 2 && reply4CPS == UartState_NoSend)
		  {
			  service_send = 0;
			  PrepForSend(true);
			  ReplyPacket4USER();
		  }
	  }
	  else
	  {
		  service_send = 0;
	  }

	  /*if(toogleLed)//teszt;
	  {
		  sprintf((char*)UART_buf, "vals U%d R%d F%d\r\n", lamp_state_n, lamp_state, PH_M[6]);

		  HAL_UART_Transmit_IT(&huart1, UART_buf, sizeof(UART_buf));
		  toogleLed = 0;
		  //printf("in main loop \r \n");
	  }*/

		if(pack_in_user_buffer)
		{
			chSum = 0;
			index = 1;
			while (index < (PacketIn_USER - 2))
			{
				chSum += user_in_buff[index++];
			}

			if(chSum == user_in_buff[PacketIn_USER - 2])
			{
				//HAL_UART_Transmit_IT(&huart1, user_in_buff, sizeof(user_in_buff));

				ADC_MAX = (user_in_buff[1] << 8) + user_in_buff[2];
				for (index = 3,k = 0; index < PacketIn_USER-2; index+=2, k++)
					ADC_MIN[k] = (user_in_buff[index] << 8) + user_in_buff[index+1];
				flash_write = true;

			}


			header_received_user = false;
			HAL_UART_Receive_IT(&huart1, &user_in_buff[0], BYTE);
			pack_in_user_buffer = 0;
		}

		if(pack_in_cps_buffer)
		{
			if((cps_in_buff[1] & 0x3F) == ((~cps_in_buff[2]) & 0x3F) && ((cps_in_buff[1] & 0xC0) == 0x40) && ((cps_in_buff[2] & 0xC0) == 0x40))
			{
				//jo packet

				//najavan kivul minden mas torolve
				led_requested &= 0x40; //amikor meri majd a detinputot (hogy ne torolje ki, azt amit kimert)

				//felulirva a ledertek
				led_requested |= cps_in_buff[3]; //ebben benne van a najava es az error erteke is(hogy ha van error akkor beleirja)

				//error led pislogtatas vagy cps error es vilagitson
				if((~led_requested) & 0x80)
				{
					 if(err_blink >= 8)
					 {
						 led_requested |= 0x80;
						 err_blink = 0;
					 }
					 else
						 led_requested &= 0x7F;

					 //cps_nem_kuldott_errort
					errorLed = false;

				}
				else
				{
					//cps_kuldte_error
					errorLed = true;
				}

				//lamp ertek varas elokeszites

			/*#if CPS_VERSION == 2
				lamp_state_a = 0;
				lamp_state_n = 0;
				lamp_state = 0;
			#else
				lamp_state = 0;//cps_in_buff[1];//dbg:ez a teszt idejere
			#endif*/
				//if(HAL_GPIO_ReadPin(JUMPER_GPIO_Port, JUMPER_Pin) == GPIO_PIN_RESET){	//rajta van = rovid csomag
					lamp_state = 0;
				/*}
				else
				{
					lamp_state_a = 0;
					lamp_state_n = 0;
					lamp_state = 0;
				}*/

				lamp_requested = cps_in_buff[1];

				Change_LEDs(convert_LEDs(led_requested), convert_LEDs(led_state));
				Change_LAMPs(convert_LAMPs(lamp_requested));



			}

			pack_in_cps_buffer = false;
			thisIDreceived = false;
			memset(cps_in_buff,0,4);//cps buff hossz
			RestartTimer(&nextVSGtimeout);
			//EnableTimer(&nextVSGtimeout);
			uartrx = HAL_UART_Receive_DMA(&huart2, &cps_in_buff[0], BYTE);
			  if(uartrx != HAL_OK){
				  rx_uart_err_cnt++;
			  }
			  else
			    {
			  	  rx_uart_err_cnt = 0;
			    }
			//HAL_UART_Receive_IT(&huart2, &cps_in_buff[0], BYTE);
		}

		if(reply4CPS == UartState_Ready2Send)
		{
			reply4CPS = UartState_Sending;
			ReplyPacket4CPS();
			RestartTimer(&flashtimer);
		}

		if(flashtimer.interrupt_flag && reply4CPS == UartState_NoSend)
		{
			DisableTimer(&flashtimer);
		}

		if(flashtimer.enabled && reply4CPS == UartState_NoSend)
		{
			if(flash_write)
			{
				flash_write = false;
				WriteValuetoFlash();
				flash_read = true;
			}
			else if(flash_read)
			{
				flash_read = false;
				InitValuefromFlash();
			}
		}

		if(i2cTimeout.interrupt_flag)
		{
			RestartTimer(&i2cTimeout);
			DisableTimer(&i2cTimeout);
			if(i2c_status != I2Cstate_NotReady)
			{
				i2c_status = I2Cstate_NotReady;
			}
		}

		if(uartrx == HAL_BUSY && huart2.RxState == HAL_UART_STATE_READY)
		{
			//HAL_UART_Receive_IT(&huart2, &cps_in_buff[0], BYTE);
			uartrx = HAL_UART_Receive_DMA(&huart2, &cps_in_buff[0], BYTE);
			if(uartrx != HAL_OK){
				rx_uart_err_cnt++;
			}
			else
			{
				rx_uart_err_cnt = 0;
			}
		}

		if(commTimeout.interrupt_flag)
		{
			//error led beallitas, ha nem volt kigyujtva
			if((~led_requested) & 0x80)
			{
			   if(led_requested & 0x80)
			   {
			   		led_requested &= ~0x80;
			   }
			   else
			   {
			   		led_requested |= 0x80;
			   }
			}

			led_requested &= 0x80;
			lamp_requested = 0x00;

			Change_LEDs(convert_LEDs(led_requested), convert_LEDs(led_state));
			Change_LAMPs(convert_LAMPs(lamp_requested));

			HAL_UART_AbortReceive(&huart2);
			waitingNextVSG = false;
			thisIDreceived = false;
			memset(cps_in_buff,0,sizeof(cps_in_buff));

			//HAL_UART_Receive_IT(&huart2, &cps_in_buff[0], BYTE);
			uartrx = HAL_UART_Receive_DMA(&huart2, &cps_in_buff[0], BYTE);
			  if(uartrx != HAL_OK){
				  rx_uart_err_cnt++;
			  }
			  else
			    {
			  	  rx_uart_err_cnt = 0;
			    }
			RestartTimer(&commTimeout);
		}

		if(thisVSGtimeout.interrupt_flag)
		{
			RestartTimer(&thisVSGtimeout);
			DisableTimer(&thisVSGtimeout);
			HAL_UART_AbortReceive(&huart2); //&huart2
			thisIDreceived = false;
			memset(cps_in_buff,0,sizeof(cps_in_buff));//cps buff hossz


			//HAL_UART_Receive_IT(&huart2, &cps_in_buff[0], BYTE);
			uartrx = HAL_UART_Receive_DMA(&huart2, &cps_in_buff[0], BYTE);
			  if(uartrx != HAL_OK){
				  rx_uart_err_cnt++;
			  }
			  else
			    {
			  	  rx_uart_err_cnt = 0;
			    }

		}

		if(nextVSGtimeout.interrupt_flag)
		{
			HAL_UART_AbortReceive(&huart2); //&huart2
			waitingNextVSG = false;
			thisIDreceived = false;
			memset(cps_in_buff,0,sizeof(cps_in_buff));

			//HAL_UART_Receive_IT(&huart2, &cps_in_buff[0], BYTE);
			uartrx = HAL_UART_Receive_DMA(&huart2, &cps_in_buff[0], BYTE);
			  if(uartrx != HAL_OK){
				  rx_uart_err_cnt++;
			  }
			  else
			    {
			  	  rx_uart_err_cnt = 0;
			    }

			RestartTimer(&nextVSGtimeout);
			DisableTimer(&nextVSGtimeout);

		}



  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 240;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */
  //__HAL_RCC_I2C1_CLK_DISABLE();
  //__HAL_RCC_I2C1_FORCE_RESET();
  //__HAL_RCC_I2C1_RELEASE_RESET();
  //__HAL_RCC_I2C1_CLK_ENABLE();
  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 8400-1;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 2;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 840-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 2;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */
/*//#if CPS_VERSION == 0
  if(HAL_GPIO_ReadPin(JUMPER_GPIO_Port, JUMPER_Pin) == GPIO_PIN_RESET){	//rajta van = rovid csomag
  huart2.Init.BaudRate = 19200;	//old cps
  }
//#endif*/
  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 20400;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
  /* DMA1_Stream5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
  /* DMA1_Stream6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);
  /* DMA1_Stream7_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream7_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream7_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, WRLED_Pin|WRHSEC_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, AD7_Pin|AD6_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, AD5_Pin|AD4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, AD3_Pin|AD2_Pin|AD1_Pin|AD0_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, OE_OUT_CN_Pin|OE_VLTG_CHK_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : JUMPER_Pin MID3_Pin MID2_Pin */
  GPIO_InitStruct.Pin = JUMPER_Pin|MID3_Pin|MID2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : WRLED_Pin WRHSEC_Pin AD7_Pin AD6_Pin */
  GPIO_InitStruct.Pin = WRLED_Pin|WRHSEC_Pin|AD7_Pin|AD6_Pin;
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

  /*Configure GPIO pins : AD3_Pin AD2_Pin AD1_Pin AD0_Pin
                           OE_OUT_CN_Pin OE_VLTG_CHK_Pin */
  GPIO_InitStruct.Pin = AD3_Pin|AD2_Pin|AD1_Pin|AD0_Pin
                          |OE_OUT_CN_Pin|OE_VLTG_CHK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : MID1_Pin MID0_Pin */
  GPIO_InitStruct.Pin = MID1_Pin|MID0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	if(hi2c -> Instance == I2C1)
	{
		i2c_status |= I2Cstate_ReadDone;
		i2c_status &= ~I2Cstate_ReadPending;
	}
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	if(hi2c-> Instance == I2C1)
	{
		i2c_status |= I2Cstate_ReadReady;
		i2c_status &= ~I2Cstate_TransmitStarted;
	}
}

void HAL_TIM_PeriodElapsedCallback (TIM_HandleTypeDef* htim)
{
	if(htim -> Instance == TIM1){	//0.0002s

		if(++mstimer>5000)	//1s
		{
			mstimer=0;
			//toogleLed = 1;
			//tx_i2c_err_cnt = 0;
			//rx_i2c_err_cnt = 0;
		}

		if(waitingNextVSG)
			TickTimer(&nextVSGtimeout);

		if(thisIDreceived)
			TickTimer(&thisVSGtimeout);

		TickTimer(&delaycntr);
		TickTimer(&flashtimer);
		TickTimer(&commTimeout);
		TickTimer(&i2cTimeout);
	}

	if(htim -> Instance == TIM2){ //0.00002s
		if(++mstimer2 > 5) //5 = 0.1ms
		{
			mstimer2 = 0;
			sendComm4ADC = 1;

			/*if(i2c_status == I2Cstate_ResetDone)
			{
				i2c_status =  I2Cstate_NotReady;
			}*/

		}

	}

}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart -> Instance == USART1)
	{

	}

	if(huart -> Instance == USART2)
	{
		reply4CPS = UartState_NoSend;
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart -> Instance == USART1){ //from User
		if(!header_received_user)
		{
			if(user_in_buff[0] == Start_Byte){
				header_received_user = true;
				HAL_UART_Receive_IT(&huart1, &user_in_buff[1], PacketIn_USER-1);
			}
			else
			{
				HAL_UART_Receive_IT(&huart1, &user_in_buff[0], BYTE);
			}
		}
		else
		{
			if(user_in_buff[PacketIn_USER-1] == Stop_Byte)
			{
				pack_in_user_buffer = true;
			}
			else
			{
				header_received_user = false;
				HAL_UART_Receive_IT(&huart1, &user_in_buff[0], BYTE);
			}

		}

	}

	if(huart -> Instance == USART2){ //from CPS

		if(!waitingNextVSG)
		{
			if(!thisIDreceived)
			{
				//waiting for packet start byte
				if(cps_in_buff[0] == VSG_ID)
				{
					thisIDreceived = true;
					//receive next bytes
					//HAL_UART_Receive_IT(&huart2, &cps_in_buff[1], 3);//3=(packet size-1)
					uartrx = HAL_UART_Receive_DMA(&huart2, &cps_in_buff[1], 3);
					  if(uartrx != HAL_OK){
						  rx_uart_err_cnt++;
					  }
					  else
					    {
					  	  rx_uart_err_cnt = 0;
					    }
					RestartTimer(&thisVSGtimeout);
					//EnableTimer(&thisVSGtimeout);
				}
				else
				{
					//inchr_cps = 0;
					memset(cps_in_buff, 0, sizeof(cps_in_buff));
					//HAL_UART_Receive_IT(&huart2, &cps_in_buff[0], BYTE);
					uartrx = HAL_UART_Receive_DMA(&huart2, &cps_in_buff[0], BYTE);
					  if(uartrx != HAL_OK){
						  rx_uart_err_cnt++;
					  }
					  else
					    {
					  	  rx_uart_err_cnt = 0;
					    }
				}

			}
			else{
				//rest of packet in buffer OR not their packet.
				if(cps_in_buff[0] == VSG_ID)
				{
					waitingNextVSG = true;
					pack_in_cps_buffer = true;
					DisableTimer(&thisVSGtimeout);
					//u2 is:
					RestartTimer(&commTimeout);
				}
				else
				{
					memset(cps_in_buff, 0, sizeof(cps_in_buff));
					//HAL_UART_Receive_IT(&huart2, &cps_in_buff[0], BYTE);
					uartrx = HAL_UART_Receive_DMA(&huart2, &cps_in_buff[0], BYTE);
					  if(uartrx != HAL_OK){
						  rx_uart_err_cnt++;
					  }//ide nem is megy bele?
					  else
					    {
					  	  rx_uart_err_cnt = 0;
					    }
				}
				thisIDreceived = false;
			}
		}
		else
		{
			//waiting for next packet
			if(cps_in_buff[0] == VSG_ID_NEXT){
				reply4CPS = UartState_Ready2Send;
				waitingNextVSG = false;
				DisableTimer(&nextVSGtimeout);
				err_blink++;
			}

			memset(cps_in_buff, 0, sizeof(cps_in_buff));
			//HAL_UART_Receive_IT(&huart2, &cps_in_buff[0], BYTE);
			uartrx = HAL_UART_Receive_DMA(&huart2, &cps_in_buff[0], BYTE);
			  if(uartrx != HAL_OK){
				  rx_uart_err_cnt++;
			  }
			  else
			    {
			  	  rx_uart_err_cnt = 0;
			    }

		}
		//RestartTimer(&commTimeout);

	}
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
  if (huart->ErrorCode & HAL_UART_ERROR_ORE) {
    //overrun error
    if(huart->Instance == USART2)
    {
       //printf("overrun occured on usart2\n");
    }
    __HAL_UART_CLEAR_OREFLAG(huart);
  }
  if (huart->ErrorCode & HAL_UART_ERROR_PE) {
    // parity error
  }
  if (huart->ErrorCode & HAL_UART_ERROR_FE) {
    // frame error
  }

  if(huart->ErrorCode & HAL_UART_ERROR_NE)
  {
	  __HAL_UART_CLEAR_NEFLAG(huart);
  }
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c) {

    if(hi2c->Instance == I2C1)
    {
       //printf("error occured on i2c1 %i\n", hi2c->ErrorCode);
       if(hi2c->ErrorCode & HAL_I2C_ERROR_ARLO)
       {
    	   __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_ARLO);
       }

       if(hi2c->ErrorCode & HAL_I2C_ERROR_BERR)
       {
    	   __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_BERR);
       }

       if(hi2c->ErrorCode & HAL_I2C_ERROR_TIMEOUT)
       {
    	   /*__HAL_I2C_DISABLE(hi2c);
    	   asm("nop");
    	   __HAL_I2C_ENABLE(hi2c);*/
       }
    }


}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  __disable_irq();
  while (1)
  {
	  //error bit, hogy a timerban ne pislogtassa, esetleg be is allitani hogy egjen az error.
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
