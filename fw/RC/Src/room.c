/**
 ******************************************************************************
 * File Name          : signaling.c
 * Date               : 28/02/2016 23:16:19
 * Description        : audio visual signaling software modul
 ******************************************************************************
 */
 
#if (__ROOM_H__ != FW_BUILD)
    #error "room header version mismatch"
#endif 
/* Includes ------------------------------------------------------------------*/
#include "display.h"
#include "logger.h"
#include "eeprom.h"
#include "rc522.h"
#include "rs485.h"
#include "owire.h"
#include "room.h"
#include "main.h"
#include "dio.h"
/* Defines    ----------------------------------------------------------------*/
/* Types  --------------------------------------------------------------------*/
__IO ROOM_StatusTypeDef ROOM_Status = ROOM_IDLE;
__IO ROOM_StatusTypeDef ROOM_PreStatus = ROOM_IDLE;
/* Macros     ----------------------------------------------------------------*/
/* Variables  ----------------------------------------------------------------*/
__IO uint32_t room_fl;
__IO uint32_t unix_rtc;
__IO uint32_t unix_room;
int8_t  temp_mv;
uint8_t temp_sp;
uint8_t temp_dif;
uint8_t temp_cfg;
uint8_t bedd_cnt;
uint8_t bedd_tim;
/* Private prototypes    -----------------------------------------------------*/
static void TEMP_Controller(void);
/* Program code   ------------------------------------------------------------*/
void ROOM_Service(void)
{
    uint8_t buf[8], wrsta = 100;    // max 100 write trials
	static uint8_t _min = 60U;      // enable first entry
    static uint32_t init = 0;       // enable first entry
    if (eComState == COM_PACKET_RECEIVED) RS485_Service();
    /****************************************************/
    /*      C H E C K       R O O M     P O W E R       */
    /****************************************************/
    if (unix_rtc && unix_room)
    {   /* skeep room power timer check till valid rtc time */
        if ((unix_room <= unix_rtc) && IsPowerContactorActiv())
        {
            unix_room = 0;
            PowerContactorOff();
        }
    }
    /** ================================================*/
	/**     T E M P E R A T U R E   R E G U L A T O R   */
	/** ================================================*/
    if (IsTempLuxRTActive() 
    && (IsExtWinDoorSwClosed()  || !IsExtWinDoorSwEnabled())
    && (IsWinDoorSwClosed()     || !IsWinDoorSwEnabled())
    &&  IsPowerContactorActiv())    TempRegEnable();    // if all condition true, enable output controll
    else                            TempRegDisable();
    if (IsTempRegActiv())           TEMP_Controller();  // execute controll loop
    if (IsTempRegActiv()
    &&  IsTempRegEnabled() 
    &&  IsTempRegOutputActiv())     HVACOutputOn();     // energize cool/heat actuator
    else                            HVACOutputOff();    // turn off cool/heat actuator
    /****************************************************/
	/*	R O O M   S T A T U S   P R O C E S S I N G		*/
	/****************************************************/
	if (_min != rtime.Minutes)
	{
		_min = rtime.Minutes;
        /****************************************************/
        /*	        R O O M         S T A T U S  	        */
        /****************************************************/
		if(((ROOM_Status    == ROOM_BUSY)
        ||  (ROOM_Status    == ROOM_CLEANING_REQ)
        ||  (ROOM_Status    == ROOM_BEDDING_REQ))
        &&  (rtime.Hours    == ROOM_CLEANING_TIME) 
        &&  (rtime.Minutes  == 0)
        &&  IsPwrExpTimeActiv())
		{	/* compare both timers rounded to minute */
            if ((unix_room & 0xFFFFFFC0U) <= (unix_rtc & 0xFFFFFFC0U)) 
            {   /*  room general cleaning flag is set when room power timer expire */
                buf[0] = 0;
                EEPROM_Save(EE_BEDNG_CNT_ADD, buf, 1);
                ROOM_Status = ROOM_GENERAL_REQ;
                PwrExpTimeReset();  // this is only place to reset this flag
                buf[0] = unix_room >> 24;
                buf[1] = unix_room >> 16;
                buf[2] = unix_room >>  8;
                buf[3] = unix_room;
                buf[4] = 0x0U;
                buf[5] = 0x0U;  // room power expiry time set by user
                EEPROM_Save(EE_ROOM_PWRTOUT, buf, 0x6U);
            }
            else if ((unix_room & 0xFFFFFFC0U) > (unix_rtc & 0xFFFFFFC0U))
            {   /* room cleaning request flag is daily set at 8 AM */
                ROOM_Status = ROOM_CLEANING_REQ;
                if (bedd_tim)
                {   /*  check is bedding period set */
                    ++bedd_cnt;
                    if (bedd_cnt >= bedd_tim)
                    {   /* load, increase and check bedding counter */
                        bedd_cnt = 0;   // reset counter if overload
                        ROOM_Status = ROOM_BEDDING_REQ; // set flag
                    }
                    EEPROM_Save(EE_BEDNG_CNT_ADD, &bedd_cnt, 1); // write counter back to memory
                }
            }
		}		
	}
    /*  reset user DND CALL switch panel to default state  */
    if (IsRstRoomSignSwActiv()) 
    {   /* power cycle output with 0,5s off time */
        RstRoomSignSwReset();
        UserCapSwPanelOff();
        DOUT_Service();
        if (IsCallMaidActiv())
        {
            CallMaidReset();
            LEDGreenOff();
            LogEvent.log_event = HANDMAID_SWITCH_OFF;
            LOGGER_Write();
            StatUpdSet();
        }
        
        if (IsDonNotDisturbActiv())
        {
            DoNotDisturb_Off();
            DISP_DndImgDelete();
            LogEvent.log_event = DO_NOT_DISTURB_SWITCH_OFF;
            LOGGER_Write();
            StatUpdSet();
        }
        HAL_Delay(ROOM_STATUS_TOGGLE_TIME);
        UserCapSwPanelOn();
    }
    /****************************************************/
	/*		R O O M		S T A T U S		S I G N A L		*/
	/****************************************************/
	if ((ROOM_PreStatus != ROOM_Status) || !init)
	{
        if (!init) wrsta = 0;
        init = 1;
        
        switch (ROOM_Status)
        {
            case ROOM_IDLE:
            {
                ROOM_PreStatus = ROOM_Status;
                ZEROFILL(buf, sizeof(buf));
                EEPROM_Save(EE_BEDNG_CNT_ADD, buf, 2); // clear bedding replacement counter and period 
                EEPROM_Save(EE_ROOM_PWRTOUT,  buf, 6); // clear room power timer
                DISP_SosAlarmImgDel();
                DISP_FireAlarmImgDel();
                DISP_FireExitImgDel();
                DISP_DndImgDelete();
                DISP_CleanUpImgDel();
                DISP_GenClnImgDelete();
                DISP_BeddRepImgDelete();
                DISP_MinibarUsedImgDel();
                DISP_RoomNumImgSet();
                UserCapSwPanelOff();
                RoomCleaningReset();
                PowerContactorOff();
                unix_room = 0; // clear room power timer
                ROOM_Status = ROOM_READY;
                PwrExpTimeReset();
                return;
                break;
            }	
            
            case ROOM_READY:
            {
                ROOM_PreStatus = ROOM_Status;
                DISP_GenClnImgDelete();
                DISP_MinibarUsedImgDel();
                DISP_BeddRepImgDelete();
                DISP_RoomNumImgSet();
                DISP_CleanUpImgDel();
                DISP_DndImgDelete();
                RoomCleaningReset();
                RstRoomSignSwSet(); // reset DND/CALL room signal panel
                //unix_room = 0; // clear room power timer
                break;
            }
            
            case ROOM_BUSY:
            {
                ROOM_PreStatus = ROOM_Status;
                DISP_GenClnImgDelete();
                DISP_MinibarUsedImgDel();
                DISP_BeddRepImgDelete();
                DISP_RoomNumImgSet();
                DISP_CleanUpImgDel();
                RoomCleaningReset();
                UserCapSwPanelOn();
                break;
            }
            
            case ROOM_CLEANING_REQ:
            {
                ROOM_PreStatus = ROOM_Status;
                DISP_GenClnImgDelete();
                DISP_BeddRepImgDelete();
                DISP_CleanUpImage();
                RoomCleaningReset();
                UserCapSwPanelOn();
                break;
            }
            
            case ROOM_BEDDING_REQ:
            {
                ROOM_PreStatus = ROOM_Status;
                DISP_GenClnImgDelete();
                DISP_CleanUpImgDel();
                UserCapSwPanelOn();
                RoomCleaningReset();
                DISP_BeddRepImg();
                break;
            }
            
            case ROOM_GENERAL_REQ:
            {
                ROOM_PreStatus = ROOM_Status;
                DISP_GeneralCleanUpImg();
                DISP_BeddRepImgDelete();
                DISP_CleanUpImgDel();
                RoomCleaningReset();
                UserCapSwPanelOn();
                break;
            }
            
            case ROOM_CLEANING_RUN:    
            {
                UserCapSwPanelOff();
                RoomCleaningSet();
                wrsta = 0; // skipp writing new status
                break;
            }
            
            
            case ROOM_UNUSABLE:
            {
                ROOM_PreStatus = ROOM_Status;
                DISP_GenClnImgDelete();
                DISP_OutOfSrvcImgSet();
                DISP_MinibarUsedImgDel();
                DISP_BeddRepImgDelete();
                DISP_CleanUpImgDel();
                UserCapSwPanelOff();
                unix_room = 0; // clear room power timer
                break;
            }
            
            case ROOM_SOS_ALARM:
            {
                if (!IsDISP_SosAlarmImgActiv())
                {
                    DISP_SosAlarmImage();
                    BUZZ_State = BUZZ_SOS_ALARM;
                    LogEvent.log_event = SOS_ALARM_TRIGGER;
                    LOGGER_Write(); 
                }
                else wrsta = 0; // skipp writing new status
                break;
            }
            
            case ROOM_FIRE_ALARM:
            {
                if (!IsDISP_FireAlarmImgActiv())
                {
                    DISP_FireAlarmImage();
                    BUZZ_State = BUZZ_FIRE_ALARM;
                    LogEvent.log_event = FIRE_ALARM_TRIGGER;
                    LOGGER_Write();
                }
                else wrsta = 0; // skipp writing new status
                break;
            }
            
            case ROOM_FIRE_EXIT:
            {
                if (!IsDISP_FireExitImgActiv())
                {
                    DISP_FireExitImage();
                    BUZZ_State = BUZZ_FIRE_ALARM;
                    LogEvent.log_event = FIRE_EXIT_TRIGGER;
                    LOGGER_Write();
                }
                else wrsta = 0; // skipp writing new status
                break;
            }
            
            case ROOM_RESET_ALARM:
            {
                if (IsDISP_SosAlarmImgActiv())
                {
                    DISP_SosAlarmImgDel();
                    LogEvent.log_event = SOS_ALARM_RESET;
                    LOGGER_Write();
                }
                
                if (IsDISP_FireAlarmImgActiv())
                {
                    DISP_FireAlarmImgDel();
                    LogEvent.log_event = FIRE_ALARM_RESET;
                    LOGGER_Write();
                }
                
                if (IsDISP_FireExitImgActiv())
                {
                    DISP_FireExitImgDel();
                    LogEvent.log_event = FIRE_EXIT_RESET;
                    LOGGER_Write();
                }
                ROOM_Status = ROOM_PreStatus;
                BUZZ_State = BUZZ_OFF;
                init = 0; // get back
                wrsta = 0; // skipp writing new status
                break;
            }
        }
        
        while (wrsta)
        {
            --wrsta;
            buf[0] = ROOM_PreStatus;
            buf[1] = ROOM_Status;
            EEPROM_Save(EE_ROOM_PRESTAT_ADD, buf, 2);
            buf[2] = EE_ROOM_PRESTAT_ADD>>8;
            buf[3] = EE_ROOM_PRESTAT_ADD;
            HAL_I2C_Master_Transmit (&hi2c1, I2CEE_ADD, &buf[2], 2, DRV_TOUT);
            HAL_I2C_Master_Receive  (&hi2c1, I2CEE_ADD, &buf[4], 2, DRV_TOUT);
            if ((buf[0] == buf[4]) && (buf[1] == buf[5])) break;
            if (!wrsta)  ErrorHandler(EEPROM_FUNC, I2C_DRV);
        }
	}
}


static void TEMP_Controller(void)
{	
    /** ============================================================================*/
	/**    C A L C U L A T E   R O O M   T E M P E R A T U R E   R E G U L A T O R 	*/
	/** ============================================================================*/
    if      (IsTempRegHeating())
    {
        if     (temp_mv >= temp_sp)             TempRegOutputOff();
        else if(temp_mv < (temp_sp - temp_dif)) TempRegOutputOn();	
    }
    else if (IsTempRegCooling())
    {
        if     (temp_mv > (temp_sp + temp_dif)) TempRegOutputOn();
        else if(temp_mv < temp_sp)              TempRegOutputOff();
    }
}
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
