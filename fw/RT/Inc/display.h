/**
 ******************************************************************************
 * File Name          : display.h
 * Date               : 10.3.2018
 * Description        : GUI Display Module Header
 ******************************************************************************
 *
 *
 ******************************************************************************
 */
 
 
 /* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DISP_H__
#define __DISP_H__                           FW_BUILD // version


/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx.h"
#include "main.h"
/* Exported Define -----------------------------------------------------------*/
extern EDIT_Handle     hEDIT_RFSen;
/* Exported types ------------------------------------------------------------*/
typedef enum	// display button states
{
	RELEASED    = 0U,
	PRESSED     = 1U,
    BUTTON_SHIT = 2U
	
}BUTTON_StateTypeDef;

typedef struct
{
    uint8_t index;      // index position in weather forecast screen
    uint8_t week_day;   // day in week
    uint8_t cloudness;  // used to select icon
    int8_t high_temp;   // maximun daily temperature color red value
    int8_t low_temp;    // minimal daily temperature colour blue value
	
}FORECAST_DayTypeDef;

extern FORECAST_DayTypeDef FORECAST_Day[];
/** ==========================================================================*/
/**                                                                           */
/**    	  W E A T H E R     F O R E C A S T     P A C K E T                   */
/**                                                                           */
/** ==========================================================================*/
/*
*	BYTE0:	DATA_TIME_STAMP			uint8 	24 ~ 31
	BYTE1:	DATA_TIME_STAMP			uint8	16 ~ 23
	BYTE2:	DATA_TIME_STAMP			uint8	 8 ~ 15
	BYTE3:	DATA_TIME_STAMP			uint8	 0 ~ 7
*   BYTE4:	ACTUAL TEMPERATURE		int8  -60 ~ +60
*   BYTE5:	ACTUAL CLOUDNESS		uint8 	 1 ~ 7
*   BYTE6:	DAY + 1 HI TEMPERATURE	int8  -60 ~ +60
*   BYTE7:	DAY + 1 LO TEMPERATURE	int8  -60 ~ +60
*   BYTE8:	DAY + 1 CLOUDS			uint8	 1 ~ 7
*   BYTE9:	DAY + 2 HI TEMPERATURE	int8  -60 ~ +60
*   BYTE10:	DAY + 2 LO TEMPERATURE	int8  -60 ~ +60
*   BYTE11:	DAY + 2 CLOUDS			uint8	 1 ~ 7
*   BYTE12:	DAY + 3 HI TEMPERATURE	int8  -60 ~ +60
*   BYTE13:	DAY + 3 LO TEMPERATURE	int8  -60 ~ +60
*   BYTE14:	DAY + 3 CLOUDS			uint8	 1 ~ 7
*   BYTE15:	DAY + 4 HI TEMPERATURE	int8  -60 ~ +60
*	BYTE16:	DAY + 4 LO TEMPERATURE	int8  -60 ~ +60
*   BYTE17:	DAY + 4 CLOUDS			uint8	 1 ~ 7
*   BYTE18:	DAY + 5 HI TEMPERATURE	int8  -60 ~ +60
*   BYTE19:	DAY + 5 LO TEMPERATURE	int8  -60 ~ +60
*   BYTE20:	DAY + 5 CLOUDS			uint8	 1 ~ 7
*
*/
/* Exported variables  -------------------------------------------------------*/
extern uint32_t disp_fl;
extern uint32_t thst_fl;
extern uint8_t disp_img_id, disp_img_time, last_img_id;
extern uint8_t disp_high_bcklght, disp_low_bcklght;
extern uint8_t scrnsvr_ena_hour, scrnsvr_dis_hour;
extern uint8_t scrnsvr_tout, scrnsvr_clk_clr, scrnsvr_semiclk_clr;
extern int8_t wfc_buff[];
extern uint16_t rfsen[];
extern int16_t room_temp;
extern int16_t room_ntc_temp;
extern int8_t  ntc_offset;
extern uint8_t thst_min_sp;
extern uint8_t thst_max_sp;
extern uint8_t thst_sp;
extern uint8_t thst_dif;
extern uint8_t thst_cfg;
extern uint8_t rfsen_sel;
/* Exported macros     -------------------------------------------------------*/
/** ==========================================================================*/
/**                 T H E R M O S T A T         F L A G S                     */
/** ==========================================================================*/
#define TempRegRemOn()                      (temp_cfg |=(0x1U << 0))   // config On: controll loop is executed periodicaly
#define TempRegRemOff()                     (temp_cfg &=(~(1U << 0)))  // config Off:controll loop stopped, 
#define IsTempRegRemActiv()                 (temp_cfg & (0x1U << 0))

#define TempRegRemHeating()                 (temp_cfg |=(0x1U << 1))   // config Heating: output activ for setpoint value 
#define TempRegRemCooling()                 (temp_cfg &=(~(1U << 1)))  // config Cooling: opposite from heating
#define IsTempRegRemHeating()               (temp_cfg & (0x1U << 1))   // abbove measured value
#define IsTempRegRemCooling()               (!IsTempRegRemHeating())

