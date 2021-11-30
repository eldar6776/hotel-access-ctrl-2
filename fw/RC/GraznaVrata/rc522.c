/**
 ******************************************************************************
 * File Name          : mfrc522.c
 * Date               : 28/02/2016 23:16:19
 * Description        : mifare RC522 software modul
 ******************************************************************************
 *
 *
 ******************************************************************************
 */


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


#if (__RC522_H__ != FW_BUILD)
    #error "rc522 header version mismatch"
#endif
/* Imported Type  ------------------------------------------------------------*/
/* Imported Variable  --------------------------------------------------------*/
/* Imported Function  --------------------------------------------------------*/
/* Private Define ------------------------------------------------------------*/
#define RC522_POWER_ON_DELAY_TIME           8901U	// 8 s on power up reader is disbled 
#define RC522_PROCESS_TIME					45U	    // 89 ms read rate
#define RC522_HMRST			                2345U	// reset handmaid card status
#define RC522_READ    						0x7FU	// RC522 i2c read address
#define RC522_WRITE   						0x7EU	// RC522 i2c write address
#define RC522_BSIZE              		    16U     // block size
/** ==========================================================================*/
/**			M I F A R E		C A R D		S E C T O R		A D D R E S S E		  */
/** ==========================================================================*/
#define SECTOR_0							0x00U
#define SECTOR_1							0x04U
#define SECTOR_2							0x08U
#define SECTOR_3							0x0CU
#define SECTOR_4							0x10U
#define SECTOR_5							0x14U
#define SECTOR_6							0x18U
#define SECTOR_7							0x1CU
#define SECTOR_8							0x20U
/** ==========================================================================*/
/**			R C 5 2 2			C O M M A N D			L I S T				  */
/** ==========================================================================*/
#define PCDLE							    0x00U   //NO action; Cancel the current command
#define PCD_AUTHENT							0x0EU   //Authentication Key
#define PCD_RECEIVE							0x08U   //Receive Data
#define PCD_TRANSMIT						0x04U   //Transmit data
#define PCD_TRANSCEIVE						0x0CU   //Transmit and receive data,
#define PCD_RESETPHASE						0x0FU   //Reset
#define PCD_CALCCRC							0x03U   //CRC Calculate
/* Mifare_One card command word */
#define PICC_REQIDL							0x26U   // find the antenna area does not enter hibernation
#define PICC_REQALL							0x52U   // find all the cards antenna area
#define PICC_ANTICOLL						0x93U   // anti-collision
#define PICC_SELECTTAG						0x93U   // election card
#define PICC_AUTHENT1A						0x60U   // authentication key A
#define PICC_AUTHENT1B						0x61U   // authentication key B
#define PICC_READ							0x30U   // Read Block
#define PICC_WRITE							0xA0U   // write block
#define PICC_DECREMENT						0xC0U   // debit
#define PICC_INCREMENT						0xC1U   // recharge
#define PICC_RESTORE						0xC2U   // transfer block data to the buffer
#define PICC_TRANSFER						0xB0U   // save the data in the buffer
#define PICC_HALT							0x50U   // Sleep
/** ==========================================================================*/
/**			R C 5 2 2			R E G I S T E R			L I S T				  */
/** ==========================================================================*/
//Page 0: Command and Status
#define RC522_REG_RESERVED00				0x00U    
#define RC522_REG_COMMAND					0x01U    
#define RC522_REG_COMM_IE_N					0x02U    
#define RC522_REG_DIV1_EN					0x03U    
#define RC522_REG_COMM_IRQ					0x04U    
#define RC522_REG_DIV_IRQ					0x05U
#define RC522_REG_ERROR						0x06U    
#define RC522_REG_STATUS1					0x07U    
#define RC522_REG_STATUS2					0x08U    
#define RC522_REG_FIFO_DATA					0x09U
#define RC522_REG_FIFO_LEVEL				0x0AU
#define RC522_REG_WATER_LEVEL				0x0BU
#define RC522_REG_CONTROL					0x0CU
#define RC522_REG_BIT_FRAMING				0x0DU
#define RC522_REG_COLL						0x0EU
#define RC522_REG_RESERVED01				0x0FU
//Page 1: Command 
#define RC522_REG_RESERVED10				0x10U
#define RC522_REG_MODE						0x11U
#define RC522_REG_TX_MODE					0x12U
#define RC522_REG_RX_MODE					0x13U
#define RC522_REG_TX_CONTROL				0x14U
#define RC522_REG_TX_AUTO					0x15U
#define RC522_REG_TX_SELL					0x16U
#define RC522_REG_RX_SELL					0x17U
#define RC522_REG_RX_THRESHOLD				0x18U
#define RC522_REG_DEMOD						0x19U
#define RC522_REG_RESERVED11				0x1AU
#define RC522_REG_RESERVED12				0x1BU
#define RC522_REG_MIFARE					0x1CU
#define RC522_REG_RESERVED13				0x1DU
#define RC522_REG_RESERVED14				0x1EU
#define RC522_REG_SERIALSPEED				0x1FU
//Page 2: CFG    
#define RC522_REG_RESERVED20				0x20U  
#define RC522_REG_CRC_RESULT_M				0x21U
#define RC522_REG_CRC_RESULT_L				0x22U
#define RC522_REG_RESERVED21				0x23U
#define RC522_REG_MOD_WIDTH					0x24U
#define RC522_REG_RESERVED22				0x25U
#define RC522_REG_RF_CFG					0x26U
#define RC522_REG_GS_N						0x27U
#define RC522_REG_CWGS_PREG					0x28U
#define RC522_REG_MODGS_PREG				0x29U
#define RC522_REG_T_MODE					0x2AU
#define RC522_REG_T_PRESCALER				0x2BU
#define RC522_REG_T_RELOAD_H				0x2CU
#define RC522_REG_T_RELOAD_L				0x2DU
#define RC522_REG_T_COUNTER_VALUE_H			0x2EU
#define RC522_REG_T_COUNTER_VALUE_L			0x2FU
//Page 3:TestRegister 
#define RC522_REG_RESERVED30				0x30U
#define RC522_REG_TEST_SEL1					0x31U
#define RC522_REG_TEST_SEL2					0x32U
#define RC522_REG_TEST_PIN_EN				0x33U
#define RC522_REG_TEST_PIN_VALUE			0x34U
#define RC522_REG_TEST_BUS					0x35U
#define RC522_REG_AUTO_TEST					0x36U
#define RC522_REG_VERSION					0x37U
#define RC522_REG_ANALOG_TEST				0x38U
#define RC522_REG_TEST_ADC1					0x39U 
#define RC522_REG_TEST_ADC2					0x3AU  
#define RC522_REG_TEST_ADC0					0x3BU  
#define RC522_REG_RESERVED31				0x3CU  
#define RC522_REG_RESERVED32				0x3DU
#define RC522_REG_RESERVED33				0x3EU  
#define RC522_REG_RESERVED34				0x3FU
/* Private Type --------------------------------------------------------------*/
typedef enum 
{
	MI_OK 			= 0x0U,
	MI_NOTAGERR		= 0x1U,
    MI_ERR			= 0x2U,
	MI_SKIP_OVER	= 0x3U

} RC522_StatusTypeDef;


