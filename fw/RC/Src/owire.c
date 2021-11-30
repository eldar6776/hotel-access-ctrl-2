/**
 ******************************************************************************
 * File Name          : OW.c
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
#include "display.h"
#include "logger.h"
#include "eeprom.h"
#include "rc522.h"
#include "rs485.h"
#include "owire.h"
#include "room.h"
#include "main.h"
#include "dio.h"
//#include "rf24.h"
/* Imported Type  ------------------------------------------------------------*/
/* Imported Variable  --------------------------------------------------------*/
/* Imported Function  --------------------------------------------------------*/
/* Private Type --------------------------------------------------------------*/
#ifdef  OW_DS18B20 
TempSensorTypeDef ds18b20_1;
#endif
/* Private Define ------------------------------------------------------------*/
/* Private Variable ----------------------------------------------------------*/
uint8_t ow_add[9];
uint8_t rt_img;
uint8_t ow_ifa;
uint8_t ow_gra;
uint8_t ow_bra;
uint8_t ow_bps;
uint8_t ow_dev;
/* Macros     ----------------------------------------------------------------*/
/* Private prototypes    -----------------------------------------------------*/
#ifdef  OW_DS18B20
uint32_t ow_dscnt;
static uint8_t OW_Reset(void);
static uint8_t OW_ReceiveBit(void);
static void OW_SendByte(uint8_t data);
static void OW_SendBit(uint8_t send_bit);
static uint8_t OW_Search(TempSensorTypeDef* ds18b20, uint8_t* sensor_cnt);
#endif
static void OW_SetUsart(uint8_t bps, uint8_t word_lenght);
/* Program code   ------------------------------------------------------------*/
/**
  * @brief
  * @param
  * @retval
  */
void OW_Init(void)
{
    if (ow_dev) TempSenLuxRTSet(); // set flag to enable conditional processe
    
#ifdef  OW_DS18B20 
    else OW_ScanBus();
#endif
}
/**
  * @brief
  * @param
  * @retval
  */
