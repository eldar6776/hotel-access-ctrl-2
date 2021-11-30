/**
 ******************************************************************************
 * File Name          : display.h
 * Date               : 10.3.2018
 * Description        : GUI display module header
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
*
*
*
				******************************************
				**										**
				**		SERVICE	MENU DISPLAY SCREEN 1	**<<<<<<<<
				**										**		/\
				******************************************		/\
				**		DIGITAL   INPUT   SETTINGS		**		/\
				**		--------------------------		**		/\
				**		CARD STACKER      SET-> ON		**		/\
				**		SOS ALARM               ON		**		/\
				**		SOS RESET              OFF		**		/\
				**		HANDMAID CALL          OFF		**		/\
				**		MINIBAR SENSOR    DISABLED		**		/\
				**		BALCONY DOOR            ON		**		/\
				**		DND SWITCH             OFF		**		/\
				**		ENTRY DOOR        DISABLED		**		|
				**		--------------------------		**		|
				**		ENTER              >> NEXT		**>>>	|
				******************************************	V	|
				**		NEXT >>				SELECT 		**	V	|
				******************************************	V	|
				**		SET	++			  DESELECT		**	V	|
				******************************************	V	|
															V	|
															V	|
															V	|
															V	|
				******************************************	V	|
				**										**	V	|
				**      SERVICE	MENU DISPLAY SCREEN 2	**<<<	|
				**										**		|
				******************************************		|
				**		DIGITAL  OUTPUT   SETTINGS		**		|
				**		--------------------------		**		|
				**		POWER CONTACTOR			ON		**		|
				**		DND & HM RESET			ON		**		/\
				**		BALCONY LIGHT		   OFF		**		/\
				**		DOOR BELL			   OFF		**		/\
				**		HVAC POWER			   OFF		**		/\
				**		HVAC THERMOSTAT		   OFF		**		/\
				**		DOOR LOCK		  SET-> ON		**		|
				**		PCB BUZZER			   OFF		**		|
				**		--------------------------		**		|
				**		ENTER 	           >> NEXT		**>>>	|
				******************************************	V	|
				**		NEXT >>				SELECT 		**	V	|
				******************************************	V	|
				**		SET	++			  DESELECT		**	V	|
				******************************************	V	|
															V	|
															V	|
															V	|
															V	|
															V	|
				******************************************	V	|
				**										**	V	|
				**		SERVICE	MENU DISPLAY SCREEN 3	**<<<	|
				**										**		|
				******************************************		|
				**		SETUP  SYSTEM  SETTINGS  1		**		|
				**		--------------------------		**		|
				**		RESTART CONTROLLER				**		|
				**		PREVIEW DISPLAY IMAGES			**		/\
				**		REQUEST IMAGES UPDATE			**		/\
				**		REQUEST FIRMWARE UPDATE			**		/\
				**		SCAN ONEWIRE: LUX Term. x3		**		/\
				**		SET RS485 ADDRESS    12345		**		/\
				**		SET SYSTEM ID	     65432		**		|
				**		SET TIME  12:00 01.01.2018		**		|
				**		--------------------------		**		|
				**		ENTER 	           >> NEXT		**>>>	|
				******************************************	V	|
				**		NEXT >>				SELECT 		**	V	|
				******************************************	V	|
				**		SET	++			  DESELECT		**	V	|
				******************************************	V	|
															V	|
															V	|
															V	|
															V	|
															V	|
				******************************************	V	|
				**										**	V	|
				**		SERVICE	MENU DISPLAY SCREEN 4	**<<<	|
				**										**		|
				******************************************		|
				**		SETUP  SYSTEM  SETTINGS  2		**		|
				**		--------------------------		**		|
				**		DISPLAY CARD TEXT	   OFF		**		/\
				**		TAMPER PROTECTION		ON		**		/\
				**		WIRELESS RADIO			ON		**		/\
				**		RADIO ADDRESS		 RC001		**		/\
				**		RADIO CHANEL			22		**		/\
				**		BUZZER VOLUME          123		**		|
				**		DOORLOCK POWER	       255		**		|
				**		LOAD SYSTEM DEFAULT				**		|
				**		--------------------------		**		|
				**		ENTER 	           >> NEXT		**>>>	|
				******************************************	V	|
				**		NEXT >>				SELECT 		**	V	|
				******************************************	V	|
				**		SET	++			  DESELECT		**	V	|
				******************************************	V	|
															V	|
															V	|
															V	|
															V	|
															V	|
				******************************************	V	|
				**										**	V	|
				**		SERVICE	MENU DISPLAY SCREEN 5	**<<<	|
				**										**		|
				******************************************		|
				**		EXTEND  ACCESS	PERMISSION		**		|
				**		--------------------------		**		|
				**		PERMITTED ADDRESS 1: 12345		**		/\
				**		PERMITTED ADDRESS 2: 00000		**		/\
				**		PERMITTED ADDRESS 3: 00000		**		/\
				**		PERMITTED ADDRESS 4: 00000		**		/\
				**		PERMITTED ADDRESS 5: 00000		**		/\
				**		PERMITTED ADDRESS 6: 00000		**		/\
				**		PERMITTED ADDRESS 7: 00000		**		/\
				**		PERMITTED ADDRESS 8: 00000		**		/\
				**		--------------------------		**		|
				**		ENTER 	           >> NEXT		**>>>>>>|
				******************************************
				**		NEXT >> 			SELECT 		**
				******************************************
				**		SET	++            DESELECT		**
				******************************************
            
            
******************************************************************************
*/


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DISP_H__
#define __DISP_H__					FW_BUILD	// version


