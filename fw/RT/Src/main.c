/**
 ******************************************************************************
 * File Name        : main.c
 * rtcdt            : 10.3.2018.
 * Description      : hotel room thermostat main function
 ******************************************************************************
 *
 *
 ******************************************************************************
 */
#if (__MAIN_H__ != FW_BUILD)
    #error "main header version mismatch"
#endif

#ifndef ROOM_THERMOSTAT
    #error "room thermostat not selected for application in common.h"
#endif

#ifndef APPLICATION
    #error "application not selected for application type in common.h"
#endif
/* Includes ------------------------------------------------------------------*/
#include "png.h"
#include "main.h"
#include "rs485.h"
#include "logger.h"
#include "display.h"
#include "onewire.h"
#include "stm32746g.h"
#include "stm32746g_ts.h"
#include "stm32746g_qspi.h"
#include "stm32746g_sdram.h"
#include "stm32746g_eeprom.h"
/* Constants ----------------------------------------------------------------*/
/* Imported Type  ------------------------------------------------------------*/
/* Imported Variable  --------------------------------------------------------*/
/* Imported Function  --------------------------------------------------------*/
//__STATIC_INLINE void DelayUS(__IO uint32_t micros) {
//	__IO uint32_t usdel = micros * 50;
//	while (usdel--) {
//	}
//}
/* Private Type --------------------------------------------------------------*/
RTC_TimeTypeDef rtctm;
RTC_DateTypeDef rtcdt;
RTC_HandleTypeDef hrtc;
CRC_HandleTypeDef hcrc;
ADC_HandleTypeDef hadc3;
TIM_HandleTypeDef htim9;
I2C_HandleTypeDef hi2c4;
I2C_HandleTypeDef hi2c3;
SPI_HandleTypeDef hspi2;
IWDG_HandleTypeDef hiwdg;
QSPI_HandleTypeDef hqspi;
LTDC_HandleTypeDef hltdc;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
DMA2D_HandleTypeDef hdma2d;
/* Private Define ------------------------------------------------------------*/
#define TOUCH_SCREEN_UPDATE_TIME			50U     // 50ms touch screen update period
#define TOUCH_SCREEN_LAYER                  1U      // touch screen layer event
#define AMBIENT_NTC_RREF                    10000U  // 10k NTC value of at 25 degrees
#define AMBIENT_NTC_B_VALUE                 3977U   // NTC beta parameter
#define AMBIENT_NTC_PULLUP                  10000U	// 10k pullup resistor
#define FANC_NTC_RREF                       2000U  	// 2k fancoil NTC value of at 25 degrees
#define FANC_NTC_B_VALUE                    3977U   // NTC beta parameter
#define FANC_NTC_PULLUP                     2200U	// 2k2 pullup resistor
#define ADC_READOUT_PERIOD                  234U    // 89 ms ntc conversion rate
#define FAN_CONTROL_LOOP_PERIOD			    200U    // fan speed control loop
#define BUZZER_CLICK_TIME                   20U     // single 50 ms tone when button pressed
#define BUZZER_SHORT_TIME                   100U    // 100 ms buzzer activ then repeat pause
#define BUZZER_MIDDLE_TIME                  500U    // 500 ms buzzer activ then repeat pause
#define BUZZER_LONG_TIME                    1000U   // 1 s buzzer activ then repeat pause
#define TRIAC_ON_PULSE                      5U      // 500 us triac fire pulse duration
#define SYSTEM_STARTUP_TIME                 8765U   // 8s application startup time
#define RFADDR_MIN                          10       
#define RFADDR_MAX                          9999
/* Private Variable ----------------------------------------------------------*/
__IO uint32_t sys_flags = 0;
__IO uint8_t sys_stat   = 0;
uint32_t thstfl_memo    = 0;
uint8_t buzz_sig_id     = 0;
uint8_t buzz_sig_time   = 0;
static uint32_t rstsrc  = 0;
static uint8_t spirx[16] = {0};
static uint8_t spibcnt = 0;
__IO uint16_t rfsenadr = 0;
__IO uint8_t rfsensta = 0;
//const uint8_t rfinit[81]= {0x0B,0x06,0x0C,0x00,0x0D,0x10,0x0E,0xB6,0x0F,0xE4,0x10,0xC8,0x11,0x93,0x12,0x11,0x13,0xA2,0x14,0xF8,\
//                           0x0A,0x00,0x15,0x34,0x21,0x56,0x22,0x10,0x17,0x2C,0x18,0x38,0x19,0x16,0x1A,0x6C,0x1B,0x03,0x1C,0x40,\
//                           0x1D,0x91,0x23,0xA9,0x24,0x2A,0x25,0x00,0x26,0x11,0x29,0x59,0x2C,0x81,0x2D,0x35,0x2E,0x0B,0x00,0x0A,\
//                           0x02,0x07,0x07,0x48,0x08,0x44,0x09,0x00,0x06,0x03,0x16,0x18,0x1E,0x01,0x1F,0xC2,0x20,0xB8,0x3E,0x60};
/* Private Macro -------------------------------------------------------------*/
/* Private Function Prototype ------------------------------------------------*/
void MX_IWDG_Init(void);
static void RAM_Init(void);
static void ADC3_Read(void);
static void MPU_Config(void);
static void MX_RTC_Init(void);
static void MX_CRC_Init(void);
static void SaveResetSrc(void);
static void CACHE_Config(void);
static void MX_GPIO_Init(void);
void MX_TIM3_Init(void);
static void MX_TIM9_Init(void);
static void MX_ADC3_Init(void);
static void MX_SPI2_Init(void);
static void MX_UART1_Init(void);
static void MX_UART2_Init(void);
static void MX_CRC_DeInit(void);
static void MX_RTC_DeInit(void);
static void MX_TIM9_DeInit(void);
static void MX_I2C3_DeInit(void);
static void MX_I2C4_DeInit(void);
static void MX_ADC3_DeInit(void);
static void BUZZER_Service(void);
static void MX_SPI2_DeInit(void);
static void MX_GPIO_DeInit(void);
static void MX_UART1_DeInit(void);
static void MX_UART2_DeInit(void);
static void SystemClock_Config(void);
static void TOUCH_SCREEN_Service(void);
static uint32_t RTC_GetUnixTimeStamp(RTC_t* data);
static float ROOM_GetTemperature(uint16_t adc_value);
/* Program Code  -------------------------------------------------------------*/
/**
  * @brief
  * @param
  * @retval
  */