void OW_Service(void)
{
    static HAL_StatusTypeDef stat;
    static uint8_t owbuf[64];
    static uint8_t pcnt = 0U;
    static uint8_t repcnt = 0U;
    static uint32_t owtmr = 0U;
    static uint8_t errcnt = 0U;
    static uint32_t cs_delay = 0;
    
    if (eComState == COM_PACKET_RECEIVED) RS485_Service();
    if (!ow_dev || IsRS485_UpdateActiv())       return; // get back if ongoing update or device list empty
    if ((HAL_GetTick() - owtmr) < OW_UPDATE)    return; // and there is timer for regulatied timing 
    if (IsStatUpdActiv()) StatUpdReset(), pcnt = 0U, repcnt = 0U; // send new status to onewire slave devices
    /** ============================================================================*/
    /**			    S E T           O N E W I R E           C O M M A N D			*/
    /** ============================================================================*/        
    if      (!pcnt && !repcnt)
    {
        //pcnt = ow_dev;            // enable next request setting address index to last device
        pcnt = 2;                   // one thermostat and one card stacker with constant addresse 1 and 2
        repcnt = TXREP_CNT;         // resend packet in group link file transfer
        CRC_ResetDR();              // reset crc to default value
        owbuf[0] = STX;             // start of text to wakeup all device receivers 
        owbuf[1] = 1;               // this is address mark 9 bit set high
        owbuf[2] = STX;             // calculate crc8 with first byte included
        owbuf[3] = DEF_RT_OWGRA;    // send button state to group address
        owbuf[4] = ow_ifa;          // send interface address
        owbuf[5] = 2;               // number of payload bytes
        owbuf[6] = RT_SET_BTN_STA;  // command to receiver
        owbuf[7] = 0;               // clear flags before settings
        if (IsRoomCleaningActiv())      owbuf[7] |= (1U<<0);
        if (IsDonNotDisturbActiv())     owbuf[7] |= (1U<<1);
        if (ROOM_Status==ROOM_SOS_ALARM)owbuf[7] |= (1U<<2);
        if (IsCallMaidActiv())          owbuf[7] |= (1U<<3);
        if (IsTempRegEnabled())         owbuf[7] |= (1U<<4);
        if (IsWinDoorSwClosed())        owbuf[7] |= (1U<<5);
        if (IsPowerContactorActiv())    owbuf[7] |= (1U<<6);
        if (IsDoorBellActiv())          owbuf[7] |= (1U<<7);
        owbuf[8] = (uint8_t) CRC_Calculate8(&owbuf[2], 6);
    }
    else if (pcnt && !repcnt)
    {
        //if (pcnt) --pcnt;         // set address list index
        repcnt = MAXREP_CNT;        // repeat request 9 times to non responsive device 
        CRC_ResetDR();              // reset crc to default value
        owbuf[0] = STX;             // start of text to wakeup all device receivers 
        owbuf[1] = 1;            // this is address mark 9 bit set high
        owbuf[2] = STX;             // calculate crc8 with first byte included
        //owbuf[3] = ow_add[pcnt];  // address only joined device
        owbuf[3] = pcnt;
        owbuf[4] = ow_ifa;
        owbuf[5] = 1;
        owbuf[6] = GET_SYS_STAT;
        owbuf[7] = (uint8_t) CRC_Calculate8(&owbuf[2], 0x5U);
        if (pcnt) --pcnt;
    }
    /** ============================================================================*/
    /**			   S E N D          O N E W I R E           C O M M A N D			*/
    /** ============================================================================*/ 
    OW_SetUsart(BR_9600, WL_9BIT);  // set usart to 9 bit mode and send address with address mark 
    stat = HAL_UART_Transmit(&huart2, owbuf, 1, PAK_TOUT(0x1U, BR_9600)); // first two from buffer to wake up receivers
    OW_SetUsart(BR_9600, WL_8BIT);  // set usart to 8 bit mode and send command packet
    stat += HAL_UART_Transmit(&huart2, &owbuf[3], owbuf[5]+4, PAK_TOUT((owbuf[5]+4), BR_9600));
    /** ==========================================================================*/
    /**			    G E T		T H E R M O S T A T     S T A T E			      */
    /** ==========================================================================*/
    if (owbuf[6] == GET_SYS_STAT)
    {
        CRC_ResetDR();
        ZEROFILL (&owbuf[16], COUNTOF(owbuf) - 16U);
        __HAL_UART_FLUSH_DRREGISTER(&huart2);
        stat += HAL_UART_Receive(&huart2, &owbuf[16], 0x9U, PAK_TOUT(0x9U, BR_9600));
        if ((stat == HAL_OK)  // check received packet 
        && (owbuf[16] == STX) 
        && (owbuf[17] == owbuf[4])
        && (owbuf[18] == owbuf[3]) 
        && (owbuf[19] == 4U)      
        && (owbuf[24] == (uint8_t) CRC_Calculate8(&owbuf[16], 0x8U)))
        {
            if      (owbuf[3] == 1U) // call to room thermostat
            {
                repcnt = 0U; // reset repeat counter if response packet received
                errcnt = 0U; // reset non responsive device trials counter 
                TempSenLuxRTSet(); // and enable thermostat flag if disabled by error counter
                //
                // BALCONY & WINDOWS RADIO SENSOR REMOTE SWITCH
                //
                if      ((owbuf[20]  & (1U<<0)) && !IsExtWinDoorSwClosed())
                {
                    ExtWinDoorSwClosed();
                    ExtWinDoorSwEnable(); // first time closed state include switch in application
                    LogEvent.log_event = BALCON_DOOR_CLOSED;
                    LOGGER_Write();
                }
                else if (!(owbuf[20] & (1U<<0)) && IsExtWinDoorSwClosed())
                {
                    ExtWinDoorSwOpen();
                    TempRegDisable();
                    LogEvent.log_event = BALCON_DOOR_OPENED;
                    LOGGER_Write();
                }
                //
                // DND BUTTON STATE
                //
                if      (((owbuf[20] & (1U<<1)) != 0U) && !IsDonNotDisturbActiv())
                {
                    DoNotDisturb_On();
                    DISP_DndImg();
                    LogEvent.log_event = DO_NOT_DISTURB_SWITCH_ON;
                    LOGGER_Write();
                }
                else if (((owbuf[20] & (1U<<1)) == 0U) && IsDonNotDisturbActiv())
                {
                    DoNotDisturb_Off();
                    DISP_DndImgDelete();
                    LogEvent.log_event = DO_NOT_DISTURB_SWITCH_OFF;
                    LOGGER_Write();
                }
                //
                // SOS BUTTON STATE
                //
                if      (((owbuf[20] & (1U<<2)) != 0U) && (ROOM_Status != ROOM_SOS_ALARM))
                {                
                    ROOM_Status = ROOM_SOS_ALARM;
                }
                else if (((owbuf[20] & (1U<<2)) == 0U) && (ROOM_Status == ROOM_SOS_ALARM))
                {
                    ROOM_Status = ROOM_RESET_ALARM;
                }
                //
                // HM CALL BUTTON STATE
                //
                if      (((owbuf[20] & (1U<<3)) != 0U) && !IsCallMaidActiv())
                {
                    CallMaidSet();
                    LEDGreenOn();
                    LogEvent.log_event = HANDMAID_SWITCH_ON;
                    LOGGER_Write();
                }
                else if (((owbuf[20] & (1U<<3)) == 0U) && IsCallMaidActiv())
                {
                    CallMaidReset();
                    LEDGreenOff();
                    LogEvent.log_event = HANDMAID_SWITCH_OFF;
                    LOGGER_Write();
                }
                //
                // OPEN DOOR BUTTON STATE
                //
                if      (((owbuf[20] & (1U<<4)) != 0U) && !IsDoorlockRemoteOpenActiv())
                {
                    DoorlockRemoteOpenSet();
                    LogEvent.log_event = DOOR_LOCK_USER_OPEN;
                    LOGGER_Write();
                    DoorLockCoil_On();
                    DISP_CardValidImage();
                    BUZZ_State = BUZZ_CARD_VALID;
                    RC522_ExtendDoorlockTimeSet();
                    RC522_ClearData();
                }
                else if (((owbuf[20] & (1U<<4)) == 0U) && IsDoorlockRemoteOpenActiv())
                {
                    DoorlockRemoteOpenReset();
                }
                //
                // OK BUTTON STATE
                //
                if ((owbuf[20] & (1U<<5)) != 0U) {}//ButtonOkActiv
                //
                // NTC ERROR LOG
                //
                if ((owbuf[20] & (1U<<6)) != 0U) 
                {
                    LogEvent.log_event = RT_DISP_NTC_ERR;
                    LogEvent.log_card_id[0] = DEF_RT_OWIFA;
                    LOGGER_Write();
                }
                //
                // AT LEAST ONE BUTTON STATE CHANGED
                //
                if ((owbuf[20] & (1U<<7)) != 0U) {}//ButtonRemoteSet
                //
                // SET POINT & MEASURE VALUE UPDATE
                //
                temp_sp = owbuf[21];
                temp_mv = (int8_t)owbuf[22];
                rt_img = owbuf[23];
            }
            else if (owbuf[3] == 2U) // call to card stacker
            {
                //
                //  CARD STACKER STATE
                //
                if      (((owbuf[20] & (1U<<1)) != 0U) && !IsCardInStackerActiv())    // CARD INSERTED IN STACKER EVENT    
                {
                    CardInStackerSet();
                    if (ROOM_Status != ROOM_CLEANING_RUN) UserCapSwPanelOn();
                    if (!IsPowerContactorActiv())
                    {
                        PowerContactorOn();
                        LogEvent.log_event = CARD_STACKER_ON;
                        LOGGER_Write();
                        StatUpdSet();
                    }
                    LEDRedOn();
                    cs_delay = 0;
                }
                else if (((owbuf[20] & (1U << 1)) == 0U) && IsCardInStackerActiv())  // CARD REMOVED FROM STACKER EVENT
                {
                    CardInStackerReset();
                    cs_delay = HAL_GetTick();
                }
            }
        }            
    }
    if (repcnt) --repcnt;   // repeat request max trial time, and after clear temperature from display
    if (!repcnt && !pcnt && (++errcnt > OW_MAXERR)) TempSenLuxRTReset(); // onewire bus error or no device connected 
    owtmr = HAL_GetTick();  // ow service update rate
    
    if(cs_delay)
    {
        if ((HAL_GetTick() - cs_delay) >= 15000)
        {
            if (IsPowerContactorActiv())
            {
                PowerContactorOff();
                LogEvent.log_event = CARD_STACKER_OFF;
                TempRegDisable();
                LOGGER_Write();
                StatUpdSet();
            }
            LEDRedOff(); 
            cs_delay = 0;
        }
    }
    
}
/**
  * @brief
  * @param
  * @retval
  */
