/**
 ******************************************************************************
 * File Name          : one_wire.c
 * Date               : 17/11/2016 00:59:00
 * Description        : one wire communication modul header
 ******************************************************************************
 */
 
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __OW_H__
#define __OW_H__					FW_BUILD	// version


/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include "common.h"
/* Exporeted Types   ---------------------------------------------------------*/
#ifdef  OW_DS18B20  
typedef struct
{
	uint8_t	sensor_id;
	uint8_t rom_code[8];
	int temp_mv;
	
}TempSensorTypeDef;
#endif
/* Exporeted Define   --------------------------------------------------------*/
/* Exporeted Variable   ------------------------------------------------------*/
extern uint8_t ow_add[9];
extern uint8_t rt_img;
extern uint8_t ow_ifa;
extern uint8_t ow_gra;
extern uint8_t ow_bra;
extern uint8_t ow_bps;
extern uint8_t ow_dev;
/* Exporeted  Macro  ---------------------------------------------------------*/
/* Exported Function   -------------------------------------------------------*/
void OW_Init(void);
void OW_Service(void);
void OW_ScanBus(void);
uint8_t OW_RS485_Bridge(uint8_t *buff);
#endif
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/

