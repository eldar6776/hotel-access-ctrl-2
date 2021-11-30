/**
 ******************************************************************************
 * File Name          : main.c
 * Date               : 9.3.2018.
 * Description        : Hotel Room Controller Program Code Header
 ******************************************************************************
 *
 *
 ******************************************************************************
 */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H__
#define __MAIN_H__					FW_BUILD	// version
/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx.h"
#include <string.h>
#include "common.h"
/* Exported Define    --------------------------------------------------------*/
/** ==========================================================================*/
/**             G P I O   P O R T   &   P I N    A L I A S                    */
/** ==========================================================================*/
#define DISP_LED_PWM_Pin			GPIO_PIN_0
#define DISP_LED_PWM_Port 		    GPIOA
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
#define CAPS_ALERT_Pin				SWDIO_Pin
#define CAPS_ALERT_Port				SWDIO_Port
#define SWDIO_Port					GPIOA
#define SWCLK_Pin					GPIO_PIN_14
#define RF24_IRQ_Pin			    SWCLK_Pin
#define RF24_IRQ_Port			    SWCLK_Port
#define SWCLK_Port					GPIOA
#define RF24_CE_Pin                 GPIO_PIN_15
#define RF24_CE_Port			    GPIOA
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
#define DISP_DC_Pin				    GPIO_PIN_10
#define DISP_DC_Port				GPIOB
#define DISP_CS_Pin				    GPIO_PIN_11
#define DISP_CS_Port				GPIOB
#define FLASH_CS_Pin				GPIO_PIN_12
#define FLASH_CS_Port				GPIOB
#define SPI2_SCK_Pin				GPIO_PIN_13
#define SPI2_SCK_Port				GPIOB
#define SPI2_MISO_Pin				GPIO_PIN_14
#define SPI2_MISO_Port				GPIOB
#define SPI2_MOSI_Pin				GPIO_PIN_15
#define SPI2_MOSI_Port				GPIOB
#define RF24_CSN_Pin                GPIO_PIN_13
#define RF24_CSN_Port			    GPIOC
/* Exported types ------------------------------------------------------------*/
typedef enum 
{
    BUZZ_RDY                = ((uint8_t)0x0U),
    BUZZ_OFF                = ((uint8_t)0x1U),
    BUZZ_CARD_INVALID       = ((uint8_t)0x2U),
    BUZZ_CARD_VALID         = ((uint8_t)0x3U),
    BUZZ_DOOR_BELL          = ((uint8_t)0x4U),
    BUZZ_CLEANING_END       = ((uint8_t)0x5U),
    BUZZ_SOS_ALARM          = ((uint8_t)0x6U),
    BUZZ_FIRE_ALARM         = ((uint8_t)0x7U),
    BUZZ_CLICK
	
}SIG_BUZZER_TypeDef;

extern RTC_TimeTypeDef rtime;
extern RTC_DateTypeDef rdate;
extern RTC_HandleTypeDef hrtc;
extern CRC_HandleTypeDef hcrc;
extern I2C_HandleTypeDef hi2c1;
extern SPI_HandleTypeDef hspi2;
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern IWDG_HandleTypeDef hiwdg;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern SIG_BUZZER_TypeDef BUZZ_State;
/* Exporeted Variable  -------------------------------------------------------*/
/* Exported Macro ------------------------------------------------------------*/
/* Exported Function  ------------------------------------------------------- */
void MX_TIM1_Init (void);
void MX_TIM3_Init (void);
void BootloaderExe(void);
void ErrorHandler (uint8_t function, uint8_t driver);
#endif /* __MAIN_H__ */
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