typedef struct
{
    uint8_t block_0[RC522_BSIZE];
    uint8_t block_1[RC522_BSIZE];
    uint8_t block_2[RC522_BSIZE];
	
} RC522_SectorTypeDef;

RC522_SectorTypeDef sector_0;
RC522_SectorTypeDef sector_1;
RC522_SectorTypeDef sector_2;
RC522_CardDataTypeDef sCard;
RC522_CardDataTypeDef sExtCard;
/**
*---------------     card data predefined addresse    --------------------
*/
#define CARD_USER_FIRST_NAME_ADD		(sector_0.block_1[0])
#define CARD_USER_LAST_NAME_ADD         (sector_0.block_2[0])
#define CARD_USER_GROUP_ADD             (sector_1.block_0[0])
#define CARD_SYSTEM_ADD				    (sector_1.block_1[0])
#define CARD_EXPIRY_TIME_ADD			(sector_2.block_0[0])
#define CARD_CTRL_ADD				    (sector_2.block_0[6])
#define CARD_USER_INVALIDITY_ADD		(sector_2.block_0[8])
#define CARD_USER_LANGUAGE_ADD			(sector_2.block_0[9])
#define CARD_USER_LOGO_ADD				(sector_2.block_0[10])
#define CARD_USER_GENDER_ADD			(sector_2.block_0[11])
/* Private Variable ----------------------------------------------------------*/
uint32_t rc522_fl;
uint8_t card_id[5];
uint8_t system_id[2];
uint8_t card_serial[5];
uint8_t mifare_keya[6]; 
uint8_t mifare_keyb[6];
uint8_t permitted_add[8][2];
uint8_t rc522_rx_buff[RC522_BSIZE];
uint8_t rc522_tx_buff[RC522_BSIZE];
uint8_t reset_card_serial[5] = {0x34U, 0x75U, 0xA6U, 0xA7U, 0x40U};
/* Private Macro -------------------------------------------------------------*/
/* Private Function Prototype ------------------------------------------------*/
static void RC522_Halt(void);
static void RC522_AntennaOn(void);
static RC522_StatusTypeDef RC522_ReadCard(void);
static uint8_t RC522_SelectTag(uint8_t* serNum);
static uint8_t RC522_ReadRegister(uint8_t addr);
static RC522_StatusTypeDef RC522_VerifyData(void);
static RC522_StatusTypeDef RC522_Check(uint8_t* id);
static void RC522_SetBitMask(uint8_t reg, uint8_t mask);
static void RC522_ClearBitMask(uint8_t reg, uint8_t mask);
static void RC522_WriteRegister(uint8_t addr, uint8_t val);
static RC522_StatusTypeDef RC522_Anticoll(uint8_t* serNum);
static RC522_StatusTypeDef RC522_Read(uint8_t blockAddr, uint8_t* recvData);
static RC522_StatusTypeDef RC522_Request(uint8_t reqMode, uint8_t* TagType);
static void RC522_CalculateCRC(uint8_t* pIndata, uint8_t len, uint8_t* pOutData);
static RC522_StatusTypeDef RC522_Auth(uint8_t authMode, uint8_t BlockAddr, uint8_t* Sectorkey, uint8_t* serNum);
static RC522_StatusTypeDef RC522_ToCard(uint8_t command, uint8_t* sendData, uint8_t sendLen, uint8_t* backData, uint16_t* backLen);
/* Program Code  -------------------------------------------------------------*/
/**
  * @brief
  * @param
  * @retval
  */
