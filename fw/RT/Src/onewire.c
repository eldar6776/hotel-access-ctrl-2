/**
 ******************************************************************************
 * File Name          : one_wire.c
 * Date               : 17/11/2016 00:59:00
 * Description        : one wire communication modul
 ******************************************************************************
 *
 *
 ******************************************************************************
 */

#if (__OW_H__ != FW_BUILD)
    #error "onewire header version mismatch"
#endif 
/* Includes ------------------------------------------------------------------*/
#include "png.h"
#include "main.h"
#include "rs485.h"
#include "logger.h"
#include "display.h"
#include "onewire.h"
#include "stm32746g.h"
#include "stm32746g_ts.h"
#include "stm32746g_qspi.h"
#include "stm32746g_sdram.h"
#include "stm32746g_eeprom.h"
/* Imported Type  ------------------------------------------------------------*/
/* Imported Variable  --------------------------------------------------------*/
/* Imported Function  --------------------------------------------------------*/
/* Private Type --------------------------------------------------------------*/
static RX_TypeDef   OW_State;
static LinkTypeDef  OW_Link;
#ifdef  OW_DS18B20   // define structure used for dallas onewire temp. sensor
typedef struct
{
	uint8_t	sensor_id;
	uint8_t rom_code[8];
	int16_t temperature;
	
}TempSensorTypeDef;

TempSensorTypeDef ds18b20_1;
#endif
/* Private Define ------------------------------------------------------------*/
/* Private Variable ----------------------------------------------------------*/
uint8_t ow_bps;
uint8_t ow_ifa;
uint8_t ow_gra;
uint8_t ow_bra;
uint16_t ow_bcnt;
uint32_t ow_flags;
uint32_t ow_rxtmr;
uint32_t ow_txtmr;
uint32_t ow_rxtout;
uint32_t ow_txtout;
uint32_t owtmr;
uint32_t owtout;
static uint8_t ow_rxbuf[OW_BSIZE];
static uint8_t ow_txbuf[OW_BSIZE];
#ifdef  OW_DS18B20 // define some variables for onewire dallas temp. sensor
static uint8_t ow_last_discrepancy;
static uint8_t ow_last_device_flag;
uint8_t ow_last_family_discrepancy;
uint8_t ow_sensor_number;
#endif
/* Constants ----------------------------------------------------------------*/              
/* Private Macro -------------------------------------------------------------*/
/* Private Function Prototype ------------------------------------------------*/
#ifdef  OW_DS18B20 // define functions to config and use dallas onewire temp. sensors
static uint8_t OW_Reset(void);
static uint8_t OW_ReadByte(void);
static void OW_SendByte(uint8_t data);
static void OW_SendBit(uint8_t send_bit);
static uint8_t OW_ReceiveBit(void);
static void OW_Send(uint8_t *command, uint8_t lenght);
static void OW_Receive(uint8_t *data, uint8_t lenght);
static uint8_t OW_ReadROM(uint8_t *ow_address);
static uint8_t OW_CrcCheck(uint8_t *ow_address, uint8_t lenght);
static void OW_Pack (uint8_t cmd, uint8_t buffer[8]);
static uint8_t OW_Unpack (uint8_t buffer[8]);
static uint16_t OW_ScratchpadToTemperature(uint16_t scratchpad);
static void OW_ResetSearch(void);
static uint8_t OW_Search(TempSensorTypeDef* ds18b20, uint8_t* sensor_cnt);
static void OW_Select(uint8_t* addr);
#endif
static void OW_SetUsart(uint8_t brate, uint8_t bsize);
/* Program Code  -------------------------------------------------------------*/
/**
  * @brief
  * @param
  * @retval
  */
void OW_Init(void)
{
    OW_SetUsart(BR_9600, WL_9BIT);
    __HAL_UART_FLUSH_DRREGISTER(&huart2);
    HAL_UART_Receive_IT(&huart2, ow_rxbuf, 1U);
    ow_rxtmr = HAL_GetTick();
    ow_rxtout = REC_TOUT;
    OW_State = RX_START;
    OW_Link = NOLINK;
    ow_bcnt = 0U;
}
/**
  * @brief
  * @param
  * @retval
  */
