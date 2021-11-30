/**
  ******************************************************************************
  * File Name          : mXconstants.h
  * Description        : This file contains the common defines of the application
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 STMicroelectronics
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
  

#ifndef __MAIN_H__
#define __MAIN_H__                          FW_BUILD // version

#include "stm32f1xx.h"
#include <string.h>
#include "common.h"

/* Exported macro ------------------------------------------------------------*/
/* ABSoulute value */
/* Exported functions ------------------------------------------------------- */
#define I2CEE_DENSITY 						0x4000U     // available i2c eeprom memory bytes 16 KByte (128kbits)
#define I2CEE_NUM_OF_PAGE				    0x100U      // number of pages
#define I2CEE_PAGE_BYTE_SIZE   			    0x40U       // max. number of bytes per single page
#define I2CEE_END_ADDR					    0x3FFFU     // last i2c eeprom address
#define I2CEE_TIMEOUT						0x10U       // 16 ms i2c eeprom to complete
#define I2CEE_TRIALS						0x200       // 512 i2c eeprom trials to complete
#define I2CEE_ADDR    						0xA0U       // i2c eeprom address

#define FLASH_BUFFER_SIZE					0x400U      // 1024 bytes temp buffer size used to copy firmware
#define FLASH_DENSITY           			0x80000U    // external flash available memory bytes
#define FLASH_NUM_OF_64K_BLOCKS 		    8           // number of 64KB block
#define FLASH_NUM_OF_32K_BLOCKS 		    16          // double times of 32KB blocks
#define FLASH_NUM_OF_4K_BLOCKS  		    128         // 128 times 4KB cluster
#define FLASH_BLOCK_PAGE_COUNT    		    16          // number of one operation writable pages in one block
#define FLASH_PAGE_BYTE_SIZE      		    256    		// number of bytes per page
#define FLASH_TIMEOUT					    500
#define FLASH_MANUFACTURER_ID			    ((uint8_t)0x1f)	// ATMEL id
#define FLASH_DEVICE_ID1					((uint8_t)0x44)	// 010-AT25f serie 00100-4Mbit density
#define FLASH_DEVICE_ID2					((uint8_t)0x01)	// 1-first major revision
#define FLASH_EXTENDED_DEVICE_INFO		    ((uint8_t)0x00)

#define SECT_0_START_ADDR				    ((uint32_t)0x00000000)
#define SECT_1_START_ADDR				    ((uint32_t)0x00010000)
#define SECT_2_START_ADDR				    ((uint32_t)0x00020000)
#define SECT_3_START_ADDR				    ((uint32_t)0x00030000)
#define SECT_4_START_ADDR				    ((uint32_t)0x00040000)
#define SECT_5_START_ADDR				    ((uint32_t)0x00050000)
#define SECT_6_START_ADDR				    ((uint32_t)0x00060000)
#define SECT_7_START_ADDR				    ((uint32_t)0x00070000)
#define SECT_8_START_ADDR				    ((uint32_t)0x00078000)
#define SECT_9_START_ADDR                   ((uint32_t)0x0007A000)
#define SECT_10_START_ADDR				    ((uint32_t)0x0007C000)