int main(void)
 {
    SYSTEM_StartupSet();
    SaveResetSrc();
	MPU_Config();
	CACHE_Config();
	HAL_Init();
	SystemClock_Config();
#ifdef	USE_WATCHDOG
	MX_IWDG_Init();
#endif
    MX_CRC_Init();
	MX_RTC_Init();
	MX_ADC3_Init();
    MX_UART1_Init();
	MX_UART2_Init();
	MX_TIM9_Init();
	MX_GPIO_Init();
    MX_SPI2_Init();
	MX_QSPI_Init();
    QSPI_MemMapMode();
    EE_Init();
    TOUCH_SCREEN_Init();
	SDRAM_Init();
    LOGGER_Init();
    RAM_Init();
	DISP_Init();
	OW_Init();
    RS485_Init();
#ifdef	USE_WATCHDOG
	HAL_IWDG_Refresh(&hiwdg);
#endif


	while(1)
	{
        ADC3_Read();
        TOUCH_SCREEN_Service();
		DISP_Service();
        BUZZER_Service();
        RS485_Service();
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
void Restart(void)
{
    MX_GPIO_DeInit();
    MX_ADC3_DeInit();
    MX_I2C3_DeInit();
    MX_I2C4_DeInit();
    MX_TIM9_DeInit();
    MX_SPI2_DeInit();
    MX_UART1_DeInit();
    MX_UART2_DeInit();
    HAL_QSPI_DeInit(&hqspi);
    MX_RTC_DeInit();
    MX_CRC_DeInit();
    HAL_RCC_DeInit();
    HAL_DeInit();
    SCB_DisableICache();
    SCB_DisableDCache();
    HAL_NVIC_SystemReset();
}
/**
  * @brief
  * @param
  * @retval
  */
void MX_IWDG_Init(void)
{
    hiwdg.Instance = IWDG;
    hiwdg.Init.Prescaler = IWDG_PRESCALER_32; //(1/(32000/32))*4095 = 4,095s
    hiwdg.Init.Window = 4095;
    hiwdg.Init.Reload = 4095;
    if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
    {
        Restart();
    }
}
/**
  * @brief  Convert from Binary to 2 digit BCD.
  * @param  Value: Binary value to be converted.
  * @retval Converted word
  */
void RTC_GetDateTime(RTC_t* data, uint32_t format)
{
	uint32_t unix;

	/* Get rtctm */
	HAL_RTC_GetTime(&hrtc, &rtctm, format);
	/* Format hours */
	data->hours = rtctm.Hours;
	data->minutes = rtctm.Minutes;
	data->seconds = rtctm.Seconds;
	/* Get subseconds */
	data->subseconds = RTC->SSR;
	/* Get rtcdt */
	HAL_RTC_GetDate(&hrtc, &rtcdt, format);
	/* Format rtcdt */
	data->year = rtcdt.Year;
	data->month = rtcdt.Month;
	data->date = rtcdt.Date;
	data->day = rtcdt.WeekDay;
	/* Calculate unix offset */
	unix = RTC_GetUnixTimeStamp(data);
	data->unix = unix;
}
/**
  * @brief  Convert from Binary to 2 digit BCD.
  * @param  Value: Binary value to be converted.
  * @retval Converted word
  */
void RTC_GetDateTimeFromUnix(RTC_t* data, uint32_t unix)
{
	uint16_t year;

	/* Store unix rtctm to unix in struct */
	data->unix = unix;
	/* Get seconds from unix */
	data->seconds = unix % 60U;
	/* Go to minutes */
	unix /= 60U;
	/* Get minutes */
	data->minutes = unix % 60U;
	/* Go to hours */
	unix /= 60U;
	/* Get hours */
	data->hours = unix % 24U;
	/* Go to days */
	unix /= 24U;
	/* Get week day */
	/* Monday is day one */
	data->day = (unix + 3U) % 7U + 1U;
	/* Get year */
	year = 1970U;

	while (1U)
	{
		if (LEAP_YEAR(year))
		{
			if (unix >= 366U) unix -= 366U;
			else break;
		}
		else if (unix >= 365U) unix -= 365U;
		else break;
		year++;
	}

	/* Get year in xx format */
	data->year = (uint8_t) (year - 2000U);

	/* Get month */
	for (data->month = 0; data->month < 12; data->month++)
    {
		if (LEAP_YEAR(year) && unix >= (uint32_t)rtc_month[1][data->month])
        {
			unix -= rtc_month[1][data->month];
		}
        else if (unix >= (uint32_t)rtc_month[0][data->month])
        {
			unix -= rtc_month[0][data->month];
		}
        else  break;
	}

	/* Get month */
	/* Month starts with 1 */
	data->month++;
	/* Get rtcdt */
	/* rtcdt starts with 1 */
	data->date = unix + 1U;
}


/**
  * @brief
  * @param
  * @retval
  */
void ErrorHandler(uint8_t function, uint8_t driver)
{
    LogEvent.log_type = driver;
    LogEvent.log_group = function;
    LogEvent.log_event = FUNC_OR_DRV_FAIL;
    if (driver != I2C_DRV) LOGGER_Write();
    Restart();
}
/**
  * @brief
  * @param
  * @retval
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart)
{
    if      (huart->Instance == USART1)
	{
        RS485_RxCpltCallback();
    }
    else if (huart->Instance == USART2)
	{
        OW_RxCpltCallback();
    }
}
/**
  * @brief
  * @param
  * @retval
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef* huart)
{
    if      (huart->Instance == USART1)
	{
        RS485_TxCpltCallback();
    }
    else if (huart->Instance == USART2)
	{
        OW_TxCpltCallback();
    }
}
/**
  * @brief
  * @param
  * @retval
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef* huart)
{
    if      (huart->Instance == USART1)
	{
        RS485_ErrorCallback();
    }
    else if (huart->Instance == USART2)
	{
        OW_ErrorCallback();
    }
}
/**
  * @brief
  * @param
  * @retval
  */
void HAL_RTC_MspInit(RTC_HandleTypeDef *hrtc)
{
	__HAL_RCC_RTC_ENABLE();
}
/**
  * @brief
  * @param
  * @retval
  */
void HAL_RTC_MspDeInit(RTC_HandleTypeDef *hrtc)
{
	__HAL_RCC_RTC_DISABLE();
}
/**
  * @brief
  * @param
  * @retval
  */
void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    GPIO_InitTypeDef  GPIO_InitStruct;

    if (hspi->Instance == SPI2)
    {
        __HAL_RCC_SPI2_CLK_ENABLE();
      
        __HAL_RCC_GPIOB_CLK_ENABLE();
        /**SPI2 GPIO Configuration    
        PB12     ------> SPI2_NSS
        PB13     ------> SPI2_SCK
        PB15     ------> SPI2_MOSI 
        */
        GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_15;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        /* SPI2 interrupt Init */
        HAL_NVIC_SetPriority(SPI2_IRQn, 3, 0);
        HAL_NVIC_EnableIRQ(SPI2_IRQn);
    }
}
/**
  * @brief SPI MSP De-Initialization
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO and NVIC configuration to their default state
  * @param hspi: SPI handle pointer
  * @retval None
  */
void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi)
{
    if(hspi->Instance == SPI2)
    {
        __HAL_RCC_SPI2_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15);
    }
}
/**
  * @brief
  * @param
  * @retval
  */
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
    ++spibcnt;
    
    switch (spibcnt)
    {
        case 6:
            spibcnt = 0x0;
            if ((spirx[5] & 0xF) > 0x1) break;
            if (((spirx[4] << 0x4)|(spirx[5] >> 0x4)) < RFADDR_MIN) break;
            if (((spirx[4] << 0x4)|(spirx[5] >> 0x4)) > RFADDR_MAX) break;
            if ((rfsensta != 0x10+(spirx[5] & 0xF))
            ||  (rfsenadr != ((spirx[4]<<0x4)|(spirx[5]>>0x4))))
            {
                rfsensta = 0x10|(spirx[5] & 0xF);
                rfsenadr = ((spirx[4]<<0x4)|(spirx[5]>>0x4));
                DISP_RFSensor(rfsenadr, rfsensta); // update sensor event
            }
        case 5:
        case 4:
            if (spirx[3] != 0x0) spibcnt = 0x0;
        case 3:
            if (spirx[2] != 0x3) spibcnt = 0x0;
        case 2:
            if (spirx[1] != 0x3) spibcnt = 0x0;
        case 1:
            if (spirx[0] != 0x3) spibcnt = 0x0;
        case 0:
        default:
            break;
    }
    
    if (!spibcnt) ZEROFILL(spirx,sizeof(spirx));
    HAL_SPI_Receive_IT(&hspi2, &spirx[spibcnt], 0x1);
}
/**
  * @brief
  * @param
  * @retval
  */
