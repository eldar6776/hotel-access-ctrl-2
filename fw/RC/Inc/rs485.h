/**
 ******************************************************************************
 * File Name          : rs485.h
 * Date               : 28/02/2016 23:16:19
 * Description        : rs485 communication modul header
 ******************************************************************************
 *
 *	RS485 DEFAULT INTERFACE ADDRESS         0xffff
 *	RS485_DEFFAULT GROUP ADDRESS            0x6776
 * 	RS485_DEFFAULT BROADCAST ADDRESS        0x9999
 *	RS485 DEFAULT BAUDRATE                  115200
 *
 *
 ******************************************************************************
 */
 
 
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RS485_H__
#define __RS485_H__					FW_BUILD	// version


/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include "main.h"
/* Exported Type  ------------------------------------------------------------*/
typedef enum
{
	COM_INIT = 0U,
	COM_PACKET_PENDING,
	COM_PACKET_RECEIVED,
	COM_RECEIVE_SUSPEND,
	COM_ERROR
	
}eComStateTypeDef;

extern eComStateTypeDef eComState;
/* Exported variables  -------------------------------------------------------*/
extern __IO uint32_t packet_type;
extern __IO uint32_t receive_pcnt;
extern __IO uint32_t rs485_timer;
extern __IO uint32_t rs485_flags;
extern __IO uint32_t rs485_sender_addr;
extern __IO uint32_t rs485_pak_dlen;
extern __IO uint32_t rs485_pak_cksm;
extern __IO uint32_t rxtx_tmr;
extern uint8_t sys_stat;
extern uint8_t activ_cmd;
extern uint8_t rs485_bps;
extern uint8_t fw_upd_stat;
extern uint8_t rs485_buff[];
extern uint8_t rs485_ifa[2];
extern uint8_t rs485_gra[2];
extern uint8_t rs485_bra[2];
/* Exported macros     -------------------------------------------------------*/
#define SYS_NewLogSet()						(sys_stat |= (1U << 0))
#define SYS_NewLogReset()					(sys_stat &= (~ (1U << 0)))
#define IsSYS_NewLogSet()					((sys_stat & (1U << 0)) != 0U)

#define SYS_LogListFullSet()				(sys_stat |= (1U << 1))
#define SYS_LogListFullReset()				(sys_stat &= (~ (1U << 1)))
#define IsSYS_LogListFullSet()				((sys_stat & (1U << 1)) != 0U)

#define SYS_FileTransferSuccessSet()		(sys_stat |= (1U << 2))
#define SYS_FileTransferSuccessReset()		(sys_stat &= (~ (1U << 2)))
#define IsSYS_FileTransferSuccessSet()		((sys_stat & (1U << 2)) != 0U)

#define SYS_FileTransferFailSet()			(sys_stat |= (1U << 3))
#define SYS_FileTransferFailReset()			(sys_stat &= (~ (1U << 3)))
#define IsSYS_FileTransferFailSet()			((sys_stat & (1U << 3)) != 0U)

#define SYS_UpdateSuccessSet()				(sys_stat |= (1U << 4))
#define SYS_UpdateSuccessReset()			(sys_stat &= (~ (1U << 4)))
#define IsSYS_UpdateSuccessSet()			((sys_stat & (1U << 4)) != 0U)

#define SYS_UpdateFailSet()					(sys_stat |= (1U << 5))
#define SYS_UpdateFailReset()				(sys_stat &= (~ (1U << 5)))
#define IsSYS_UpdateFailSet()				((sys_stat & (1U << 5)) != 0U)

#define SYS_ImageUpdateRequestSet()			(sys_stat |= (1U << 6))
#define SYS_ImageUpdateRequestReset()		(sys_stat &= (~ (1U << 6)))
#define IsSYS_ImageUpdateRequestSet()		((sys_stat & (1U << 6)) != 0U)

#define SYS_FwrUpdRequestSet()		        (sys_stat |= (1U << 7))
#define SYS_FwrUpdRequestReset()	        (sys_stat &= (~ (1U << 7)))
#define IsSYS_FwrUpdRequestSet()	        ((sys_stat & (1U << 7)) != 0U)

#define RS485_StartTimer(TIME)				(rs485_timer = TIME)
#define RS485_StopTimer()					(rs485_timer = 0U)
#define IsRS485_TimerExpired()				(rs485_timer == 0U)

#define RS485_StartUpdate()					(rs485_flags |= (1U << 0))
#define RS485_StopUpdate()					(rs485_flags &= (~ (1U << 0)))
#define IsRS485_UpdateActiv()				((rs485_flags & (1U << 0)) != 0U)

#define RS485_ResponsePacketReady()			(rs485_flags |= (1U << 1))
#define RS485_NoResponse()					(rs485_flags &= (~ (1U << 1)))
#define IsRS485_ResponsePacketPending()		((rs485_flags & (1U << 1)) != 0U)
/* Exported functions ------------------------------------------------------- */
void RS485_Init(void);
void RS485_Service(void);
#endif
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