void OW_Service(void)
{
    FwInfoTypeDef RunFwInf;
    FwInfoTypeDef NewFwInf;
    FwInfoTypeDef BkpFwInf;
    uint8_t runfw = 0x0U;
    uint8_t newfw = 0x0U;
    uint8_t bkpfw = 0x0U;
    uint8_t updfw = 0x0U;
    uint32_t poscmd = 0x0U;
    uint8_t crc8 = 0x0U;
    static uint32_t pkt_next = 0x0U;
    static uint32_t pkt_total = 0x0U;
    static uint32_t pkt_crc = 0x0U;
    static uint32_t upd_sta = 0x0U;
    static uint32_t fil_crc = 0x0U;
    static uint32_t fil_bsize = 0x0U;
    static uint32_t fil_waddr = 0x0U;

    
    if      (OW_State != RX_LOCK) 
    {
        if((HAL_GetTick() - ow_rxtmr) >= ow_rxtout)  OW_State = RX_ERROR;
        /*  wait for 1 minute till abort started file update*/
        if(owtmr)
        {   /* during this time try to continue file download */
            if ((HAL_GetTick() - owtmr) >= owtout) 
            {
                owtmr = 0U;
                owtout = 0x0U;
                upd_sta = 0U;
                fil_crc = 0U;
                pkt_crc = 0U;
                pkt_next = 0U;
                pkt_total = 0U;
                fil_bsize = 0U;
                fil_waddr = 0U;
                StopUpdate();
                OW_Init();
            }
        }
    }
    
    
    if      (OW_State == RX_LOCK)
    {
        if(huart2.gState != HAL_UART_STATE_READY)
        {
            if((HAL_GetTick() - ow_txtmr) >= ow_txtout)  OW_State = RX_ERROR;
        }
        else  OW_Init();
    }
    else if (OW_State == RX_READY)
    {
        HAL_Delay(2);
        ZEROFILL(ow_txbuf, COUNTOF(ow_txbuf));
        ow_txbuf[3] = 0x2U;  // set two byte response data size 
        ow_txbuf[4] = NAK; // set default response to Negativ Acknowledge
        ow_txbuf[5] = ow_rxbuf[4]; // echo command as default
        /** ==========================================================================*/
        /**         O N E W I R E       P A C K E T         R E C E I V E D           */
        /** ==========================================================================*/
        if (IsUpdateActiv() && (ow_rxbuf[4] != GET_SYS_STAT) && (ow_rxbuf[4] != RT_SET_BTN_STA))
        {   // response to onewire controller get and set status request 
            if (pkt_next == (((ow_rxbuf[4] << 8) & 0xFF00U) | ow_rxbuf[5]))
            {
                owtout = (REC_TOUT * 10U);
                owtmr = HAL_GetTick(); // reload timout timer
                MX_QSPI_Init();
                QSPI_Write  (&ow_rxbuf[6], fil_waddr, ow_rxbuf[3] - 0x2U);
                fil_waddr += (ow_rxbuf[3] - 0x2U);
                MX_QSPI_Init();
                QSPI_MemMapMode();
                
                /*  file transfer completed, check file crc */
                if (pkt_next == pkt_total)
                {
                    pkt_crc = 0x0U;
                    CRC_ResetDR();  // reset crc register to default start value
                    pkt_crc = CRC_Calculate8 ((uint8_t*)RT_NEW_FILE_ADDR, fil_bsize); // calculate  crc for all file 8 bit bytes
                    if (fil_crc == pkt_crc)
                    {
                        poscmd = RESTART_CTRL;          // restart to bootloader do his work
                        ow_txbuf[4] = ACK;              // confirm file transfer
                        ow_txbuf[5] = FILE_RECEIVED;    // data check success
                    }
                    else ow_txbuf[5] = FILE_CRC_FAIL;   // error response
                    LogEvent.log_event = ow_txbuf[5];   // write log event for file transfer ressult
                    LOGGER_Write();
                    /* reset all variable used to track file transfer */
                    owtmr = 0x0U;
                    owtout = 0x0U;
                    upd_sta = 0x0U;
                    fil_crc = 0x0U;
                    pkt_crc = 0x0U;
                    pkt_next = 0x0U;
                    pkt_total = 0x0U;
                    fil_bsize = 0x0U;
                    fil_waddr = 0x0U;
                    StopUpdate();
                }
                else 
                {
                    ++pkt_next;                             // set next expected packet number
                    ow_txbuf[4] = ACK;                      // confirm previous packet
                    ow_txbuf[5] = pkt_next & 0xFFU;         // next expected packet lsb
                }
            }
            else
            {   /* keep connection activ if group or broadcast received packet is repeated */
                if ((OW_Link == GROUP) || (OW_Link == BROADCAST)) owtmr = HAL_GetTick();
                ow_txbuf[5] = pkt_next & 0xFFU; // if wrong packet number received, send next expected packet lsb
            }
        }
        else if (!IsSYSTEM_StartupActiv() && (!IsButtonRemoteActiv() || (ow_rxbuf[4] == GET_SYS_STAT)))
        {   // first send new state and then update set from remote controller 
            switch (ow_rxbuf[4])
            {
                /** ==========================================================================*/
                /**			S E T		N E W		B U T T O N 		S T A T E			  */
                /** ==========================================================================*/
                case RT_SET_BTN_STA:
                case RT_RST_DRBL:
                {
                    // ROOM CLEANING DISABLE TOUCH EVENTS
                    if        (ow_rxbuf[5] & (1U<<0))           DISP_CleaningSet();
                    else                                        DISP_CleaningReset();
                    // BUTTON DND STATE
                    if      (((ow_rxbuf[5] & (1U<<1)) != 0U) && !IsButtonDndActiv())
                    {
                        ButtonDndSet();
                        ButtonCallReset();
                        ButtonUpdateSet();
                        SetpointUpdateSet(); // save new button state
                    }
                    else if (((ow_rxbuf[5] & (1U<<1)) == 0U) && IsButtonDndActiv())
                    {
                        ButtonDndReset();
                        ButtonUpdateSet();
                        SetpointUpdateSet(); // save new button state
                    }
                    // BUTTON SOS STATE
                    if      (((ow_rxbuf[5] & (1U<<2)) != 0U) && !IsButtonSosActiv())
                    {
                        ButtonSosSet();
                        ButtonUpdateSet();
                        SetpointUpdateSet(); // save new button state
                    }
                    else if (((ow_rxbuf[5] & (1U<<2)) == 0U) && IsButtonSosActiv())
                    {
                        ButtonSosReset();
                        ButtonUpdateSet();
                        SetpointUpdateSet(); // save new button state
                    }
                    // BUTTON_CALMAID_STATE
                    if      (((ow_rxbuf[5] & (1U<<3)) != 0U) && !IsButtonCallActiv()) 
                    {
                        ButtonDndReset();
                        ButtonCallSet();
                        ButtonUpdateSet();
                        SetpointUpdateSet(); // save new button state
                    }
                    else if (((ow_rxbuf[5] & (1U<<3)) == 0U) && IsButtonCallActiv())
                    {
                        ButtonCallReset();
                        ButtonUpdateSet();
                        SetpointUpdateSet(); // save new button state
                    }
                    // ROOM CONTROLLER HVAC CONTACTOR STATUS
                    if      ((ow_rxbuf[5] & (1U<<4)) == 0U)    TempRegDisable();
                    else if ((ow_rxbuf[5] & (1U<<4)) != 0U)    TempRegEnable();
                    // ROOM CONTROLLER BALCONY AND WINDOWS SWITCH STATUS
                    if      ((ow_rxbuf[5] & (1U<<5)) == 0U)    ExtSwRemoteOpen();
                    else if ((ow_rxbuf[5] & (1U<<5)) != 0U)    ExtSwRemoteClosed();
                    // ROOM CONTROLER POWER CONTACTOR STATUS
                    if      ((ow_rxbuf[5] & (1U<<6)) == 0U)    PowerContactorOff();
                    else if ((ow_rxbuf[5] & (1U<<6)) != 0U)    PowerContactorOn();
                    // ROOM CONTROLLER DOOR BELL TASTER STATUS
                    if      (((ow_rxbuf[5]& (1U<<7)) == 0U) && (IsDoorBellOn())) DoorBellOff();
                    else if (((ow_rxbuf[5]& (1U<<7)) != 0U) && (!IsDoorBellOn()))
                    {
                        DoorBellOn();
                        disp_img_id    = 1U;
                        disp_img_time  = 1U;
                        buzz_sig_time  = 2U;
                        buzz_sig_id    = 2U;
                        DISP_UpdateSet();
                    }
                    // BREAKE DOORBELL DISPLAY SCREEN MESSAGE BY DOOR OPEN EVENT
                    if (ow_rxbuf[4] == RT_RST_DRBL)
                    {
                        if ((disp_img_id  % 10U) == 1U)
                        {
                            if  (last_img_id == 10U) DISP_FaultSet();
                            else DISP_GuiSet();
                        }
                    }
                    ow_txbuf[4] = ACK; // SET RESPONSE
                    break;
                }
                /** ==========================================================================*/
                /**			S E T		N E W		D A T E 	& 		T I M E				  */
                /** ==========================================================================*/
                case SET_RTC_DATE_TIME:
                {    
                    if (!IsRtcValidActiv() || (rtctm.Minutes != ow_rxbuf[10]))
                    {
                        rtcdt.WeekDay    = ow_rxbuf[5];
                        if (rtcdt.WeekDay == 0x0U) rtcdt.WeekDay = 0x7U;
                        rtcdt.Date       = ow_rxbuf[6];
                        rtcdt.Month      = ow_rxbuf[7];
                        rtcdt.Year       = ow_rxbuf[8];
                        rtctm.Hours      = ow_rxbuf[9];
                        rtctm.Minutes    = ow_rxbuf[10];
                        rtctm.Seconds    = ow_rxbuf[11];
                        HAL_RTC_SetTime (&hrtc, &rtctm, RTC_FORMAT_BCD);
                        HAL_RTC_SetDate (&hrtc, &rtcdt, RTC_FORMAT_BCD);
                        RtcValidSet();
                    }
                    ow_txbuf[4] = ACK;  // SET RESPONSE
                    break;
                }
                /** ==========================================================================*/
                /**			S E T		T H E R M O S T A T 	P A R A M E T E R S			  */
                /** ==========================================================================*/                
                case SET_ROOM_TEMP:
                {
                    thst_sp  = ow_rxbuf[5];
                    thst_dif = ow_rxbuf[6];
                    thst_cfg = ow_rxbuf[7];
                    if      (thst_sp > thst_max_sp) thst_sp = thst_max_sp;
                    else if (thst_sp < thst_min_sp) thst_sp = thst_min_sp;
                    SetpointUpdateSet();
                    ow_txbuf[4] = ACK; // SET RESPONSE
                    break;
                }
                /** ==========================================================================*/
                /**		    S E T		N E W		D I S P L A Y    M E S S A G E            */
                /** ==========================================================================*/
                case RT_SET_DISP_STA:             
                {
                    if (ow_ifa == ow_rxbuf[5]) // room thermostat id is onewire interface address
                    {
                        if (disp_img_id != ow_rxbuf[6]) // display image number
                        {
                            disp_img_id     = ow_rxbuf[6];
                            disp_img_time   = ow_rxbuf[7]; // display image timeout
                            buzz_sig_id     = ow_rxbuf[8]; // buzzer signal mode
                            buzz_sig_time   = ow_rxbuf[9]; // buzzer signal timer
                            DISP_UpdateSet(); 
                            BtnOkRelease();
                            ow_txbuf[4] = ACK; // SET POSITIVE RESPONSE
                        }
                    }
                    break;
                }
                /** ==========================================================================*/
                /**         S E T   N E W   W E A T H E R   F O R E C A S T   I N F O         */
                /** ==========================================================================*/
                case RT_UPD_WFC:
                {
                    if (ow_rxbuf[3] == WFC_DSIZE + 0x1U)
                    {
                        memcpy(wfc_buff, &ow_rxbuf[5], WFC_DSIZE);
                        WFC_UpdateSet();
                        ow_txbuf[4] = ACK; // SET POSITIVE RESPONSE
                    }
                    break;
                }
                /** ==========================================================================*/
                /** http://web.infoimediadownloadMobilePoint.apkido=43707&r=101&gu=65483865jhgfzut7fjhgjd!&n=0  */
                /** ==========================================================================*/
                case RT_UPD_QRC:
                {
                    if (ow_rxbuf[3] < QRC_DSIZE) 
                    {
                        upd_sta = 0x0U;
                        ow_rxbuf[ow_rxbuf[3] + 0x4U] = 0x0U;
                        uint32_t b2wr =  (ow_rxbuf[3] + 0x1U);
                        uint32_t wraddr = EE_QR_CODE;
                        uint8_t *pbuf = &ow_rxbuf[5];
                        
                        while(b2wr)
                        {
                            if (b2wr > EE_PGSIZE) 
                            {
                                upd_sta += EE_WriteData(EE_ADDR, wraddr, pbuf, EE_PGSIZE);
                                b2wr -= EE_PGSIZE;
                                wraddr += EE_PGSIZE;
                                pbuf += EE_PGSIZE;
                            }
                            else 
                            {
                                upd_sta += EE_WriteData(EE_ADDR, wraddr, pbuf, b2wr);
                                b2wr = 0x0U;
                            }
                            upd_sta += EE_WaitStbySta();
                        }
                        if (upd_sta == 0x0U) ow_txbuf[4] = ACK; // SET POSITIVE RESPONSE
                    }
                    break;
                }
                /** ==========================================================================*/
                /**                                                                           */
                /** ==========================================================================*/
                case RT_DISP_QRC:
                {
                    DISP_QR_Code(); // set response from display function
                    ow_txbuf[4] = ACK; // SET POSITIVE RESPONSE
                    break;
                }
                /** ==========================================================================*/
                /**                                                                           */
                /** ==========================================================================*/
                case RT_DISP_MSG:
                {
                    DISP_Message(&ow_rxbuf[5]);  // set response from display function
                    ow_txbuf[4] = ACK;              // SET POSITIVE RESPONSE
                    break;
                }
                /** ==========================================================================*/
                /**         D O W N L O A D     F I L E     T O     Q S P I     F L A S H     */
                /** ==========================================================================*/
                case RT_DWNLD_LOGO: // download new user logo.png image 
                case RT_DWNLD_BLDR: // download new bootloader firmware version    
                case RT_DWNLD_FWR:  // download new application firmware version
                {
                    upd_sta   =   ow_rxbuf[4];
                    pkt_total = ((ow_rxbuf[5] << 8)| ow_rxbuf[6]);
                    fil_bsize = ((ow_rxbuf[7] <<24)|(ow_rxbuf[8] <<16)|(ow_rxbuf[9] <<8)|ow_rxbuf[10]);
                    fil_crc   = ((ow_rxbuf[11]<<24)|(ow_rxbuf[12]<<16)|(ow_rxbuf[13]<<8)|ow_rxbuf[14]);
                    fil_waddr = RT_NEW_FILE_ADDR;
                    pkt_next  = 0x1U;
                    MX_QSPI_Init(); // reinit qspi interface to execute sector erase command
                    if (QSPI_Erase(fil_waddr, fil_waddr + fil_bsize) == QSPI_OK) 
                    {
                        StartUpdate();
                        ow_txbuf[4] = ACK;      // acknowledge command executed
                        ow_txbuf[5] = 0x1U;     // next expected packet lsb only
                        owtmr = HAL_GetTick();  // start timeout timer
                        owtout = (REC_TOUT * 10U);
                    }
                    else 
                    {
                        StopUpdate();
                        OW_State = RX_ERROR;
                        if      (upd_sta == RT_DWNLD_LOGO)  LogEvent.log_event = IMG_UPD_FAIL;
                        else if (upd_sta == RT_DWNLD_BLDR)  LogEvent.log_event = BLDR_UPD_FAIL;
                        else if (upd_sta == RT_DWNLD_FWR)   LogEvent.log_event = FW_UPD_FAIL;
                        LOGGER_Write();
                        ow_txbuf[3] = 0x2U;
                        ow_txbuf[4] = NAK; 
                        ow_txbuf[5] = LogEvent.log_event;
                        owtmr = 0x0U;
                        owtout = 0x0U;
                        upd_sta = 0x0U;
                        fil_crc = 0x0U;
                        pkt_crc = 0x0U;
                        pkt_next = 0x0U;
                        pkt_total = 0x0U;
                        fil_bsize = 0x0U;
                        fil_waddr = 0x0U;
                    }
                    MX_QSPI_Init();         // reinit interface again to 
                    QSPI_MemMapMode();      // reinit qspi interface to execute sector erase command
                    break;
                }
                /** ==========================================================================*/
                /**                      S O F T W A R E     R E S T A R T                    */
                /** ==========================================================================*/
                /**     S T A R T   B O O T L O A D E R     T O     W R I T E   U P D A T E   */
                /** ==========================================================================*/
                case RESTART_CTRL:
                case START_BLDR:
                {
                    ow_txbuf[4] = ACK;      // SET POSITIVE RESPONSE
                    poscmd = RESTART_CTRL;  // software reset after response
                    break;
                }
                /** ==========================================================================*/
                /**			    G E T	        B U T T O N 		S T A T E                 */
                /** ==========================================================================*/
                case GET_SYS_STAT:
                {
                    ZEROFILL(ow_txbuf, COUNTOF(ow_txbuf));                                        
                                                ow_txbuf[3] = 0x4U;
                    if (IsExtSwClosed())        ow_txbuf[4] |= (0x1U << 0x0U);
                    if (IsButtonDndActiv())     ow_txbuf[4] |= (0x1U << 0x1U);
                    if (IsButtonSosActiv())     ow_txbuf[4] |= (0x1U << 0x2U);
                    if (IsButtonCallActiv())    ow_txbuf[4] |= (0x1U << 0x3U);
                    if (IsButtonOpenActiv())    ow_txbuf[4] |= (0x1U << 0x4U), ButtonOpenReset();
                    if (IsButtonOkActiv())      ow_txbuf[4] |= (0x1U << 0x5U), ButtonOkReset();
                    if (IsNtcErrorActiv())      ow_txbuf[4] |= (0x1U << 0x6U), NtcErrorReset();
                    if (IsButtonRemoteActiv())  ow_txbuf[4] |= (0x1U << 0x7U), ButtonRemoteReset();
                                                ow_txbuf[5]  = thst_sp;
                                                ow_txbuf[6]  = (room_temp / 10);
                                                ow_txbuf[7]  = disp_img_id;
                    break;
                }
                /** ==========================================================================*/
                /**         G E T	       A P P L I C A T I O N  		S T A T E             */
                /** ==========================================================================*/
                case GET_APPL_STAT:
                {
                    ZEROFILL(ow_txbuf, COUNTOF(ow_txbuf));
                    mem_set(&ow_txbuf[6], 'X', 52);// sensor error
                    if (IsNtcValidActiv()) 	
                    {
                        if (room_temp < 0) 
                        {
                                                    ow_txbuf[7] = '-';
                            Int2Str(        (char*)&ow_txbuf[8], -1*room_temp, 3);
                        }
                        else 
                        {
                                                    ow_txbuf[7] = '+';
                            Int2Str(        (char*)&ow_txbuf[8],  room_temp, 3);
                        }
                        if  (IsTempRegEnabled())    ow_txbuf[11] = 'E';
                        else                        ow_txbuf[11] = 'D';
                                                    ow_txbuf[12] = 'H';
                        Int2Str (           (char*)&ow_txbuf[13], thst_sp,  2);
                        Int2Str (           (char*)&ow_txbuf[15], thst_dif, 2);
                    }
                    mem_set(&ow_txbuf[17], '0', 16);
                    if (IsExtSwClosed())        ow_txbuf[17] = '1';
                    if (IsButtonDndActiv())     ow_txbuf[18] = '1';
                    if (IsButtonSosActiv())     ow_txbuf[19] = '1';
                    if (IsButtonCallActiv())    ow_txbuf[20] = '1';
                    if (IsButtonOpenActiv())    ow_txbuf[21] = '1';
                    if (IsButtonOkActiv())      ow_txbuf[22] = '1';
                    if (IsNtcErrorActiv())      ow_txbuf[23] = '1';
                    if (IsButtonRemoteActiv())  ow_txbuf[24] = '1';
                    if (IsWFC_ValidActiv())     ow_txbuf[25] = '1';
                    if (IsRtcValidActiv())      ow_txbuf[26] = '1';
                    if (IsDISP_CleaningActiv()) ow_txbuf[27] = '1';
                    if (IsBtnOkPressed())       ow_txbuf[28] = '1';
                    ow_txbuf[33] = 'R';
                    ow_txbuf[34] = 'T';
                    ow_txbuf[41] = version>>16;
                    ow_txbuf[42] = version>>8;
                    ow_txbuf[43] = version;
                    Hex2Str((char*)&ow_txbuf[35], &ow_txbuf[41], 6);
                    ow_txbuf[41] = TOCHAR(ow_ifa);
                    Int2Str((char*)&ow_txbuf[42], disp_img_id,  2);
                    Int2Str((char*)&ow_txbuf[44], disp_img_time,2);
                    Int2Str((char*)&ow_txbuf[46], buzz_sig_id,  2);
                    Int2Str((char*)&ow_txbuf[48], buzz_sig_time,2);
                    ow_txbuf[50] = '\0';
                    ow_txbuf[3] = 46;
                    ow_txbuf[4] = ACK;
                    break;						
                }
                /** ==========================================================================*/
                /**			    G E T	     L O G          F R O M         L I S T           */
                /** ==========================================================================*/
                case GET_LOG_LIST:
                {
                    ZEROFILL(ow_txbuf, COUNTOF(ow_txbuf));
                    ow_txbuf[4] = ACK;
                    if (LOGGER_Read(&ow_txbuf[5]) == LOGGER_OK) ow_txbuf[3] = LOG_DSIZE + 0x1U;
                    break;
                }
                /** ==========================================================================*/
                /**     D E L E T E 	     L O G          F R O M         L I S T           */
                /** ==========================================================================*/
                case DEL_LOG_LIST:
                {
                    LOGGER_Delete();
                    ow_txbuf[4] = ACK;
                    break;
                }
            }             
        }
        /**
        ***********************************************************************************
        *  send response to direct request send to this interface address and to request
        *  for bus controller to resolve addresse if this device is bus master controller 
        ***********************************************************************************
        */
        if(OW_Link == P2P)
        {
            ow_txbuf[0] = STX;
            ow_txbuf[1] = ow_rxbuf[2];
            ow_txbuf[2] = ow_rxbuf[1];
            __HAL_CRC_DR_RESET(&hcrc);
            ow_txbuf[ow_txbuf[3]+4] = (uint8_t) CRC_Calculate8(ow_txbuf, ow_txbuf[3]+4);
            HAL_UART_Transmit(&huart2, ow_txbuf, ow_txbuf[3]+5, 100);
        }
        OW_State = RX_LOCK;
        ow_txtout = OW_PKTIME;
        ow_txtmr = HAL_GetTick();        
        /**     
        ***********************************************************************************
        *  in order to response to every request, some slow processing will be done after  
        *  usart transmission, like restart or file copy witch may fall and leed to restart
        ***********************************************************************************
        */
        switch (poscmd)
        {
            case RESTART_CTRL:
                Restart(); // call software restart
                break;
            case RT_DWNLD_LOGO:
                if (QSPI2QSPI_Copy (RT_NEW_FILE_ADDR, RT_LOGO_ADDR, fil_bsize) == QSPI_OK) LogEvent.log_event = IMG_UPDATED;
                else LogEvent.log_event = IMG_UPD_FAIL;
                LOGGER_Write();
                break;
            case RT_DWNLD_BLDR:
                ResetFwInfo(&RunFwInf);
                ResetFwInfo(&NewFwInf);
                RunFwInf.ld_addr = RT_BLDR_ADDR;
                NewFwInf.ld_addr = RT_NEW_FILE_ADDR;
                runfw = GetFwInfo (&RunFwInf);  // running bootloader version info
                newfw = GetFwInfo (&NewFwInf);  // new bootloader version info
                updfw = IsNewFwUpdate(&RunFwInf, &NewFwInf); // check is new file update
                if (!updfw) // all version info ok and new file is update
                {   // if fail to backup bootloader, write error log and abort update 
                    if (FLASH2QSPI_Copy (RunFwInf.ld_addr, RT_BLDR_BKP_ADDR, RunFwInf.size) != QSPI_OK) 
                    {   // if fail to backup firmware, write error log and abort update 
                        LogEvent.log_event = FILE_BACKUP_FAIL; // inform system controller 
                        LOGGER_Write(); // write log to log list
                        MX_QSPI_Init(); // reinit interface again to 
                        QSPI_MemMapMode(); // reinit qspi interface to execute sector erase command
                    }   // if backup copy succseed, write new bootloader
                    else if (QSPI2FLASH_Copy (NewFwInf.ld_addr, NewFwInf.wr_addr, NewFwInf.size) != QSPI_OK)
                    {
                        LogEvent.log_event = FILE_BACKUP_FAIL; // fail to backup bootloader fw
                        LOGGER_Write(); // write log to inform system controller and abort update 
                        MX_QSPI_Init(); // reinit interface again to 
                        QSPI_MemMapMode(); // reinit qspi interface to execute sector erase command 
                    }
                }
                /**
                *********************************************************************
                *   after previous function  erase and copy  check again 
                *   bootloader for error and try recovery from backup if eny
                *********************************************************************
                */
                ResetFwInfo(&RunFwInf);
                ResetFwInfo(&BkpFwInf);
                RunFwInf.ld_addr = RT_BLDR_ADDR;
                BkpFwInf.ld_addr = RT_BLDR_BKP_ADDR;
                runfw = GetFwInfo (&RunFwInf);
                bkpfw = GetFwInfo (&BkpFwInf);
                if(runfw) // if running bootloader not valid
                {   
                    if (!bkpfw) // fail again, this is critical error, inform system controller and do not restart or power off 
                    {   // before bootloader is recovered, it will fail to boot, leaving icsp programming as only option
                        if (QSPI2FLASH_Copy (BkpFwInf.ld_addr, BkpFwInf.wr_addr, BkpFwInf.size) != QSPI_OK)
                        {   // if fail to backup firmware, write error log and abort update 
                            LogEvent.log_event = FILE_BACKUP_FAIL; // inform system controller
                            MX_QSPI_Init(); // reinit interface again to 
                            QSPI_MemMapMode(); // reinit qspi interface to execute sector erase command
                        }
                        else LogEvent.log_event = BKP_RECOVERED; // if backup bootloader writen succesfully
                    }
                    else LogEvent.log_event = BLDR_UPD_FAIL; // bootloader backup  file info error 
                } 
                else LogEvent.log_event = BLDR_UPDATED; // bootloader backup  file info error  
                LOGGER_Write();     // write log event
                break;                
        }
    }
    else if (OW_State == RX_ERROR)
    {
        OW_Init();
    }
    else if (OW_State == RX_INIT)
    {
        OW_Init();
    }
}
/**
  * @brief
  * @param
  * @retval
  */
