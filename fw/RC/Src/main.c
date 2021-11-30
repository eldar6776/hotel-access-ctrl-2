/**
 ******************************************************************************
 * File Name          : main.c
 * Date               : 9.3.2018.
 * Description        : hotel room controller main function
 ******************************************************************************
 *
 *	sys_stat
 *	0 -> 1 = log added to list
 *	1 -> 1 = log list full
 *	2 -> 1 = file transfer successful
 *	3 -> 1 = file transfer fail
 *	4 -> 1 = sos alarm activ
 *	5 -> 1 =
 *	6 -> 1 =
 *	7 -> 1 =
 *
 ******************************************************************************
 */

#if (__MAIN_H__ != FW_BUILD)
    #error "main header version mismatch"
#endif

#ifndef ROOM_CONTROLLER
    #error "room controller not selected for application in common.h"
#endif

#ifndef APPLICATION
    #error "application not selected for application type in common.h"
#endif
/* Includes ------------------------------------------------------------------*/
#include "display.h"
#include "logger.h"
#include "eeprom.h"
#include "rc522.h"
#include "rs485.h"
#include "owire.h"
#include "room.h"
#include "main.h"
#include "dio.h"
//#include "rf24.h"
/* Constants ----------------------------------------------------------------*/
/* Private Define  ----------------------------------------------------------*/
#define TMR1_REQUIRED_PWM_FREQUENCY     2770U                                               // pwm timer output (required) frequency (Hz)
#define TMR1_INPUT_CLOCK                72000000U                                           // pwm timer input clock frequency (Hz)
#define TMR1_PWM_RESOLUTION             100U                                                // pwm timer output (required) resolution (number of pwm steps)
#define TMR1_COUNTER_FREQUENCY          (TMR1_REQUIRED_PWM_FREQUENCY * TMR1_PWM_RESOLUTION) // calculate required timer ferquency
#define TMR1_PRESCALER                  ((TMR1_INPUT_CLOCK / TMR1_COUNTER_FREQUENCY) -1U)   // calculate timer prescaler value

#define TMR2_REQUIRED_PWM_FREQUENCY     1000U                                               // pwm timer output (required) frequency (Hz)
#define TMR2_INPUT_CLOCK                72000000U                                           // pwm timer input clock frequency (Hz)
#define TMR2_PWM_RESOLUTION             1000U                                               // pwm timer output (required) resolution (number of pwm steps)
#define TMR2_COUNTER_FREQUENCY          (TMR2_REQUIRED_PWM_FREQUENCY * TMR2_PWM_RESOLUTION) // calculate required timer ferquency
#define TMR2_PRESCALER                  ((TMR2_INPUT_CLOCK / TMR2_COUNTER_FREQUENCY) -1U)   // calculate timer prescaler value