uint8_t OW_RS485_Bridge(uint8_t *buff)
{
    uint8_t txhdr[2];
    uint8_t rxhdr[8];
    uint32_t repcnt = 0;
    HAL_StatusTypeDef stat;
    
    CRC_ResetDR();
    repcnt = 5U;
    txhdr[0] = STX;     // start of text to wakeup all device receivers 
    txhdr[1] = 1;    // this is address mark 9 bit set high
    buff[0] = STX;      // set for crc calculation
    buff[2] = ow_ifa;   // insert sender interface address 
    buff[buff[3]+4] = (uint8_t) CRC_Calculate8(buff, buff[3]+4); // recalculate packet crc8 
    
    while(repcnt)
    {
        OW_SetUsart(BR_9600, WL_9BIT);
        stat  = HAL_UART_Transmit (&huart2, txhdr, 1, PAK_TOUT(1, BR_9600)); 
        OW_SetUsart(BR_9600, WL_8BIT);
        stat += HAL_UART_Transmit (&huart2, &buff[1], (buff[3]+4), PAK_TOUT((buff[3]+4), BR_9600));
        if ((buff[1] == DEF_OWBRA) 
        ||  (buff[1] == DEF_HC_OWGRA) 
        ||  (buff[1] == DEF_CR_OWGRA) 
        ||  (buff[1] == DEF_RC_OWGRA) 
        ||  (buff[1] == DEF_RT_OWGRA)) return (ESC); // brake here for request without response
        __HAL_UART_FLUSH_DRREGISTER (&huart2);
        ZEROFILL(rxhdr, COUNTOF(rxhdr));
        stat += HAL_UART_Receive (&huart2, rxhdr, 7, OW_PKTIME);
        if ((stat     == HAL_OK)
        &&  (rxhdr[0] == STX) 
        &&  (rxhdr[1] == buff[2]) 
        &&  (rxhdr[2] == buff[1])
        &&  (rxhdr[3] > 1))
        {
            CRC_ResetDR();
            if (rxhdr[3] == 2) 
            {   /* simple response to acknowledge received packet */
                if (rxhdr[6] == (uint8_t) CRC_Calculate8(rxhdr, 6))
                {   // copy cmd echo or response data byte to first buffer byte,
                    buff[5] = 1; // response packet data payload size
                    buff[0] = rxhdr[5]; // it will be included in response from bridge
                    return   (rxhdr[4]); // and send request status response from addressed device
                }   // if response packet check fail, try again till max. trials
                else stat = HAL_ERROR; 
            }
            else
            {   /* receive extende data direct to call buffer */
                buff[0] = rxhdr[5]; // copy response byte
                buff[5] = rxhdr[3]; // response packet data payload size
                buff[7] = rxhdr[6]; // copy first received payload byte
                stat = HAL_UART_Receive (&huart2, &buff[8], buff[5]-2, OW_PKTIME); // get all packet data
                CRC_Calculate8(rxhdr, 7); // calculate first part response crc and try send again if transfer not success
                if (buff[buff[5]+5] == (uint8_t) CRC_Calculate8(&buff[8], buff[5]-3)) 
                {
                    buff[buff[5]+5] = '\0';
                    return (rxhdr[4]); // return response
                }
                else stat = HAL_ERROR;
            }
        }
        --repcnt;   // valid response should allready return to caller 
        HAL_Delay(RX2TX_DEL);
    }
    return (NAK);
}
/**
  * @brief
  * @param
  * @retval
  */