void OW_TxCpltCallback(void)
{

}
/**
  * @brief
  * @param
  * @retval
  */
void OW_RxCpltCallback(void)
{
    uint8_t crc8 = 0x0U;
    
    switch(OW_State)
    {
        case RX_START:
            OW_SetUsart(BR_9600, WL_8BIT);
            if(ow_rxbuf[ow_bcnt] == STX) OW_State = RX_RECADDR;
            else  OW_State = RX_INIT;
            break;
        case RX_RECADDR:
            if      (ow_rxbuf[ow_bcnt] == ow_ifa)  OW_Link = P2P;
            else if (ow_rxbuf[ow_bcnt] == ow_gra)  OW_Link = GROUP;
            else if (ow_rxbuf[ow_bcnt] == ow_bra)  OW_Link = BROADCAST;
            if      (OW_Link == NOLINK) OW_State = RX_INIT;  
            else OW_State = RX_SNDADDR;
            break;
        case RX_SNDADDR:
            OW_State = RX_SIZE;
            break;
        case RX_SIZE:
            OW_State = RX_PAYLOAD;
            break;
        case RX_PAYLOAD:
            if(ow_rxbuf[3] == (ow_bcnt - 0x3U)) OW_State = RX_CRC8;
            break;
        case RX_CRC8:
            __HAL_CRC_DR_RESET(&hcrc);
            crc8 = CRC_Calculate8(ow_rxbuf, ow_bcnt);
            if(crc8 == ow_rxbuf[ow_bcnt]) OW_State = RX_READY;
            else OW_State = RX_INIT;
        case RX_READY:
        case RX_LOCK:
        case RX_ERROR:
        case RX_INIT:
            break;
    }
    
    if (OW_State <= RX_READY) return;
    ++ow_bcnt;
    ow_rxtout = RX_TOUT;
    ow_rxtmr = HAL_GetTick();
    HAL_UART_Receive_IT(&huart2, &ow_rxbuf[ow_bcnt], 0x1U);
}
/**
  * @brief
  * @param
  * @retval
  */
