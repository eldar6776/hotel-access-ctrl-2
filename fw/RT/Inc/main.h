/**
 ******************************************************************************
 * File Name          : main.c
 * Date               : 10.3.2018.
 * Description        : Hotel Room Thermostat Program Header
 ******************************************************************************
 *
 *
 ******************************************************************************
 */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H__
#define __MAIN_H__                              FW_BUILD // version



/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "Resource.h"
#include "DIALOG.h"
#include "GUI.h"
#include "common.h"
/* Exported types ------------------------------------------------------------*/
typedef struct 
{
	uint8_t seconds;     	/*!< Seconds parameter, from 00 to 59 */
	uint16_t subseconds; 	/*!< Subsecond downcounter. When it reaches zero, it's reload value is the same as */
	uint8_t minutes;     	/*!< Minutes parameter, from 00 to 59 */
	uint8_t hours;       	/*!< Hours parameter, 24Hour mode, 00 to 23 */
	uint8_t day;         	/*!< Day in a week, from 1 to 7 */
	uint8_t date;        	/*!< Date in a month, 1 to 31 */
	uint8_t month;       	/*!< Month in a year, 1 to 12 */
	uint8_t year;        	/*!< Year parameter, 00 to 99, 00 is 2000 and 99 is 2099 */
	uint32_t unix;       	/*!< Seconds from 01.01.1970 00:00:00 */
	
} RTC_t;
/* Exported constants --------------------------------------------------------*/
/* Exported variable  --------------------------------------------------------*/
extern __IO uint32_t sys_flags;
extern __IO  uint8_t sys_stat;
extern uint32_t thstfl_memo;
extern uint8_t dispfl_memo;
extern uint8_t buzz_sig_time;
extern uint8_t buzz_sig_id;
/* Exported macros  --------------------------------------------------------*/
#define SYS_NewLogSet()             (sys_stat |=  (1U<<0))
#define SYS_NewLogReset()           (sys_stat &=(~(1U<<0)))
#define IsSYS_NewLogSet()           (sys_stat &   (1U<<0))
#define SYS_LogListFullSet()        (sys_stat |=  (1U<<1))
#define SYS_LogListFullReset()      (sys_stat &=(~(1U<<1)))
#define IsSYS_LogListFullSet()      (sys_stat &   (1U<<1))
#define SYS_FileRxOkSet()           (sys_stat |=  (1U<<2))
#define SYS_FileRxOkReset()         (sys_stat &=(~(1U<<2)))
#define IsSYS_FileRxOkSet()         (sys_stat &   (1U<<2))
#define SYS_FileRxFailSet()         (sys_stat |=  (1U<<3))
#define SYS_FileRxFailReset()       (sys_stat &=(~(1U<<3)))
#define IsSYS_FileRxFailSet()       (sys_stat &   (1U<<3))
#define SYS_UpdOkSet()              (sys_stat |=  (1U<<4))
#define SYS_UpdOkReset()            (sys_stat &=(~(1U<<4)))
#define IsSYS_UpdOkSet()            (sys_stat &   (1U<<4))
#define SYS_UpdFailSet()            (sys_stat |=  (1U<<5))
#define SYS_UpdFailReset()          (sys_stat &=(~(1U<<5)))
#define IsSYS_UpdFailSet()          (sys_stat &   (1U<<5))
#define SYS_ImageRqSet()            (sys_stat |=  (1U<<6))
#define SYS_ImageRqReset()          (sys_stat &=(~(1U<<6)))
#define IsSYS_ImageRqSet()          (sys_stat &   (1U<<6))
#define SYS_FwRqSet()               (sys_stat |=  (1U<<7))
#define SYS_FwRqReset()             (sys_stat &=(~(1U<<7)))
#define IsSYS_FwRqSet()             (sys_stat &   (1U<<7))

#define BUZZER_StartSignal(s)       ((sys_flags|=  (1U<<0)),(buzz_sig_id = s))
#define BUZZER_SignalOn()           (sys_flags |=  (1U<<0))
#define BUZZER_SignalOff()          (sys_flags &=(~(1U<<0)))
#define IsBUZZER_SignalActiv()      (sys_flags &   (1U<<0))
#define SYSTEM_StartupSet()         (sys_flags |=  (1U<<1)) 
#define SYSTEM_StartupReset()       (sys_flags &=(~(1U<<1)))
#define IsSYSTEM_StartupActiv()     (sys_flags &   (1U<<1))
#define TOUCH_SCREEN_Enable()       (sys_flags |=  (1U<<2)) 
#define TOUCH_SCREEN_Disable()      (sys_flags &=(~(1U<<2)))
#define IsTOUCH_SCREEN_Enabled()    (sys_flags &   (1U<<2))
#define ExtSwClosed()               (sys_flags |=  (1U<<3)) 
#define ExtSwOpen()                 (sys_flags &=(~(1U<<3)))
#define IsExtSwClosed()             (sys_flags &   (1U<<3))
#define NTC_RefRes10K()             (sys_flags |=  (1U<<4)) 
#define NTC_RefRes100K()            (sys_flags &=(~(1U<<4)))
#define IsNTC_RefRes10K()           (sys_flags &   (1U<<4))
#define IsNTC_RefRes100K()          (!IsNTC_RefRes10K())

#define BUZZER_On()         (HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, GPIO_PIN_SET))
#define BUZZER_Off()        (HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, GPIO_PIN_RESET))
#define IsBUZZER_On()       (HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_4) == GPIO_PIN_SET)
/* Exported hal handler --------------------------------------------------------*/
extern RTC_TimeTypeDef rtctm;
extern RTC_DateTypeDef rtcdt;
extern CRC_HandleTypeDef hcrc;
extern RTC_HandleTypeDef hrtc;
extern I2C_HandleTypeDef hi2c3;
extern I2C_HandleTypeDef hi2c4;
extern TIM_HandleTypeDef htim9;
extern SPI_HandleTypeDef hspi2;
extern QSPI_HandleTypeDef hqspi; 
extern IWDG_HandleTypeDef hiwdg;
extern LTDC_HandleTypeDef hltdc;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern DMA2D_HandleTypeDef hdma2d;
/* Exported function --------------------------------------------------------*/
void Restart(void);
void FAN_SetControlType(uint8_t fan_ctrl_type);
void RTC_GetDateTime(RTC_t* data, uint32_t format);
void ErrorHandler(uint8_t function, uint8_t driver);
void RTC_GetDateTimeFromUnix(RTC_t* data, uint32_t unix);
uint8_t FLASH_CopyFile(uint32_t ldadd, uint32_t wradd, uint32_t size);
#endif /* __MAIN_H */
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
