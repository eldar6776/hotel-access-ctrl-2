/**
 ******************************************************************************
 * File Name          : tft_lcd.h
 * Date               : 28/02/2016 23:16:19
 * Description        : tft lcd display software modul header
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


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TFT_LCD_H__
#define __TFT_LCD_H__					051018	// version


/* Include  ------------------------------------------------------------------*/
#include "stm32f1xx.h"


/* Exported Type  ------------------------------------------------------------*/
/* Exported Define  ----------------------------------------------------------*/
#define DISPLAY_HORIZONTAL      					1U
//#define DISPLAY_VERTICAL      					2U
#define ROTATE_DISPLAY								3U
#define LCD_DEFAULT_BRIGHTNESS						500U
#define LCD_DISPLAY_TIMEOUT                         5U      // 5 ms lcd access timeout
/** ==========================================================================*/
/**  I L I 9 3 4 1	  D I S P L A Y		L E V E L	  1		C O M M A N D	  */
/** ==========================================================================*/
#define ILI9341_NOP                                 0x00U
#define ILI9341_SOFT_RESET                          0x01U
#define ILI9341_RD_DISP_ID                          0x04U
#define ILI9341_RD_DISP_STATUS                      0x09U    
#define ILI9341_RD_DISP_POWER_MODE                  0x0aU
#define ILI9341_RD_DISP_MADCTL                      0x0bU
#define ILI9341_RD_DISP_PIXEL_FORMAT                0x0cU
#define ILI9341_RD_DISP_IMAGE_FORMAT                0x0dU
#define ILI9341_RD_DISP_SIGNAL_MODE                 0x0eU
#define ILI9341_RD_DISP_SELF_DIAG                   0x0fU
#define ILI9341_ENTER_SLEEP_MODE                    0x10U
#define ILI9341_SLEEP_OUT                           0x11U
#define ILI9341_PARTIAL_MODE_ON                     0x12U
#define ILI9341_NORMAL_DISP_MODE_ON                 0x13U
#define ILI9341_DISP_INVERSION_OFF                  0x20U
#define ILI9341_DISP_INVERSION_ON                   0x21U
#define ILI9341_GAMMA_SET                           0x26U
#define ILI9341_DISP_OFF                            0x28U
#define ILI9341_DISP_ON                             0x29U
#define ILI9341_COLUMN_ADDRESS_SET                  0x2aU
#define ILI9341_PAGE_ADDRESS_SET                    0x2bU
#define ILI9341_MEMORY_WRITE                        0x2cU
#define ILI9341_COLOR_SET                           0x2dU
#define ILI9341_MEMORY_READ                         0x2eU
#define ILI9341_PARTIAL_AREA                        0x30U
#define ILI9341_VERTICAL_SCROLLING_DEF              0x33U
#define ILI9341_TEARING_EFFECT_LINE_OFF             0x34U
#define ILI9341_TEARING_EFFECT_LINE_ON              0x35U
#define ILI9341_MEMORY_ACCESS_CONTROL               0x36U
#define ILI9341_VERT_SCROLLING_START_ADDRESS        0x37U
#define ILI9341_IDLE_MODE_OFF                       0x38U
#define ILI9341_IDLE_MODE_ON                        0x39U
#define ILI9341_PIXEL_FORMAT_SET                    0x3aU
#define ILI9341_WR_MEMORY_CONTINUE                  0x3cU
#define ILI9341_RD_MEMORY_CONTINUE                  0x3eU
#define ILI9341_SET_TEAR_SCANLINE                   0x44U
#define ILI9341_GET_SCANLINE                        0x45U
#define ILI9341_WR_DISP_BRIGHTNESS                  0x51U
#define ILI9341_RD_DISP_BRIGHTNESS                  0x52U
#define ILI9341_WR_CTRL_DISP                        0x53U
#define ILI9341_RD_CTRL_DISP                        0x54U
#define ILI9341_WR_CONTENT_ADAPTIV_BRIGHT_CTRL      0x55U
#define ILI9341_RD_CONTENT_ADAPTIV_BRIGHT_CTRL      0x56U
#define ILI9341_WR_CABCD_MINIMUM_BRIGHTENSS         0x5eU
#define ILI9341_RD_CABCD_MINIMUM_BRIGHTENSS         0x5fU
#define ILI9341_RD_ID_1                             0xdaU
#define ILI9341_RD_ID_2                             0xdbU
#define ILI9341_RD_ID_3                             0xdcU
/** ==========================================================================*/
/**  I L I 9 3 4 1	  D I S P L A Y		L E V E L	  2		C O M M A N D	  */
/** ==========================================================================*/
#define ILI9341_RGB_INTERFACE_SIGNAL_CTRL           0xb0U
#define ILI9341_FRAME_RATE_CTRL                     0xb1U
#define ILI9341_FRAME_RATE_CTRL_IDLE_8_COLORS       0xb2U
#define ILI9341_FRAME_RATE_CTRL_PARTIAL_FULL        0xb3U
#define ILI9341_DISP_INVERSION_CTRL                 0xb4U
#define ILI9341_BLANKING_PORCH_CTRL                 0xb5U
#define ILI9341_DISP_FUNCTION_CTRL                  0xb6U
#define ILI9341_ENTRY_MODE_SET                      0xb7U
#define ILI9341_BACKLIGHT_CTRL_1                    0xb8U
#define ILI9341_BACKLIGHT_CTRL_2                    0xb9U
#define ILI9341_BACKLIGHT_CTRL_3                    0xbaU
#define ILI9341_BACKLIGHT_CTRL_4                    0xbbU
#define ILI9341_BACKLIGHT_CTRL_5                    0xbcU
#define ILI9341_BACKLIGHT_CTRL_7                    0xbeU
#define ILI9341_BACKLIGHT_CTRL_8                    0xbfU
#define ILI9341_POWER_CTRL_1                        0xc0U
#define ILI9341_POWER_CTRL_2                        0xc1U
#define ILI9341_VCOM_CTRL_1                         0xc5U
#define ILI9341_VCOM_CTRL_2                         0xc7U
#define ILI9341_NV_MEMORY_WR                        0xd0U
#define ILI9341_NV_MEMORY_PROTECTION_KEY            0xd1U
#define ILI9341_NV_MEMORY_STATUS_RD                 0xd2U
#define ILI9341_READ_ID_4                           0xd3U
#define ILI9341_POSITIVE_GAMMA_CORRECTION           0xe0U
#define ILI9341_NEGATIVE_GAMMA_CORRECTION           0xe1U
#define ILI9341_DIGITAL_GAMMA_CTRL_1                0xe2U
#define ILI9341_DIGITAL_GAMMA_CTRL_2                0xe3U
#define ILI9341_INTERFACE_CTRL                      0xf6U
/** ==========================================================================*/
/**  I L I 9 3 4 1	  D I S P L A Y		E X T.   R E G .   C O M M A N D	  */
/** ==========================================================================*/
#define ILI9341_POWER_CTRL_A                        0xcbU
#define ILI9341_POWER_CTRL_B                        0xcfU
#define ILI9341_DRIVER_TIMING_CTRL_A                0xe8U
#define ILI9341_DRIVER_TIMING_CTRL_B                0xeaU
#define ILI9341_POWER_ON_SEQUENCE_CTRL              0xedU
#define ILI9341_ENABLE_3G                           0xf2U
#define ILI9341_PUMP_RATIO_CTRL                     0xf7U
/** ==========================================================================*/
/**		  I L I 9 3 4 1		  D I S P L A Y		  O R I E N T A T I O N		  */
/** ==========================================================================*/
#ifdef DISPLAY_HORIZONTAL
#define LCD_W 320U
#define LCD_H 240U
#endif