#define TMR3_REQUIRED_PWM_FREQUENCY     1000U                                               // pwm timer output (required) frequency (Hz)
#define TMR3_INPUT_CLOCK                72000000U                                           // pwm timer input clock frequency (Hz)
#define TMR3_PWM_RESOLUTION             100U                                                // pwm timer output (required) resolution (number of pwm steps)
#define TMR3_COUNTER_FREQUENCY          (TMR3_REQUIRED_PWM_FREQUENCY * TMR3_PWM_RESOLUTION) // calculate required timer ferquency
#define TMR3_PRESCALER                  ((TMR3_INPUT_CLOCK / TMR3_COUNTER_FREQUENCY) -1U)   // calculate timer prescaler value
/* Imported -----------------------------------------------------------------*/
extern void HAL_TIM_MspPostInit(TIM_HandleTypeDef* htim);
/* Variables ----------------------------------------------------------------*/
static uint32_t reset_source;
/* Private defines    --------------------------------------------------------*/
RTC_TimeTypeDef rtime;
RTC_DateTypeDef rdate;
RTC_HandleTypeDef hrtc;
CRC_HandleTypeDef hcrc;
I2C_HandleTypeDef hi2c1;
SPI_HandleTypeDef hspi2;
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
IWDG_HandleTypeDef hiwdg;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
SIG_BUZZER_TypeDef BUZZ_State = BUZZ_RDY;
/* Private function prototypes -----------------------------------------------*/
static void RAM_Init(void);
static void MX_RTC_Init(void);
static void MX_CRC_Init(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI2_Init(void);
static void MX_TIM2_Init(void);
static void SIGNAL_Buzzer(void);
static void RestartSource(void);
static void MX_USART1_Init(void);
static void MX_USART2_Init(void);
static void SystemClock_Config(void);
#ifdef	USE_WATCHDOG
static void MX_IWDG_Init(void);
#endif
/* Program Code --------------------------------------------------------------*/
/**
  * @brief
  * @param
  * @retval
  */
int main(void)
{
	RestartSource();
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_CRC_Init();
    MX_I2C1_Init();
#ifdef	USE_WATCHDOG
	MX_IWDG_Init();
#endif    
	MX_RTC_Init();
    MX_SPI2_Init();
    RAM_Init();
    MX_TIM1_Init();
    MX_TIM2_Init();
    MX_TIM3_Init();
    MX_USART1_Init();
    MX_USART2_Init();
//    HAL_Delay(123); // wait cap sensor
    DIO_Init();
    DISP_Init();
    RC522_Init();
    LOGGER_Init();
    RS485_Init();    
    OW_Init();
    
    while (1)
    {
        DIO_Service();
        RS485_Service();
        RC522_Service();
        DISP_Service();
        ROOM_Service();        
        OW_Service();
#ifdef	USE_WATCHDOG
		HAL_IWDG_Refresh(&hiwdg);
#endif
    }
}
/**
  * @brief
  * @param
  * @retval
  */
void ErrorHandler(uint8_t function, uint8_t driver)
{
    static uint8_t drv;
    
    if (drv == driver) BootloaderExe(); // on reentrance error restart
    else drv = driver;
    
    switch (driver)
    {
        case SPI_DRV:
            HAL_SPI_MspDeInit(&hspi2);
            MX_SPI2_Init();
            break;
        case USART_DRV:
            HAL_UART_MspDeInit(&huart1);
            HAL_UART_MspDeInit(&huart2);
            MX_USART1_Init();
            MX_USART2_Init();
            break;
        case RTC_DRV:
            HAL_RTC_MspDeInit(&hrtc);
            SystemClock_Config();
            MX_RTC_Init();
            break;
        case TMR_DRV:
            HAL_TIM_PWM_DeInit(&htim1);
            HAL_TIM_PWM_DeInit(&htim2);
            HAL_TIM_PWM_DeInit(&htim3);
            MX_TIM1_Init();
            MX_TIM2_Init();
            MX_TIM3_Init();
            break;
        case CRC_DRV:
            HAL_CRC_MspDeInit(&hcrc);
            MX_CRC_Init();
            break;
        case I2C_DRV:
            HAL_I2C_MspDeInit(&hi2c1);
            MX_I2C1_Init();
            break;
        default:
            BootloaderExe();
            break;
    }

    LogEvent.log_event = FUNC_OR_DRV_FAIL;
    LogEvent.log_group = function;
    LogEvent.log_type = driver;
    LOGGER_Write();
    drv = 0;
}
/**
  * @brief
  * @param
  * @retval
  */
void HAL_SYSTICK_Callback(void)
{
    if (rs485_timer)
	{
        --rs485_timer;
		if (!rs485_timer) RS485_Init();
	}
    
    if (BUZZ_State) SIGNAL_Buzzer();
}
/**
  * @brief
  * @param
  * @retval
  */
void BootloaderExe(void)
{
    HAL_CRC_MspDeInit(&hcrc);
    HAL_I2C_MspDeInit(&hi2c1);
    HAL_RTC_MspDeInit(&hrtc);
    HAL_SPI_MspDeInit(&hspi2);
    HAL_TIM_PWM_DeInit(&htim1);
    HAL_TIM_PWM_DeInit(&htim2);
    HAL_TIM_PWM_DeInit(&htim3);
    HAL_UART_MspDeInit(&huart1);
    HAL_UART_MspDeInit(&huart2);
    HAL_DeInit();
    HAL_FLASH_OB_Launch();
}
/**
  * @brief
  * @param
  * @retval
  */
void MX_TIM1_Init(void)
{
    TIM_ClockConfigTypeDef sClockSourceConfig;
    TIM_MasterConfigTypeDef sMasterConfig;
    TIM_OC_InitTypeDef sConfigOC;

    htim1.Instance = TIM1;
    htim1.Init.Prescaler = TMR1_PRESCALER;
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Period = TMR1_PWM_RESOLUTION;
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0U;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)ErrorHandler(MAIN_FUNC, TMR_DRV);
    
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK) ErrorHandler(MAIN_FUNC, TMR_DRV);

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)ErrorHandler(MAIN_FUNC, TMR_DRV);

    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0U;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)ErrorHandler(MAIN_FUNC, TMR_DRV);

    HAL_TIM_MspPostInit(&htim1);
}
/**
  * @brief
  * @param
  * @retval
  */
