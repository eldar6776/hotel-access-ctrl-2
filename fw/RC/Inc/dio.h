/**
 ******************************************************************************
 * File Name          : dio_interface.h
 * Date               : 28/02/2016 23:16:19
 * Description        : digital in/out and capacitive sensor  modul header
 ******************************************************************************
 *
 *  LUX Room Controller			->  STM32F103C8 GPIO
 * ----------------------------------------------------------------------------
 *	DIN 0  Indor Card Reader	->	PB0
 *	DIN 1  SOS Alarm Switch		->	PB1
 *	DIN 2  SOS Reset Switch		->	PB2
 *	DIN 3  Call Handmaid Switch	->	PB3
 *	DIN 4  Minibar Sensor		->	PB4
 *	DIN 5  Balcony Door Sensor	->	PB5
 *	DIN 6  DND Switch			->	PB6
 *	DIN 7  Entry Door Sensor	->	PB7
 *
 *	DOUT 0 Power Contactor		->	PA4
 *	DOUT 1 DND Modul Power		->	PA5
 *	DOUT 2 Door Bell			->	PA7
 *
 ******************************************************************************
 */
 
 
 /* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DIO_INTERFACE_H__
#define __DIO_INTERFACE_H__					FW_BUILD	// version


/* Include  ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
/* Exporeted Types   ---------------------------------------------------------*/
/* Exporeted Define   --------------------------------------------------------*/
/* Exporeted Variable   ------------------------------------------------------*/
extern __IO uint32_t dout_0_7_rem;
extern __IO uint32_t din_cap_sen;
extern __IO uint32_t dio_flags;
extern __IO uint32_t din_state;
extern __IO uint32_t dout_0_7;
extern __IO uint32_t din_0_7;
extern uint8_t din_cfg[8];
extern uint8_t dout_cfg[8];
extern uint8_t hc595_dout;
extern uint8_t buzzer_volume;
extern uint8_t doorlock_force;
/* Exporeted  Macro  ---------------------------------------------------------*/
/***********************************************************************
**	 7 4 H C 5 9 5 	  S H I F T 	 R E G I S T E R 	C O N T R O L
***********************************************************************/
#define LEDGreenOn()        	        (hc595_dout |=  (1U<<0))
#define LEDGreenOff()       	        (hc595_dout &=(~(1U<<0)))
#define IsLEDGreenActiv()               (hc595_dout &   (1U<<0))
#define LEDRedOn()                      (hc595_dout |=  (1U<<1))
#define LEDRedOff()     	            (hc595_dout &=(~(1U<<1)))
#define IsLEDRedActiv()                 (hc595_dout &   (1U<<1))
#define LEDBlueOn()     		        (hc595_dout |=  (1U<<2))
#define LEDBlueOff()    		        (hc595_dout &=(~(1U<<2)))
#define IsLEDBlueActiv()                (hc595_dout &   (1U<<2))
#define LEDWhiteOn()                    (hc595_dout |=  (1U<<3))
#define LEDWhiteOff()                   (hc595_dout &=(~(1U<<3)))
#define IsLEDWhiteActiv()               (hc595_dout &   (1U<<3))
#define DOUT_3_SetHigh()				(hc595_dout |=  (1U<<4))
#define DOUT_3_SetLow()					(hc595_dout &=(~(1U<<4)))
#define Get_DOUT_3_State()				(hc595_dout &   (1U<<4))
#define DOUT_4_SetHigh()				(hc595_dout |=  (1U<<5))
#define DOUT_4_SetLow()					(hc595_dout &=(~(1U<<5)))
#define Get_DOUT_4_State()				(hc595_dout &   (1U<<5))
#define DISP_RST_SetHigh()			    (hc595_dout |=  (1U<<6))
#define DISP_RST_SetLow()			    (hc595_dout &=(~(1U<<6)))
#define RC522_RST_SetHigh()				(hc595_dout |=  (1U<<7))
#define RC522_RST_SetLow()				(hc595_dout &=(~(1U<<7)))
/***********************************************************************
**	 		A U X I L I A R Y	 			F L A G S 		
***********************************************************************/
#define DoNotDisturb_On()				(dio_flags |=  (1U<<0))
#define DoNotDisturb_Off()				(dio_flags &=(~(1U<<0)))
#define IsDonNotDisturbActiv()			(dio_flags &   (1U<<0))
#define RightButtonSet()			    (dio_flags |=  (1U<<1))
#define RightButtonReset()		        (dio_flags &=(~(1U<<1)))
#define IsRightButtonActiv()		    (dio_flags &   (1U<<1))
#define LeftButtonSet()			        (dio_flags |=  (1U<<2))
#define LeftButtonReset()		        (dio_flags &=(~(1U<<2)))
#define IsLeftButtonActiv()		        (dio_flags &   (1U<<2))
#define CallMaidSet()				    (dio_flags |=  (1U<<3))
#define CallMaidReset()				    (dio_flags &=(~(1U<<3)))
#define IsCallMaidActiv()			    (dio_flags &   (1U<<3))
#define DoorlockRemoteOpenSet()         (dio_flags |=  (1U<<4))
#define DoorlockRemoteOpenReset()       (dio_flags &=(~(1U<<4)))
#define IsDoorlockRemoteOpenActiv()     (dio_flags &   (1U<<4))
#define EntryDoorOpenSet()		        (dio_flags |=  (1U<<5))
#define EntryDoorOpenReset()            (dio_flags &=(~(1U<<5)))
#define IsEntryDoorOpen()               (dio_flags &   (1U<<5))
#define ExtWinDoorSwClosed()            (dio_flags |=  (1U<<6))
#define ExtWinDoorSwOpen()              (dio_flags &=(~(1U<<6)))
#define IsExtWinDoorSwClosed()          (dio_flags &   (1U<<6))
#define EntryDoorSwEnable()             (dio_flags |=  (1U<<7))   // eneable input on first time state change
#define IsEntryDoorSwEnabled()          (dio_flags &   (1U<<7))   // use flag to engage input
#define ExtWinDoorSwEnable()            (dio_flags |=  (1U<<8))   // eneable input on first time state change
#define IsExtWinDoorSwEnabled()         (dio_flags &   (1U<<8))   // use flag to engage input
#define WinDoorSwEnable()               (dio_flags |=  (1U<<9))   // eneable input on first time state change
#define IsWinDoorSwEnabled()            (dio_flags &   (1U<<9))   // use flag to engage input
#define BalconyDoorClosed()             (dio_flags |=  (1U<<10))
#define BalconyDoorOpen()               (dio_flags &=(~(1U<<10)))
#define IsBalconyDoorClosed()           (dio_flags &   (1U<<10))
/***********************************************************************
**	 D I G I T A L		 I N P U T		0 ~	7		S T A T E S		
***********************************************************************/
#define IsIndorCardReaderActiv()        (din_0_7 & (1U<<0))   /* pin 14 "ODLAGAC ULAZ" prema Edinovoj skici   */
#define IsSOSSwClosed()                 (din_0_7 & (1U<<1))
#define IsSOSResetSwClosed()            (din_0_7 & (1U<<2))
#define IsCallMaidSwClosed()            (din_0_7 & (1U<<3))
#define IsMinibarSwClosed()             (din_0_7 & (1U<<4))
#define IsWinDoorSwClosed()             (din_0_7 & (1U<<5))
#define IsDNDSwClosed() 	            (din_0_7 & (1U<<6))
#define IsEntryDoorSwClosed()           (din_0_7 & (1U<<7))   /* pin 7 "SENZOR VRATA" prema Edinovoj skici    */
/***********************************************************************
**	 C A P A C I T I V E		S E N S O R 		S T A T E S		
***********************************************************************/
#define IsHandmaidSwitchActiv()			(din_cap_sen &   (1U<<0))
#define IsDoorBellSwitchActiv()			(din_cap_sen &   (1U<<2))
#define CAP1293_SensorPresent()			(din_cap_sen |=  (1U<<7))
#define CAP1293_SensorNotPresent()		(din_cap_sen &=(~(1U<<7)))
#define IsCAP1293_Present()				(din_cap_sen &   (1U<<7))
/***********************************************************************
**	 D I G I T A L		O U T P U T		0 ~	7		C O N T R O L	
***********************************************************************/
#define PowerContactorOn()              (dout_0_7 |=  (1U<<0))    /* pin 30 "IZLAZ ODLAGACA" prema Edinovoj skici */
#define PowerContactorOff()             (dout_0_7 &=(~(1U<<0)))
#define IsPowerContactorActiv()			(dout_0_7 &   (1U<<0))
#define UserCapSwPanelOn()        	    (dout_0_7 |=  (1U<<1))
#define UserCapSwPanelOff()   		    (dout_0_7 &=(~(1U<<1)))
#define IsUserCapSwPanelActiv()		    (dout_0_7 &   (1U<<1))
#define WellcomeLight_On()              (dout_0_7 |=  (1U<<2))
#define WellcomeLight_Off()             (dout_0_7 &=(~(1U<<2)))
#define IsWellcomeLightActiv()          (dout_0_7 &   (1U<<2))
#define DoorBell_On()              		(dout_0_7 |=  (1U<<3))   
#define DoorBell_Off()             		(dout_0_7 &=(~(1U<<3)))
#define IsDoorBellActiv()              	(dout_0_7 &   (1U<<3))
#define HVACOutputOn()                  (dout_0_7 |=  (1U<<4))    
#define HVACOutputOff()                 (dout_0_7 &=(~(1U<<4)))
#define IsHVACOutputActiv()             (dout_0_7 &   (1U<<4))
#define HVAC_Thermostat_On()			(dout_0_7 |=  (1U<<5))
#define HVAC_Thermostat_Off()			(dout_0_7 &=(~(1U<<5)))
#define IsHVAC_ThermorstatActiv()		(dout_0_7 &   (1U<<5))
#define DoorLockCoil_On()               (dout_0_7 |=  (1U<<6))
#define DoorLockCoil_Off()              (dout_0_7 &=(~(1U<<6)))
#define IsDoorLockCoilActiv()			(dout_0_7 &   (1U<<6))
#define Buzzer_On()                     (dout_0_7 |=  (1U<<7))
#define Buzzer_Off()                    (dout_0_7 &=(~(1U<<7)))
#define IsBuzzerActiv()					(dout_0_7 &   (1U<<7))
/* Exported Function   -------------------------------------------------------*/
void DIO_Init(void);
void DIO_Service(void);
void DOUT_Service(void);
void DOUT_Doorlock(void);
void DOUT_Buzzer(void);
#endif
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