#ifdef DISPLAY_VERTICAL
#define LCD_W 240
#define LCD_H 320
#endif

#define DISPLAY_PIXEL_CNT  (LCD_W * LCD_H)
/** ==========================================================================*/
/**		  I L I 9 3 4 1			  D I S P L A Y			C O L O R S	 	  	  */
/** ==========================================================================*/
#define WHITE				0xFFFFU
#define BLACK				0x0000U
#define BLUE				0x001FU
#define BRED				0xF81FU
#define GRED				0xFFE0U
#define GBLUE				0x07FFU
#define RED					0xF800U
#define MAGENTA				0xF81FU
#define GREEN				0x07E0U
#define CYAN				0x7FFFU
#define YELLOW				0xFFE0U
#define BROWN				0xBC40U
#define BRRED				0xFC07U
#define GRAY				0x8430U
#define DARKBLUE			0x01CFU
#define LIGHTBLUE			0x7D7CU
#define GRAYBLUE			0x5458U
#define LIGHTGREEN			0x841FU
#define LGRAY				0xC618U
#define LGRAYBLUE			0xA651U
#define LBBLUE          	0x2B12U
/** ==========================================================================*/
/**	I L I 9 3 4 1	D I S P L A Y    F O N T	A N D	 T E X T	L I N E   */
/** ==========================================================================*/
#define SMALL_SIZE_FONT		1U
#define MIDDLE_SIZE_FONT	2U
#define BIG_SIZE_FONT		3U
/*	smal font number of horisontal text lines	*/
#define Line0_S       		0U
#define Line1_S      		16U
#define Line2_S      		32U
#define Line3_S     		48U
#define Line4_S      		64U
#define Line5_S     		80U
#define Line6_S     		96U
#define Line7_S   			112U
#define Line8_S 			128U
#define Line9_S				144U
#define Line10_S			160U
#define Line11_S			176U
#define Line12_S			192U
#define Line13_S			208U
#define Line14_S			224U
/*	middle font number of horisontal text lines	*/
#define Line0_M       		0U
#define Line1_M      		20U
#define Line2_M      		40U
#define Line3_M     		60U
#define Line4_M      		80U
#define Line5_M     		100U
#define Line6_M     		120U
#define Line7_M   			140U
#define Line8_M 			160U
#define Line9_M				180U
#define Line10_M			200U
#define Line11_M			220U
/*	big font number of horisontal text lines	*/
#define Line0_B       		0U
#define Line1_B     		26U
#define Line2_B     		52U
#define Line3_B     		78U
#define Line4_B       		104U
#define Line5_B       		130U
#define Line6_B      		146U
#define Line7_B      		182U
#define Line8_B      		214U


/* Exported Variable   -------------------------------------------------------*/
extern uint16_t BACK_COLOR, POINT_COLOR;
extern uint8_t font_size;
extern uint8_t asc_1608[];
extern uint16_t asc_1220[];
extern uint16_t asc_1626[];


/* Exported Macro ------------------------------------------------------------*/
/* Exported Function   -------------------------------------------------------*/
void DISPLAY_Init(void); 
void DISPLAY_Clear(uint16_t color);
void DISPLAY_WriteRegister(uint8_t data);
void DISPLAY_ShowChar(uint16_t x, uint16_t y, uint8_t num);
uint8_t DISPLAY_ReadCommand(uint8_t command, uint8_t index);
void DISPLAY_PrintString(uint16_t x, uint16_t y, uint8_t *p);
void DISPLAY_ShowString(uint16_t x, uint16_t y, const char *p);
void DISPLAY_ShowNum(uint16_t x, uint16_t y, uint32_t num, uint8_t len);
void DISPLAY_AddressSet(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);


#endif
/******************************   END OF FILE  **********************************/