void MX_TIM3_Init(void)
{
	TIM_MasterConfigTypeDef sMasterConfig;
	TIM_OC_InitTypeDef sConfigOC;

	htim3.Instance = TIM3;
	htim3.Init.Prescaler = TMR3_PRESCALER;
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = TMR3_PWM_RESOLUTION;
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

	if (HAL_TIM_PWM_Init(&htim3) != HAL_OK) ErrorHandler(MAIN_FUNC, TMR_DRV);
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;

	if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK) ErrorHandler(MAIN_FUNC, TMR_DRV);
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 0U;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK) ErrorHandler(MAIN_FUNC, TMR_DRV);
    
	HAL_TIM_MspPostInit(&htim3);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
}
/**
  * @brief
  * @param
  * @retval
  */
static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

	/**Initializes the CPU, AHB and APB busses clocks
	*/
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_LSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
	RCC_OscInitStruct.LSEState = RCC_LSE_ON;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.LSIState = RCC_LSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;

	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) ErrorHandler(MAIN_FUNC, RTC_DRV);

	/**Initializes the CPU, AHB and APB busses clocks
	*/
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)ErrorHandler(MAIN_FUNC, RTC_DRV);

	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
	PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
	PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)ErrorHandler(MAIN_FUNC, RTC_DRV);

	/**Configure the Systick interrupt time
	*/
//	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);
	/**Configure the Systick
	*/
//	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
	/* SysTick_IRQn interrupt configuration */
//	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_CRC_Init(void)
{
	hcrc.Instance = CRC;
	HAL_CRC_Init(&hcrc);
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_I2C1_Init(void)
{
    if(HAL_I2C_GetState(&hi2c1) == HAL_I2C_STATE_RESET)
    {
        hi2c1.Instance = I2C1;
        hi2c1.Init.ClockSpeed       = 400000;
        hi2c1.Init.DutyCycle        = I2C_DUTYCYCLE_2;
        hi2c1.Init.OwnAddress1      = 0U;
        hi2c1.Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
        hi2c1.Init.DualAddressMode  = I2C_DUALADDRESS_DISABLE;
        hi2c1.Init.OwnAddress2      = 0U;
        hi2c1.Init.GeneralCallMode  = I2C_GENERALCALL_DISABLE;
        hi2c1.Init.NoStretchMode    = I2C_NOSTRETCH_DISABLE;
        if (HAL_I2C_Init(&hi2c1) != HAL_OK) ErrorHandler(MAIN_FUNC, I2C_DRV);
    }
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_RTC_Init(void)
{
    hrtc.Instance = RTC;
    hrtc.Init.OutPut = RTC_OUTPUTSOURCE_NONE;
    hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
    HAL_RTC_Init(&hrtc);
    
	if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1) == 0xA5A5U)
	{
        RtcTimeValidSet();
        rdate.Date      = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR2);
		rdate.Month     = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR3);
		rdate.WeekDay   = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR4);
		rdate.Year      = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR5);
	}
    else
    {
        RtcTimeValidReset();
        rtime.Hours     = 0x12U;
        rtime.Minutes   = 0x0U;
        rtime.Seconds   = 0x0U;
        rdate.Date      = 0x1U;
        rdate.Month     = 0x1U;
        rdate.WeekDay   = 0x1U;
        rdate.Year      = 0x20U;
        HAL_RTC_SetTime(&hrtc, &rtime, RTC_FORMAT_BCD);
    }
    HAL_RTC_SetDate(&hrtc, &rdate, RTC_FORMAT_BCD);
    HAL_RTC_WaitForSynchro(&hrtc);
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0xA5A5U);
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_SPI2_Init(void)
{
    hspi2.Instance = SPI2;
    hspi2.Init.Mode = SPI_MODE_MASTER;
    hspi2.Init.Direction = SPI_DIRECTION_2LINES;
    hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi2.Init.NSS = SPI_NSS_SOFT;
    hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
    hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi2.Init.CRCPolynomial = 7;
    if(HAL_SPI_Init(&hspi2) != HAL_OK) ErrorHandler(MAIN_FUNC, SPI_DRV);
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_TIM2_Init(void)
{
	TIM_MasterConfigTypeDef sMasterConfig;
	TIM_OC_InitTypeDef sConfigOC;

	htim2.Instance = TIM2;
	htim2.Init.Prescaler = TMR2_PRESCALER;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = TMR2_PWM_RESOLUTION;
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

	if (HAL_TIM_PWM_Init(&htim2) != HAL_OK) ErrorHandler(MAIN_FUNC, TMR_DRV);
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;

	if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK) ErrorHandler(MAIN_FUNC, TMR_DRV);
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 100U;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK) ErrorHandler(MAIN_FUNC, TMR_DRV);
    
	HAL_TIM_MspPostInit(&htim2);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_USART1_Init(void)
{
    huart1.Instance = USART1;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    huart1.Init.BaudRate = bps[rs485_bps];
    if(HAL_UART_Init(&huart1) != HAL_OK) ErrorHandler(MAIN_FUNC, USART_DRV);
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_USART2_Init(void)
{
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 9600U;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.WordLength = UART_WORDLENGTH_9B;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    if(HAL_UART_Init(&huart2) != HAL_OK) ErrorHandler(MAIN_FUNC, USART_DRV);
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();


	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_7|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_15, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11|GPIO_PIN_12, GPIO_PIN_SET);

	/*Configure GPIO pin : PC13 */
	GPIO_InitStruct.Pin = GPIO_PIN_13;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pins : PA4 PA5 PA6 PA7 PA8 PA15 */
	GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_7|GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : PA11 PA12 */
	GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : PB0 PB1 PB2 PB3
						   PB4 PB5 PB6 PB7 */
	GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
						  |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pins : PB10 PB11 PB12 */
	GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}