void OW_ErrorCallback(void)
{
    __HAL_UART_CLEAR_PEFLAG (&huart2);
    __HAL_UART_CLEAR_FEFLAG (&huart2);
    __HAL_UART_CLEAR_NEFLAG (&huart2);
    __HAL_UART_CLEAR_IDLEFLAG(&huart2);
    __HAL_UART_CLEAR_OREFLAG(&huart2);
    __HAL_UART_FLUSH_DRREGISTER(&huart2);
    huart2.ErrorCode = HAL_UART_ERROR_NONE;
    OW_State = RX_ERROR;
}
/**
  * @brief
  * @param
  * @retval
  */
static void OW_SetUsart(uint8_t brate, uint8_t bsize)
{
    HAL_NVIC_DisableIRQ(USART2_IRQn);
    HAL_UART_DeInit(&huart2);
    
	huart2.Instance        		= USART2;
    huart2.Init.BaudRate        = bps[brate];
    if      (bsize == WL_9BIT) huart2.Init.WordLength = UART_WORDLENGTH_9B;
	else if (bsize == WL_8BIT) huart2.Init.WordLength = UART_WORDLENGTH_8B;
    
	huart2.Init.StopBits   		= UART_STOPBITS_1;
	huart2.Init.Parity     		= UART_PARITY_NONE;
	huart2.Init.HwFlowCtl  		= UART_HWCONTROL_NONE;
	huart2.Init.Mode       		= UART_MODE_TX_RX;
	huart2.Init.OverSampling	= UART_OVERSAMPLING_16;
    huart2.Init.OneBitSampling  = UART_ONE_BIT_SAMPLE_DISABLE;
	huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    
    if(bsize == WL_9BIT)
    {
        if (HAL_MultiProcessor_Init(&huart2, STX, UART_WAKEUPMETHOD_ADDRESSMARK) != HAL_OK) ErrorHandler(OW_FUNC, USART_DRV);
        HAL_MultiProcessor_EnableMuteMode(&huart2);
        HAL_MultiProcessor_EnterMuteMode(&huart2);        
    }
    else if(bsize == WL_8BIT)
    {
        if (HAL_UART_Init(&huart2) != HAL_OK) ErrorHandler(MAIN_FUNC, USART_DRV);
    }
    
    HAL_NVIC_SetPriority(USART2_IRQn, 3, 0);
	HAL_NVIC_EnableIRQ(USART2_IRQn);
}
/**
  * @brief
  * @param
  * @retval
  */
