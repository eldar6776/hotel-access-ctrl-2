/**
 ******************************************************************************
 * File Name          : rc522.h
 * Date               : 08/05/2016 23:15:16
 * Description        : mifare RC522 modul header
 ******************************************************************************
 *
 *
 ******************************************************************************
 */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RC522_H__
#define __RC522_H__					        FW_BUILD	// version
/* Include  ------------------------------------------------------------------*/
#include "stm32f1xx.h"
/* Exported Type  ------------------------------------------------------------*/
/* Exported Define  ----------------------------------------------------------*/
#define RC522_CARD_OK_TOUT			        3210U   // 3 s reader unlisten time after card read
//#define RC522_CARD_OK_TOUT                321U    // 0,3 s doorlock on time for sky pass reader
#define RC522_CARD_FAIL_TOUT		        987U    // ~1 s reader unlisten time after card read
/* card user groups predefine     -------------------------------------------*/
#define USERGRP_GUEST				        ('G')
#define USERGRP_MAID			            ('H')
#define USERGRP_MANAGER				        ('M')
#define USERGRP_SERVICER                    ('S')
#define USERGRP_PRESET				        ('P')
/* card data invalid date predefine    --------------------------------------*/
#define CARD_PENDING						0U	    // card data status type
#define CARD_VALID							0x06U	// card data status type
#define CARDID_INVALID                      ((uint8_t)0xF5U)
#define USERGRP_INVALID					    ((uint8_t)0xF6U)	
#define ROOMADDR_INVALID				    ((uint8_t)0xF7U)
#define SYSTEMID_INVALID					((uint8_t)0xF8U)
#define EXPIRYTIME_INVALID					((uint8_t)0xF9U)
#define CARDID_DATA_INVALID                 ((uint8_t)0xFAU)
#define USERGRP_DATA_INVALID				((uint8_t)0xFBU)
#define ROOMADDR_DATA_INVALID			    ((uint8_t)0xFCU)
#define SYSTEMID_DATA_INVALID				((uint8_t)0xFDU)
#define EXPIRYTIME_DATA_INVALID             ((uint8_t)0xFEU)
/* Exported types    ---------------------------------------------------------*/
typedef struct
{
	uint8_t card_status;
    uint8_t user_group;
    uint16_t system_id;
    uint16_t controller_id;
	uint8_t expiry_time[6];
	uint8_t card_id[5];
	
}RC522_CardDataTypeDef;

extern RC522_CardDataTypeDef sCard;
extern RC522_CardDataTypeDef sExtCard;
/* Exported variables  -------------------------------------------------------*/
extern uint32_t rc522_fl;
extern uint8_t system_id[];
extern uint8_t mifare_keya[];
extern uint8_t mifare_keyb[];
extern uint8_t rc522_rx_buff[];
extern uint8_t rc522_tx_buff[];
extern uint8_t permitted_add[8][2];
/* Exported macros     -------------------------------------------------------*/
#define RC522_HandmaidReentranceEnable()    (rc522_fl |=(0x1U << 0))
#define RC522_HandmaidReentranceDisable()   (rc522_fl &=(~(1U << 0)))
#define IsRC522_HandmaidReentranceActiv()   (rc522_fl & (0x1U << 0))
#define RC522_ExtendDoorlockTimeSet()       (rc522_fl |=(0x1U << 1))
#define RC522_ExtendDoorlockTimeReset()     (rc522_fl &=(~(1U << 1)))
#define IsRC522_ExtendDoorlockTimeActiv()   (rc522_fl & (0x1U << 1)) 
/* Exported functions  -------------------------------------------------------*/
void RC522_Init(void);
void RC522_Service(void);
void RC522_ClearData(void);
#endif
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
