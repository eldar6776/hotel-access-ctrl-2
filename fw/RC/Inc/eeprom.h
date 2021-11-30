/**
 ******************************************************************************
 * File Name          : eeprom.h
 * Date               : 28/02/2016 23:16:19
 * Description        : eeprom memory manager modul header
 ******************************************************************************
*
* DISPLAY           pins    ->  STM32F103 Rubicon controller
* ----------------------------------------------------------------------------
* DISPLAY   +3V3    pin 1   ->  controller +3V3
* DISPLAY   GND     pin 2   ->  controller VSS
* DISPLAY   CS      pin 3   ->  PA8
* DISPLAY   RST     pin 4   ->  PA3
* DISPLAY   DC      pin 5   ->  PA2
* DISPLAY   MOSI    pin 6   ->  PA7 - SPI1 MOSI
* DISPLAY   SCK     pin 7   ->  PA5 - SPI1 SCK
* DISPLAY   LED     pin 8   ->  PB7 - PWM TIM4 CH2
* DISPLAY   MISO    pin 9   ->  PA6 - SPI1 MISO
* SD CARD   CS      pin 10  ->  PA4
* 
*
******************************************************************************
*/
#ifndef __EEPROM_H__
#define __EEPROM_H__					    FW_BUILD	// version

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
/* Defines    ----------------------------------------------------------------*/
#define FLASH_MANUFACTURER_WINBOND      0x00EF4017U     //  JEDEC device id
#define FLASH_DENSITY                   0x08000000U     // available memory bytes
#define FLASH_SECTORS                   16U
#define FLASH_64K_BLOCKS                128U       
#define FLASH_32K_BLOCKS                256U
#define FLASH_4K_BLOCKS 		        2048U
#define FLASH_4K_ERASE_TIMEOUT          400U            // datasheet max. 4k block erase time is 60 ms typical 
#define FLASH_32K_ERASE_TIMEOUT         1600U           // datasheet max. 32k block erase time is 120 ms typical
#define FLASH_64K_ERASE_TIMEOUT         2000U           // datasheet max. 64k block erase time is 150 ms typical
#define FLASH_CHIP_ERASE_TIMEOUT        100000U         // datasheet max. chip erase time is 20 s typical

#define FLASH_MANUFACTURER_ID			((uint8_t)0x90U) // get manufacturer id
#define FLASH_UNIQUE_ID					((uint8_t)0x4bU)
#define FLASH_ENABLE_RESET				((uint8_t)0x66U)
#define FLASH_EXTENDED_DEVICE_INFO		((uint8_t)0x00U)
#define FLASH_WRITE_STATUS_REG          ((uint8_t)0x01U)
#define FLASH_PAGE_PGM             		((uint8_t)0x02U) 
#define FLASH_READ                      ((uint8_t)0x03U)    
#define FLASH_WRITE_DISABLE            	((uint8_t)0x04U) 
#define FLASH_READ_STATUS_REG_1         ((uint8_t)0x05U)    
#define FLASH_WRITE_ENABLE             	((uint8_t)0x06U)
#define FLASH_READ_FAST					((uint8_t)0x0bU)  
#define FLASH_4K_BLOCK_ERASE            ((uint8_t)0x20U)
#define FLASH_READ_STATUS_REG_2         ((uint8_t)0x35U)
#define FLASH_PROTECT_SECTOR            ((uint8_t)0x36U)
#define FLASH_UNPROTECT_SECTOR          ((uint8_t)0x39U)
#define FLASH_READ_SECTOR_PROT          ((uint8_t)0x3cU)
#define FLASH_32K_BLOCK_ERASE           ((uint8_t)0x52U)
#define FLASH_CHIP_ERASE               	((uint8_t)0x60U)
#define FLASH_RESET                		((uint8_t)0x99U)
#define FLASH_JEDEC_ID                 	((uint8_t)0x9fU)
#define FLASH_RESUME_POWER_DOWN    		((uint8_t)0xabU)
#define FLASH_SEQUENTIAL_PGM            ((uint8_t)0xadU)
#define FLASH_SEQUENTIAL_PGM_2          ((uint8_t)0xafU)
#define FLASH_POWER_DOWN               	((uint8_t)0xb9U)   
#define FLASH_CHIP_ERASE_2     			((uint8_t)0xc7U)
#define FLASH_64K_BLOCK_ERASE           ((uint8_t)0xd8U)
#define FLASH_STATUS_BUSY_MASK          ((uint8_t)0x01U)
#define FLASH_STATUS_WRITE_ENABLE_MASK  ((uint8_t)0x02U)