#ifdef  OW_DS18B20 
static uint8_t OW_Reset(void)
{
	OW_SetUsart(BR_9600, WL_8BIT);
	ow_rxbuf[0] = 0xf0U;
	HAL_UART_Transmit(&huart2, ow_rxbuf, 1, OW_TOUT);
	HAL_UART_Receive(&huart2, ow_rxbuf, 1, OW_TOUT);
	OW_SetUsart(BR_115200, WL_8BIT);
	if((ow_rxbuf[0] != 0xf0U) && (ow_rxbuf[0] != 0x00U) && (ow_rxbuf[0] != 0xffU)) return (1U);
	else return(0U);	
}
/**
  * @brief
  * @param
  * @retval
  */
static void OW_SendByte(uint8_t data)
{
	uint32_t i;
	
	for(i = 0U; i < 8U; i++, (data = data >> 1U))
	{
		OW_SendBit(data & (1U << 0));
	}
}
/**
  * @brief
  * @param
  * @retval
  */
static uint8_t OW_ReadByte(void)
{
	uint8_t rd_byte, i;
	
	for(i = 0U; i < 8U; i++)
	{
		rd_byte = (rd_byte >> 1U) + 0x80U * OW_ReceiveBit();
	}
	
	return rd_byte;
}
/**
  * @brief
  * @param
  * @retval
  */
