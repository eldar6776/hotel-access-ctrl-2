/**
 ******************************************************************************
 * File Name          : signaling.h
 * Date               : 28/02/2016 23:16:19
 * Description        : audio visual signaling software modul header
 ******************************************************************************
 */
 
#ifndef __ROOM_H__
#define __ROOM_H__					    FW_BUILD	// version
/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include "common.h"
/* Defines    ----------------------------------------------------------------*/
#define ROOM_CLEANING_TIME			        0x08U   // 8h AM when clean up icon display start time
#define NORMAL_CHECK_OUT_TIME		        0x12U   // 									
#define RFID_LED_TOGGLE_TIME		        250U    // 250 ms rfid led normal blink time
#define ROOM_STATUS_TOGGLE_TIME		        500U    // 500 ms status toggle timer for DND modul to reset
/* Types  --------------------------------------------------------------------*/
extern __IO ROOM_StatusTypeDef ROOM_Status;
extern __IO ROOM_StatusTypeDef ROOM_PreStatus;
/* Variables  ----------------------------------------------------------------*/
extern __IO uint32_t unix_room;
extern __IO uint32_t unix_rtc;
extern __IO uint32_t room_fl;
extern uint8_t temp_cfg;
extern uint8_t temp_dif;
extern int8_t  temp_mv;
extern uint8_t temp_sp;
extern uint8_t bedd_cnt;
extern uint8_t bedd_tim;
/* Macros     ----------------------------------------------------------------*/
/***********************************************************************
**          T H E R M O S T A T		C O N T R O L L E R
***********************************************************************/
#define StatUpdSet()			            (room_fl |=(0x1U << 0))
#define StatUpdReset()					    (room_fl &=(~(1U << 0)))
#define IsStatUpdActiv()                    (room_fl & (0x1U << 0))

#define TempSenLuxRTSet()                   (room_fl |=(0x1U << 1))
#define TempSenLuxRTReset()                 (room_fl &=(~(1U << 1)))
#define IsTempLuxRTActive()                 (room_fl & (0x1U << 1))

#define TempSenDS18Set()                    (room_fl |=(0x1U << 2))
#define TempSenDS18Reset()                  (room_fl &=(~(1U << 2)))
#define IsTempSenDS18Active()               (room_fl & (0x1U << 2))

#define TempSenNTCSet()                     (room_fl |=(0x1U << 3))
#define TempSenNTCReset()                   (room_fl &=(~(1U << 3)))
#define IsTempSenNTCActive()                (room_fl & (0x1U << 3))

#define RstRoomSignSwSet()                  (room_fl |=(0x1U << 4))
#define RstRoomSignSwReset()                (room_fl &=(~(1U << 4)))
#define IsRstRoomSignSwActiv()              (room_fl & (0x1U << 4))

#define PwrExpTimeSet()                     (room_fl |=(0x1U << 5))
#define PwrExpTimeReset()                   (room_fl &=(~(1U << 5)))
#define IsPwrExpTimeActiv()                 (room_fl & (0x1U << 5))

#define RtcTimeValidSet()                   (room_fl |=(0x1U << 6))
#define RtcTimeValidReset()                 (room_fl &=(~(1U << 6)))
#define IsRtcTimeValid()                    (room_fl & (0x1U << 6))

#define RoomCleaningSet()                   (room_fl |=(0x1U << 7))     // disable touch sensors due room cleaning
#define RoomCleaningReset()                 (room_fl &=(~(1U << 7)))
#define IsRoomCleaningActiv()               (room_fl & (0x1U << 7))

#define CardInStackerSet()                  (room_fl |=(0x1U << 8))     // valid card inside stacker
#define CardInStackerReset()                (room_fl &=(~(1U << 8)))    // card removed from stacker
#define IsCardInStackerActiv()              (room_fl & (0x1U << 8))     // check card stacker status

#define TempRegOn()                         (temp_cfg |=(0x1U << 0))    // config On: controll loop is executed periodicaly
#define TempRegOff()                        (temp_cfg &=(~(1U << 0)))   // config Off:controll loop stopped, 
#define IsTempRegActiv()                    (temp_cfg & (0x1U << 0))

#define TempRegHeating()                    (temp_cfg |=(0x1U << 1))    // config Heating: output activ for setpoint value 
#define IsTempRegHeating()                  (temp_cfg & (0x1U << 1))    // abbove measured value
#define IsTempRegCooling()                  (!(temp_cfg&(0x1U << 1)))
#define TempRegCooling()                    (temp_cfg &=(~(1U << 1)))   // config Cooling: opposite from heating

#define TempRegEnable()                     (temp_cfg |=(0x1U << 2))    // conditional flag Enable: controll loop set output state
#define TempRegDisable()                    (temp_cfg &=(~(1U << 2)))   // conditional flag Disable:output is forced to inactiv state, controll loop cannot change outpu
#define IsTempRegEnabled()                  (temp_cfg & (0x1U << 2))

#define TempRegOutputOn()                   (temp_cfg |=(0x1U << 3))    // status On: output demand for actuator to inject energy in to system
#define TempRegOutputOff()                  (temp_cfg &=(~(1U << 3)))   // status Off:stop demanding energy for controlled system, setpoint is reached
#define IsTempRegOutputActiv()              (temp_cfg & (0x1U << 3))

#define IsTempRegSta(x)                     (x & (0x1U << 0)) // remote state config flag
#define IsTempRegMod(x)                     (x & (0x1U << 1)) // remote mode config flag
#define IsTempRegCtr(x)                     (x & (0x1U << 2)) // remote control flag
#define IsTempRegOut(x)                     (x & (0x1U << 3)) // remote output flag
#define IsTempRegNewSta(x)                  (x & (0x1U << 4)) // use remote state config flag 
#define IsTempRegNewMod(x)                  (x & (0x1U << 5)) // use remote mode config flag
#define IsTempRegNewCtr(x)                  (x & (0x1U << 6)) // use remote control flag
#define IsTempRegNewOut(x)                  (x & (0x1U << 7)) // use remote output flag
#define IsTempRegNewCfg(x)                  (x & 0xF0) // use at least one remote flag
/* Function prototypes    ----------------------------------------------------*/
void ROOM_Service(void);
#endif
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