/**
  * @brief
  * @param
  * @retval
  */
static void RAM_Init(void)
{
    uint32_t t;
	uint8_t ebuf[EE_VAR_END_ADD];
    ebuf[0] = 0;
    ebuf[1] = 0;
    if (!FLASH_ReleasePowerDown())                                                          ErrorHandler(MAIN_FUNC, SPI_DRV);   // spi flash memory chip error
    if (HAL_I2C_Master_Transmit(&hi2c1, I2CEE_ADD, ebuf, 2, DRV_TOUT) != HAL_OK)            ErrorHandler(MAIN_FUNC, I2C_DRV);   // i2c hal driver error
    if (HAL_I2C_Master_Receive (&hi2c1, I2CEE_ADD, ebuf, sizeof(ebuf), DRV_TOUT) != HAL_OK) ErrorHandler(MAIN_FUNC, I2C_DRV);   // i2c hal driver error
    ROOM_PreStatus  = (ROOM_StatusTypeDef) ebuf[EE_ROOM_PRESTAT_ADD];           // room previous status address
    ROOM_Status     = (ROOM_StatusTypeDef) ebuf[EE_ROOM_STAT_ADD];              // room status address
    fw_upd_stat     = ebuf[EE_FW_UPDATE_STATUS];    // firmware update status
    temp_sp         = ebuf[EE_ROOM_TEMP_SP];        // room setpoint temp in degree of Celsious
    temp_dif        = ebuf[EE_ROOM_TEMP_DIFF];      // room tempreature on / off difference
    temp_cfg        = ebuf[EE_ROOM_TEMP_CFG];       // room thermostat config
    rs485_bps       = ebuf[EE_RSBPS];               // rs485 interface bpsrate
    lcd_bcklght     = (ebuf[EE_LCD_BRIGHTNESS]<<8)|ebuf[EE_LCD_BRIGHTNESS+1];   // lcd display backlight LED 
    disp_sta        = ebuf[EE_DISP_STATUS_ADD];     // display status flags
    buzzer_volume   = ebuf[EE_BUZZER_VOLUME_ADD];   // buzzer volume address
    doorlock_force  = ebuf[EE_DOORLOCK_FORCE_ADD];  // doorlock force address
    disp_rot        = ebuf[EE_DISP_ROTATION_ADD];   // display orientation address
//    rf24chanel      = ebuf[EE_RADIO_CHANEL_ADD];
//    rf24address     = ebuf[EE_RADIO_ADDRESS_ADD];
//    rf24cfg         = ebuf[EE_RADIO_CFG_ADD];
    bedd_cnt        = ebuf[EE_BEDNG_CNT_ADD];       // room bedding replacement counter
    bedd_tim        = ebuf[EE_BEDNG_REPL_ADD];      // room bedding replacement period
    mem_cpy (din_cfg,  &ebuf[EE_DIN_CFG_ADD_1],  8);
    mem_cpy (dout_cfg, &ebuf[EE_DOUT_CFG_ADD_1], 8);
    /* room power expiry date & time in unix time stamp format  */
    unix_room = ((ebuf[EE_ROOM_PWRTOUT]<<24)|(ebuf[EE_ROOM_PWRTOUT+1]<<16)|(ebuf[EE_ROOM_PWRTOUT+2]<<8)|ebuf[EE_ROOM_PWRTOUT+3]); 
    if (ebuf[EE_ROOM_PWRTOUT+5] == 1) PwrExpTimeSet();
    
    for (t = 0; t < 2; t++)     // rs485 addresse & system id
    {
        rs485_ifa[t] = ebuf[EE_RSIFA + t];  // rs485 interface address
        rs485_gra[t] = ebuf[EE_RSGRA + t];  // rs485 group address
        rs485_bra[t] = ebuf[EE_RSBRA + t];  // rs485 broadcast address
        system_id[t] = ebuf[EE_SYSID + t];  // system id (system unique number)
    }

    for (t = 0; t < 6; t++)     // mifare access authentication keys
    {
       
        mifare_keya[t] = ebuf[EE_MIFARE_KEYA + t];    // key A 
        mifare_keyb[t] = ebuf[EE_MIFARE_KEYB + t];    // key B 
    }
 
    for (t = 0; t < 8; t++)     // additional permited addresse
    {
        permitted_add[t][0] = ebuf[EE_PERM_EXTADD1 + (t*2)];    // msb
        permitted_add[t][1] = ebuf[EE_PERM_EXTADD1 + (t*2)+1];  // lsb
        if(din_cfg[t] > 5) din_cfg[t] = 3;
        if(dout_cfg[t]> 5) dout_cfg[t]= 3;
    }
    ow_ifa = ebuf[EE_OWIFA];    // onewire interface address
    ow_gra = ebuf[EE_OWGRA];    // onewire group address
    ow_bra = ebuf[EE_OWBRA];    // onewire broadcast address
    ow_bps = ebuf[EE_OWBPS];    // onewire interface baud rate
    
    if      (ow_ifa <   DEF_RC_OWIFA)               ow_ifa = DEF_RC_OWIFA;
    else if (ow_ifa >= (DEF_RC_OWIFA + OW_DEV_CNT)) ow_ifa = DEF_RC_OWIFA;
    if      (ow_gra <   DEF_RC_OWGRA)               ow_gra = DEF_RC_OWGRA;
    else if (ow_gra >= (DEF_RC_OWGRA + OW_DEV_CNT)) ow_gra = DEF_RC_OWGRA;
    if      (ow_bra <   DEF_OWBRA)                  ow_bra = DEF_OWBRA;
    else if (ow_bra == 0xFFU)                       ow_bra = DEF_OWBRA;
    if     (!ow_bps || (ow_bps > BUFFERSIZE(bps)))  ow_bps = 2;
    
    ow_dev = 0;  
    for (t = 0; t < 9; t++) // number of connected onewire device
    {
        ow_add[ow_dev] = ebuf[EE_OWADD1+t];   // onewire device address list
        if (ow_add[ow_dev] && (ow_add[ow_dev] < DEF_RT_OWGRA)) ++ow_dev;    // increase number of connected device
    }    
	
	if (ISVALIDDEC(rs485_bps))  rs485_bps = TODEC(rs485_bps);
    else                        rs485_bps = 6;  // default bpsrate 115200bps
	
    if (reset_source)
	{
        LogEvent.log_event = reset_source;  // where  PIN_RESET is ((uint8_t)0xd0)
		LOGGER_Write();
	}
    
    if (fw_upd_stat == BLDR_UPD_OK)
    {
        SYS_UpdateSuccessSet();
        DISP_FwrUpdd();
        LogEvent.log_event = FW_UPDATED;
        LOGGER_Write();
    }
    
    if (fw_upd_stat == BLDR_UPD_ERR)
    {
        SYS_UpdateFailSet();
        DISP_FwrUpdFail();
        LogEvent.log_event = FW_UPD_FAIL;
        LOGGER_Write();
    }

    if (fw_upd_stat)
    {
        fw_upd_stat = 0;
        ebuf[0] = EE_FW_UPDATE_STATUS>>8;
        ebuf[1] = EE_FW_UPDATE_STATUS;
        ebuf[2] = 0;
        if (HAL_I2C_Master_Transmit (&hi2c1, I2CEE_ADD, ebuf, 3, DRV_TOUT) != HAL_OK)   ErrorHandler(MAIN_FUNC, I2C_DRV);
        if (HAL_I2C_IsDeviceReady   (&hi2c1, I2CEE_ADD, DRV_TRIAL, DRV_TOUT) != HAL_OK) ErrorHandler(MAIN_FUNC, I2C_DRV);
    }
}
/**
  * @brief
  * @param
  * @retval
  */