static uint8_t OW_ReceiveBit(void)
{
	uint8_t txd, rxd;
	txd = 0xffU;	
	HAL_UART_Transmit(&huart2, &txd, 1U, OW_TOUT);	
	HAL_UART_Receive(&huart2, &rxd, 1U, OW_TOUT);	
	if(rxd == txd) return(1U);
	else return(0U);
}
/**
  * @brief
  * @param
  * @retval
  */
static void OW_SendBit(uint8_t send_bit)
{
	uint8_t txb, rxb;	
	if(send_bit == 0U)  txb = 0x00U;
	else txb = 0xffU;	
	HAL_UART_Transmit(&huart2, &txb, 1U, OW_TOUT);
	HAL_UART_Receive(&huart2, &rxb, 1U, OW_TOUT);
}
/**
  * @brief
  * @param
  * @retval
  */
static void OW_Send(uint8_t *command, uint8_t lenght)
{
	uint32_t i;
	
	uint32_t one_wire_lenght = lenght * 8U;
	
	for (i = 0U;  i < lenght; i++) 
	{
		OW_Pack(command[i], &(ow_rxbuf[i * 8U]));
	}
	
	HAL_UART_Transmit(&huart2, ow_rxbuf, one_wire_lenght, OW_TOUT);
}
/**
  * @brief
  * @param
  * @retval
  */