void RC522_Init(void) 
{
	RC522_RST_SetLow(); 
	DOUT_Service();	
	HAL_Delay(10);    
	RC522_RST_SetHigh();
	DOUT_Service();
	HAL_Delay(50);
	RC522_WriteRegister(RC522_REG_COMMAND, PCD_RESETPHASE);	
	RC522_WriteRegister(RC522_REG_T_MODE, 0x8DU);
	RC522_WriteRegister(RC522_REG_T_PRESCALER, 0x3EU);
    RC522_WriteRegister(RC522_REG_T_RELOAD_L, 30U);	
	RC522_WriteRegister(RC522_REG_T_RELOAD_H, 0U);
    RC522_WriteRegister(RC522_REG_RF_CFG, 0x70U);			// 48dB gain	
	RC522_WriteRegister(RC522_REG_TX_AUTO, 0x40U);
	RC522_WriteRegister(RC522_REG_MODE, 0x3DU);
	RC522_AntennaOn();
	RC522_HandmaidReentranceDisable();
}
/**
  * @brief
  * @param
  * @retval
  */
void RC522_Service(void)
{	
    uint32_t i;
    uint8_t ee_buff[8];
    RTC_TimeTypeDef untm;
    RTC_DateTypeDef undt;
    static uint32_t mifare_time = 0U;    
	static uint32_t mifare_timer = 0U;
    static uint32_t handmaid_card_time = 0U;    
    static uint32_t handmaid_card_timer = 0;
	static uint8_t handmaid_card_cycles = 0U;
    
    if (IsRC522_ExtendDoorlockTimeActiv())
    {
        RC522_ExtendDoorlockTimeReset();
        mifare_timer = HAL_GetTick(); 
        mifare_time = RC522_CARD_OK_TOUT;
    }
    
	if ((HAL_GetTick() - handmaid_card_timer) >= handmaid_card_time)
	{
		if ((handmaid_card_cycles == 0x1U) && (ROOM_Status == ROOM_CLEANING_RUN)) RC522_HandmaidReentranceEnable();
		handmaid_card_cycles = 0x0U;
	}
    
    if (IsRS485_UpdateActiv()) return;
	if (eComState == COM_PACKET_RECEIVED) RS485_Service();	
    if ((HAL_GetTick() - mifare_timer) >= mifare_time) mifare_timer = HAL_GetTick();
    else return;
    //if ((RC522_Check(card_serial)== MI_OK) || (sExtCard.card_status == CARD_VALID))
	if (RC522_Check(card_serial)== MI_OK)
	{
		RC522_ClearData();
        
//        if (sExtCard.card_status   == CARD_VALID) 
//        {
//            sCard.card_status   = sExtCard.card_status;
//            sCard.user_group    = sExtCard.user_group;
//            sCard.system_id     = sExtCard.system_id;
//            sCard.controller_id = sExtCard.controller_id;
//            mem_cpy(sCard.expiry_time, sExtCard.expiry_time, 6);
//            mem_cpy(sCard.card_id, sExtCard.card_id, 5);
//            
//            mem_set(sExtCard.expiry_time,0,6);
//            mem_set(sExtCard.card_id,0,5);
//            sExtCard.controller_id = 0;
//            sExtCard.card_status = 0;
//            sExtCard.system_id = 0;
//        }
//        else if (RC522_ReadCard() == MI_OK) RC522_VerifyData();
//		else return;
        
        if (RC522_ReadCard()    != MI_OK) return;
        RC522_VerifyData();
		if ((sCard.system_id    != SYSTEMID_INVALID)
		&&	(sCard.system_id    != SYSTEMID_DATA_INVALID)
		&& ((sCard.card_status  == CARD_VALID)
		||	(sCard.user_group   == USERGRP_MANAGER)
		||	(sCard.user_group   == USERGRP_SERVICER)
		||	(sCard.user_group   == USERGRP_MAID)))
		{	
            mem_copy(LogEvent.log_card_id, sCard.card_id, 0x5U);
            if  ((sCard.user_group == USERGRP_MANAGER) || (sCard.user_group == USERGRP_SERVICER))
            {
                if      (sCard.user_group == USERGRP_MANAGER)   LogEvent.log_event = MANAGER_CARD;
                else if (sCard.user_group == USERGRP_SERVICER)  LogEvent.log_event = SERVICE_CARD;
                if (ROOM_Status < ROOM_UNUSABLE) unix_room = unix_rtc + SECONDS_PER_HOUR; // enable room power for one hour
                LOGGER_Write();
            }
			else if (sCard.user_group == USERGRP_MAID)
			{
                LogEvent.log_event = HANDMAID_CARD;
                if (!IsIndorCardReaderActiv() && !IsDonNotDisturbActiv())
                {
                    ++handmaid_card_cycles;
                    BUZZ_State = BUZZ_DOOR_BELL;
                    mifare_time = RC522_CARD_FAIL_TOUT;
                    handmaid_card_time = RC522_HMRST;
                    handmaid_card_timer = HAL_GetTick();
                    
                    if ((ROOM_Status == ROOM_CLEANING_REQ) 
                    ||  (ROOM_Status == ROOM_BEDDING_REQ) 
                    ||  (ROOM_Status == ROOM_GENERAL_REQ))
                    {
                        handmaid_card_cycles = 0U;
                        unix_room = unix_rtc + SECONDS_PER_HOUR; // enable room power for one hour 
                        ROOM_Status = ROOM_CLEANING_RUN;
                        LOGGER_Write();
                    }
                    else if (handmaid_card_cycles > 0x2U)
                    {
                        RstRoomSignSwSet(); // reset user signal switch panel
                        ROOM_Status = ROOM_READY;
                        BUZZ_State = BUZZ_CLEANING_END;
                        mifare_time = RC522_CARD_OK_TOUT;
                        LogEvent.log_event = HANDMAID_SERVICE_END;
                        LOGGER_Write();
                    }
                    else if (ROOM_Status != ROOM_CLEANING_RUN)
                    {   /*  open room with handmaid card if alarm activ*/
                        
                        /* change room state only if no activ alarm*/
                        if (ROOM_Status < ROOM_UNUSABLE)
                        {   
                            RstRoomSignSwSet(); // reset user signal switch panel
                            handmaid_card_cycles = 0U;
                            unix_room = unix_rtc + SECONDS_PER_HOUR; // enable room power for one hour 
                            LOGGER_Write();
                        }
                    }
                    RC522_Init();
                }
                else if (IsIndorCardReaderActiv() || IsDonNotDisturbActiv())
                {
                    ++handmaid_card_cycles;
                    handmaid_card_time = RC522_HMRST;
                    handmaid_card_timer = HAL_GetTick();
                    if (handmaid_card_cycles > 0x2U)
                    {
                        ROOM_Status = ROOM_BUSY;
                        BUZZ_State = BUZZ_CLEANING_END;
                        mifare_time = RC522_CARD_OK_TOUT;
                        if (IsCallMaidActiv()) RstRoomSignSwSet(); // reset user signal switch panel
                        LogEvent.log_event = HANDMAID_SERVICE_END;
                        LOGGER_Write();
                    }
                    else
                    {
                        BUZZ_State = BUZZ_DOOR_BELL;
                        mifare_time = RC522_CARD_FAIL_TOUT;
                    }
                    RC522_Init();
                }
			}
            else if (ROOM_Status == ROOM_UNUSABLE)
            {
                handmaid_card_cycles = 0x1U;
                DISP_OutOfSrvcImgSet();
                BUZZ_State = BUZZ_CARD_INVALID;
                mifare_time = RC522_CARD_FAIL_TOUT;
            }
            else if (sCard.user_group == USERGRP_GUEST)
            {
                PwrExpTimeSet();    // room power expiry time valid
                undt.Date   = sCard.expiry_time[0];
                undt.Month  = sCard.expiry_time[1];
                undt.Year   = sCard.expiry_time[2];
                untm.Hours  = sCard.expiry_time[3];
                untm.Minutes= sCard.expiry_time[4];
                untm.Seconds= sCard.expiry_time[5];
                unix_room = rtc2unix(&untm, &undt);
                ee_buff[0] = unix_room >> 24;
                ee_buff[1] = unix_room >> 16;
                ee_buff[2] = unix_room >>  8;
                ee_buff[3] = unix_room;
                ee_buff[4] = 0x0U;
                ee_buff[5] = 0x1U;  // room power expiry time set by user
                EEPROM_Save(EE_ROOM_PWRTOUT, ee_buff, 0x6U);
                if ((ROOM_Status == ROOM_IDLE) || (ROOM_Status == ROOM_READY)) ROOM_Status = ROOM_BUSY;
                LogEvent.log_event = GUEST_CARD;
                LOGGER_Write();
            }
            
			if      (!handmaid_card_cycles)
			{
                if(IsDISP_UserCardInfoTextEnabled())
                {
                    /**
                    *	will use rc522 rx and tx buffer as temp for displaying user data
                    */
                    ZEROFILL(rc522_rx_buff, COUNTOF(rc522_rx_buff));
                    ZEROFILL(rc522_tx_buff, COUNTOF(rc522_tx_buff));
                    
                    i = 0U;

                    do	// copy user data from card to display buffer
                    {	
                        if (ISLETTER(sector_0.block_1[i])) rc522_rx_buff[i] = sector_0.block_1[i];
                        if (ISLETTER(sector_0.block_2[i])) rc522_tx_buff[i] = sector_0.block_2[i];
                        ++i;
                    }
                    while(i < 16U);

                    if(sector_2.block_0[11] == 'M') rc522_rx_buff[0] |= 0x80U;
                }
				DoorLockCoil_On();
				PowerContactorOn();
				DISP_CardValidImage();			
				BUZZ_State = BUZZ_CARD_VALID;
                mifare_time = RC522_CARD_OK_TOUT;
			}
            else if (handmaid_card_cycles  > 0x2U) handmaid_card_cycles = 0x0U;
		}
		else
		{
			handmaid_card_cycles = 0U;
			BUZZ_State = BUZZ_CARD_INVALID;
            mifare_time = RC522_CARD_FAIL_TOUT;
			
			if (sCard.system_id == SYSTEMID_INVALID) 
			{
				DISP_WrongRoomImage();
				LogEvent.log_event = WRONG_SYSID;
			}
			else if(sCard.user_group == USERGRP_GUEST) 
			{
				if (sCard.controller_id == ROOMADDR_INVALID) 
				{
					DISP_WrongRoomImage();
					LogEvent.log_event = WRONG_ROOM;
				}
				else if(sCard.expiry_time[0] == EXPIRYTIME_INVALID)
				{
					DISP_TimeExpiredImage();
					LogEvent.log_event = CARD_EXPIRED;
				}
				else
				{
					DISP_CardInvalidImage();
					LogEvent.log_event = CARDID_INVALID;
				}
			}
			else 
			{
				LogEvent.log_event = UNKNOWN_CARD;
				DISP_CardInvalidImage();
			}
			mem_copy(LogEvent.log_card_id, sCard.card_id, 5U);		
			LOGGER_Write();			
		}		
	}
	else if (IsRC522_HandmaidReentranceActiv())
	{
		LOGGER_Write();
		DoorLockCoil_On();
		PowerContactorOn();
		DISP_CardValidImage();			
		BUZZ_State = BUZZ_CARD_VALID;
		RC522_HandmaidReentranceDisable();
        mifare_time = RC522_CARD_OK_TOUT;
        unix_room += SECONDS_PER_HOUR;
	}
	else 
    {
        if(!(dout_0_7_rem & (0x1U << 6))) DoorLockCoil_Off();
        mifare_time = RC522_PROCESS_TIME;
    }
}
/**
  * @brief
  * @param
  * @retval
  */