static void RAM_Init(void)
{
    uint8_t ebuf[64];

    HAL_I2C_Mem_Read(&hi2c4, EE_ADDR, EE_THST_FLAGS, I2C_MEMADD_SIZE_16BIT, ebuf, 64U, EETOUT);
    thstfl_memo         =(ebuf[0]<<24)|(ebuf[1]<<16)|(ebuf[2]<<8)|ebuf[3];
    thst_min_sp         = ebuf[4];
    thst_max_sp         = ebuf[5];
    thst_sp             = ebuf[6];
    ntc_offset          = ebuf[7];
    ow_bps              = ebuf[8];
    ow_ifa              = ebuf[9];
    ow_gra              = DEF_RT_OWGRA;
    ow_bra              = DEF_OWBRA;
    disp_low_bcklght    = ebuf[10];
    disp_high_bcklght   = ebuf[11];
    scrnsvr_tout        = ebuf[12];
    scrnsvr_ena_hour    = ebuf[13];
    scrnsvr_dis_hour    = ebuf[14];
    scrnsvr_clk_clr     = ebuf[15];
    scrnsvr_semiclk_clr = ebuf[16];
    sys_stat            = ebuf[17];

    rfsen[0] = (ebuf[32]<<8)|ebuf[33];
    rfsen[1] = (ebuf[34]<<8)|ebuf[35];
    rfsen[2] = (ebuf[36]<<8)|ebuf[37];
    rfsen[3] = (ebuf[38]<<8)|ebuf[39];
    rfsen[4] = (ebuf[40]<<8)|ebuf[41];
    rfsen[5] = (ebuf[42]<<8)|ebuf[43];
    rfsen[6] = (ebuf[44]<<8)|ebuf[45];
    rfsen[7] = (ebuf[46]<<8)|ebuf[47];

    if ((thst_min_sp > 45U) || (thst_min_sp < 10U)) thst_min_sp = 16U;
    if ((thst_max_sp > 45U) || (thst_max_sp < 10U)) thst_max_sp = 32U;
    if (thst_sp > thst_max_sp) thst_sp = thst_max_sp;
    if (thst_sp < thst_min_sp) thst_sp = thst_min_sp;
    if ((ntc_offset > 100) || (ntc_offset < -100)) ntc_offset = 0;
    if (ow_bps  >= COUNTOF(bps)) ow_bps = 0x2U; // default   9600 bps
    if ((ow_ifa > DEF_RT_OWIFA) || (!ow_ifa)) ow_ifa = DEF_RT_OWIFA;
    if ((disp_low_bcklght  < DISP_BRGHT_MIN) || (disp_low_bcklght  > DISP_BRGHT_MAX)) disp_low_bcklght  = DISP_BRGHT_MIN;
    if ((disp_high_bcklght < DISP_BRGHT_MIN) || (disp_high_bcklght > DISP_BRGHT_MAX)) disp_high_bcklght = DISP_BRGHT_MAX;
    if (!scrnsvr_tout || (scrnsvr_tout > 240U))                     scrnsvr_tout        = 0x1U;
    if (Bcd2Dec(scrnsvr_ena_hour) > 23U)                            scrnsvr_ena_hour    = 0x23U;
    if (Bcd2Dec(scrnsvr_dis_hour) > 23U)                            scrnsvr_dis_hour    = 0x23U;
    if (!scrnsvr_clk_clr     ||(scrnsvr_clk_clr     > COLOR_BSIZE)) scrnsvr_clk_clr     = COLOR_BSIZE;
    if (!scrnsvr_semiclk_clr ||(scrnsvr_semiclk_clr > COLOR_BSIZE)) scrnsvr_semiclk_clr = COLOR_BSIZE;

    if      (!rfsen[0]||(rfsen[0]==0xFFFFU))rfsen[7]=0U,rfsen[6]=0U,rfsen[5]=0U,rfsen[4]=0U,rfsen[3]=0U,rfsen[2]=0U,rfsen[1]=0U,rfsen[0]=0U;
    else if (!rfsen[1]||(rfsen[1]==0xFFFFU))rfsen[7]=0U,rfsen[6]=0U,rfsen[5]=0U,rfsen[4]=0U,rfsen[3]=0U,rfsen[2]=0U,rfsen[1]=0U;
    else if (!rfsen[2]||(rfsen[2]==0xFFFFU))rfsen[7]=0U,rfsen[6]=0U,rfsen[5]=0U,rfsen[4]=0U,rfsen[3]=0U,rfsen[2]=0U;
    else if (!rfsen[3]||(rfsen[3]==0xFFFFU))rfsen[7]=0U,rfsen[6]=0U,rfsen[5]=0U,rfsen[4]=0U,rfsen[3]=0U;
    else if (!rfsen[4]||(rfsen[4]==0xFFFFU))rfsen[7]=0U,rfsen[6]=0U,rfsen[5]=0U,rfsen[4]=0U;
    else if (!rfsen[5]||(rfsen[5]==0xFFFFU))rfsen[7]=0U,rfsen[6]=0U,rfsen[5]=0U;
    else if (!rfsen[6]||(rfsen[6]==0xFFFFU))rfsen[7]=0U,rfsen[6]=0U;
    else if (!rfsen[7]||(rfsen[7]==0xFFFFU))rfsen[7]=0U;

    LogEvent.log_event = 0x0U;
    if (IsSYS_UpdOkSet())
    {
        SYS_UpdOkReset();
        DISP_FwUpdSet();
        LogEvent.log_event = FW_UPDATED;
    }
    else if(IsSYS_UpdFailSet())
    {
        SYS_UpdFailReset();
        DISP_FwUpdFailSet();
        LogEvent.log_event = FW_UPD_FAIL;
    }
    if (LogEvent.log_event)
    {
        ebuf[0] = sys_stat;
        HAL_I2C_Mem_Write    (&hi2c4, EE_ADDR, EE_SYS_STATE, I2C_MEMADD_SIZE_16BIT, ebuf, 0x1U, EETOUT);
        HAL_I2C_IsDeviceReady(&hi2c4, EE_ADDR, EE_MAX_TRIALS, EETOUT);
    }
	if (rstsrc != 0x0U) LogEvent.log_event = rstsrc;  // where  PIN_RESET is ((uint8_t)0xd0)
    if (LogEvent.log_event) LOGGER_Write();
}
/**
  * @brief
  * @param
  * @retval
  */