static void OW_Receive(uint8_t *data, uint8_t lenght)
{
	uint32_t i;
	uint32_t ow_lenght = lenght * 8U;
	uint8_t tx_byte = 0xffU;
	
	for(i = 0U; i < ow_lenght; i++)
	{
		HAL_UART_Transmit(&huart2, &tx_byte, 1U, OW_TOUT);
		HAL_UART_Receive(&huart2, &ow_rxbuf[i], 1U, OW_TOUT);
	}
	
	for(i = 0U; i < lenght; i++)
	{
		data[i] = OW_Unpack(&(ow_rxbuf[i * 8U]));
	}
}
/**
  * @brief
  * @param
  * @retval
  */
static uint8_t OW_ReadROM(uint8_t *ow_address)
{
	uint8_t crc;
	
	if(OW_Reset() != 0U)
	{
		OW_SendByte(OW_RDROM);
		OW_Receive(ow_address, 8U);
		crc = OW_CrcCheck(ow_address, 7U);
		if((crc != ow_address[7U]) || (crc == 0U))return (1U);
		else return(0U);
	}
	else return (2U);
}
/**
  * @brief
  * @param
  * @retval
  */
static uint8_t OW_CrcCheck(uint8_t *ow_address, uint8_t lenght)
{
	uint8_t crc = 0U;
	uint8_t i, j;

	for (i = 0U; i < lenght; i++) 
	{
		uint8_t inbyte = ow_address[i];
		
		for (j = 0U; j < 8U; j++) 
		{
			uint8_t mix = (crc ^ inbyte) & 0x01U;
			crc >>= 1U;
			if (mix) 
			crc ^= 0x8CU;
			inbyte >>= 1U;
		}
	}
	
	return crc;
}
/**
  * @brief
  * @param
  * @retval
  */