/* Include  ------------------------------------------------------------------*/
#include "stm32f1xx.h"
#include "main.h"
/* Exported Define  ----------------------------------------------------------*/
#define MENU_TIMEOUT                    30000U  // 30 sec. display_menu timeout 
#define LCD_DEFAULT_BRIGHTNESS          500U
#define MAX_SCREENS_IN_MENU             5U
#define MAX_ITEMS_IN_SCREEN             8U
#define VALUE_BUFF_SIZE                 16U
#define IDLE                            0U
#define PRINT                           1U
#define ACTIV                           2U
#define CHANGED                         3U
#define SELECTED                        4U
#define NOT_SELECTED                    5U
#define PENDING                         6U
#define SELECT_ITEM                     7U
#define VALUE_IN_BUFFER                 8U
#define LABEL_ITEM                      9U
#define TIME_SETUP                      10U
/* Exported Type  ------------------------------------------------------------*/
typedef enum
{
    FONT_IDLE               = ((uint8_t)0x00U),
    SMALL_FONT              = ((uint8_t)0x01U),
    MIDDLE_FONT	            = ((uint8_t)0x02U),
    BIG_FONT		        = ((uint8_t)0x03U)
    
}FONT_SizeTypeDef;


typedef enum
{
    TASTER_IDLE             = ((uint8_t)0x00U),     // no taster event 
    LEFT_TASTER_PRESSED     = ((uint8_t)0x01U),     // handmaid taster pressed event
    RIGHT_TASTER_PRESSED    = ((uint8_t)0x02U),     // doorbell taster pressed event
    TASTER_END              = ((uint8_t)0x03U)      // set menu structure
    
}eTasterEventTypeDef;