void OW_ScanBus(void)
{
    uint8_t owbuf[12];
    uint32_t repcnt = 0x0U;
    HAL_StatusTypeDef stat;
    uint8_t owifa = DEF_RT_OWIFA;       // default interface address is first room thermostat address
    ow_dev = 0x0U;                      // clear number of onewire bus connected device
    TempSenDS18Reset();                 // clear flag for connected Dallas DS18B20
    TempSenLuxRTReset();                // clear flag for connected room thermostats 
    ZEROFILL(ow_add, COUNTOF(ow_add));  // clear address list
#ifdef	USE_WATCHDOG
    HAL_IWDG_Refresh(&hiwdg);           // start new watchdog cycle 
#endif    
    /*  search onewire bus for connected room thermostats */
    do
    {
        OW_SetUsart(BR_9600, WL_9BIT);
        owbuf[0] = STX;  // start of text to wakeup all device receivers 
        owbuf[1] = 0x1U; // address mark 9 bit high
        HAL_UART_Transmit (&huart2,   owbuf,  0x1U,   PAK_TOUT(0x1U, BR_9600));
        OW_SetUsart (BR_9600, WL_8BIT);
        CRC_ResetDR();
        owbuf[1] = owifa;
        owbuf[2] = ow_ifa;
        owbuf[3] = 0x1U;
        owbuf[4] = GET_SYS_STAT;
        owbuf[5] = (uint8_t) CRC_Calculate8   (owbuf,   0x5U);
        stat = HAL_UART_Transmit    (&huart2, &owbuf[1],0x5U, PAK_TOUT(0x5U, BR_9600));
        __HAL_UART_FLUSH_DRREGISTER (&huart2);
        stat += HAL_UART_Receive    (&huart2,  owbuf,   0x9U, PAK_TOUT(0x9U, BR_9600));
        CRC_ResetDR();
        if ((stat    == HAL_OK) 
        && (owbuf[0] == STX)        
        && (owbuf[1] == ow_ifa) 
        && (owbuf[2] == owifa)  
        && (owbuf[3] == 0x4U)     
        && (owbuf[8] == (uint8_t) CRC_Calculate8(owbuf, 0x8U)))
        {
            ow_add[ow_dev] = owifa; // save device address
            TempSenLuxRTSet();      // set flag for one or more room thermostats connected to onewrite bus
            repcnt = 0x0U;          // reset repeated cycle counter
            ++ow_dev;               // number of connected device
            ++owifa;                // next device address
        }
        else
        {
            ++repcnt;               // repeated request cycle counter for non responsive address
            if(repcnt >= 0x5U)      // after max. number of trials 
            {
                repcnt = 0x0U;      // reset repeated cycle counter
                ++owifa;            // set next device address
            }
        }
        HAL_Delay(2);
    }
    while(owifa < (DEF_RT_OWIFA + OW_DEV_CNT));
    EEPROM_Save(EE_OWADD1, ow_add, OW_DEV_CNT);  // save room thermostats address list to eeprom
    
#ifdef  OW_DS18B20 
	if(OW_Search(&ds18b20_1, &ow_dev))
	{
		TempSenDS18Set();   // found one or more Dallas DS18B20 temperature sensors connected to onewrite bus
	}
#endif
}
/**
  * @brief
  * @param
  * @retval
  */