static void OW_Pack(uint8_t command, uint8_t buffer[8])
{
	uint32_t i;
	
	for (i = 0U;  i < 8U; i++)
	{
		buffer[i] = (command & (1U << i)) ? 0xffU : 0x00U;
	}
}
/**
  * @brief
  * @param
  * @retval
  */
static uint8_t OW_Unpack (uint8_t buffer[8])
{
	uint32_t i;
	uint8_t res = 0U;

	for (i = 0U; i < 8U; i++) 
	{
		if (buffer[i] == 0xffU)
		{
			res |=  (1U << i);
		}
	}

	return res;
}
/**
  * @brief
  * @param
  * @retval
  */
static uint16_t OW_ScratchpadToTemperature(uint16_t scratchpad) 
{
    uint16_t result;
	
	if(scratchpad & (1U << 15))
	{
		scratchpad = ~scratchpad + 1U;
		result = scratchpad >> 4U; 							// cijelobrojni dio temperature
		result *= 10U; 										// 22 -> 220
		result += (((scratchpad & 0x000fU) *625U) / 1000U);
		result |= 0x8000U; 									// add minus sign
	}
	else
	{
		result = scratchpad >> 4U; 							// cijelobrojni dio temperature
		result *= 10U; 										// 22 -> 220
		result += (((scratchpad & 0x000fU) *625U) / 1000U);	// add decimal part
	}
    
    return result;
}
/**
  * @brief
  * @param
  * @retval
  */
static void OW_ResetSearch(void) 
{
	ow_last_discrepancy = 0U;
	ow_last_family_discrepancy = 0U;
	ow_last_device_flag = 0U;
	ow_sensor_number = 0U;
}
/**
  * @brief
  * @param
  * @retval
  */
static uint8_t OW_Search(TempSensorTypeDef* ds18b20, uint8_t* sensor_cnt) 
{
	static uint8_t init_cnt = 0U;
	uint8_t last_zero, rom_byte_number, search_result;
	uint8_t id_bit, cmp_id_bit, id_bit_number;
	uint8_t rom_byte_mask, search_direction;

	id_bit_number = 1U;
	last_zero = 0U;
	rom_byte_number = 0U;
	rom_byte_mask = 1U;

	if (ow_last_device_flag == 0U)
	{
		if (OW_Reset() == 0U)
		{
			ow_last_discrepancy = 0U;
			ow_last_device_flag = 0U;
			ow_last_family_discrepancy = 0U;
			return (0U);
		}

		OW_SendByte(OW_SRCHROM); 

		do{
			id_bit = OW_ReceiveBit();
			cmp_id_bit = OW_ReceiveBit();
			
			if ((id_bit == 1U) && (cmp_id_bit == 1U)) break;
			else
			{
				if (id_bit != cmp_id_bit) search_direction = id_bit;  // bit write value for search
				else
				{
					if (id_bit_number < ow_last_discrepancy)
					{
						search_direction = ((ds18b20->rom_code[rom_byte_number] & rom_byte_mask) > 0U);
					}
					else search_direction = (id_bit_number == ow_last_discrepancy);
					
					if (search_direction == 0U)
					{
						last_zero = id_bit_number;
						if (last_zero < 9U)  ow_last_family_discrepancy = last_zero;
					}
				}
				
				if (search_direction == 1) ds18b20->rom_code[rom_byte_number] |= rom_byte_mask;
				else ds18b20->rom_code[rom_byte_number] &= ~rom_byte_mask;
				
				OW_SendBit(search_direction);
				id_bit_number++;
				rom_byte_mask <<= 1U;
				
				if (rom_byte_mask == 0U)
				{
					rom_byte_number++;
					rom_byte_mask = 1U;
				}
			}
		} while(rom_byte_number < 8U);
		
		if (!(id_bit_number < 65U))
		{
			search_result = 1U;
			ow_last_discrepancy = last_zero;
			if (ow_last_discrepancy == 0U) ow_last_device_flag = 1U;
		}
	}
	
	if ((search_result == 0U) || (ds18b20->rom_code[0] == 0U))
	{
		ow_last_discrepancy = 0U;
		ow_last_device_flag = 0U;
		ow_last_family_discrepancy = 0U;
		return (0U);
	} 
	else 
	{
		init_cnt++;
		*sensor_cnt = init_cnt;
		ds18b20->sensor_id = init_cnt;
		return (init_cnt);
	}	
}
/**
  * @brief
  * @param
  * @retval
  */
static void OW_Select(uint8_t* addr) 
{
	uint8_t i;
	
	OW_SendByte(OW_MCHROM);
	
	for (i = 0U; i < 8U; i++) 
	{
		OW_SendByte(*(addr + i));
	}
}
#endif
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