void RC522_ClearData(void)
{
    sCard.card_status = 0U;
    sCard.user_group = 0U;
    sCard.system_id = 0U;
    sCard.controller_id = 0U;
    ZEROFILL(rc522_rx_buff, COUNTOF(rc522_rx_buff));
    ZEROFILL(rc522_tx_buff, COUNTOF(rc522_tx_buff));
	mem_set(sCard.card_id, 0U, sizeof(sCard.card_id));
	mem_set(sCard.expiry_time, 0U, sizeof(sCard.expiry_time));
    ZEROFILL(sector_0.block_0, COUNTOF(sector_0.block_0));
    ZEROFILL(sector_0.block_1, COUNTOF(sector_0.block_1));
    ZEROFILL(sector_0.block_2, COUNTOF(sector_0.block_2));
    ZEROFILL(sector_1.block_0, COUNTOF(sector_1.block_0));
    ZEROFILL(sector_1.block_1, COUNTOF(sector_1.block_1));
    ZEROFILL(sector_1.block_2, COUNTOF(sector_1.block_2));
    ZEROFILL(sector_2.block_0, COUNTOF(sector_2.block_0));
    ZEROFILL(sector_2.block_1, COUNTOF(sector_2.block_1));
    ZEROFILL(sector_2.block_2, COUNTOF(sector_2.block_2));
}
/**
  * @brief
  * @param
  * @retval
  */