static void SaveResetSrc(void)
{
    if      (__HAL_RCC_GET_FLAG(RCC_FLAG_LPWRRST))  rstsrc = LOW_POWER_RESET;
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST))   rstsrc = POWER_ON_RESET;
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST))   rstsrc = SOFTWARE_RESET;
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST))  rstsrc = IWDG_RESET;
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST))   rstsrc = PIN_RESET;
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST))  rstsrc = WWDG_RESET;
    else                                            rstsrc = 0U;
	 __HAL_RCC_CLEAR_RESET_FLAGS();
}
/**
  * @brief
  * @param
  * @retval
  */
static void MPU_Config(void)
{
	MPU_Region_InitTypeDef MPU_InitStruct;

	/* Disable the MPU */
	HAL_MPU_Disable();

	/* Configure the MPU attributes as WT for SRAM */
	MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress      = 0x20010000U;
	MPU_InitStruct.Size             = MPU_REGION_SIZE_256KB;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
	MPU_InitStruct.IsCacheable      = MPU_ACCESS_CACHEABLE;
	MPU_InitStruct.IsShareable      = MPU_ACCESS_SHAREABLE;
	MPU_InitStruct.Number           = MPU_REGION_NUMBER0;
	MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
	MPU_InitStruct.SubRegionDisable = 0U;
	MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	/* Configure the MPU attributes for Quad-SPI area to strongly ordered
	 This setting is essentially needed to avoid MCU blockings!
	 See also STM Application Note AN4861 */
	MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
	MPU_InitStruct.Number           = MPU_REGION_NUMBER2;
	MPU_InitStruct.BaseAddress      = 0x90000000U;
	MPU_InitStruct.Size             = MPU_REGION_SIZE_256MB;
	MPU_InitStruct.SubRegionDisable = 0U;
	MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
	MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
	MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
	MPU_InitStruct.IsShareable      = MPU_ACCESS_SHAREABLE;
	MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
	MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	/* Configure the MPU attributes for the QSPI 64MB to normal memory Cacheable, must reflect the real memory size */
	MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
	MPU_InitStruct.Number           = MPU_REGION_NUMBER3;
	MPU_InitStruct.BaseAddress      = 0x90000000U;
	MPU_InitStruct.Size             = MPU_REGION_SIZE_16MB; // Set region size according to the QSPI memory size
	MPU_InitStruct.SubRegionDisable = 0U;
	MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
	MPU_InitStruct.IsShareable      = MPU_ACCESS_SHAREABLE;
	MPU_InitStruct.IsCacheable      = MPU_ACCESS_CACHEABLE;
	MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	/* Configure the MPU attributes for SDRAM_Banks area to strongly ordered
	 This setting is essentially needed to avoid MCU blockings!
	 See also STM Application Note AN4861 */
	MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
	MPU_InitStruct.Number           = MPU_REGION_NUMBER4;
	MPU_InitStruct.BaseAddress      = 0xC0000000U;
	MPU_InitStruct.Size             = MPU_REGION_SIZE_512MB;
	MPU_InitStruct.SubRegionDisable = 0U;
	MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
	MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
	MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
	MPU_InitStruct.IsShareable      = MPU_ACCESS_SHAREABLE;
	MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
	MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	/* Configure the MPU attributes for SDRAM 8MB to normal memory Cacheable */
	MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
	MPU_InitStruct.Number           = MPU_REGION_NUMBER5;
	MPU_InitStruct.BaseAddress      = 0xC0000000U;
	MPU_InitStruct.Size             = MPU_REGION_SIZE_8MB;
	MPU_InitStruct.SubRegionDisable = 0U;
	MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
	MPU_InitStruct.IsShareable      = MPU_ACCESS_SHAREABLE;
	MPU_InitStruct.IsCacheable      = MPU_ACCESS_CACHEABLE;
	MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	/* Enable the MPU */
	HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

	/* Disable FMC bank1 (0x6000 0000 - 0x6FFF FFFF), since it is not used.
	This setting avoids unnedded speculative access to the first FMC bank.
	See also STM Application Note AN4861 */
	FMC_Bank1->BTCR[0] = 0x000030D2U;
}
/**
  * @brief
  * @param
  * @retval
  */
