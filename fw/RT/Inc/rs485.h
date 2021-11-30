/**
 ******************************************************************************
 * File Name          : rs485.h
 * Date               : 28/02/2016 23:16:19
 * Description        : rs485 communication modul header
 ******************************************************************************
 *
 ******************************************************************************
 */
 
 
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RS485_H__
#define __RS485_H__                         FW_BUILD // version


/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx.h"
/* Exported Type  ------------------------------------------------------------*/
/* Exported variables  -------------------------------------------------------*/
extern uint32_t rs_flags;
extern uint8_t  rs_ifa[2];
extern uint8_t  rs_gra[2];
extern uint8_t  rs_bra[2];
extern uint8_t  rs_bps;
/* Exported Define  ----------------------------------------------------------*/
/* Exported macros     -------------------------------------------------------*/
#define RS485_StartUpdate()					(rs_flags |= (0x1U << 0x0U))
#define RS485_StopUpdate()					(rs_flags &= (~ (0x1U << 0x0U)))
#define IsRS485_UpdateActiv()				((rs_flags & (0x1U << 0x0U)) != 0x0U)
#define RS485_ResponseSet()                 (rs_flags |= (0x1U << 0x1U))
#define RS485_ResponseReset()               (rs_flags &= (~ (0x1U << 0x1U)))
#define IsRS485_ResponseRdy()               ((rs_flags & (0x1U << 0x1U)) != 0x0U)
#define RFSenCfgSet()                       (rs_flags |= (0x1U << 0x2U))
#define RFSenCfgReset()                     (rs_flags &= (~ (0x1U << 0x2U)))
#define IsRFSenCfgActiv()                   ((rs_flags & (0x1U << 0x2U)) != 0x0U)
#define RFSenAddSet()                       (rs_flags |= (0x1U << 0x3U))
#define RFSenAddReset()                     (rs_flags &= (~ (0x1U << 0x3U)))
#define IsRFSenAddActiv()                   ((rs_flags & (0x1U << 0x3U)) != 0x0U)
#define RFSenNewSet()                       (rs_flags |= (0x1U << 0x4U))
#define RFSenNewReset()                     (rs_flags &= (~ (0x1U << 0x4U)))
#define IsRFSenNewActiv()                   ((rs_flags & (0x1U << 0x4U)) != 0x0U)
/* Exported functions ------------------------------------------------------- */
void RS485_Init(void);
void RS485_Service(void);
void RS485_RxCpltCallback(void);
void RS485_TxCpltCallback(void);
void RS485_ErrorCallback(void);
#endif
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