RC522_StatusTypeDef RC522_Check(uint8_t* id) 
{
	RC522_StatusTypeDef status;
	status = RC522_Request(PICC_REQIDL, id);					// Find cards, return card type
	if (status == MI_OK) status = RC522_Anticoll(id);			// Card detected. Anti-collision, return card serial number 4 bytes
	RC522_Halt();												// Command card into hibernation 
	return status;
}
/**
  * @brief
  * @param
  * @retval
  */
void RC522_WriteRegister(uint8_t addr, uint8_t val) 
{
	uint8_t txbuff[2];
    txbuff[0] = addr;     // set value
    txbuff[1] = val;     // set value
	if (HAL_I2C_Master_Transmit(&hi2c1, RC522_WRITE, (uint8_t*) &txbuff, 2U, DRV_TOUT) != HAL_OK) ErrorHandler(RC522_FUNC,  I2C_DRV);
}
/**
  * @brief
  * @param
  * @retval
  */
uint8_t RC522_ReadRegister(uint8_t addr) 
{
	uint8_t rxval = 0x0U;
	if (HAL_I2C_Master_Transmit(&hi2c1, RC522_WRITE, (uint8_t*) &addr, 1U, DRV_TOUT) != HAL_OK)    ErrorHandler(RC522_FUNC,  I2C_DRV);
	if (HAL_I2C_Master_Receive (&hi2c1, RC522_READ,  (uint8_t*) &rxval, 1U, DRV_TOUT) != HAL_OK)   ErrorHandler(RC522_FUNC,  I2C_DRV);
    return (rxval);
}
/**
  * @brief
  * @param
  * @retval
  */
void RC522_SetBitMask(uint8_t reg, uint8_t mask) 
{   
	RC522_WriteRegister(reg, RC522_ReadRegister(reg) | mask);
}
/**
  * @brief
  * @param
  * @retval
  */
void RC522_ClearBitMask(uint8_t reg, uint8_t mask)
{  
	RC522_WriteRegister(reg, RC522_ReadRegister(reg) & (~mask));
}
/**
  * @brief
  * @param
  * @retval
  */
void RC522_AntennaOn(void) 
{ 
	uint8_t temp;

	temp = RC522_ReadRegister(RC522_REG_TX_CONTROL);
	if ((temp & 0x03U) == 0U) RC522_SetBitMask(RC522_REG_TX_CONTROL, 0x03U);
}


static RC522_StatusTypeDef RC522_Request(uint8_t reqMode, uint8_t* TagType) 
{
	RC522_StatusTypeDef status;  
	uint16_t backBits;                                  //The received data bits

	RC522_WriteRegister(RC522_REG_BIT_FRAMING, 0x07U);	// TxLastBits = BitFramingReg[2..0]	???
	TagType[0] = reqMode;
	status = RC522_ToCard(PCD_TRANSCEIVE, TagType, 1U, TagType, &backBits);
	if ((status != MI_OK) || (backBits != 0x10U)) status = MI_ERR;
	return status;
}
/**
  * @brief
  * @param
  * @retval
  */