#define I2CEE_ADD   						0xA0
#define I2C_EE_DENSITY 						16384U  // available memory bytes
#define I2CEE_PGNUM				            256U    // number of pages
#define I2CEE_PGBSZ   			            64U     // number of bytes per page
/** ==========================================================================*/
/**     	S Y S T E M   C O N F I G   M E M O R Y   A D D R E S S E      	  */
/** ==========================================================================*/

#define EE_VAR_START_ADD                    20	// eeprom variable first address
#define EE_ROOM_PRESTAT_ADD				    21	// room pre status address
#define EE_ROOM_STAT_ADD				    22	// room status address

#define EE_FW_UPDATE_BYTE_CNT               33	// firmware update byte count
#define EE_FW_UPDATE_STATUS                 37	// firmware update status
#define EE_ROOM_TEMP_SP                     38	// room setpoint temp  in degree of Celsious
#define EE_ROOM_TEMP_DIFF		            39	// room tempreature on / off difference
#define EE_RSIFA			                40	// rs485 device address
#define EE_RSGRA				            42	// rs485 group broadcast address
#define EE_RSBRA			                44	// rs485 broadcast address msb
#define EE_RSBPS					        46	// rs485 interface baudrate
#define EE_ROOM_PWRTOUT 				    48	// room power expiry date time
#define EE_SYSID				            54	// system id (system unique number)
#define EE_BEDNG_CNT_ADD		            56	// rfid signal user selected options
#define	EE_BEDNG_REPL_ADD		            57	// do-not-disturb signal user selected options
#define EE_ROOM_TEMP_CFG					58	// Room Controlle Thermostat Config:OFF/HEATING/COOLING
#define EE_LCD_BRIGHTNESS					59  // Display backlight 

#define EE_MIFARE_KEYA                      64	// mifare access authentication key A
#define EE_MIFARE_KEYB                      72	// mifare access authentication key B 
#define EE_USRGR_ADD                        80  // permited group first address
#define EE_USRGR_ADD1_ID                    80	// mifare access permited group 1 id
#define EE_USRGR_ADD2_ID                    81	// mifare access permited group 2 id
#define EE_USRGR_ADD3_ID                    82	// mifare access permited group 3 id
#define EE_USRGR_ADD4_ID                    83	// mifare access permited group 4 id
#define EE_USRGR_ADD5_ID                    84  // mifare access permited group 5 id
#define EE_USRGR_ADD6_ID                    85  // mifare access permited group 6 id
#define EE_USRGR_ADD7_ID                    86  // mifare access permited group 7 id
#define EE_USRGR_ADD8_ID                    87  // mifare access permited group 8 id
#define EE_USRGR_ADD9_ID                    88  // mifare access permited group 9 id
#define EE_USRGR_ADD10_ID                   89  // mifare access permited group 10 id
#define EE_USRGR_ADD11_ID                   90  // mifare access permited group 11 id
#define EE_USRGR_ADD12_ID                   91  // mifare access permited group 12 id
#define EE_USRGR_ADD13_ID                   92  // mifare access permited group 13 id
#define EE_USRGR_ADD14_ID                   93  // mifare access permited group 14 id
#define EE_USRGR_ADD15_ID                   94  // mifare access permited group 15 id
#define EE_USRGR_ADD16_ID                   95  // mifare access permited group 16 id
#define EE_PERM_EXTADD1                     96	// additional permited address 1
#define EE_PERM_EXTADD2                     98 	// additional permited address 2
#define EE_PERM_EXTADD3                     100	// additional permited address 3
#define EE_PERM_EXTADD4                     102	// additional permited address 4
#define EE_PERM_EXTADD5                     104	// additional permited address 5
#define EE_PERM_EXTADD6                     106 // additional permited address 6
#define EE_PERM_EXTADD7                     108	// additional permited address 7
#define EE_PERM_EXTADD8                     110	// additional permited address 8
#define EE_OWIFA                            112	// onewire interface address
#define EE_OWGRA                            113	// onewire group address
#define EE_OWBRA			                114	// onewire broadcast address
#define EE_OWBPS                            115	// onewire interface baudrate
#define EE_OWADD1                           116	// onewire slave address 1 
#define EE_OWADD2                           117	// onewire slave address 2
#define EE_OWADD3                           118	// onewire slave address 3 
#define EE_OWADD4                           119	// onewire slave address 4 
#define EE_OWADD5                           120	// onewire slave address 5 
#define EE_OWADD6                           121	// onewire slave address 6 
#define EE_OWADD7                           122	// onewire slave address 7 
#define EE_OWADD8                           123	// onewire slave address 8 
#define EE_OWADD9                           124	// onewire slave address 9
#define EE_OWADD10                          125	// onewire slave address 10 
#define EE_OWADD11                          126	// onewire slave address 11
#define EE_OWADD12                          127	// onewire slave address 12
#define EE_DISP_STATUS_ADD                  128	// display status flags
#define EE_BUZZER_VOLUME_ADD                129 // buzzer volume address
#define EE_DOORLOCK_FORCE_ADD               130	// doorlock force address
#define EE_DISP_ROTATION_ADD                131	// display rotation address
#define EE_RADIO_CHANEL_ADD                 132 // radio chanel address
#define EE_RADIO_ADDRESS_ADD                133 // radio address LSB
#define EE_RADIO_CFG_ADD                    134 // radio config address

