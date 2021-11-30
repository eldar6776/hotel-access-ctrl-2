/**
  ******************************************************************************
  * @file    rtc.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    11-November-2013
  * @brief   RTC functions
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "rtc.h"
#include "main.h"
/* Private typedef -----------------------------------------------------------*/
static RTC_TimeTypeDef  rtc_time;
static RTC_DateTypeDef  rtc_date;
RTC_StateTypeDef        RTC_State;
/* Private define ------------------------------------------------------------*/
/* Private constants  --------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
void RTC_Config(void)
{
	SysTick_Config(SystemCoreClock / 1000U);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, ENABLE);
	
	if (RTC_ReadBackupRegister(RTC_BKP_DR1) != 0xA5A5U)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
		PWR_BackupAccessCmd(ENABLE);
		RCC_BackupResetCmd(ENABLE);
		RCC_BackupResetCmd(DISABLE);
		RCC_LSEConfig(RCC_LSE_ON);
		while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
        {
        }
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
		RCC_RTCCLKCmd(ENABLE);
        RTC_WriteProtectionCmd(DISABLE);
        RTC_WaitForSynchro();
        RTC_WriteBackupRegister(RTC_BKP_DR1, 0xA5A5U);
        RTC_State = RTC_INVALID;
	}
	else
	{
		if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)        // Power On Reset occurred
        {
        }
		else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)   // External Reset occurred
        {
        }
        RTC_WaitForSynchro();
        RTC_State = RTC_VALID;
	}
	RCC_ClearFlag();
}
/**
  * @brief
  * @param  
  * @retval 
  */
void RTC_GetDateTime(RTC_t* data) 
{
    uint32_t days = 0x0U;
	uint16_t year = 0x0U, i;
    
	RTC_GetTime(RTC_Format_BCD, &rtc_time);
    /* Format hours */
	data->hours     = rtc_time.RTC_Hours;
	data->minutes   = rtc_time.RTC_Minutes;
	data->seconds   = rtc_time.RTC_Seconds;	
	/* Get subseconds */
	data->subseconds = RTC->SSR;
    /* Get rtcdt */
    RTC_GetDate(RTC_Format_BCD, &rtc_date);
    /* Format rtcdt */
	data->year      = rtc_date.RTC_Year;
	data->month     = rtc_date.RTC_Month;
	data->date      = rtc_date.RTC_Date;
	data->weekday   = rtc_date.RTC_WeekDay;
    /* Calculate unix offset */
    data->unix = 0x0U;
    year = (uint16_t) (Bcd2Dec(data->year) + 2000U);
    /* RTC_Year is below offset year */
	if (year < UNIX_OFFSET_YEAR) return;
	/* Days in back years */
	for (i = UNIX_OFFSET_YEAR; i < year; i++) 
	{
		days += DAYS_IN_YEAR(i);
	}	
	/* Days in current year */
	for (i = 0x1U; i < Bcd2Dec(data->month); i++) 
	{
		days += rtc_month[LEAP_YEAR(year)][i - 0x1U];
	}
    /* Day starts with 1 */
	days += Bcd2Dec(data->date) - 0x1U;
	data->unix = days * SECONDS_PER_DAY;
	data->unix += Bcd2Dec(data->hours)   * SECONDS_PER_HOUR;
	data->unix += Bcd2Dec(data->minutes) * SECONDS_PER_MINUTE;
	data->unix += Bcd2Dec(data->seconds);
}
/**
  * @brief
  * @param  
  * @retval 
  */
void RTC_SetAlarmA(RTC_t* data)
{ 
    NVIC_InitTypeDef    NVIC_InitStructure;
    EXTI_InitTypeDef    EXTI_InitStructure;
    RTC_AlarmTypeDef    RTC_AlarmTypeInitStructure;
    RTC_TimeTypeDef     RTC_TimeTypeInitStructure;

    RTC_TimeTypeInitStructure.RTC_Hours                 = data->hours;
    RTC_TimeTypeInitStructure.RTC_Minutes               = data->minutes;
    RTC_TimeTypeInitStructure.RTC_Seconds               = data->seconds;
    RTC_TimeTypeInitStructure.RTC_H12                   = RTC_H12_AM;
    RTC_AlarmTypeInitStructure.RTC_AlarmDateWeekDay     = data->weekday;
    RTC_AlarmTypeInitStructure.RTC_AlarmDateWeekDaySel  = RTC_AlarmDateWeekDaySel_Date; 
    RTC_AlarmTypeInitStructure.RTC_AlarmMask            = RTC_AlarmMask_None;  
    RTC_AlarmTypeInitStructure.RTC_AlarmTime            = RTC_TimeTypeInitStructure;
    EXTI_InitStructure.EXTI_Line                        = EXTI_Line17;
    EXTI_InitStructure.EXTI_Mode                        = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger                     = EXTI_Trigger_Rising; 
    EXTI_InitStructure.EXTI_LineCmd                     = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannel                  = RTC_Alarm_IRQn; 
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority= 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority       = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd               = ENABLE;
    
    RTC_AlarmCmd            (RTC_Alarm_A,DISABLE);
    RTC_SetAlarm            (RTC_Format_BIN,RTC_Alarm_A, &RTC_AlarmTypeInitStructure);
    RTC_ClearITPendingBit   (RTC_IT_ALRA);
    EXTI_ClearITPendingBit  (EXTI_Line17);
    RTC_ITConfig            (RTC_IT_ALRA,ENABLE);
    RTC_AlarmCmd            (RTC_Alarm_A,ENABLE);
    EXTI_Init               (&EXTI_InitStructure);
    NVIC_Init               (&NVIC_InitStructure);
}
/***************** (C) COPYRIGHT JUBERA D.O.O. SARAJEVO ***********************/