static void OW_SetUsart(uint8_t b_rate, uint8_t b_lenght)
{
	huart2.Instance = USART2;
    huart2.Init.BaudRate = bps[b_rate];
    if      (b_lenght == WL_9BIT) huart2.Init.WordLength = UART_WORDLENGTH_9B;
	else if (b_lenght == WL_8BIT) huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_DeInit (&huart2) != HAL_OK) ErrorHandler(OW_FUNC, USART_DRV);
    if (HAL_UART_Init   (&huart2) != HAL_OK) ErrorHandler(OW_FUNC, USART_DRV);
}
#ifdef  OW_DS18B20 
static uint8_t OW_Search(TempSensorTypeDef* ds18b20, uint8_t* sensor_cnt)
{
	static uint8_t init_cnt = 0U;
	uint8_t last_zero, rom_byte_number, search_result;
	uint8_t id_bit, cmp_id_bit, id_bit_number;
	uint8_t rom_byte_mask, search_direction;
    static uint8_t ow_last_discrepancy = 0U;
    static uint8_t ow_last_device_flag = 0U;

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


static uint8_t OW_Reset(void)
{
    uint8_t onew_buff[2];

	OW_SetUsart(BR_9600, WL_8BIT);
	onew_buff[0] = 0xF0U;
    onew_buff[1] = 0U;
	if (HAL_UART_Transmit(&huart2, onew_buff, 1U, OW_TOUT) != HAL_OK) ErrorHandler(OW_FUNC, USART_DRV);
	if (HAL_UART_Receive(&huart2, &onew_buff[1], 1U, OW_TOUT) != HAL_OK) ErrorHandler(OW_FUNC, USART_DRV);
	OW_SetUsart(BR_115200, WL_8BIT);

	if((onew_buff[1] != 0xF0U) && (onew_buff[1] != 0U) && (onew_buff[1] != 0xFFU)) return (1U);
	else return(0U);
}


static void OW_SendByte(uint8_t data)
{
	uint32_t i;

	for(i = 0U; i < 8U; i++)
	{
		OW_SendBit(data & (1U << i));
	}
}


static uint8_t OW_ReceiveBit(void)
{
	uint8_t onew_buff[2];

	onew_buff[0] = 0xFFU;
    onew_buff[1] = 0U;
	if(HAL_UART_Transmit(&huart2, onew_buff, 1U, OW_TOUT) != HAL_OK) ErrorHandler(OW_FUNC, USART_DRV);
	if(HAL_UART_Receive(&huart2, &onew_buff[1], 1U, OW_TOUT) != HAL_OK) ErrorHandler(OW_FUNC, USART_DRV);

	if(onew_buff[0] == onew_buff[1]) return(1U);
	else return(0U);
}


static void OW_SendBit(uint8_t send_bit)
{
	uint8_t onew_buff[2];

	if(send_bit == 0U)  onew_buff[0] = 0U;
	else onew_buff[0] = 0xFFU;
    onew_buff[1] = 0U;
	if(HAL_UART_Transmit(&huart2, onew_buff, 1U, OW_TOUT) != HAL_OK) ErrorHandler(OW_FUNC, USART_DRV);
	if(HAL_UART_Receive(&huart2, &onew_buff[1], 1U, OW_TOUT) != HAL_OK) ErrorHandler(OW_FUNC, USART_DRV);
}
#endif
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