static void RestartSource(void)
{
    reset_source = 0x0U;
    if      (__HAL_RCC_GET_FLAG(RCC_FLAG_LPWRRST))  reset_source = LOW_POWER_RESET;
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST))   reset_source = POWER_ON_RESET;
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST))   reset_source = SOFTWARE_RESET;
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST))  reset_source = IWDG_RESET;
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST))   reset_source = PIN_RESET;
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST))  reset_source = WWDG_RESET;            
	 __HAL_RCC_CLEAR_RESET_FLAGS();
}
/**
  * @brief
  * @param
  * @retval
  */
static void SIGNAL_Buzzer(void)
{
    static uint32_t signal_pcnt = 0x0U;
    static uint32_t signal_timer = 0x0U;
    static uint32_t signal_time = 0x0U;
    /************************************************/
	/*		B U Z Z E R			S I G N A L			*/
	/************************************************/
    if((HAL_GetTick() - signal_timer) >= signal_time)
    {
        signal_timer = HAL_GetTick();
        
        switch(BUZZ_State)
        {
            case BUZZ_OFF:
            {
                Buzzer_Off();
                signal_timer = 0x0U;
                signal_time = 0x0U;
                signal_pcnt = 0x0U;
                BUZZ_State = BUZZ_RDY;
                break;
            } 
            
            
            case BUZZ_CARD_INVALID:     //  3 short buzzer bips
            {       
                if(!IsBuzzerActiv()) Buzzer_On();
                else Buzzer_Off();
                signal_time = 50U;
                ++signal_pcnt;
                if(signal_pcnt >= 0x5U)  BUZZ_State = BUZZ_OFF;
                break;
            } 
            
            
            case BUZZ_CARD_VALID:       //  2 short buzzer bips
            {
                Buzzer_On();
                signal_time = 500U;
                BUZZ_State = BUZZ_OFF;
                break;
            } 
            
            
            case BUZZ_DOOR_BELL:        //  1 short buzzer bips
            {    
                Buzzer_On();
                signal_time = 50U;
                BUZZ_State = BUZZ_OFF;
                break;
            } 
            
            case BUZZ_CLICK:        //  1 short buzzer bips
            {    
                Buzzer_On();
                signal_time = 10U;
                BUZZ_State = BUZZ_OFF;
                break;
            } 
            
            case BUZZ_CLEANING_END:
            {    
                Buzzer_On();
                signal_time = 700U;
                BUZZ_State = BUZZ_OFF; 
                break;
            } 
            
            
            case BUZZ_SOS_ALARM:
            {  
                BUZZ_State = BUZZ_OFF; 
                break;
            } 
            
            
            case BUZZ_FIRE_ALARM:
            { 
                if((ROOM_Status != ROOM_FIRE_ALARM) && (ROOM_Status != ROOM_FIRE_EXIT))
                {
                    BUZZ_State = BUZZ_OFF;
                }
                else
                {
                    if(!IsBuzzerActiv()) 
                    {
                        Buzzer_On();
                        signal_time = 50U;
                    }
                    else 
                    {
                        Buzzer_Off();
                        signal_time = 600U;
                    }
                }
                break;
            }  
            
            case BUZZ_RDY:
            default:
            {
                BUZZ_State = BUZZ_OFF; 
                break;
            }
        }
        DOUT_Buzzer();
    }
    
}
/**
  * @brief
  * @param
  * @retval
  */
#ifdef USE_WATCHDOG
static void MX_IWDG_Init(void)
{
    hiwdg.Instance = IWDG;
    hiwdg.Init.Reload = 4095U;
    hiwdg.Init.Prescaler = IWDG_PRESCALER_64;
    HAL_IWDG_Init(&hiwdg);
}
#endif
/**
  * @brief
  * @param
  * @retval
  */
#ifdef USE_FULL_ASSERT
/**
 * @brief Reports the name of the source file and the source line number
 * where the assert_param error has occurred.
 * @param file: pointer to the source file name
 * @param line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
      ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */

}

#endif
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