static RC522_StatusTypeDef RC522_ToCard(uint8_t command, uint8_t* sendData, uint8_t sendLen, uint8_t* backData, uint16_t* backLen) 
{
	uint8_t irqEn = 0U;
	uint8_t waitIRq = 0U;
	uint32_t n, i, lastBits;
    RC522_StatusTypeDef status = MI_ERR;

	switch (command) 
	{
		case PCD_AUTHENT:
			irqEn = 0x12U;
			waitIRq = 0x10U;            
			break;
		case PCD_TRANSCEIVE:         
			irqEn = 0x77U;
			waitIRq = 0x30U;            
			break;
		default:
			break;
	}	
	RC522_WriteRegister(RC522_REG_COMM_IE_N, irqEn | 0x80U);
	RC522_ClearBitMask(RC522_REG_COMM_IRQ, 0x80U);
	RC522_SetBitMask(RC522_REG_FIFO_LEVEL, 0x80U);
	RC522_WriteRegister(RC522_REG_COMMAND, PCDLE);

	for (i = 0U; i < sendLen; i++) 
    {   
		RC522_WriteRegister(RC522_REG_FIFO_DATA, sendData[i]);   //Writing data to the FIFO
	}

	RC522_WriteRegister(RC522_REG_COMMAND, command);            //Execute the command
    
	if (command == PCD_TRANSCEIVE) 
    {    
		RC522_SetBitMask(RC522_REG_BIT_FRAMING, 0x80U);     //StartSend=1,transmission of data starts  
    }
    /**
    *   Waiting to receive data to complete
    */
	i = 200U;	//i according to the clock frequency adjustment, the operator M1 card maximum waiting time_m 25ms???
    
	do {
        /**
        *   CommIrqReg[7..0]
        *   Set1 TxIRq RxIRq IdleIRq HiAlerIRq LoAlertIRq ErrIRq TimerIRq
        */
//        if (eComState == COM_PACKET_RECEIVED) RS485_Service();
		n = RC522_ReadRegister(RC522_REG_COMM_IRQ);
		--i;
	} 
    while ((i != 0U) && ((n & 0x01U) == 0U) && ((n & waitIRq) == 0U));          // End of do...while loop            
    /**
    *   StartSend=0
    */
	RC522_ClearBitMask(RC522_REG_BIT_FRAMING, 0x80U);

	if (i != 0U)  
    {
		if ((RC522_ReadRegister(RC522_REG_ERROR) & 0x1BU) == 0U) 
        {
			status = MI_OK;
            
			if (n & irqEn & 0x01U) status = MI_NOTAGERR;
			if (command == PCD_TRANSCEIVE) 
            {
				n = RC522_ReadRegister(RC522_REG_FIFO_LEVEL);
				lastBits = (RC522_ReadRegister(RC522_REG_CONTROL) & 0x07U);
				if (lastBits != 0U) *backLen = ((n - 1U) * 8U + lastBits);  
                else *backLen = (n * 8U);  
				if (n == 0U) n = 1U;
				if (n > RC522_BSIZE) n = RC522_BSIZE;   
				/**
                *   Reading the received data in FIFO
                */
				for (i = 0U; i < n; i++) 
                {
					backData[i] = RC522_ReadRegister(RC522_REG_FIFO_DATA);
				}
			}
		} 
        else status = MI_ERR;
	}
	return (status);
}
/**
  * @brief
  * @param
  * @retval
  */
static RC522_StatusTypeDef RC522_Anticoll(uint8_t* serNum) 
{	
	uint32_t bcnt;
    uint16_t blen;
	uint8_t snum = 0U;
	RC522_StatusTypeDef status;

	RC522_WriteRegister(RC522_REG_BIT_FRAMING, 0U);   // TxLastBists = BitFramingReg[2..0]
	serNum[0] = PICC_ANTICOLL;
	serNum[1] = 0x20U;
	status = RC522_ToCard(PCD_TRANSCEIVE, serNum, 2U, serNum, &blen);

	if (status == MI_OK) 
    {
		/**
        *   Check card serial number
        */
		for (bcnt = 0U; bcnt < 4U; bcnt++) 
        {
			snum ^= serNum[bcnt];
		}        
		if (snum != serNum[bcnt]) status = MI_ERR;
	}
	return (status);
}
/**
  * @brief
  * @param
  * @retval
  */
static void RC522_CalculateCRC(uint8_t*  pIndata, uint8_t len, uint8_t* pOutData)
{ 
	uint32_t i, n;

	RC522_ClearBitMask(RC522_REG_DIV_IRQ, 0x04U);       // CRCIrq = 0
	RC522_SetBitMask(RC522_REG_FIFO_LEVEL, 0x80U);      // Clear the FIFO pointer
//	RC522_WriteRegister(RC522_REG_COMMAND, PCDLE);    
	/**
    *   Write_RC522(CommandReg, PCDLE);
    *   Writing data to the FIFO
    */
	for (i = 0U; i < len; i++) 
    {
		RC522_WriteRegister(RC522_REG_FIFO_DATA, *(pIndata + i)); 
	}
    /**
	*   Start CRC calculation
    */
	RC522_WriteRegister(RC522_REG_COMMAND, PCD_CALCCRC);
    /**
	*   Wait for CRC calculation to complete
    */
	i = 0xFFU;
    
	do 
    {
//        if (eComState == COM_PACKET_RECEIVED) RS485_Service();
        n = RC522_ReadRegister(RC522_REG_DIV_IRQ);
		--i;
	} 
    while ((i != 0U) && ((n & 0x04U) == 0U));              // wait for CRCIrq = 1
	
//	RC522_WriteRegister(RC522_REG_COMMAND, PCDLE);
	/**
    *   Read CRC calculation result
    */
	pOutData[0] = RC522_ReadRegister(RC522_REG_CRC_RESULT_L);
	pOutData[1] = RC522_ReadRegister(RC522_REG_CRC_RESULT_M);
}
/**
  * @brief
  * @param
  * @retval
  */