#define SPI_EE_WRITE_STATUS_REG       		((uint8_t)0x01)
#define SPI_EE_PAGE_PGM             		((uint8_t)0x02) 
#define SPI_EE_READ                			((uint8_t)0x03)    
#define SPI_EE_WRITE_DISABLE            	((uint8_t)0x04) 
#define SPI_EE_READ_STATUS_REG       		((uint8_t)0x05)    
#define SPI_EE_WRITE_ENABLE             	((uint8_t)0x06)
#define SPI_EE_READ_FAST					((uint8_t)0x0b)  
#define SPI_EE_4K_BLOCK_ERASE				((uint8_t)0x20)
#define SPI_EE_PROTECT_SECTOR				((uint8_t)0x36)
#define SPI_EE_UNPROTECT_SECTOR				((uint8_t)0x39)
#define SPI_EE_READ_SECT_PROT				((uint8_t)0x3c)
#define SPI_EE_32K_BLOCK_ERASE        		((uint8_t)0x52)
#define SPI_EE_CHIP_ERASE               	((uint8_t)0x60)
#define SPI_EE_JEDEC_ID                 	((uint8_t)0x9f)
#define SPI_EE_RESUME_POWER_DOWN    		((uint8_t)0xab)
#define SPI_EE_SEQUENTIAL_PGM				((uint8_t)0xad)
#define SPI_EE_SEQUENTIAL_PGM_2				((uint8_t)0xaf)
#define SPI_EE_POWER_DOWN               	((uint8_t)0xb9)   
#define SPI_EE_CHIP_ERASE_2     			((uint8_t)0xc7)
#define SPI_EE_64K_BLOCK_ERASE				((uint8_t)0xd8)
#define STATUS_REG_BUSY_MASK				((uint8_t)0x01)
#define STATUS_REG_WRITE_ENA_MASK		    ((uint8_t)0x02)
/** ==========================================================================*/
/**             G P I O   P O R T   &   P I N    A L I A S                    */
/** ==========================================================================*/
#define DISPLAY_LED_PWM_Pin			GPIO_PIN_0
#define DISPLAY_LED_PWM_Port 		GPIOA
#define AMBIENT_LIGHT_SENSOR_Pin	GPIO_PIN_1
#define AMBIENT_LIGHT_SENSOR_Port	GPIOA
#define ONE_WIRE_TX_Pin				GPIO_PIN_2
#define ONE_WIRE_TX_Port			GPIOA
#define ONE_WIRE_RX_Pin				GPIO_PIN_3
#define ONE_WIRE_RX_Port			GPIOA
#define DOUT_0_Pin					GPIO_PIN_4
#define DOUT_0_Port					GPIOA
#define DOUT_1_Pin					GPIO_PIN_5
#define DOUT_1_Port					GPIOA
#define DOORLOCK_PWM_Pin			GPIO_PIN_6
#define DOORLOCK_PWM_Port			GPIOA
#define DOUT_2_Pin					GPIO_PIN_7
#define DOUT_2_Port					GPIOA
#define SOUND_PWM_Pin				GPIO_PIN_8
#define SOUND_PWM_Port				GPIOA
#define USART_1_TX_Pin				GPIO_PIN_9
#define USART_1_TX_Port				GPIOA
#define USART_1_RX_Pin				GPIO_PIN_10
#define USART_1_RX_Port				GPIOA
#define RS485_DIR_Pin				GPIO_PIN_11
#define RS485_DIR_Port				GPIOA
#define SHIFT_CLK_Pin				GPIO_PIN_12
#define SHIFT_CLK_Port				GPIOA
#define SWDIO_Pin					GPIO_PIN_13
#define SWDIO_Port					GPIOA
#define SWCLK_Pin					GPIO_PIN_14
#define SWCLK_Port					GPIOA
#define CAPS_ALERT_Pin				SWDIO_Pin
#define CAPS_ALERT_Port				SWDIO_Port
#define NRF24L01_IRQ_Pin			SWCLK_Pin
#define NRF24L01_IRQ_Port			SWCLK_Port

#define DIN_0_Pin					GPIO_PIN_0
#define DIN_0_Port					GPIOB
#define DIN_1_Pin					GPIO_PIN_1
#define DIN_1_Port					GPIOB
#define DIN_2_Pin					GPIO_PIN_2
#define DIN_2_Port					GPIOB
#define DIN_3_Pin					GPIO_PIN_3
#define DIN_3_Port					GPIOB
#define DIN_4_Pin					GPIO_PIN_4
#define DIN_4_Port					GPIOB
#define DIN_5_Pin					GPIO_PIN_5
#define DIN_5_Port					GPIOB
#define DIN_6_Pin					GPIO_PIN_6
#define DIN_6_Port					GPIOB
#define DIN_7_Pin					GPIO_PIN_7
#define DIN_7_Port					GPIOB

#define I2C1_SCL_Pin				GPIO_PIN_8
#define I2C1_SCL_Port				GPIOB
#define I2C1_SDA_Pin				GPIO_PIN_9
#define I2C1_SDA_Port				GPIOB
#define DISPLAY_DC_Pin				GPIO_PIN_10
#define DISPLAY_DC_Port				GPIOB
#define DISPLAY_CS_Pin				GPIO_PIN_11
#define DISPLAY_CS_Port				GPIOB
#define FLASH_CS_Pin				GPIO_PIN_12
#define FLASH_CS_Port				GPIOB
#define SPI2_SCK_Pin				GPIO_PIN_13
#define SPI2_SCK_Port				GPIOB
#define SPI2_MISO_Pin				GPIO_PIN_14
#define SPI2_MISO_Port				GPIOB
#define SPI2_MOSI_Pin				GPIO_PIN_15
#define SPI2_MOSI_Port				GPIOB

#define NRF24L01_CSN_Pin			GPIO_PIN_13
#define NRF24L01_CSN_Port			GPIOC
/* Includes ------------------------------------------------------------------*/
/* Types    ------------------------------------------------------------------*/
/* Macros --------------------------------------------------------------------*/
extern CRC_HandleTypeDef hcrc;
#endif // __MAIN_H__
/***************** (C) COPYRIGHT JUBERA D.O.O. SARAJEVO ***********************/