#define TempRegRemEnable()                  (temp_cfg |=(0x1U << 2))   // conditional flag Enable: controll loop set output state
#define TempRegRemDisable()                 (temp_cfg &=(~(1U << 2)))  // conditional flag Disable:output is forced to inactiv state, controll loop cannot change outpu
#define IsTempRegRemEnabled()               (temp_cfg & (0x1U << 2))

#define TempRegRemOutputOn()                (temp_cfg |=(0x1U << 3))   // status On: output demand for actuator to inject energy in to system
#define TempRegRemOutputOff()               (temp_cfg &=(~(1U << 3)))  // status Off:stop demanding energy for controlled system, setpoint is reached
#define IsTempRegRemOutputActiv()           (temp_cfg & (0x1U << 3))
/** ==========================================================================*/
/**                     D I S P L A Y           F L A G S                     */
/** ==========================================================================*/
#define DISP_UpdateSet()                    (disp_fl  |=(0x1U << 0))
#define DISP_UpdateReset()                  (disp_fl  &=(~(1U << 0)))
#define IsDISP_UpdateActiv()                (disp_fl  & (0x1U << 0))
#define DISP_FaultSet()                     ((disp_fl |=(0x1U << 1)),DISP_UpdateSet(),last_img_id=10U,disp_img_id=10U,disp_img_time=0x0U,buzz_sig_time=0x0U,buzz_sig_id=0x0U)
#define DISP_FaultReset()                   (disp_fl  &=(~(1U << 1)))
#define IsDISP_FaultActiv()                 (disp_fl  & (0x1U << 1))
#define DISP_GuiSet()                       ((disp_fl |=(0x1U << 2)),DISP_UpdateSet(),last_img_id=0x0U,disp_img_id=0x0U,disp_img_time=0x0U,buzz_sig_time=0x0U,buzz_sig_id=0x0U)
#define DISP_GuiReset()                     (disp_fl  &=(~(1U << 2)))
#define IsDISP_GuiActiv()                   (disp_fl  & (0x1U << 2))
#define ButtonOpenSet()                     (disp_fl  |=(0x1U << 3)) 
#define ButtonOpenReset()                   (disp_fl  &=(~(1U << 3)))
#define IsButtonOpenActiv()                 (disp_fl  & (0x1U << 3))
#define ButtonOkSet()                       (disp_fl  |=(0x1U << 4)) 
#define ButtonOkReset()                     (disp_fl  &=(~(1U << 4)))
#define IsButtonOkActiv()                   (disp_fl  & (0x1U << 4))
#define ButtonUpdateSet()                   (disp_fl  |=(0x1U << 5)) 
#define ButtonUpdateReset()                 (disp_fl  &=(~(1U << 5)))
#define IsButtonUpdateActiv()               (disp_fl  & (0x1U << 5))
#define SetpointUpdateSet()                 (disp_fl  |=(0x1U << 6)) 
#define SetpointUpdateReset()               (disp_fl  &=(~(1U << 6)))
#define IsSetpointUpdateActiv()             (disp_fl  & (0x1U << 6))
#define RoomTempUpdateSet()                 (disp_fl  |=(0x1U << 7)) 
#define RoomTempUpdateReset()               (disp_fl  &=(~(1U << 7)))
#define IsRoomTempUpdateActiv()             (disp_fl  & (0x1U << 7))
#define NtcUpdateSet()                      (disp_fl  |=(0x1U << 8)) 
#define NtcUpdateReset()                    (disp_fl  &=(~(1U << 8)))
#define IsNtcUpdateActiv()                  (disp_fl  & (0x1U << 8))
#define NtcValidSet()                       (disp_fl  |=(0x1U << 9))
#define NtcValidReset()                     (disp_fl  &=(~(1U << 9)))
#define IsNtcValidActiv()                   (disp_fl  & (0x1U << 9))
#define RtcValidSet()                       (disp_fl  |=(0x1U << 10))
#define RtcValidReset()                     (disp_fl  &=(~(1U << 10)))
#define IsRtcValidActiv()                   (disp_fl  & (0x1U << 10))
#define WFC_UpdateSet()                     (disp_fl  |=(0x1U << 11))
#define WFC_UpdateReset()                   (disp_fl  &=(~(1U << 11)))
#define IsWFC_UpdateActiv()                 (disp_fl  & (0x1U << 11))
#define WFC_ValidSet()                      (disp_fl  |=(0x1U << 12))
#define WFC_ValidReset()                    (disp_fl  &=(~(1U << 12)))
#define IsWFC_ValidActiv()                  (disp_fl  & (0x1U << 12))
#define ScrnsvrEnable()                     (disp_fl  |=(0x1U << 13)) 
#define ScrnsvrDisable()                    (disp_fl  &=(~(1U << 13)))
#define IsScrnsvrEnabled()                  (disp_fl  & (0x1U << 13))
#define ScrnsvrInitSet()                    (disp_fl  |=(0x1U << 14))
#define ScrnsvrInitReset()                  (disp_fl  &=(~(1U << 14)))
#define IsScrnsvrInitActiv()                (disp_fl  & (0x1U << 14))
#define ScrnsvrSet()                        (disp_fl  |=(0x1U << 15)) 
#define ScrnsvrReset()                      (disp_fl  &=(~(1U << 15)))
#define IsScrnsvrActiv()                    (disp_fl  & (0x1U << 15))
#define ScreenInitSet()                     (disp_fl  |=(0x1U << 16))
#define ScreenInitReset()                   (disp_fl  &=(~(1U << 16)))
#define IsScreenInitActiv()                 (disp_fl  & (0x1U << 16))
#define DISP_FwUpdSet()                     (disp_fl  |=(0x1U << 17))
#define DISP_FwUpdReset()                   (disp_fl  &=(~(1U << 17)))
#define IsDISP_FwUpdActiv()                 (disp_fl  & (0x1U << 17))
#define DISP_FwUpdFailSet()                 (disp_fl  |=(0x1U << 18))
#define DISP_FwUpdFailReset()               (disp_fl  &=(~(1U << 18)))
#define IsDISP_FwUpdFailActiv()             (disp_fl  & (0x1U << 18))
#define DISP_RFSenSet()                     (disp_fl  |=(0x1U << 19))
#define DISP_RFSenReset()                   (disp_fl  &=(~(1U << 19)))
#define IsDISP_RFSenActiv()                 (disp_fl  & (0x1U << 19))
#define DISP_CleaningSet()                  (disp_fl  |=(0x1U << 20))
#define DISP_CleaningReset()                (disp_fl  &=(~(1U << 20)))
#define IsDISP_CleaningActiv()              (disp_fl  & (0x1U << 20))
#define BtnOkPresed()                       (disp_fl  |=(0x1U << 21)) 
#define BtnOkRelease()                      (disp_fl  &=(~(1U << 21)))
#define IsBtnOkPressed()                    (disp_fl  & (0x1U << 21))
/** ==========================================================================*/
/**       M E M O R I S E D     A N D     R E S T O R E D     F L A G S       */
/** ==========================================================================*/
#define ThermostatOn()                      (thst_fl |= (0x1U << 0))
#define ThermostatOff()                     (thst_fl &= (~ (0x1U << 0)))
#define IsThermostatActiv()                 ((thst_fl & (0x1U << 0)) != 0x0U)
#define ButtonDndSet()                      (thst_fl |= (0x1U << 1)) 
#define ButtonDndReset()                    (thst_fl &= (~ (0x1U << 1)))
#define IsButtonDndActiv()                  ((thst_fl & (0x1U << 1)) != 0x0U)
#define ButtonSosSet()                      (thst_fl |= (0x1U << 2)) 
#define ButtonSosReset()                    (thst_fl &= (~ (0x1U << 2)))
#define IsButtonSosActiv()                  ((thst_fl & (0x1U << 2)) != 0x0U)
#define ButtonCallSet()                     (thst_fl |= (0x1U << 3)) 
#define ButtonCallReset()                   (thst_fl &= (~ (0x1U << 3)))
#define IsButtonCallActiv()                 ((thst_fl & (0x1U << 3)) != 0x0U)
#define ScrnsvrClkSet()                     (thst_fl |= (0x1U << 4))           // flag for screensaver digital clock selection
#define ScrnsvrClkReset()                   (thst_fl &= (~ (0x1U << 4)))
#define IsScrnsvrClkActiv()                 ((thst_fl & (0x1U << 4)) != 0x0U)
#define ScrnsvrSemiClkSet()                 (thst_fl |= (0x1U << 5))           // flag for screensaver small logo sze digital clock selection
#define ScrnsvrSemiClkReset()               (thst_fl &= (~ (0x1U << 5)))
#define IsScrnsvrSemiClkActiv()             ((thst_fl & (0x1U << 5)) != 0x0U)
#define NtcErrorSet()                       (thst_fl |= (0x1U << 6))
#define NtcErrorReset()                     (thst_fl &= (~ (0x1U << 6)))
#define IsNtcErrorActiv()                   ((thst_fl & (0x1U << 6)) != 0x0U)
#define TempRegEnable()                     (thst_fl |= (0x1U << 0x7U))            // conditional flag Enable: controll loop set output state
#define TempRegDisable()                    (thst_fl &= (~ (0x1U << 0x7U)))        // conditional flag Disable:output is forced to inactiv state, controll loop cannot change outpu
#define IsTempRegEnabled()                  ((thst_fl & (0x1U << 0x7U)) != 0x0U)
/* Exported functions  -------------------------------------------------------*/
void DISP_Init(void);
void DISP_Service(void);
uint8_t DISP_QR_Code(void);
void DISP_SetBrightnes(uint8_t val);
uint8_t DISP_Message(uint8_t *buff);
void DISP_RFSensor(uint16_t addr, uint8_t stat);
#endif
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