static uint8_t RC522_SelectTag(uint8_t* serNum) 
{
	uint32_t bcnt;
	uint16_t recb;
	uint8_t buffer[9], size;
    RC522_StatusTypeDef status;

	buffer[0] = PICC_SELECTTAG;
	buffer[1] = 0x70U;
    
	for (bcnt = 0U; bcnt < 5U; bcnt++) 
    {
		buffer[bcnt + 2U] = *(serNum + bcnt);
	}    
	RC522_CalculateCRC(buffer, 7U, &buffer[7]);		//??
	status = RC522_ToCard(PCD_TRANSCEIVE, buffer, 9U, buffer, &recb);
	if ((status == MI_OK) && (recb == 0x18U)) size = buffer[0]; 
    else size = 0U;
	return (size);
}
/**
  * @brief
  * @param
  * @retval
  */
static RC522_StatusTypeDef RC522_Auth(uint8_t authMode, uint8_t BlockAddr, uint8_t* Sectorkey, uint8_t* serNum) 
{
	uint32_t bcnt;
    uint16_t recb;
	uint8_t buff[12];
    RC522_StatusTypeDef status;
    
	//Verify the command block address + sector + password + card serial number
	buff[0] = authMode;
	buff[1] = BlockAddr;
    
	for (bcnt = 0U; bcnt < 6U; bcnt++) 
    { 
		buff[bcnt + 2U] = *(Sectorkey + bcnt); 
	}
    
	for (bcnt = 0U; bcnt < 4U; bcnt++) 
    {
		buff[bcnt + 8U] = *(serNum + bcnt);
	}
    
	status = RC522_ToCard(PCD_AUTHENT, buff, 12U, buff, &recb);
	if ((status != MI_OK) || ((RC522_ReadRegister(RC522_REG_STATUS2) & 0x08U) == 0U)) status = MI_ERR;
	return (status);
}


static RC522_StatusTypeDef RC522_Read(uint8_t blockAddr, uint8_t* recvData) 
{
	RC522_StatusTypeDef status;
	uint16_t unLen;
    uint8_t sendData[8];
	sendData[0] = PICC_READ;
	sendData[1] = blockAddr;
	RC522_CalculateCRC(sendData, 2U, &sendData[2]);
	status = RC522_ToCard(PCD_TRANSCEIVE, sendData, 4U, recvData, &unLen);
	if ((status != MI_OK) || (unLen != 0x90U)) status = MI_ERR;
	return (status);
}
/**
  * @brief
  * @param
  * @retval
  */
static void RC522_Halt(void) 
{
	uint16_t unLen;
	uint8_t buff[4]; 

	buff[0] = PICC_HALT;
	buff[1] = 0U;
	RC522_CalculateCRC(buff, 2U, &buff[2]);
	RC522_ToCard(PCD_TRANSCEIVE, buff, 4U, buff, &unLen);
    RC522_ClearBitMask(0x08U, 0x08U);
}


static RC522_StatusTypeDef RC522_ReadCard(void)
{
    uint8_t str[RC522_BSIZE];
    RC522_StatusTypeDef status;
	
    status = RC522_Request(PICC_REQIDL, str);	
    status += RC522_Anticoll(str);
    mem_copy(card_id, str, 5U);
    RC522_SelectTag(card_id);	
	
    if (!RC522_Auth(PICC_AUTHENT1A, SECTOR_0, mifare_keya, card_id) && !status)
    {
        status  = RC522_Read((SECTOR_0 + 1U), &sector_0.block_1[0]);
        status += RC522_Read((SECTOR_0 + 2U), &sector_0.block_2[0]);
        if (!RC522_Auth(PICC_AUTHENT1A, SECTOR_1, mifare_keya, card_id) && !status)
        {
            status  = RC522_Read((SECTOR_1 + 0U), &sector_1.block_0[0]);
            status += RC522_Read((SECTOR_1 + 1U), &sector_1.block_1[0]);
            if (!RC522_Auth(PICC_AUTHENT1A, SECTOR_2, mifare_keya, card_id) && !status)
            {
                status  = RC522_Read((SECTOR_2 + 0U), &sector_2.block_0[0]);
            }
        }
	}
    RC522_Halt();
	return (status);
}
/**
  * @brief
  * @param
  * @retval
  */