static void CACHE_Config(void)
{
//	SCB_EnableICache();
//	SCB_EnableDCache();
    (*(uint32_t *) 0xE000ED94) &= ~0x5;
	(*(uint32_t *) 0xE000ED98) = 0x0; //MPU->RNR
	(*(uint32_t *) 0xE000ED9C) = 0x20010000 | 1 << 4; //MPU->RBAR
	(*(uint32_t *) 0xE000EDA0) = 0 << 28 | 3 << 24 | 0 << 19 | 0 << 18 | 1 << 17 | 0 << 16 | 0 << 8 | 30 << 1 | 1 << 0; //MPU->RASE  WT
	(*(uint32_t *) 0xE000ED94) = 0x5;

	SCB_InvalidateICache();

	/* Enable branch prediction */
	SCB->CCR |= (1 << 18);
	__DSB();

	SCB_EnableICache();

	SCB_InvalidateDCache();
	SCB_EnableDCache();
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
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    /** Configure LSE Drive Capability
    */
    HAL_PWR_EnableBkUpAccess();
    __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);
    /** Configure the main internal regulator output voltage
    */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    /** Initializes the CPU, AHB and APB busses clocks
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE
                              |RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
    RCC_OscInitStruct.LSIState = RCC_LSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 4;
    RCC_OscInitStruct.PLL.PLLN = 200;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        ErrorHandler(MAIN_FUNC, SYS_CLOCK);
    }
    /** Activate the Over-Drive mode
    */
    if (HAL_PWREx_EnableOverDrive() != HAL_OK)
    {
        ErrorHandler(MAIN_FUNC, SYS_CLOCK);
    }
    /** Initializes the CPU, AHB and APB busses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6) != HAL_OK)
    {
        ErrorHandler(MAIN_FUNC, SYS_CLOCK);
    }
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC|RCC_PERIPHCLK_RTC
                              |RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USART2
                              |RCC_PERIPHCLK_I2C3|RCC_PERIPHCLK_I2C4;
    PeriphClkInitStruct.PLLSAI.PLLSAIN = 57;
    PeriphClkInitStruct.PLLSAI.PLLSAIR = 3;
    PeriphClkInitStruct.PLLSAI.PLLSAIQ = 2;
    PeriphClkInitStruct.PLLSAI.PLLSAIP = RCC_PLLSAIP_DIV2;
    PeriphClkInitStruct.PLLSAIDivQ = 1;
    PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_4;
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
    PeriphClkInitStruct.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
    PeriphClkInitStruct.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
    PeriphClkInitStruct.Uart4ClockSelection = RCC_UART4CLKSOURCE_PCLK1;
    PeriphClkInitStruct.I2c3ClockSelection = RCC_I2C3CLKSOURCE_PCLK1;
    PeriphClkInitStruct.I2c4ClockSelection = RCC_I2C4CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
        ErrorHandler(MAIN_FUNC, SYS_CLOCK);
    }

	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000U);
	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_RTC_Init(void)
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};
    /**Initialize RTC Only
    */
    hrtc.Instance = RTC;
    hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
    hrtc.Init.AsynchPrediv = 127;
    hrtc.Init.SynchPrediv = 255;
    hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
    hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
    if (HAL_RTC_Init(&hrtc) != HAL_OK)
    {
        ErrorHandler(MAIN_FUNC, RTC_DRV);
    }

    if(HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0) != 0x32F2)
    {


        /**Initialize RTC and set the Time and Date
        */
        sTime.Hours = 0x0U;
        sTime.Minutes = 0x0U;
        sTime.Seconds = 0x0U;
        sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
        sTime.StoreOperation = RTC_STOREOPERATION_RESET;
        if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
        {
            ErrorHandler(MAIN_FUNC, RTC_DRV);
        }

        sDate.WeekDay = RTC_WEEKDAY_MONDAY;
        sDate.Month = RTC_MONTH_JANUARY;
        sDate.Date = 0x1U;
        sDate.Year = 0x0U;

        if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
        {
            ErrorHandler(MAIN_FUNC, RTC_DRV);
        }

        HAL_RTCEx_BKUPWrite(&hrtc,RTC_BKP_DR0,0x32F2);
    }
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_RTC_DeInit(void)
{
	HAL_RTC_DeInit(&hrtc);
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_TIM9_Init(void)
{
	TIM_OC_InitTypeDef sConfigOC;
	GPIO_InitTypeDef GPIO_InitStruct;

	__HAL_RCC_TIM9_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();

	htim9.Instance = TIM9;
	htim9.Init.Prescaler = 200U;
	htim9.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim9.Init.Period = 1000U;
	htim9.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim9.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

	if (HAL_TIM_PWM_Init(&htim9) != HAL_OK)
	{
		ErrorHandler(MAIN_FUNC, TMR_DRV);
	}

	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = DISP_BRGHT_MAX;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;

	if (HAL_TIM_PWM_ConfigChannel(&htim9, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
	{
		ErrorHandler(MAIN_FUNC, TMR_DRV);
	}

	/**TIM9 GPIO Configuration
    PE5     ------> TIM9_CH1
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF3_TIM9;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	HAL_TIM_PWM_Start(&htim9, TIM_CHANNEL_1);
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_TIM9_DeInit(void)
{
	__HAL_RCC_TIM9_CLK_DISABLE();
	HAL_GPIO_DeInit(GPIOE, GPIO_PIN_5);
	HAL_TIM_PWM_DeInit(&htim9);
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_UART1_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStruct;

    if (HAL_UART_DeInit(&huart1) != HAL_OK) ErrorHandler(MAIN_FUNC, USART_DRV);
	/* Peripheral clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10    ------> USART1_RX
    PA12    ------> USART1_DE
    */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(USART1_IRQn, 3, 0);
	HAL_NVIC_EnableIRQ(USART1_IRQn);

	huart1.Instance = USART1;
	huart1.Init.BaudRate = 115200U;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

    if (HAL_UART_Init(&huart1) != HAL_OK) ErrorHandler(RS485_FUNC, USART_DRV);
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_UART1_DeInit(void)
{
	__HAL_RCC_USART1_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_12);
    HAL_NVIC_DisableIRQ(USART1_IRQn);
    HAL_UART_DeInit(&huart1);
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_UART2_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStruct;

	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_USART2_CLK_ENABLE();


	/**USART2 GPIO Configuration
	PD5     ------> USART2_TX
	PD6     ------> USART2_RX
	*/
	GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	HAL_NVIC_SetPriority(USART2_IRQn, 3, 0);
	HAL_NVIC_EnableIRQ(USART2_IRQn);

	huart2.Instance = USART2;
	huart2.Init.BaudRate = 9600U;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

    if (HAL_UART_DeInit(&huart2) != HAL_OK) ErrorHandler(MAIN_FUNC, USART_DRV);
	if (HAL_UART_Init(&huart2) != HAL_OK) ErrorHandler(MAIN_FUNC, USART_DRV);
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_UART2_DeInit(void)
{
	__HAL_RCC_USART2_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_5|GPIO_PIN_6);
    HAL_NVIC_DisableIRQ(USART2_IRQn);
    HAL_UART_DeInit(&huart2);
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_CRC_Init(void)
{
    hcrc.Instance                       = CRC;
    hcrc.Init.DefaultPolynomialUse      = DEFAULT_POLYNOMIAL_ENABLE;
    hcrc.Init.DefaultInitValueUse       = DEFAULT_INIT_VALUE_ENABLE;
    hcrc.Init.InputDataInversionMode    = CRC_INPUTDATA_INVERSION_NONE;
    hcrc.Init.OutputDataInversionMode   = CRC_OUTPUTDATA_INVERSION_DISABLE;
    hcrc.InputDataFormat                = CRC_INPUTDATA_FORMAT_BYTES;
    __HAL_RCC_CRC_CLK_ENABLE();

    if (HAL_CRC_Init(&hcrc) != HAL_OK)
    {
        ErrorHandler(MAIN_FUNC, CRC_DRV);
    }
//    hcrc.Instance = CRC;
//    __HAL_RCC_CRC_CLK_ENABLE();
//
//	if (HAL_CRC_Init(&hcrc) != HAL_OK)
//	{
//		ErrorHandler(MAIN_FUNC, CRC_DRV);
//	}
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_CRC_DeInit(void)
{
    __HAL_RCC_CRC_CLK_DISABLE();
	HAL_CRC_DeInit(&hcrc);
}
/**
  * @brief
  * @param
  * @retval
  */
static void ADC3_Read(void)
{
    ADC_ChannelConfTypeDef sConfig;
    static uint32_t sys_startup = 0;
    static uint32_t adc_tmr = 0;
    static uint32_t ambient_ntc_sample_cnt = 0;
    static uint16_t ambient_ntc_sample_value[10] = {0};
    static float adc_calc;
    uint32_t tmp;

    if (!sys_startup) sys_startup = HAL_GetTick();
    else if (IsSYSTEM_StartupActiv())
    {
        if ((HAL_GetTick() - sys_startup) >= SYSTEM_STARTUP_TIME)
        {
            SYSTEM_StartupReset();
            ExtSwClosed();
            thst_fl = thstfl_memo;
            thstfl_memo = 0;
            disp_img_id = 0;
            DISP_UpdateSet();
        }
    }
    if ((HAL_GetTick() - adc_tmr) < ADC_READOUT_PERIOD) return;
    else adc_tmr = HAL_GetTick();
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;
    sConfig.Channel = ADC_CHANNEL_11;
    HAL_ADC_ConfigChannel(&hadc3, &sConfig);
    HAL_ADC_Start(&hadc3);
    HAL_ADC_PollForConversion(&hadc3, 10);
    ambient_ntc_sample_value[ambient_ntc_sample_cnt] = HAL_ADC_GetValue(&hadc3);
    if(++ambient_ntc_sample_cnt >  9) ambient_ntc_sample_cnt = 0;
    tmp = 0;
    for(uint8_t t = 0; t < 10; t++) tmp += ambient_ntc_sample_value[t];
    tmp = tmp/10;
    if((tmp < 100) || (tmp > 4000))
    {
        if (IsNtcValidActiv() && !IsSYSTEM_StartupActiv()) NtcErrorSet();
        NtcValidReset();
        room_temp = 0;
    }
    else
    {
        NtcValidSet();
        adc_calc = ROOM_GetTemperature(tmp);
        room_ntc_temp = adc_calc*10;
        if (room_temp != room_ntc_temp + ntc_offset)
        {
            room_temp = room_ntc_temp + ntc_offset;
            RoomTempUpdateSet();
        }
    }
}
/**
  * @brief
        adc_cnt = 0U;
  * @param
  * @retval
  */
static void MX_ADC3_Init(void)
{
    ADC_ChannelConfTypeDef sConfig;
	GPIO_InitTypeDef GPIO_InitStruct;

	__HAL_RCC_ADC3_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();

	GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	hadc3.Instance = ADC3;
	hadc3.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
	hadc3.Init.Resolution = ADC_RESOLUTION_12B;
	hadc3.Init.ScanConvMode = DISABLE;
	hadc3.Init.ContinuousConvMode = DISABLE;
	hadc3.Init.DiscontinuousConvMode = DISABLE;
	hadc3.Init.NbrOfDiscConversion = 0U;
	hadc3.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc3.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc3.Init.NbrOfConversion = 1U;
	hadc3.Init.DMAContinuousRequests = DISABLE;
	hadc3.Init.EOCSelection = ADC_EOC_SINGLE_CONV;

	if(HAL_ADC_Init(&hadc3) != HAL_OK)
	{
		ErrorHandler(MAIN_FUNC, ADC_DRV);
	}

	sConfig.Channel = ADC_CHANNEL_11;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;
	sConfig.Offset = 0U;
	HAL_ADC_ConfigChannel(&hadc3, &sConfig);
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_ADC3_DeInit(void)
{
	__HAL_RCC_ADC3_CLK_DISABLE();
	HAL_GPIO_DeInit(GPIOC, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
	HAL_ADC_DeInit(&hadc3);
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_I2C3_DeInit(void)
{
    __HAL_RCC_I2C3_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_8);
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_9);
    HAL_I2C_DeInit(&hi2c3);
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_I2C4_DeInit(void)
{
    __HAL_RCC_I2C4_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_12 | GPIO_PIN_13);
    HAL_GPIO_DeInit(GPIOG, GPIO_PIN_2);
    HAL_I2C_DeInit(&hi2c4);
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
    __HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
    
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, GPIO_PIN_RESET); 
    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_3, GPIO_PIN_RESET); // nrf24 radio modul disable

    
	GPIO_InitStruct.Pin = GPIO_PIN_8;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_4;  // nss
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    
	GPIO_InitStruct.Pin = GPIO_PIN_3;  // ce
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_GPIO_DeInit(void)
{
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_12);
	HAL_GPIO_DeInit(GPIOC, GPIO_PIN_8);
	HAL_GPIO_DeInit(GPIOD, GPIO_PIN_4|GPIO_PIN_7);
    HAL_GPIO_DeInit(GPIOG, GPIO_PIN_3|GPIO_PIN_13|GPIO_PIN_14);
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_SPI2_Init(void)
{
    hspi2.Instance = SPI2;
    hspi2.Init.Mode = SPI_MODE_SLAVE;
    hspi2.Init.Direction = SPI_DIRECTION_2LINES_RXONLY;
    hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi2.Init.NSS = SPI_NSS_HARD_INPUT;
    hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi2.Init.CRCPolynomial = 7;
    hspi2.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
    hspi2.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
    if (HAL_SPI_Init(&hspi2) != HAL_OK)
    {
        ErrorHandler(MAIN_FUNC, SPI_DRV);
    }
    HAL_SPI_Receive_IT(&hspi2, &spirx[spibcnt], 1);
}
/**
  * @brief
  * @param
  * @retval
  */
static void MX_SPI2_DeInit(void)
{
    __HAL_RCC_SPI2_CLK_DISABLE();
  
    /**SPI2 GPIO Configuration    
    PB12     ------> SPI2_NSS
    PB13     ------> SPI2_SCK
    PB15     ------> SPI2_MOSI 
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_15);

    /* SPI2 interrupt DeInit */
    HAL_NVIC_DisableIRQ(SPI2_IRQn);
}
/**
  * @brief
  * @param
  * @retval
  */
static void TOUCH_SCREEN_Service(void)
{
    static GUI_PID_STATE TS_State = {0, 0, 0, 0};
    static uint32_t ts_update_tmr = 0U;
    __IO TS_StateTypeDef  ts;
    uint16_t xDiff, yDiff;

    if (IsDISP_CleaningActiv()) return; // don't update touch event till room cleaning activ,
    else if(HAL_GetTick() >= ts_update_tmr)
    {
        ts_update_tmr = HAL_GetTick() + TOUCH_SCREEN_UPDATE_TIME;
        BSP_TS_GetState((TS_StateTypeDef *)&ts);

        if((ts.touchX[0] >= LCD_GetXSize()) ||
           (ts.touchY[0] >= LCD_GetYSize()))
        {
            ts.touchX[0] = 0U;
            ts.touchY[0] = 0U;
            ts.touchDetected = 0U;
        }
        xDiff = (TS_State.x > ts.touchX[0]) ? (TS_State.x - ts.touchX[0]) : (ts.touchX[0] - TS_State.x);
        yDiff = (TS_State.y > ts.touchY[0]) ? (TS_State.y - ts.touchY[0]) : (ts.touchY[0] - TS_State.y);

        if((TS_State.Pressed != ts.touchDetected) || (xDiff > 30U) || (yDiff > 30U))
        {
            TS_State.Pressed = ts.touchDetected;
            TS_State.Layer = TOUCH_SCREEN_LAYER;

            if(ts.touchDetected)
            {
                TS_State.x = ts.touchX[0];
                TS_State.y = ts.touchY[0];
                GUI_TOUCH_StoreStateEx(&TS_State);
            }
            else
            {
                GUI_TOUCH_StoreStateEx(&TS_State);
                TS_State.x = 0;
                TS_State.y = 0;
            }
        }
    }

}
/**
  * @brief
  * @param
  * @retval
  */
static void BUZZER_Service(void)
{
    static uint32_t buzzer_repeat_tmr = 0U;
    static uint32_t buzzer_mode_tmr = 0U;
    static uint32_t buzzer_pcnt = 0U;


    if(IsBUZZER_SignalActiv())
	{
        switch (buzz_sig_id)
        {
            case BUZZER_OFF:
            {
                buzzer_pcnt = 0U;
                BUZZER_SignalOff();
                break;
            }

            case BUZZER_SHORT:
            {
                if (HAL_GetTick() > buzzer_mode_tmr)
                {
                    if      (buzzer_pcnt == 0U)
                    {
                        if(HAL_GetTick() > buzzer_repeat_tmr)
                        {
                            BUZZER_On();
                            buzzer_mode_tmr = HAL_GetTick() + BUZZER_SHORT_TIME;
                            ++buzzer_pcnt;
                        }
                    }
                    else if (buzzer_pcnt == 1U)
                    {
                        BUZZER_Off();
                        buzzer_pcnt = 0U;
                        if  (buzz_sig_time == 0U) buzzer_mode_tmr = HAL_GetTick() + BUZZER_SHORT_TIME;
                        else buzzer_repeat_tmr = (HAL_GetTick() + (buzz_sig_time * 500U));
                    }
                }
                break;
            }

            case BUZZER_MIDDLE:
            {
                if(HAL_GetTick() > buzzer_mode_tmr)
                {
                    if(buzzer_pcnt == 0U)
                    {
                        if(HAL_GetTick() > buzzer_repeat_tmr)
                        {
                            BUZZER_On();
                            buzzer_mode_tmr = HAL_GetTick() + BUZZER_MIDDLE_TIME;
                            ++buzzer_pcnt;
                        }
                    }
                    else if(buzzer_pcnt == 1U)
                    {
                        BUZZER_Off();
                        buzzer_pcnt = 0U;
                        if(buzz_sig_time == 0U) buzzer_mode_tmr = HAL_GetTick() + BUZZER_MIDDLE_TIME;
                        else buzzer_repeat_tmr = (HAL_GetTick() + (buzz_sig_time * 500U));
                    }
                }
                break;
            }

            case BUZZER_LONG:
            {
                if  (HAL_GetTick() > buzzer_mode_tmr)
                {
                    if      (buzzer_pcnt == 0U)
                     {
                        if(HAL_GetTick() > buzzer_repeat_tmr)
                        {
                            BUZZER_On();
                            ++buzzer_pcnt;
                            buzzer_mode_tmr = HAL_GetTick() + BUZZER_LONG_TIME;
                        }
                    }
                    else if (buzzer_pcnt == 1U)
                    {
                        BUZZER_Off();
                        buzzer_pcnt = 0U;
                        if(buzz_sig_time == 0U) buzzer_mode_tmr = HAL_GetTick() + BUZZER_LONG_TIME;
                        else buzzer_repeat_tmr = (HAL_GetTick() + (buzz_sig_time * 500U));
                    }
                }
                break;
            }

            case BUZZER_CLICK:
            {
                if      (buzzer_pcnt == 0U)
                {
                    BUZZER_On();
                    buzzer_mode_tmr = HAL_GetTick() + BUZZER_CLICK_TIME;
                    ++buzzer_pcnt;
                }
                else if (buzzer_pcnt == 1U)
                {
                    if(HAL_GetTick() > buzzer_mode_tmr)
                    {
                        BUZZER_Off();
                        buzz_sig_id = BUZZER_OFF;
                    }
                }
                break;
            }

            default:
            {
                BUZZER_SignalOff();
                break;
            }
        }
	}
    else
    {
        buzzer_repeat_tmr = 0U;
        buzzer_mode_tmr = 0U;
        BUZZER_SignalOff();
        buzzer_pcnt = 0U;
        BUZZER_Off();
    }
}
/**
  * @brief
  * @param
  * @retval
  */
static float ROOM_GetTemperature(uint16_t adc_value)
{
	float temperature;
    float ntc_resistance;

    ntc_resistance = (float) (AMBIENT_NTC_PULLUP * ((4095.0f / (4095 - adc_value)) - 1));
    temperature = ((AMBIENT_NTC_B_VALUE * 298.1f) /  (AMBIENT_NTC_B_VALUE + (298.1f * log(ntc_resistance / AMBIENT_NTC_RREF))) -273.1f);
    return(temperature);
}
/**
  * @brief  Convert from Binary to 2 digit BCD.
  * @param  Value: Binary value to be converted.
  * @retval Converted word
  */
static uint32_t RTC_GetUnixTimeStamp(RTC_t* data)
{
	uint32_t days = 0U, seconds = 0U;
	uint16_t i;
	uint16_t year = (uint16_t) (data->year + 2000U);
	/* Year is below offset year */
	if (year < UNIX_OFFSET_YEAR)
	{
		return 0U;
	}
	/* Days in back years */
	for (i = UNIX_OFFSET_YEAR; i < year; i++)
	{
		days += DAYS_IN_YEAR(i);
	}
	/* Days in current year */
	for (i = 1U; i < data->month; i++)
	{
		days += rtc_month[LEAP_YEAR(year)][i - 1U];
	}
	/* Day starts with 1 */
	days += data->date - 1U;
	seconds = days * SECONDS_PER_DAY;
	seconds += data->hours * SECONDS_PER_HOUR;
	seconds += data->minutes * SECONDS_PER_MINUTE;
	seconds += data->seconds;
	/* seconds = days * 86400; */
	return seconds;
}
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