#define EE_DIN_CFG_ADD_1                    144 // digital input 1 config
#define EE_DIN_CFG_ADD_2                    145 // digital input 2 config
#define EE_DIN_CFG_ADD_3                    146 // digital input 3 config
#define EE_DIN_CFG_ADD_4                    147 // digital input 4 config
#define EE_DIN_CFG_ADD_5                    148 // digital input 5 config
#define EE_DIN_CFG_ADD_6                    149 // digital input 6 config
#define EE_DIN_CFG_ADD_7                    150 // digital input 7 config
#define EE_DIN_CFG_ADD_8                    151 // digital input 8 config
#define EE_DOUT_CFG_ADD_1                   152 // digital output 1 config
#define EE_DOUT_CFG_ADD_2                   153 // digital output 2 config
#define EE_DOUT_CFG_ADD_3                   154 // digital output 3 config
#define EE_DOUT_CFG_ADD_4                   155 // digital output 4 config
#define EE_DOUT_CFG_ADD_5                   156 // digital output 5 config
#define EE_DOUT_CFG_ADD_6                   157 // digital output 6 config
#define EE_DOUT_CFG_ADD_7                   158 // digital output 7 config
#define EE_DOUT_CFG_ADD_8                   159 // digital output 8 config
#define EE_VAR_END_ADD                      160 
#define EE_LOG_START_ADD                    ((uint16_t)0x0100U)	// beginning of log list
#define EE_LOG_END_ADD                      ((uint16_t)0x3FFFU) // end of log list
/** ==========================================================================*/
/**         E X T .     F L A S H      M E M O R Y   A D D R E S S E      	  */
/** ==========================================================================*/
#define EE_IMAGE_1_ADDR				        ((uint32_t)0x00000000U)	// ROOM_NUMBER_IMAGE
#define EE_IMAGE_2_ADDR				        ((uint32_t)0x00030000U)	// DO_NOT_DISTURB_IMAGE
#define EE_IMAGE_3_ADDR				        ((uint32_t)0x00060000U)	// BEDDING_REPLACEMENT_IMAGE	
#define EE_IMAGE_4_ADDR				        ((uint32_t)0x00090000U)	// CLEAN_UP_IMAGE
#define EE_IMAGE_5_ADDR				        ((uint32_t)0x000c0000U)	// GENERAL_CLEAN_UP_IMAGE
#define EE_IMAGE_6_ADDR				        ((uint32_t)0x000f0000U)	// CARD_VALID_IMAGE	
#define EE_IMAGE_7_ADDR				        ((uint32_t)0x00120000U)	// CARD_INVALID_IMAGE
#define EE_IMAGE_8_ADDR				        ((uint32_t)0x00150000U)	// WRONG_ROOM_IMAGE
#define EE_IMAGE_9_ADDR				        ((uint32_t)0x00180000U)	// TIME_EXPIRED_IMAGE
#define EE_IMAGE_10_ADDR                    ((uint32_t)0x001b0000U)	// FIRE_ALARM_IMAGE
#define EE_IMAGE_11_ADDR                    ((uint32_t)0x001e0000U)	// FIRE_EXIT_IMAGE
#define EE_IMAGE_12_ADDR                    ((uint32_t)0x00210000U)	// MINIBAR_IMAGE
#define EE_IMAGE_13_ADDR                    ((uint32_t)0x00240000U)	// ROOM_OUT_OF_SERVICE_IMAGE
#define EE_IMAGE_14_ADDR                    ((uint32_t)0x00270000u)	// SOS_ALARM_IMAGE
#define EE_IMAGE_15_ADDR                    ((uint32_t)0x002a0000U)	// SPI flash - new image 15
#define EE_IMAGE_16_ADDR                    ((uint32_t)0x002d0000U)	// SPI flash - new image 16
#define EE_IMAGE_17_ADDR                    ((uint32_t)0x00300000U)	// SPI flash - new image 17
#define EE_IMAGE_18_ADDR                    ((uint32_t)0x00330000U)	// SPI flash - new image 18
#define EE_IMAGE_19_ADDR                    ((uint32_t)0x00360000U)	// SPI flash - new image 19
#define EE_NEW_FW_ADDR			            ((uint32_t)0x00390000U)	// NEW FIRMWARE DOWNLOAD SPACE
#define EE_RC_NEW_BLDR_ADDR                 ((uint32_t)0x003c0000U)	// NEW BOOTLOADER BINARY
#define EE_SMALL_FONT_ADDR			        ((uint32_t)0x003f0000U)	// SMALL FONTS
#define EE_MIDDLE_FONT_ADDR			        ((uint32_t)0x00420000U)	// MIDDLE FONTS
#define EE_BIG_FONT_ADDR                    ((uint32_t)0x00450000U)	// BIG FONTS
#define EE_NEW_IMAGE_ADDR			        ((uint32_t)0x00480000U)	// DISPLAY IMAGE DOWNLOAD SPACE
#define EE_OLD_FW_ADDR			            ((uint32_t)0x004b0000U)	// BACKUP FIRMWARE BINARY
#define EE_OLD_BLDR_ADDR		            ((uint32_t)0x004e0000U)	// BACKUP BOOTLOADER BINARY
#define EE_DEF_FW_ADDR		                ((uint32_t)0x00510000U)	// DEFAULT FIRMWARE BINARY
#define EE_RC_DEF_BLDR_ADDR                 ((uint32_t)0x00540000U)	// DEFAULT BOOTLOADER BINARY
/* Types  --------------------------------------------------------------------*/
/**
*   EEPROM FUNCTION RETURN STATUS
*/  
typedef enum
{
    EE_FLASH_OK         = ((uint8_t)0x00U),
    EE_FLASH_ERROR      = ((uint8_t)0x01U),
    EE_FLASH_BUSY       = ((uint8_t)0x02U),
    EE_FLASH_TOUT    = ((uint8_t)0x03U)
	
}EEPROM_StatusTypeDef;
/* Variables  ----------------------------------------------------------------*/
/* Macros   ------------------------------------------------------------------*/
//#define FLASH_CS_Low()		(HAL_GPIO_WritePin(FLASH_CS_Port, FLASH_CS_Pin, GPIO_PIN_RESET))
//#define FLASH_CS_High()		(HAL_GPIO_WritePin(FLASH_CS_Port, FLASH_CS_Pin, GPIO_PIN_SET))
/* Function prototypes    ---------------------------------------------------*/
uint8_t FLASH_ReadByte(uint32_t address);
uint16_t FLASH_ReadInt(uint32_t address);
void FLASH_ReadPage(uint32_t address, uint8_t *data, uint16_t size);
void FLASH_WritePage(uint32_t address, uint8_t *data, uint16_t size);
void FLASH_WriteStatusRegister(uint8_t status);
void FLASH_UnprotectSector(uint32_t address);
uint8_t FLASH_WaitReadyStatus(uint32_t timeout);
uint8_t FLASH_ReleasePowerDown(void);
void FLASH_Erase(uint32_t address, uint8_t erase_type);
void EEPROM_Save(uint16_t ee_address, uint8_t* value, uint16_t size);
#endif
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