static RC522_StatusTypeDef RC522_VerifyData(void)
{
	uint8_t b_cnt, ee_buf[16];
    
    mem_copy(sCard.card_id, card_serial, 5U);
    /**
	*			U S E R S  G R O U P   C H E C K
	**/	
	ee_buf[0] = (EE_USRGR_ADD >> 8);
	ee_buf[1] = (EE_USRGR_ADD & 0xFFU);                                             
	if (HAL_I2C_Master_Transmit(&hi2c1, I2CEE_ADD, ee_buf, 0x2U, DRV_TOUT) != HAL_OK)  ErrorHandler(RC522_FUNC,  I2C_DRV);
	if (HAL_I2C_Master_Receive (&hi2c1, I2CEE_ADD, ee_buf,  16U, DRV_TOUT) != HAL_OK)  ErrorHandler(RC522_FUNC,  I2C_DRV);
	sCard.user_group = USERGRP_INVALID;
    b_cnt = 0U;
	do
	{
        if (chk_chr((char*)ee_buf, sector_1.block_0[b_cnt]) == 0U)
        {
            if((sector_1.block_0[b_cnt] == 0U) || (sector_1.block_0[b_cnt] == 0xFFU))
            {
                sCard.user_group = USERGRP_DATA_INVALID;
            }
            else
            {
                sCard.user_group = sector_1.block_0[b_cnt];
                b_cnt = 16U;
            }                
        }
        ++b_cnt;
	}
    while(b_cnt < 16U);
	/**
	*			S Y S T E M   I D   C H E C K
	**/
    if (!ISVALIDDEC(sector_1.block_1[0])) sCard.system_id = SYSTEMID_DATA_INVALID;
	else 
    {
        sCard.system_id = Str2Int((char*)sector_1.block_1, 5U);
        if (sCard.system_id != (((system_id[0]) << 8) | system_id[1])) sCard.system_id = SYSTEMID_INVALID;
    }
    /**
	*			C A R D   E X P I R Y   T I M E    C H E C K
	**/
    for(b_cnt = 0U; b_cnt < 6U; b_cnt++)
    {
        if (!ISVALIDBCD(sector_2.block_0[b_cnt]))  sCard.expiry_time[0] = EXPIRYTIME_DATA_INVALID;
    }    
    if (sCard.expiry_time[0] == EXPIRYTIME_DATA_INVALID) sCard.card_status = CARDID_INVALID;
	else if((sector_2.block_0[2] > rdate.Year)
    || ((sector_2.block_0[2] == rdate.Year) && (sector_2.block_0[1] > rdate.Month))
    || ((sector_2.block_0[2] == rdate.Year) && (sector_2.block_0[1] == rdate.Month) && (sector_2.block_0[0] > rdate.Date))
    || ((sector_2.block_0[2] == rdate.Year) && (sector_2.block_0[1] == rdate.Month) && (sector_2.block_0[0] == rdate.Date) && (sector_2.block_0[3] > rtime.Hours))
    || ((sector_2.block_0[2] == rdate.Year) && (sector_2.block_0[1] == rdate.Month) && (sector_2.block_0[0] == rdate.Date) && (sector_2.block_0[3] == rtime.Hours)
    &&  (sector_2.block_0[4] >= rtime.Minutes)))
    {
        mem_copy(sCard.expiry_time, sector_2.block_0, 6U);
        
    }
    else sCard.expiry_time[0] = EXPIRYTIME_INVALID;
    /**
	*			C O N T R O L L E R    A D D R E S S   C H E C K
	**/
    sCard.controller_id = (sector_2.block_0[6] << 8)|sector_2.block_0[7];
	if      ((sCard.controller_id < FST_DEV_RSIFA) || (sCard.controller_id > LST_DEV_RSIFA)) sCard.controller_id = ROOMADDR_DATA_INVALID;
	else if  (sCard.controller_id != ((rs485_ifa[0] << 8)|rs485_ifa[1]))
	{
        sCard.controller_id = ROOMADDR_INVALID;
        for(b_cnt = 0; b_cnt < 8; b_cnt++)
        {
            if ((sector_2.block_0[6] == permitted_add[b_cnt][0]) && (sector_2.block_0[7] == permitted_add[b_cnt][1])) 
            {
                sCard.controller_id = (((sector_2.block_0[6] << 8) & 0xFF00U) | sector_2.block_0[7]);
                b_cnt = 0x8U;
            }
        }
	}
    /**
	*			G A R A G E   E N A B L E D
	**/
    if (sCard.user_group != USERGRP_SERVICER) 
    {
        if  (CARD_USER_INVALIDITY_ADD != 'I') sCard.user_group = USERGRP_INVALID;
    }
	/**
	*			S E T   C A R D     S T A T U S
	**/
	if ((sCard.user_group       == USERGRP_INVALID)
    ||  (sCard.user_group       == USERGRP_DATA_INVALID)
    ||  (sCard.expiry_time[0]   == EXPIRYTIME_DATA_INVALID)
    ||  (sCard.controller_id    == ROOMADDR_INVALID)
    ||  (sCard.controller_id    == ROOMADDR_DATA_INVALID)
    ||  (sCard.system_id        == SYSTEMID_INVALID)
    ||  (sCard.system_id        == SYSTEMID_DATA_INVALID))
    {
		 sCard.card_status = CARDID_INVALID;
    }
    else if (sCard.expiry_time[0] == EXPIRYTIME_INVALID) sCard.card_status = EXPIRYTIME_INVALID;
    else  sCard.card_status = CARD_VALID;
	/**
	*			C H E C K   I S   R E S E T   C A R D   
	**/
    if (mem_cmp(card_serial, reset_card_serial, 5) == 0x0U)
    {
        LogEvent.log_event = SOFTWARE_RESET;
        LOGGER_Write();
        BootloaderExe();
    }
	return (MI_OK);
}
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
