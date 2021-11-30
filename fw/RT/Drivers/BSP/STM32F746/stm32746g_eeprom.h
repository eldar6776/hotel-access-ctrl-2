/**
  ******************************************************************************
  * @file    stm32746g_discovery_eeprom.h
  * @author  MCD Application Team
  * @brief   This file contains all the functions prototypes for 
  *          the stm32746g_discovery_eeprom.c firmware driver.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __EEPROM_H__                        
#define __EEPROM_H__                        FW_BUILD // version

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32746g.h"


/* EEPROM hardware address and page size */ 
#define EE_PGSIZE                           64U
#define EE_PGNUM                            0x100U  // number of pages
#define EE_MAXSIZE                          0x4000U /* 64Kbit */
#define EE_ENDADDR                          0x3FFFU    
#define EE_ADDR                             0xA0U
#define EETOUT                              1000U
#define EE_WR_TIME                          15U
#define EE_TRIALS                           200U
#define EE_MAX_TRIALS                       3000U
#define EE_OK                               0x0U
#define EE_FAIL                             0x1U
#define EE_TOUT                             0x2U
/* Maximum number of trials for EE_WaitStbySta() function */
#define EE_THST_FLAGS                       0x0U
#define EE_MIN_SETPOINT                     0x4U
#define EE_MAX_SETPOINT                     0x5U
#define EE_THST_SETPOINT                    0x6U
#define EE_NTC_OFFSET                       0x7U
#define EE_OW_BPS                           0x8U
#define EE_OWIF_ADDR                        0x9U
#define EE_DISP_LOW_BCKLGHT                 0xAU
#define EE_DISP_HIGH_BCKLGHT                0xBU
#define EE_SCRNSVR_TOUT                     0xCU
#define EE_SCRNSVR_ENABLE_HOUR              0xDU
#define EE_SCRNSVR_DISABLE_HOUR             0xEU
#define EE_SCRNSVR_CLK_COLOR                0xFU
#define EE_SCRNSVR_SEMICLK_COLOR            0x10U
#define EE_SYS_STATE                        0x11U
#define EE_NTC_RES_REF                      0x12U
#define EE_NTC_RES_PULLUP                   0x16U
#define EE_NTC_RES_BETA                     0x1AU
#define EE_RF_SENSOR_1                      0x20U
#define EE_RF_SENSOR_2                      0x22U
#define EE_RF_SENSOR_3                      0x24U
#define EE_RF_SENSOR_4                      0x26U
#define EE_RF_SENSOR_5                      0x28U
#define EE_RF_SENSOR_6                      0x2AU
#define EE_RF_SENSOR_7                      0x2CU
#define EE_RF_SENSOR_8                      0x2EU
#define EE_WFORECAST                        0x40U // one page =  64 byte for weather forecast
#define EE_QR_CODE                          0x80U // two page = 128 byte for qr code 
#define EE_LOG_LIST_START_ADDR              0x100U
#define EE_LOG_LIST_END_ADDR                EE_ENDADDR
/* Link function for I2C EEPROM peripheral */
void                EE_IO_Init      (void);
void                EE_ToutCallbck  (void);
uint32_t            EE_WaitStbySta  (void);
uint32_t            EE_Init         (void);
uint8_t             EE_DeInit       (void);
void                EE_Save         (uint16_t ee_address, uint8_t* value, uint8_t size);
uint32_t            EE_ReadBuffer   (uint8_t* pBuffer, uint16_t ReadAddr, uint16_t* NumByteToRead);
uint32_t            EE_WritePage    (uint8_t* pBuffer, uint16_t WriteAddr, uint8_t* NumByteToWrite);
uint32_t            EE_WriteBuffer  (uint8_t* pBuffer, uint16_t WriteAddr, uint16_t NumByteToWrite);
HAL_StatusTypeDef   EE_WriteData    (uint16_t DevAddress, uint16_t MemAddress, uint8_t *pBuffer, uint32_t BufferSize);
HAL_StatusTypeDef   EE_ReadData     (uint16_t DevAddress, uint16_t MemAddress, uint8_t *pBuffer, uint32_t BufferSize);
HAL_StatusTypeDef   EE_IsDeviceReady(uint16_t DevAddress, uint32_t Trials);

#ifdef __cplusplus
}
#endif

#endif /* __EEPROM_H__ */

/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