typedef struct
{
    uint8_t index;              // hold index for selected menu screen
    uint8_t state;              // hold menu state flag for upper display function
    uint8_t select;             // hold menu screen select state for taster selection
    uint8_t event;              // hold menu event to process
    uint8_t item_index;         // index of current item in focus
    uint8_t value_index;        // index of current value unit in focus
    uint8_t value_edit;         // hold flag for enabled value edit mode for faster menu shift
    uint8_t increment;          // increment value for all type of menu edit types
    uint8_t item_type[8];       // label flag, item select, value edit
    uint8_t item_label[8];      // label text from list
    uint8_t item_to_edit[8];    // value edit item type number of values from item buffer
    uint8_t item_buff[8][8];    // value edit type buffer
    
}sMenuTypeDef;
/* Exported Variables  -------------------------------------------------------*/
extern uint32_t disp_fl;
extern uint8_t disp_sta;
extern uint8_t disp_rot;
extern uint16_t lcd_bcklght;
extern uint8_t jrnl_mod;
extern uint8_t jrnl_buf[JRNL_BSIZE];
extern sMenuTypeDef menu;
/* Exported Macro   --------------------------------------------------------- */
#define DISP_BldrUpdSet()                   (disp_fl |=(0x1U << 0))
#define DISP_BldrUpdReset()		            (disp_fl &=(~(1U << 0)))
#define IsDISP_BldrUpdSetActiv()		    (disp_fl & (0x1U << 0))
#define DISP_BldrUpdFailSet()			    (disp_fl |=(0x1U << 1))
#define DISP_BldrUpdFailReset()	            (disp_fl &=(~(1U << 1)))
#define IsDISP_BldrUpdFailActiv()	        (disp_fl & (0x1U << 1))
#define DISP_PreviewImage()					(disp_fl |=(0x1U << 2))
#define DISP_PreviewImgDel()			    (disp_fl &=(~(1U << 2)),    DISP_RefreshSet())
#define IsDISP_PreviewImgActiv()			(disp_fl & (0x1U << 2))
#define DISP_RoomNumImgSet()				(disp_fl |=(0x1U << 3),     DISP_RefreshSet())
#define DISP_RoomNumImgReset()              (disp_fl &=(~(1U << 3)),    DISP_RefreshSet())
#define IsDISP_RoomNumImgActiv()		    (disp_fl & (0x1U << 3))
#define DISP_DndImg()                       (disp_fl |=(0x1U << 4),     DISP_RefreshSet())
#define DISP_DndImgDelete()		            (disp_fl &=(~(1U << 4)),    DISP_RefreshSet())
#define IsDISP_DndImgActiv()                (disp_fl & (0x1U << 4))
#define DISP_SosAlarmImage()                (disp_fl |=(0x1U << 5),     DISP_RefreshSet())
#define DISP_SosAlarmImgDel()			    (disp_fl &=(~(1U << 5)),    DISP_RefreshSet())
#define IsDISP_SosAlarmImgActiv()			(disp_fl & (0x1U << 5))
#define DISP_CleanUpImage()					(disp_fl |=(0x1U << 6),     DISP_RefreshSet())
#define DISP_CleanUpImgDel()			    (disp_fl &=(~(1U << 6)),    DISP_RefreshSet())
#define IsDISP_CleanUpImgActiv()			(disp_fl & (0x1U << 6))
#define DISP_GeneralCleanUpImg()			(disp_fl |=(0x1U << 7),     DISP_RefreshSet())
#define DISP_GenClnImgDelete()              (disp_fl &=(~(1U << 7)),    DISP_RefreshSet())
#define IsDISP_GenCleanImgActiv()	        (disp_fl & (0x1U << 7))
#define DISP_CardValidImage()				(disp_fl |=(0x1U << 8),     DISP_ImgUpdSet())
#define DISP_CardValidImgDel()			    (disp_fl &=(~(1U << 8)),    DISP_RefreshSet())
#define IsDISP_CardValidImgActiv()          (disp_fl & (0x1U << 8))
#define DISP_CardInvalidImage()				(disp_fl |=(0x1U << 9),     DISP_ImgUpdSet())
#define DISP_CardInvalidImgDel()		    (disp_fl &=(~(1U << 9)),    DISP_RefreshSet())
#define IsDISP_CardInvalidImgActiv()		(disp_fl & (0x1U << 9))
#define DISP_WrongRoomImage()				(disp_fl |=(0x1U << 10),    DISP_ImgUpdSet())
#define DISP_WrongRoomImgDel()			    (disp_fl &=(~(1U << 10)),   DISP_RefreshSet())
#define IsDISP_WrongRoomImgActiv()          (disp_fl & (0x1U << 10))
#define DISP_TimeExpiredImage()				(disp_fl |=(0x1U << 11),    DISP_ImgUpdSet())
#define DISP_TimeExpiredImgDel()		    (disp_fl &=(~(1U << 11)),   DISP_RefreshSet())
#define IsDISP_TimeExpiredImgActiv()		(disp_fl & (0x1U << 11))
#define DISP_OutOfSrvcImgSet()              (disp_fl |=(0x1U << 12),    DISP_RefreshSet())
#define DISP_OutOfSrvcImgReset()	        (disp_fl &=(~(1U << 12)),   DISP_RefreshSet())
#define IsDISP_OutOfSrvcImgActiv()	        (disp_fl & (0x1U << 12))
#define DISP_MinibarUsedImage()				(disp_fl |=(0x1U << 13),    DISP_RefreshSet())
#define DISP_MinibarUsedImgDel()		    (disp_fl &=(~(1U << 13)),   DISP_RefreshSet())
#define IsDISP_MinibarUsedImgActiv()		(disp_fl & (0x1U << 13))
#define DISP_FireExitImage()                (disp_fl |=(0x1U << 14),    DISP_RefreshSet())
#define DISP_FireExitImgDel()			    (disp_fl &=(~(1U << 14)),   DISP_RefreshSet())
#define IsDISP_FireExitImgActiv()			(disp_fl & (0x1U << 14))
#define DISP_FireAlarmImage()				(disp_fl |=(0x1U << 15),    DISP_RefreshSet())
#define DISP_FireAlarmImgDel()			    (disp_fl &=(~(1U << 15)),   DISP_RefreshSet())
#define IsDISP_FireAlarmImgActiv()          (disp_fl & (0x1U << 15))
#define DISP_BeddRepImg()		            (disp_fl |=(0x1U << 16),    DISP_RefreshSet())
#define DISP_BeddRepImgDelete()             (disp_fl &=(~(1U << 16)),   DISP_RefreshSet())
#define IsDISP_BeddRepImgActiv()            (disp_fl & (0x1U << 16))
#define DISP_UpdProgMsgSet()                (disp_fl |=(0x1U << 17))
#define DISP_UpdProgMsgDel()                (disp_fl &=(~(1U << 17)))
#define IsDISP_UpdProgMsgActiv()            (disp_fl & (0x1U << 17))
#define DISP_FwrUpdd()                      (disp_fl |=(0x1U << 18))
#define DISP_FwrUpddDelete()                (disp_fl &=(~(1U << 18)))
#define IsDISP_FwrUpddActiv()               (disp_fl & (0x1U << 18))
#define DISP_FwrUpdFail()                   (disp_fl |=(0x1U << 19))
#define DISP_FwrUpdFailDelete()             (disp_fl &=(~(1U << 19)))
#define IsDISP_FwrUpdFailActiv()            (disp_fl & (0x1U << 19))


#define DISP_ImgUpdSet()                    (disp_fl |=(0x1U << 30))
#define DISP_imgUpdReset()                  (disp_fl &=(~(1U << 30)))
#define IsDISP_ImgUpdActiv()                (disp_fl & (0x1U << 30))
#define DISP_RefreshSet()					(disp_fl |=(0x1U << 31))
#define DISP_RefreshReset()                 (disp_fl &=(~(1U << 31)))
#define IsDISP_RefreshRequested()			(disp_fl & (0x1U << 31))


#define DISP_UserCardInfoTextEnable()       (disp_sta |=(0x1U << 1))
#define DISP_UserCardInfoTextDisable()      (disp_sta &=(~(1U << 1)))
#define IsDISP_UserCardInfoTextEnabled()    (disp_sta & (0x1U << 1))
/* Exported functions ------------------------------------------------------- */
void DISP_Init(void);
void DISP_Service(void);
uint8_t GetDISP_ScreenState(void);

#endif  /*__DISP_H*/

