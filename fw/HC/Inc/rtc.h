/**
  ******************************************************************************
  * @file    rtc.h
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    11-November-2013
  * @brief   Header for rtc module
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RTC_H
#define __RTC_H

/* Includes ------------------------------------------------------------------*/
//#include "global_includes.h"
#include <stm32f4xx.h>
#include "stdint.h"

/* Exported types ------------------------------------------------------------*/
typedef enum
{
    RTC_INIT        = ((uint8_t)0x0U),
    RTC_VALID       = ((uint8_t)0x1U),
    RTC_INVALID     = ((uint8_t)0x2U),
    RTC_ERROR       = ((uint8_t)0x3U)
    
}RTC_StateTypeDef;


typedef struct 
{
    uint32_t unix;       	/*!< Seconds from 01.01.1970 00:00:00 */
    uint16_t subseconds; 	/*!< Subsecond downcounter. When it reaches zero, it's reload value is the same as */
	uint8_t seconds;     	/*!< Seconds parameter, from 00 to 59 */
	uint8_t minutes;     	/*!< Minutes parameter, from 00 to 59 */
	uint8_t hours;       	/*!< Hours parameter, 24Hour mode, 00 to 23 */
	uint8_t weekday;        /*!< Day in a week, from 1 to 7 */
	uint8_t date;        	/*!< Date in a month, 1 to 31 */
	uint8_t month;       	/*!< Month in a year, 1 to 12 */
	uint8_t year;        	/*!< Year parameter, 00 to 99, 00 is 2000 and 99 is 2099 */
	
} RTC_t;

extern RTC_StateTypeDef RTC_State;
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void RTC_Config(void);
void RTC_SetAlarmA(RTC_t* data);
void RTC_GetDateTime(RTC_t* data);



#endif /* __APP_RTC_H */
/***************** (C) COPYRIGHT JUBERA D.O.O. SARAJEVO ***********************/
