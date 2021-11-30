/**
 *******************************************************************************
 * File Name          : one_wire.h
 * Date               : 17/11/2016 00:59:00
 * Description        : one wire communication modul header
 *******************************************************************************
 */                                                                             
 
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __OW_H__
#define __OW_H__                                FW_BUILD // version


/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx.h"
/* Exporeted Types   ---------------------------------------------------------*/
/* Exporeted Define   --------------------------------------------------------*/
/* Log event defines    ------------------------------------------------------*/
/* Exporeted Variable   ------------------------------------------------------*/
extern uint32_t ow_flags;
extern uint32_t ow_tmr;
extern uint8_t ow_bps;
extern uint8_t ow_ifa;
extern uint8_t ow_gra;
extern uint8_t ow_bra;
/* Exporeted  Macro  ---------------------------------------------------------*/
#define OW_StartTmr(TIME)               (ow_tmr = TIME)
#define OW_StopTmr()                    (ow_tmr = 0x0U)
#define IsOW_TmrExpired()               (ow_tmr == 0x0U)
#define ThstRemoteSet()                 (ow_flags |= (0x1U << 0x0U))
#define ThstRemoteReset()               (ow_flags &= (~ (0x1U << 0x0U)))
#define IsThstRemoteActiv()             ((ow_flags & (0x1U << 0x0U)) != 0x0U)
#define ButtonRemoteSet()               (ow_flags |= (0x1U << 0x1U))
#define ButtonRemoteReset()             (ow_flags &= (~ (0x1U << 0x1U)))
#define IsButtonRemoteActiv()           ((ow_flags & (0x1U << 0x1U)) != 0x0U)
#define ExtSwRemoteClosed()             (ow_flags |= (0x1U << 0x2U))
#define ExtSwRemoteOpen()               (ow_flags &= (~ (0x1U << 0x2U)))
#define IsExtSwRemoteClosed()           ((ow_flags & (0x1U << 0x2U)) != 0x0U)
#define PowerContactorOn()              (ow_flags |= (0x1U << 0x3U))
#define PowerContactorOff()             (ow_flags &= (~ (0x1U << 0x3U)))
#define IsPowerContactorOn()            ((ow_flags & (0x1U << 0x3U)) != 0x0U)
#define DoorBellOn()                    (ow_flags |= (0x1U << 0x4U))
#define DoorBellOff()                   (ow_flags &= (~ (0x1U << 0x4U)))
#define IsDoorBellOn()                  ((ow_flags & (0x1U << 0x4U)) != 0x0U)
#define StartUpdate()                   (ow_flags |= (0x1U << 0x5U))
#define StopUpdate()                    (ow_flags &= (~ (0x1U << 0x5U)))
#define IsUpdateActiv()                 ((ow_flags & (0x1U << 0x5U)) != 0x0U)
/* Exported Function   -------------------------------------------------------*/
void OW_Init(void);
void OW_Service(void);
void OW_ErrorCallback(void);
void OW_RxCpltCallback(void);
void OW_TxCpltCallback(void);

#endif
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/

