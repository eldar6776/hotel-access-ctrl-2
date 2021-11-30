/**
 ******************************************************************************
 * File Name          : display.c
 * Date               : 10.3.2018
 * Description        : GUI display module
 ******************************************************************************
 *
 *  display refresh 30 min
 *
 *
 *
 */
 
#if (__DISP_H__ != FW_BUILD)
    #error "display header version mismatch"
#endif 
/* Includes ------------------------------------------------------------------*/
#include "display.h"
#include "logger.h"
#include "eeprom.h"
#include "rc522.h"
#include "rs485.h"
#include "owire.h"
#include "room.h"
#include "dio.h"
//#include "rf24.h"
/* Imported Type  ------------------------------------------------------------*/
/* Imported Variable  --------------------------------------------------------*/
/* Imported Function  --------------------------------------------------------*/
/* Private Type --------------------------------------------------------------*/
/* Private Define ------------------------------------------------------------*/
///** ==========================================================================*/
///**       I L I 9 3 4 1	  C O N T R O L L E R 		C O M M A N D	        */
///** ==========================================================================*/
#define ILI9341_NOP                                 0x00U  
#define ILI9341_RD_DISP_POWER_MODE                  0x0AU
#define ILI9341_SLEEP_OUT                           0x11U
#define ILI9341_GAMMA_SET                           0x26U
#define ILI9341_DISP_ON                             0x29U
#define ILI9341_COLUMN_ADDR_SET                     0x2AU
#define ILI9341_PAGE_ADDR_SET                       0x2BU
#define ILI9341_MEMORY_WRITE                        0x2CU
#define ILI9341_MEMORY_ACCESS_CONTROL               0x36U
#define ILI9341_PIXEL_FORMAT_SET                    0x3AU
#define ILI9341_FRAME_RATE_CTRL                     0xB1U
#define ILI9341_DISP_FUNCTION_CTRL                  0xB6U
#define ILI9341_POWER_CTRL_1                        0xC0U
#define ILI9341_POWER_CTRL_2                        0xC1U
#define ILI9341_VCOM_CTRL_1                         0xC5U
#define ILI9341_VCOM_CTRL_2                         0xC7U
#define ILI9341_POSITIVE_GAMMA_CORRECTION           0xE0U
#define ILI9341_NEGATIVE_GAMMA_CORRECTION           0xE1U
/** ==========================================================================*/
/**		  I L I 9 3 4 1		  D I S P L A Y		  C O N S T A N T S 		  */
/** ==========================================================================*/
#define LCD_W 320U
#define LCD_H 240U
/** ==========================================================================*/
/**		  I L I 9 3 4 1			  D I S P L A Y			C O L O R S	 	  	  */
/** ==========================================================================*/
#define WHITE				0xFFFFU
#define BLACK				0x0000U
#define BLUE				0x001FU
#define BRED				0xF81FU
#define GRED				0xFFE0U
#define GBLUE				0x07FFU
#define RED					0xF800U
#define MAGENTA				0xF81FU
#define GREEN				0x07E0U
#define CYAN				0x7FFFU
#define YELLOW				0xFFE0U
#define BROWN				0xBC40U
#define BRRED				0xFC07U
#define GRAY				0x8430U
#define DARKBLUE			0x01CFU
#define LIGHTBLUE			0x7D7CU
#define GRAYBLUE			0x5458U
#define LIGHTGREEN			0x841FU
#define LGRAY				0xC618U
#define LGRAYBLUE			0xA651U
#define LBBLUE          	0x2B12U
/** ==========================================================================*/
/**	I L I 9 3 4 1	D I S P L A Y    F O N T	A N D	 T E X T	L I N E   */
/** ==========================================================================*/
/*	smal font number of horisontal text lines	*/
#define Line0_S       		0U
#define Line1_S      		16U
#define Line2_S      		32U
#define Line3_S     		48U
#define Line4_S      		64U
#define Line5_S     		80U
#define Line6_S     		96U
#define Line7_S   			112U
#define Line8_S 			128U
#define Line9_S				144U
#define Line10_S			160U
#define Line11_S			176U
#define Line12_S			192U
#define Line13_S			208U
#define Line14_S			224U
/*	middle font number of horisontal text lines	*/
#define Line0_M       		0U
#define Line1_M      		20U
#define Line2_M      		40U
#define Line3_M     		60U
#define Line4_M      		80U
#define Line5_M     		100U
#define Line6_M     		120U
#define Line7_M   			140U
#define Line8_M 			160U
#define Line9_M				180U
#define Line10_M			200U
#define Line11_M			220U
/*	big font number of horisontal text lines	*/
#define Line0_B       		0U
#define Line1_B     		26U
#define Line2_B     		52U
#define Line3_B     		78U
#define Line4_B       		104U
#define Line5_B       		130U
#define Line6_B      		146U
#define Line7_B      		182U
#define Line8_B      		214U
#define Line9_B      		240U
#define Line10_B      		266U
#define Line11_B      		292U
#define Line12_B      		318U
/** ==========================================================================*/
/**	    D I S P L A Y    U S E R    I N T E R F A C E    C O N S T A N T S    */
/** ==========================================================================*/
#define DISP_BSIZE						    2048U
#define MSG_BSIZE					        32U
#define DISP_IMG_TIME						5432U 				    // 5 sec. image displayed time
#define DISP_STATUS_TIME                    2345U                   // status info display time
#define DISP_UPDATE_TIME                    333U	                // update info refresh time
#define IMAGE_SIZE						    153600U				    // 320x240 RGB656 formated image
#define IMAGE_DISP_POS				        0U, 0U, 319U, 239U
#define IMAGE_MINIBAR_SIZE                  2048U				    // minibar image size
#define IMAGE_MINIBAR_POSITION              0U,0U,31U,31U           // minibar icon position
#define IMAGE_MINIBAR_ROTATED_POSITION      288U, 208U, 319U, 239U
#define ROOM_NUMBER_IMAGE                   1U  // image index alias
#define DO_NOT_DISTURB_IMAGE			    2U  // image index alias
#define BEDDING_REPLACEMENT_IMAGE		    3U  // image index alias
#define CLEAN_UP_IMAGE					    4U  // image index alias
#define GENERAL_CLEAN_UP_IMAGE			    5U  // image index alias
#define CARD_VALID_IMAGE				    6U  // image index alias
#define CARD_INVALID_IMAGE				    7U  // image index alias
#define WRONG_ROOM_IMAGE				    8U  // image index alias
#define TIME_EXPIRED_IMAGE				    9U  // image index alias
#define FIRE_ALARM_IMAGE			        10U // image index alias
#define FIRE_EXIT_IMAGE                     11U // image index alias
#define MINIBAR_IMAGE                       12U // image index alias
#define ROOM_OUT_OF_SERVICE_IMAGE           13U // image index alias
#define SOS_ALARM_IMAGE                     14U // image index alias
/* Private Constante ---------------------------------------------------------*/
const char *label [9][1] = {"     OFF", "      ON", "ENABLED ", "DISABLED", " SET->ON", "SET->OFF", "LuxRT.x ", "DS18B.x ", "INACTIVE"};
const char *screen[5][1] = {"DIGITAL   INPUT   SETTINGS",   "DIGITAL  OUTPUT   SETTINGS",   "SETUP  SYSTEM  SETTINGS  1",   "SETUP  SYSTEM  SETTINGS  2", "EXTEND  ACCESS  PERMISSION"};
const char *item [8][4] = {{"CARD STACKER     ",            "POWER CONTACTOR  ",            "RESTART CONTROLER         ",   "DISPLAY ROTATION:         "},
                           {"SOS ALARM        ",            "DND & HM RESET   ",            "PREVIEW IMAGE             ",   "DISPLAY CARD DATA"},
                           {"SOS RESET        ",            "DOOR BELL        ",            "UPDATE IMAGE              ",   "RADIO MODUL      "},
                           {"HANDMAID CALL    ",            "DOOR BELL        ",            "UPDATE FIRMWARE           ",   "RADIO ADDRESS:            "},
                           {"MINIBAR SENSOR   ",            "HVAC POWER       ",            "SCAN OW BUS:              ",   "RADIO CHANEL:             "},
                           {"BALCONY DOOR     ",            "THERMOSTAT       ",            "RS485 ADDRESS:            ",   "BUZZER VOLUME:            "},
                           {"DND SWITCH       ",            "DOOR LOCK        ",            "SYSTEM ID:                ",   "DOORLOCK FORCE:           "},
                           {"ENTRY DOOR       ",            "PCB BUZZER       ",            "RTC TIME:                 ",   "LCD BRIGHTNESS:           "}};
const char *taster[3][1] = {"ENTER              >> NEXT",   "NEXT >>             SELECT",   "SET ++            DESELECT"};
/* Private Variable ----------------------------------------------------------*/
uint32_t disp_fl;

uint16_t lcd_bcklght;
uint16_t disp_width, disp_high;
uint16_t back_color, pixel_color;

uint8_t spi_buff[8];
uint8_t disp_sta;
uint8_t disp_rot;
uint8_t jrnl_mod;
uint8_t jrnl_buf[JRNL_BSIZE];

sMenuTypeDef menu;
FONT_SizeTypeDef font_size;
/* Private Macro -------------------------------------------------------------*/
/* Private Function Prototype ------------------------------------------------*/
static void DISP_PrintNumber(uint16_t x, uint16_t y, uint32_t num, uint8_t len);
static void DISP_AddressSet(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
static void DISP_PrintCharacter(uint16_t x, uint16_t y, uint8_t num);
static void DISP_PrintString(uint16_t x, uint16_t y, const char *p);
static void DISP_ProcessMenuTasterEvent(sMenuTypeDef* menu);
static void DISP_PrintMenuScreen(sMenuTypeDef* menu);
static void DISP_PrintMenuValue(sMenuTypeDef* menu);
static void DISP_MenuService(sMenuTypeDef* menu);
static void DISP_InitMenu(sMenuTypeDef* menu);
static void DISP_WriteRegister(uint8_t data);
static void DISP_Image(uint8_t selected);
static void DISP_WriteByte(uint8_t data);
static void DISP_WriteInt(uint16_t data);
static void DISP_Clear(uint16_t color);
void DISP_SetOrientation(void);
void DISP_CardUserName(void);
void DISP_Temperature(void);
void DISP_DateTime(void);
void DISP_CheckLcd(void);
/* Program Code  -------------------------------------------------------------*/
/*************************************************************************/
/**     I N I T I A L I Z E     L C D       C O N T R O L L E R         **/
/*************************************************************************/
void DISP_Init(void)
{
	HAL_GPIO_WritePin(DISP_CS_Port, DISP_CS_Pin, GPIO_PIN_SET);
    DISP_RST_SetHigh();
    DOUT_Service();
    HAL_Delay(5U);
    DISP_RST_SetLow();
    DOUT_Service();
    HAL_Delay(50U);
    DISP_RST_SetHigh();
    DOUT_Service();
    HAL_Delay(200U);
    DISP_WriteRegister(0xEFU);
    DISP_WriteByte(0x03U); 
    DISP_WriteByte(0x80U); 
    DISP_WriteByte(0x02U);
    DISP_WriteRegister(0xCFU);
    DISP_WriteByte(0x00U); 
    DISP_WriteByte(0xC1U); 
    DISP_WriteByte(0x30U);
    DISP_WriteRegister(0xEDU);
    DISP_WriteByte(0x64U); 
    DISP_WriteByte(0x03U); 
    DISP_WriteByte(0x12U);
    DISP_WriteByte(0x81U);
    DISP_WriteRegister(0xE8U);
    DISP_WriteByte(0x85U); 
    DISP_WriteByte(0x00U); 
    DISP_WriteByte(0x78U);
    DISP_WriteRegister(0xCBU);
    DISP_WriteByte(0x39U); 
    DISP_WriteByte(0x2CU); 
    DISP_WriteByte(0x00U);
    DISP_WriteByte(0x34U); 
    DISP_WriteByte(0x02U);
    DISP_WriteRegister(0xF7U);
    DISP_WriteByte(0x20U);
    DISP_WriteRegister(0xEAU);
    DISP_WriteByte(0x00U); 
    DISP_WriteByte(0x00U);
    DISP_WriteRegister(ILI9341_POWER_CTRL_1);
    DISP_WriteByte(0x23U);
    DISP_WriteRegister(ILI9341_POWER_CTRL_2);
    DISP_WriteByte(0x10U);
    DISP_WriteRegister(ILI9341_VCOM_CTRL_1);
    DISP_WriteByte(0x3EU); 
    DISP_WriteByte(0x28U);
    DISP_WriteRegister(ILI9341_VCOM_CTRL_2);
    DISP_WriteByte(0x86);
    DISP_SetOrientation();
    DISP_WriteRegister(ILI9341_PIXEL_FORMAT_SET);
    DISP_WriteByte(0x55U);
    DISP_WriteRegister(ILI9341_FRAME_RATE_CTRL);
    DISP_WriteByte(0x00U); 
    DISP_WriteByte(0x18U);
    DISP_WriteRegister(ILI9341_DISP_FUNCTION_CTRL);
    DISP_WriteByte(0x08U); 
    DISP_WriteByte(0x82U); 
    DISP_WriteByte(0x27U);
    DISP_WriteRegister(0xF2U);
    DISP_WriteByte(0x00U);
    DISP_WriteRegister(ILI9341_GAMMA_SET);
    DISP_WriteByte(0x01U);
    DISP_WriteRegister(ILI9341_POSITIVE_GAMMA_CORRECTION);
    DISP_WriteByte(0x0FU);
    DISP_WriteByte(0x31U);
    DISP_WriteByte(0x2BU);
    DISP_WriteByte(0x0CU); 
    DISP_WriteByte(0x0EU);
    DISP_WriteByte(0x08U);
    DISP_WriteByte(0x4EU);
    DISP_WriteByte(0xF1U);
    DISP_WriteByte(0x37U);
    DISP_WriteByte(0x07U);
    DISP_WriteByte(0x10U);
    DISP_WriteByte(0x03U);
    DISP_WriteByte(0x0EU);
    DISP_WriteByte(0x09U);
    DISP_WriteByte(0x00U);
    DISP_WriteRegister(ILI9341_NEGATIVE_GAMMA_CORRECTION);
    DISP_WriteByte(0x00U);
    DISP_WriteByte(0x0EU);
    DISP_WriteByte(0x14U);
    DISP_WriteByte(0x03U);
    DISP_WriteByte(0x11U);
    DISP_WriteByte(0x07U);
    DISP_WriteByte(0x31U);
    DISP_WriteByte(0xC1U); 
    DISP_WriteByte(0x48U);
    DISP_WriteByte(0x08U);    
    DISP_WriteByte(0x0FU);
    DISP_WriteByte(0x0CU);
    DISP_WriteByte(0x31U);
    DISP_WriteByte(0x36U); 
    DISP_WriteByte(0x0FU);
    DISP_WriteRegister(ILI9341_SLEEP_OUT); 
    HAL_Delay(200U);
    DISP_WriteRegister(ILI9341_DISP_ON);
    /*************************************************************************/
    /**     D I S P L A Y       S Y S T E M         S E T T I N G S         **/
    /*************************************************************************/ 
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, lcd_bcklght);
    DISP_Clear(BLACK);
    back_color = BLACK;
    pixel_color = WHITE;   
    font_size = MIDDLE_FONT;
    if(disp_rot & 0x1U) 
    {
        DISP_PrintString(30U, Line1_M, "JUBERA d.o.o. Sarajevo");
        font_size = BIG_FONT;
        DISP_PrintString(70U, 65, "___ LUX ___");
        font_size = MIDDLE_FONT;
        DISP_PrintString(30U, Line6_M, "Room Access Controller");
    }
    else
    {
        DISP_PrintString(0x0U, Line1_M, "    JUBERA d.o.o.");
        DISP_PrintString(0x0U, Line2_M, "      Sarajevo");
        font_size = BIG_FONT;
        DISP_PrintString(0x0U, Line4_M, "_____ LUX _____");
        font_size = MIDDLE_FONT;
        DISP_PrintString(0x0U, Line6_M, "    Room Access");
        DISP_PrintString(0x0U, Line7_M, "    Controller");
    }
    DISP_RoomNumImgSet();
    HAL_Delay(1500U);
#ifdef	USE_WATCHDOG
    HAL_IWDG_Refresh(&hiwdg);
#endif
    HAL_Delay(1500U);
#ifdef	USE_WATCHDOG
    HAL_IWDG_Refresh(&hiwdg);
#endif
}
/*************************************************************************/
/**         D I S P L A Y           U S E R          G U I              **/
/*************************************************************************/ 
/**
  * @brief
  * @param
  * @retval
  */
void DISP_Service(void) 
{
    uint32_t char_cnt, t;
    uint8_t disp_buff[MSG_BSIZE];
    uint32_t row_cnt, colon_cnt;
    uint32_t space_cnt, item_cnt;
    uint32_t char_x_pos, char_y_pos;
    static uint32_t jrn_char = 0x0U;
    static uint32_t display_time = 0x0U;
    static uint32_t display_timer = 0x0U;
	static uint32_t preview_img_cnt = 0x0U;
    static uint32_t disp_menu_toutmr = 0x0U;
    static uint32_t disp_menu_toutime = 0x0U;
    /*************************************************************************/
    /**		    E N T R Y       C O N D I T I O N S     C H E C K           **/
    /*************************************************************************/	
	if (eComState == COM_PACKET_RECEIVED) RS485_Service();
    if (IsDISP_ImgUpdActiv()) DISP_imgUpdReset();
    else if ((HAL_GetTick() - display_timer) < display_time) return;
    display_timer = HAL_GetTick();
    ZEROFILL(disp_buff, COUNTOF(disp_buff));
    display_time = 0;
    DISP_CheckLcd();
    /*************************************************************************/
    /**     D I S P L A Y       U P D A T E         I N F O                 **/
    /*************************************************************************/
    if      (IsRS485_UpdateActiv())
    {
        if(!IsDISP_UpdProgMsgActiv())
        {
            DISP_UpdProgMsgSet();          // run only once per call
            display_time = DISP_STATUS_TIME;         // set time to display update info message 
            back_color = BLACK;
            if (disp_rot & 1) font_size = BIG_FONT, t = Line8_B, DISP_PrintString(0, 188U, "      ");    // clear temp_mv displayed area  
            else font_size = MIDDLE_FONT, t = 290;
                      
            if(IsDISP_BldrUpdSetActiv() || IsDISP_BldrUpdFailActiv())
            {
                if      (IsDISP_BldrUpdSetActiv()) pixel_color = GREEN, DISP_PrintString(0, t, "Bootloader Updated  ");
                else if(IsDISP_BldrUpdFailActiv()) pixel_color = RED,   DISP_PrintString(0, t, "Bootloader Failed   ");                
                DISP_BldrUpdReset();
                DISP_BldrUpdFailReset();					
                DISP_UpdProgMsgDel();
                DISP_RefreshSet();                                                          
                RS485_StopUpdate();
            }
            else
            {
                pixel_color = YELLOW,                       DISP_PrintString(0,   t, "UPDATE:             ");
                if      (activ_cmd == DWNLD_FWR_IMG)        DISP_PrintString(140, t, "FIRMWARE");
                else if (activ_cmd == DWNLD_BLDR_IMG)       DISP_PrintString(140, t, "BOOTLDR.");
                else if((activ_cmd >= DWNLD_DISP_IMG_1) && (activ_cmd <= DWNLD_DISP_IMG_25))
                {
                    DISP_PrintString(140, t,  "IMAGE");
                    DISP_PrintNumber(260, t, (activ_cmd - 0x63U), 2);
                }
            } 
        }
        return;
    }
    /*************************************************************************/
    /**     D I S P L A Y   F I L E   T R A N S F E R   S T A T U S         **/
    /*************************************************************************/
    else if (IsDISP_UpdProgMsgActiv())
    {
        DISP_UpdProgMsgDel();       // run only once per call
        display_time = DISP_STATUS_TIME;            // set time to display update info message 
        DISP_RefreshSet();                          // reload last display image after this time expire
        back_color = BLACK;
        pixel_color = GREEN;
        if (disp_rot & 0x1U) font_size = BIG_FONT, t = Line8_B, DISP_PrintString(0x0U, 188U, "      ");    // clear temp_mv displayed area  
        else font_size = MIDDLE_FONT, t = 290;           
        if      (IsSYS_FileTransferSuccessSet())    pixel_color = GREEN,DISP_PrintString(0, t, "Transfer Successful ");
        else if (IsSYS_FileTransferFailSet())       pixel_color = RED,  DISP_PrintString(0, t, "Transfer Fail       ");
        return;
    }
    /*************************************************************************/
    /**     D I S P L A Y   F I R M W A R E   U P D A T E   S T A T U S     **/
    /*************************************************************************/
    else if (IsDISP_FwrUpddActiv() || IsDISP_FwrUpdFailActiv())
    {
        back_color = BLACK;
        if (disp_rot & 0x1U) font_size = BIG_FONT, t = Line8_B, DISP_PrintString(0, 188U, "      ");    // clear temp_mv displayed area  
        else font_size = MIDDLE_FONT, t = 290;
        if      (IsDISP_FwrUpddActiv())     pixel_color = GREEN,DISP_PrintString(0, t, "Firmware Updated    ");
        else if (IsDISP_FwrUpdFailActiv())  pixel_color = RED,  DISP_PrintString(0, t, "Firmware Update Fail");        
        DISP_FwrUpdFailDelete();             // not needed to refresh display ,
        DISP_FwrUpddDelete();                // when time for this message expire,
        display_time = DISP_STATUS_TIME;             // new date & time info will override mesasge text 
        return;
    }
    /*************************************************************************/
    /**     D I S P L A Y   A L L   I M A G E   F R O M     F L A S H       **/
    /*************************************************************************/
    if      (IsDISP_PreviewImgActiv())
    {
        ++preview_img_cnt;                          // first image index is 1
        if(preview_img_cnt > SOS_ALARM_IMAGE)       // last image index is 14
        {
            preview_img_cnt = 0x0U;                   // all images,if any, are displayed from flash  memory
            DISP_PreviewImgDel();              // exit loop with index set to zero for next entrance
        }
        else 
        {
            DISP_Image(preview_img_cnt);            // display all image stored in flash memory 
            display_time = DISP_STATUS_TIME;        // set time to display image
            return;
        }
    }
    /*************************************************************************/
    /**         E V E N T   =   U S E R     C A R D     V A L I D           **/
    /*************************************************************************/   
    else if (IsDISP_CardValidImgActiv()) 
    {
        DISP_CardValidImgDel();                // first delete flag to disable reentrance if something goes wrong
        DISP_Image(CARD_VALID_IMAGE);               // now show flash memory content for image index
        display_time = DISP_IMG_TIME;               // set image diplay time
        /*************************************************************************/
        /**     D I S P L A Y   C A R D   I N F O   I F     E N A B L E D       **/
        /*************************************************************************/ 
        if(IsDISP_UserCardInfoTextEnabled()) 
        {
            DISP_CardUserName();
        }
        /*************************************************************************/
        /**     D I S P L A Y       U S E R     M E N U     O P T I O N S       **/
        /*************************************************************************/ 
        if(sCard.user_group == USERGRP_MAID)
        {
            font_size = MIDDLE_FONT;
            back_color = BLACK;
            pixel_color = YELLOW;
            DISP_PrintString(0, Line8_B, "JOURNAL");
            DISP_InitMenu(&menu);
            jrnl_mod = 1;
        }
        else if(sCard.user_group == USERGRP_GUEST)
        {
            DISP_InitMenu(&menu);
            jrnl_mod = 0x0U;
        }
        else if(sCard.user_group == USERGRP_SERVICER)
        {
            DISP_Clear(BLACK);
            back_color = BLACK;
            pixel_color = GREEN;
            font_size = MIDDLE_FONT;
            (disp_rot & 1) ? (t = Line8_B) : (t = 0);
            DISP_PrintString(0, t, "SETTINGS");
            pixel_color = WHITE;   
            font_size = SMALL_FONT;
            disp_buff[0] = 'R';
            disp_buff[1] = 'C';
            disp_buff[8] = NUL;
            disp_buff[10] = (version>>16);
            disp_buff[11] = (version>>8);
            disp_buff[12] = version;
            Hex2Str ((char*)&disp_buff[2], &disp_buff[10], 6);            
            DISP_PrintString(0, Line4_S, "Firmware:"),          DISP_PrintString(140, Line4_S,  (char*)disp_buff);
            DISP_PrintString(0, Line5_S, "System ID:"),         DISP_PrintNumber(140, Line5_S, ((system_id[0]<<8)|system_id[1]), 5);
            DISP_PrintString(0, Line6_S, "RS485 address:"),     DISP_PrintNumber(140, Line6_S, ((rs485_ifa[0]<<8)|rs485_ifa[1]), 5); 
            DISP_PrintString(0, Line7_S, "RS485 group:"),       DISP_PrintNumber(140, Line7_S, ((rs485_gra[0]<<8)|rs485_gra[1]), 5);
            DISP_PrintString(0, Line8_S, "RS485 broadcast:"),   DISP_PrintNumber(140, Line8_S, ((rs485_bra[0]<<8)|rs485_bra[1]), 5);
            DISP_PrintString(0, Line9_S, "RS485 baudrate:"),    DISP_PrintNumber(140, Line9_S, huart1.Init.BaudRate, 6); 
            DISP_PrintString(0, Line10_S,"OneWire bus:");    
            if (IsTempSenDS18Active())                          DISP_PrintString(140, Line10_S, "DS18B20");
            if (IsTempLuxRTActive())                            DISP_PrintString(140, Line10_S, "LuxM RT");
            else                                                DISP_PrintString(140, Line10_S, "no device");
            DISP_InitMenu(&menu);
            menu.state = PENDING;
            jrnl_mod = 0;
        }        
        RC522_ClearData();
        return;
    }
    /*************************************************************************/
    /**         E V E N T   =   U S E R     C A R D    I N V A L I D        **/
    /*************************************************************************/
    else if (IsDISP_CardInvalidImgActiv()) 
    {
        DISP_CardInvalidImgDel();
        DISP_Image(CARD_INVALID_IMAGE);
        display_time = DISP_IMG_TIME;
        DISP_InitMenu(&menu); 
        jrnl_mod = 0x0U;
        return;
    }
    /*************************************************************************/
    /**              D I S P L A Y      M E N U     S C R E E N             **/
    /*************************************************************************/
    else if (menu.state == PRINT)
    {
        disp_menu_toutime = MENU_TIMEOUT;
        disp_menu_toutmr = HAL_GetTick();
        DISP_PrintMenuScreen(&menu);
        DISP_PrintMenuValue(&menu);
        menu.state = ACTIV;
        return;
    }
    /*************************************************************************/
    /**     D I S P L A Y      S C R E E N         M E N U      V A L U E   **/
    /*************************************************************************/
    else if (menu.state == CHANGED)
    {
        disp_menu_toutime = MENU_TIMEOUT;
        disp_menu_toutmr = HAL_GetTick();
        DISP_PrintMenuValue(&menu);
        menu.state = ACTIV;
        return;
    }
    /*************************************************************************/
    /**     D I S P L A Y       M E N U     T I M E O U T       T I M E R   **/
    /*************************************************************************/
    if      (jrnl_mod)
    { 
        if((HAL_GetTick() - disp_menu_toutmr) >= disp_menu_toutime)
        {
            jrnl_mod = 0x0U;
            DISP_RefreshSet();    // destroy current display menu setting to null
        }
        else if((jrnl_mod == 0x2U) || (jrnl_mod == 0x4U))
        {
            font_size = MIDDLE_FONT;
            pixel_color = WHITE;
            char_cnt = 0x0U;
            row_cnt = 0x0U;
            colon_cnt = 0x0U;
            space_cnt = 0x0U;
            item_cnt = 0x0U;
            char_x_pos = 80U;
            char_y_pos = Line2_M;
            disp_menu_toutime = MENU_TIMEOUT;
            disp_menu_toutmr = HAL_GetTick();
            DISP_Clear(BLACK);
            DISP_PrintString( 60U,  Line0_M,  "CISC. ZAMJ. GEN. ZBIR");
            DISP_PrintString(0x0U,  Line8_M,  "HOTEL");
            DISP_PrintString(0x0U, Line10_M,  "--------------------------");
            DISP_PrintString(0x0U, Line11_M,  "NEXT >>               EXIT");
            
            for(t = 0x0U; t < 0x5U; t++)
            {
                DISP_PrintString (12U, Line2_M + (t * Line1_M), ".SP");
                if (jrnl_mod == 0x2U) DISP_PrintCharacter(0, Line2_M + (t * Line1_M), (t + '1'));
                else if (jrnl_mod == 0x4U) 
                {
                    DISP_PrintCharacter(0x0U, Line2_M + (t * Line1_M), (t + '6'));
                    if(t == 0x4U) DISP_PrintString(0, Line6_M, "10.S");
                }
            }            
            if (jrnl_mod == 0x4U) char_cnt = jrn_char;
            
            while((jrnl_buf[char_cnt] != 0x0U) && (jrnl_buf[char_cnt] != ';') && (item_cnt < (jrnl_mod * 12U)))
            {
                DISP_PrintCharacter(char_x_pos, char_y_pos, jrnl_buf[char_cnt]);
                char_x_pos += 12U;
                ++char_cnt;
                ++space_cnt;
                
                if(jrnl_buf[char_cnt] == ',')
                {
                    ++char_cnt;	
                    ++item_cnt;
                    ++row_cnt;
                    
                    if(row_cnt == 0x4U)
                    {			
                        row_cnt = 0x0U;
                        char_x_pos = 80U;
                        char_y_pos += 20U;
                        ++colon_cnt;
                        if(colon_cnt == 0x5U) char_y_pos += 20U;
                    }
                    else char_x_pos += (((0x4U - space_cnt) * 12U) + 15U);                
                    space_cnt = 0x0U;			
                }
            }            
            if(jrnl_mod == 0x2U) jrn_char = char_cnt;
            ++jrnl_mod;
            return;
        }
    }
    else if (menu.state != IDLE)
    {
        if((HAL_GetTick() - disp_menu_toutmr) >= disp_menu_toutime)
        {
            menu.state = IDLE;
            DISP_RefreshSet();    // destroy current display menu setting all to null
        }
        else
        {
            DISP_ProcessMenuTasterEvent(&menu);
            if(menu.event == TASTER_END)
            {
                DISP_MenuService(&menu);
                menu.event = TASTER_IDLE;
            }
            return;
        }
    }
    /*************************************************************************/
    /**   E V E N T   =   R O O M   O R   S Y S T E M   I D   I N V A L I D **/
    /*************************************************************************/
    else if (IsDISP_WrongRoomImgActiv()) 
    {
        DISP_Image(WRONG_ROOM_IMAGE);
        DISP_WrongRoomImgDel();
        display_time = DISP_IMG_TIME;
    }
    /*************************************************************************/
    /**         E V E N T   =   C A R D    T I M E      E X P I R E D       **/
    /*************************************************************************/
    else if (IsDISP_TimeExpiredImgActiv()) 
    {
        DISP_Image(TIME_EXPIRED_IMAGE);
        DISP_TimeExpiredImgDel();
        display_time = DISP_IMG_TIME;
    }
    /*************************************************************************/
    /**         E V E N T   =   R O O M   O U T   O F   S E R V I C E       **/
    /*************************************************************************/
    else if (IsDISP_OutOfSrvcImgActiv()) 
    {
        DISP_Image(ROOM_OUT_OF_SERVICE_IMAGE);
        DISP_OutOfSrvcImgReset();
        display_time = DISP_IMG_TIME;
    }
    /*************************************************************************/
    /**         D I S P L A Y       R E F R E S H       R E Q U E S T       **/
    /*************************************************************************/
    else if (IsDISP_RefreshRequested())
    {
        DISP_RefreshReset();        
        if      (IsDISP_SosAlarmImgActiv()) DISP_Image(SOS_ALARM_IMAGE);
        else if (IsDISP_FireExitImgActiv()) DISP_Image(FIRE_EXIT_IMAGE);
        else if (IsDISP_FireAlarmImgActiv())DISP_Image(FIRE_ALARM_IMAGE);
        else if (IsDISP_DndImgActiv())      DISP_Image(DO_NOT_DISTURB_IMAGE);
        else if (IsDISP_BeddRepImgActiv())  DISP_Image(BEDDING_REPLACEMENT_IMAGE);
        else if (IsDISP_CleanUpImgActiv())  DISP_Image(CLEAN_UP_IMAGE);
        else if (IsDISP_GenCleanImgActiv()) DISP_Image(GENERAL_CLEAN_UP_IMAGE);
        else if (IsDISP_RoomNumImgActiv())  DISP_Image(ROOM_NUMBER_IMAGE);
        
        if (IsDISP_MinibarUsedImgActiv() && !IsDISP_FireAlarmImgActiv()
        && !IsDISP_FireExitImgActiv()    && !IsDISP_SosAlarmImgActiv())
        {
            DISP_Image(MINIBAR_IMAGE);
        }
    }
    /*************************************************************************/
    /**         D I S P L A Y       R O O M         I N F O                 **/
    /*************************************************************************/
    else 
    {
        DISP_DateTime();
        DISP_Temperature();
    }
}
/**
  * @brief
  * @param
  * @retval
  */
static void DISP_MenuService(sMenuTypeDef* menu)
{
    uint32_t val;
    /*************************************************************************/
    /**     S E T   S E L E C T E D     M E N U     I T E M     V A L U E   **/
    /*************************************************************************/    
    if      (menu->select == 0x0U) // exit screen selected
    {
        if  (menu->index < 0x2U)         // force digital input or output state from service menu
        {
            /**
            *   disable dout forcing on exit from menu screen
            */
//            if (dout_0_7_rem & (0x1U << 0x8U)) dout_0_7_rem &= (~(0x1U << 0x8U));
//            DIN_ForcingDisabled();
        }
    }
    else if (menu->select == 0x1U) // screen selected or item value edit deselected
    {          
    }
    else if (menu->select == 0x2U) // screen item value edit mode selected
    {
        if      (menu->index  < 0x2U)   // force digital input or output state from service menu
        {
            if ((menu->increment < 0x2U) || (menu->increment > 0x5U)) menu->increment = 0x2U;
            menu->item_label[menu->item_index] = menu->increment;
            if      (menu->index == 0x0U)        // force digital input state from service menu
            {
//                for(t = 0x0U; t < 0x8U; t++)
//                {     
//                    if      (menu->item_label[t] == 0x2U) // set din to "ENABLED" state
//                    else if (menu->item_label[t] == 0x3U) // set din to "DISABLED" state
//                    else if (menu->item_label[t] == 0x4U) // set din to "ON" state
//                    else if (menu->item_label[t] == 0x5U) // set din to "OFF" state
//                }
                mem_cpy (din_cfg, &menu->item_label[0], 0x8U);
                EEPROM_Save(EE_DIN_CFG_ADD_1, &menu->item_label[0], 0x8U);
            }
            else if (menu->index == 0x1U)  // force digital output state from service menu
            {
                if      (menu->item_index == 0x2U) menu->item_label[0x3U] = menu->item_label[0x2U]; // door bell output two pin 26 i 24 
                else if (menu->item_index == 0x3U) menu->item_label[0x2U] = menu->item_label[0x3U]; // door bell output two pin 26 i 24
         
//                for (t = 0x0U; t < 0x8U; t++)
//                {      
//                    if      (menu->item_label[t] == 0x2U) // set dout to "ENABLED" state
//                    else if (menu->item_label[t] == 0x3U) // set dout to "DISABLED" state
//                    else if (menu->item_label[t] == 0x4U) // set dout to "ON" state
//                    else if (menu->item_label[t] == 0x5U) // set dout to "OFF" state
//                }
                mem_cpy (dout_cfg, &menu->item_label[0], 0x8U);
                EEPROM_Save(EE_DOUT_CFG_ADD_1, &menu->item_label[0], 0x8U);
            }
        }
        else if (menu->index == 0x2U)   // settings 1 menu state update
        {
            if      (menu->item_index == 0x0U)  // restart controller selected
            {
                BootloaderExe();
            }
            else if (menu->item_index == 0x1U)  // preview all display image selected
            {
                menu->state = IDLE;
                DISP_PreviewImage();
            }
            else if (menu->item_index == 0x2U)  // update all display image request
            {
                menu->state = IDLE;
                SYS_ImageUpdateRequestSet();
            }
            else if (menu->item_index == 0x3U)  // update controller firmware request
            {
                menu->state = IDLE;
                SYS_FwrUpdRequestSet();
            }
            else if (menu->item_index == 0x4U)  // scan OW bus
            {
                OW_ScanBus();
                menu->select = 0x1U;
                menu->item_type[4] = LABEL_ITEM;
                if (IsTempLuxRTActive())menu->item_label[4] = 0x6U;
                else                    menu->item_label[4] = 0x8U;
            }
            else if (menu->item_index == 0x5U)  // set rs485 interface address
            {
                if (menu->increment > 0x9U) menu->increment = 0x0U;
                menu->item_buff[menu->item_index][menu->value_index] = TOCHAR(menu->increment);
                val = Str2Int((char*)&menu->item_buff[menu->item_index][0], 0x0U);
                rs485_ifa[0] = ((val >> 0x8U) & 0xFFU);
                rs485_ifa[1] = (val & 0xFFU);
                EEPROM_Save(EE_RSIFA, rs485_ifa, 0x2U);
            }
            else if (menu->item_index == 0x6U)  // set system id
            {
                if (menu->increment > 0x9U) menu->increment = 0x0U;
                menu->item_buff[menu->item_index][menu->value_index] = TOCHAR(menu->increment);
                val = Str2Int((char*)&menu->item_buff[menu->item_index][0], 0x0U);
                system_id[0] = ((val >> 0x8U) & 0xFFU);
                system_id[1] = (val & 0xFFU);
                EEPROM_Save(EE_SYSID, system_id, 0x2U);
            }
            else if (menu->item_index == 0x7U)  // set date & time
            {
                if      (menu->value_index == 0x0U)       // edit hours
                {
                    if  (menu->increment > 23U) menu->increment = 0x0U;
                }
                else if (menu->value_index == 0x1U)  // edit minutes
                {
                    if  (menu->increment > 59U) menu->increment = 0x0U;
                }
                else if (menu->value_index == 0x2U)  // edit date
                {
                    if  (menu->increment > 31U) menu->increment = 0x1U;
                }
                else if (menu->value_index == 0x3U)  // edit month
                {
                    if  (menu->increment > 12U) menu->increment = 0x1U;
                }
                else if (menu->value_index == 0x4U)  // edit year
                {
                    if  (menu->increment > 99U) menu->increment = 18U;
                }
                menu->item_buff[7][menu->value_index] = Dec2Bcd(menu->increment);
                rtime.Hours   = menu->item_buff[7][0];
                rtime.Minutes = menu->item_buff[7][1];
                rdate.Date    = menu->item_buff[7][2];
                rdate.Month   = menu->item_buff[7][3];
                rdate.Year    = menu->item_buff[7][4];
                HAL_RTC_SetTime(&hrtc, &rtime, RTC_FORMAT_BCD);
                HAL_RTC_SetDate(&hrtc, &rdate, RTC_FORMAT_BCD);
                RtcTimeValidSet();
            }    
        }
        else if (menu->index == 0x3U)   // settings 2 menu state update
        {
            if      (menu->item_index == 0x0U)    // display orientation
            {
                if (menu->increment > 0x3U) menu->increment = 0x0U;
                Int2Str((char*)&menu->item_buff[0], menu->increment, 0x1U);
                disp_rot = menu->increment;
                DISP_SetOrientation();
                DISP_Clear(BLACK);
                EEPROM_Save(EE_DISP_ROTATION_ADD, &disp_rot, 0x1U);
            }
            else if (menu->item_index == 0x1U)   // display user data from card
            {
                if      (menu->increment  > 0x1U) menu->increment = 0x0U;
                if      (menu->increment == 0x0U) DISP_UserCardInfoTextDisable();
                else if (menu->increment == 0x1U) DISP_UserCardInfoTextEnable();
                menu->item_label[menu->item_index] = menu->increment;
                EEPROM_Save(EE_DISP_STATUS_ADD, &disp_sta, 0x1U);
            }
            else if (menu->item_index == 0x2U)   // radio modul
            {
//                if      (menu->increment  > 0x1U) menu->increment = 0x0U;
//                menu->item_label[menu->item_index] = menu->increment;
//                rf24cfg = menu->increment;
//                EEPROM_Save(EE_RADIO_CFG_ADD, &rf24cfg, 0x1U);
            }
            else if (menu->item_index == 0x3U)   // radio address
            {
//                if (menu->increment > 9U) menu->increment = 0x1U;
//                Int2Str((char*)&menu->item_buff[3][0], menu->increment, 0x1U);
//                rf24address = menu->increment;
//                EEPROM_Save(EE_RADIO_ADDRESS_ADD, &rf24address, 0x1U);
            }
            else if (menu->item_index == 0x4U)   // radio chanel
            {
//                if (menu->increment > 127U) menu->increment = 0x1U;
//                Int2Str((char*)&menu->item_buff[4][0], menu->increment, 0x3U);
//                rf24chanel = menu->increment;
//                EEPROM_Save(EE_RADIO_CHANEL_ADD, &rf24chanel, 0x1U);
            }
            else if (menu->item_index == 0x5U)   // set buzzer volume
            {
                if (menu->increment > 10U) menu->increment = 0x0U;
                Int2Str((char*)&menu->item_buff[5][0], menu->increment, 0x2U);
                buzzer_volume = menu->increment;
                EEPROM_Save(EE_BUZZER_VOLUME_ADD, &buzzer_volume, 0x1U);
            }
            else if (menu->item_index == 0x6U)   // set doorlock force
            {
                if (menu->increment > 10U) menu->increment = 0U;
                Int2Str((char*)&menu->item_buff[6][0], menu->increment, 0x2U);
                doorlock_force = menu->increment;
                EEPROM_Save(EE_DOORLOCK_FORCE_ADD, &doorlock_force, 0x1U);
            }
            else if (menu->item_index == 0x7U)   // lcd brightness
            {
                if      (menu->increment  > 0x9U) menu->increment = 0x1U;
                else if (menu->increment == 0x0U) menu->increment = 0x1U;
                lcd_bcklght = (menu->increment * 100U);
                Int2Str((char*)&menu->item_buff[7][0], lcd_bcklght, 0x3U);
                __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, lcd_bcklght);
                spi_buff[0] = (lcd_bcklght >> 0x8U);
                spi_buff[1] = (lcd_bcklght & 0xFFU);
                EEPROM_Save(EE_LCD_BRIGHTNESS, spi_buff, 0x2U);
            }
        }
        else if (menu->index == 0x4U)   // extended access permission
        {
            if (menu->increment > 0x9U) menu->increment = 0x0U;
            menu->item_buff[menu->item_index][menu->value_index] = TOCHAR(menu->increment);
            val = Str2Int((char*)&menu->item_buff[menu->item_index][0], 0U);
            permitted_add[menu->item_index][0] = ((val >> 0x8U) & 0xFFU);
            permitted_add[menu->item_index][1] = (val & 0xFFU);
            EEPROM_Save(EE_PERM_EXTADD1 + (menu->item_index * 0x2U), &permitted_add[menu->item_index][0], 0x2U);
        }
    }
}
/**
  * @brief
  * @param
  * @retval
  */
static void DISP_ProcessMenuTasterEvent(sMenuTypeDef* menu)
{
    /*************************************************************************/
    /**         M E N U          T A S T E R         E V E N T S            **/
    /*************************************************************************/
    if(menu->state == ACTIV)
    {
        /*************************************************************************/
        /**             S E T    M E N U    O N   L E F T  T A S T E R          **/
        /*************************************************************************/
        if      (menu->event == LEFT_TASTER_PRESSED)
        {
            Buzzer_On();
            DOUT_Buzzer();
            HAL_Delay(5);
            Buzzer_Off();
            DOUT_Buzzer();
            menu->event = TASTER_END;
            menu->state = CHANGED;
            
            if      (menu->select == 0x0U)       
            {
                menu->select = 0x1U;
                menu->item_index = 0x0U;      // menu item index start from 1, index 0 is menu screen label
                menu->value_index = 0x0U;     // 
                menu->value_edit = 0x0U;
                menu->increment = 0x0U;
            }
            else if (menu->select == 0x1U)
            {
                if(menu->value_edit == 0x1U)
                {
                    ++menu->value_index; 
                    
                    if(menu->value_index >= menu->item_to_edit[menu->item_index])
                    {
                        ++menu->item_index;
                        menu->value_edit = 0x0U;
                        menu->value_index = 0x0U;
                    }               
                }
                else ++menu->item_index;

                if(menu->item_index >= MAX_ITEMS_IN_SCREEN)
                {
                    menu->select = 0x0U;
                    menu->item_index = 0x0U;
                }
            }
            else if (menu->select == 0x2U) 
            {
                ++menu->increment;
            }
        }
        /*************************************************************************/
        /**             S E T    M E N U   O N   R I G H T   T A S T E R        **/
        /*************************************************************************/
        else if (menu->event == RIGHT_TASTER_PRESSED)
        {
            Buzzer_On();
            DOUT_Buzzer();
            HAL_Delay(5);
            Buzzer_Off();
            DOUT_Buzzer();
            menu->event = TASTER_END;
            menu->state = CHANGED;
            
            if      (menu->select == 0x0U)
            {
                menu->state = PRINT;
                if(++menu->index >= MAX_SCREENS_IN_MENU) menu->index = 0x0U;
            }
            else if (menu->select == 0x1U)
            {
                menu->select = 0x2U;
                if(menu->item_to_edit[menu->item_index] != 0x0U)  menu->value_edit = 0x1U;
            }
            else if (menu->select == 0x2U) 
            {
                menu->select = 0x1U;
            }
        }
    }
}
/**
  * @brief
  * @param
  * @retval
  */
static void DISP_PrintMenuScreen(sMenuTypeDef* menu)
{
    uint32_t t;
    
    if(disp_rot & 0x1U) font_size = MIDDLE_FONT;
    else                font_size = SMALL_FONT;
    back_color = BLACK;
    pixel_color = WHITE;
    DISP_Clear(BLACK);
    DISP_PrintString(0x0U, Line1_M, "--------------------------");
    DISP_PrintString(0x0U, Line10_M,"--------------------------");
    
    for(t = 0x0U; t < 0x8U; t++)
    {
        menu->item_label[t] = 0x0U;
        menu->item_to_edit[t] = 0x0U;
        menu->item_type[t] = LABEL_ITEM;
        ZEROFILL(menu->item_buff[t], BUFFERCNT(menu->item_buff[t]));
    }
        
    if      (menu->index == 0x0U) CharToBinStr((char*)menu->item_buff, din_0_7),    mem_cpy (&menu->item_label[0], din_cfg,  0x8U);
    else if (menu->index == 0x1U) CharToBinStr((char*)menu->item_buff, dout_0_7),   mem_cpy (&menu->item_label[0], dout_cfg, 0x8U);
    else if (menu->index == 0x2U) 
    {
        menu->item_type[0] = SELECT_ITEM;     // restart controller is screen value selectable value type
        menu->item_type[1] = SELECT_ITEM;     // preview image is screen value selectable value type
        menu->item_type[2] = SELECT_ITEM;     // image update is screen value selectable value type
        menu->item_type[3] = SELECT_ITEM;     // firmware update is screen value selectable value type
        menu->item_type[4] = SELECT_ITEM;     // scan OW bus is screen value selectable value type with otput valu type: VALUE_LUX_RTH, VALUE_DS18B20 + unit numper        
        menu->item_type[5] = VALUE_IN_BUFFER; // set rs485 address is VALUE_BUFFER type with menu value stored in value buffer
        menu->item_type[6] = VALUE_IN_BUFFER; // set system id is VALUE_BUFFER type with menu value stored in value buffer        
        menu->item_type[7] = VALUE_IN_BUFFER; // set date & time is VALUE_BUFFER type with menu value stored in value buffer
        
        menu->item_to_edit[5] = 0x5U;
        menu->item_to_edit[6] = 0x5U;
        menu->item_to_edit[7] = 0x5U;
        
        Int2Str((char*)&menu->item_buff[5][0], ((rs485_ifa[0] << 0x8U) | rs485_ifa[1]), 0x5U);  // 5 char rs485 interface address 00000~65535
        Int2Str((char*)&menu->item_buff[6][0], ((system_id[0] << 0x8U) | system_id[1]), 0x5U);  // 5 char system id value 00000 ~ 65535
        
        menu->item_buff[7][0] = rtime.Hours;    // BCD hours value 0x01~0x23
        menu->item_buff[7][1] = rtime.Minutes;  // BCD  minute value 0x00~0x59
        menu->item_buff[7][2] = rdate.Date;     // BCD  date value 0x01~0x31
        menu->item_buff[7][3] = rdate.Month;    // BCD  month value 0x01~0x12
        menu->item_buff[7][4] = rdate.Year;     // BCD  year value 0x18~0x99
    }
    else if (menu->index == 3U) 
    {
        menu->item_type[0] = VALUE_IN_BUFFER;   // display orientation value in value buffer
        //menu->item_type[1] = LABEL_ITEM;      // disabled display	write of user data from mifare id card:(user name, sex, user type-geust, handmaid, service->->)
        //menu->item_type[2] = LABEL_ITEM;      // 2,4GHz wireless radio modul disabled
        menu->item_type[3] = VALUE_IN_BUFFER;   // wireless radio modul address value in value buffer
        menu->item_type[4] = VALUE_IN_BUFFER;   // wireless radio modul chanel value in value buffer  
        menu->item_type[5] = VALUE_IN_BUFFER;   // buzzer volume value in value buffer
        menu->item_type[6] = VALUE_IN_BUFFER;   // doorlock power value in value buffer
        menu->item_type[7] = VALUE_IN_BUFFER;   // display backlight value in value buffer
      
        if(IsDISP_UserCardInfoTextEnabled()) menu->item_label[1] = 0x1U;
//        if(rf24cfg)menu->item_label[2] = 0x1U;
        
        menu->item_to_edit[0] = 1U; // edit value with one number
        menu->item_to_edit[3] = 1U; // edit value with one number
        menu->item_to_edit[4] = 3U; // edit value with 3 digits
        menu->item_to_edit[5] = 1U; // edit value with one digit 
        menu->item_to_edit[6] = 1U; // edit value with one digit 
        menu->item_to_edit[7] = 1U; // edit value with one digit 
        
        Int2Str((char*)&menu->item_buff[0], disp_rot, 0x1U);        // 1 char display orientation
//        Int2Str((char*)&menu->item_buff[3], rf24address, 0x1U);     // 1 char radio modul address
//        Int2Str((char*)&menu->item_buff[4], rf24chanel, 0x3U);      // 3 char radio chanel
        Int2Str((char*)&menu->item_buff[5], buzzer_volume, 0x2U);   // 3 char buzzer volume value 
        Int2Str((char*)&menu->item_buff[6], doorlock_force, 0x2U);  // 3 char doorlock force value
        Int2Str((char*)&menu->item_buff[7], lcd_bcklght, 0x3U);     // 3 char doorlock force value
    }
    else if (menu->index == 0x4U) 
    {
        for(t = 0x0U; t < 0x8U; t++)
        {
            menu->item_to_edit[t] = 0x5U;
            menu->item_type[t] = VALUE_IN_BUFFER;
            Int2Str((char*)&menu->item_buff[t], ((permitted_add[t][0] << 0x8U) | permitted_add[t][1]), 0x5U);
        }
    }
}
/**
  * @brief
  * @param
  * @retval
  */
static void DISP_PrintMenuValue(sMenuTypeDef* menu)
{
    uint32_t c, xpos1;
    uint32_t txt_pos, xpos2;
    char rtc[18];

    back_color = BLACK;
    pixel_color = WHITE;
    
    /* set font size and text position with display rotation*/
    if(disp_rot & 0x1U) 
    {
        txt_pos = 210U;
        font_size = MIDDLE_FONT;
    }
    else 
    {
        txt_pos = 130U;
        font_size = SMALL_FONT;
    }
    
    DISP_PrintString(0, Line11_M, taster[menu->select][0]);
     
    /* display selected item in gray background color*/
    if(menu->select == 0x0U)
    {
        back_color = GRAY;
        pixel_color = BLACK;
    }
    else
    {
        back_color = BLACK;
        pixel_color = WHITE;    
    }
    
    DISP_PrintString(0x0U, Line0_M, screen[menu->index][0]);

    for(c = 0x0U; c < 0x8U; c ++)
    {
        if((menu->item_index == c) && (menu->select > 0x0U))
        {
            back_color = GRAY;
            pixel_color = BLACK;
        }
        else
        {
            back_color = BLACK;
            pixel_color = WHITE;    
        }
        
        if(menu->index == 0x4U)
        {
            DISP_PrintString(0x0U, Line2_M + (c * Line1_M), "PERMITTED ADDRESS  :");
            DISP_PrintCharacter(txt_pos, Line2_M + (c * Line1_M), (c + '1'));
        }
        else 
        {
            DISP_PrintString(0x0U, Line2_M + (c * Line1_M), item[c][menu->index]);
        }

        if(menu->item_type[c] == VALUE_IN_BUFFER)
        {
            if((c == 0x7U) && (menu->index == 0x2U))    // date & time value
            {
                
                HEX2STR(&rtc[0], &menu->item_buff[c][0]);    // BCD hours value 0x01~0x23
                rtc[2] = ':';
                HEX2STR(&rtc[3], &menu->item_buff[c][1]);    // BCD  minute value 0x00~0x59
                rtc[5] = ' ';
                HEX2STR(&rtc[6], &menu->item_buff[c][2]);    // BCD  date value 0x01~0x31
                rtc[8] = '.';
                HEX2STR(&rtc[9], &menu->item_buff[c][3]);    // BCD  month value 0x01~0x12
                rtc[11] = '.';
                rtc[12] = '2';
                rtc[13] = '0';
                HEX2STR(&rtc[14], &menu->item_buff[c][4]);   // BCD  year value 0x18~0x99
                if (disp_rot & 0x1U)  txt_pos -= 90U;
                else txt_pos -= 50U;
                DISP_PrintString(txt_pos, Line9_M, rtc);
                if ((menu->item_index == 0x7U) && (menu->value_edit == 0x1U)) // date & time
                {
                    back_color = LGRAY;
                    pixel_color = BLACK;
                    xpos1 = txt_pos + (menu->value_index * 3U) * (0x4U + (font_size * 0x4U));
                    if (menu->value_index == 0x4U) xpos1 += (0x2U * (0x4U + (font_size * 0x4U)));
                    xpos2 = xpos1 + (0x4U + (font_size * 0x4U));
                    DISP_PrintCharacter(xpos1, Line9_M, TOCHAR((menu->item_buff[c][menu->value_index] >> 0x4U) & 0x0FU));
                    DISP_PrintCharacter(xpos2, Line9_M, TOCHAR (menu->item_buff[c][menu->value_index] & 0x0FU)); 
                } 
            }
            else                                    // 5 digit value
            {
                DISP_PrintString(txt_pos + 40U, Line2_M + (c * Line1_M), (const char*) &menu->item_buff[c][0]); 
                
                if(menu->value_edit == 0x1U)
                {
                    back_color = LGRAY;
                    pixel_color = BLACK;
                    DISP_PrintCharacter((txt_pos + 40U + (menu->value_index * (0x4U + (font_size * 0x4U)))), (Line2_M + ((menu->item_index) * Line1_M)), (menu->item_buff[menu->item_index][menu->value_index]));
                }                
            }            
        }
        else if (menu->item_type[c] == LABEL_ITEM)
        {
            if((menu->item_index == c) && (menu->select > 0x0U))
            {
                if(menu->select == 0x2U) back_color = LGRAY;
                else back_color = GRAY;
                pixel_color = BLACK;
            }
            else
            {
                back_color = BLACK;
                pixel_color = WHITE;    
            }
            
            DISP_PrintString(txt_pos, Line2_M + (c * Line1_M), label[menu->item_label[c]][0]);
            
            if((c == 0x4U) && (menu->index == 0x2U))    // display result of OW bus scan
            {
                if(ow_dev)        // do not display null result
                {
                    if(disp_rot & 0x1U)  DISP_PrintCharacter(txt_pos + 90U, Line2_M + (c * Line1_M), TOCHAR(ow_dev));
                    else DISP_PrintCharacter(txt_pos + 50U, Line2_M + (c * Line1_M), TOCHAR(ow_dev));
                }
            } 
        }
    }        
}
/**
  * @brief
  * @param
  * @retval
  */
static void DISP_InitMenu(sMenuTypeDef* menu)
{
    uint32_t t;
    menu->index = 0x0U;
    menu->state = 0x0U;
    menu->event = 0x0U;
    menu->select = 0x0U;
    menu->increment = 0x0U;
    menu->item_index = 0x0U;
    menu->value_edit = 0x0U;
    menu->value_index = 0x0U;    
    
    for(t = 0x0U; t < 0x8U; t++)
    {
        menu->item_type[t] = 0x0U;
        menu->item_label[t] = 0x0U;
        menu->item_to_edit[t] = 0x0U;
        ZEROFILL(menu->item_buff[t], BUFFERCNT(menu->item_buff[t]));
    }
}   
/*************************************************************************/
/**     C H E C K    I S   L C D   A C T I V    A N D     R U N         **/
/*************************************************************************/
void DISP_CheckLcd(void)
{
    static uint32_t check_delay = 0x0U;
    
    if ((HAL_GetTick() - check_delay) >= 2000U) check_delay = HAL_GetTick();
    else return;
    /*************************************************************************/
    /**		D I S P L A Y		P O W E R		M O D E 	C H E C K       **/
    /*************************************************************************/
    spi_buff[0] = ILI9341_RD_DISP_POWER_MODE;
    HAL_GPIO_WritePin(DISP_DC_Port, DISP_DC_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DISP_CS_Port, DISP_CS_Pin, GPIO_PIN_RESET);
    if (HAL_SPI_Transmit(&hspi2, spi_buff, 0x1U, DRV_TOUT) != HAL_OK)    ErrorHandler(DISP_FUNC, SPI_DRV);
    if (HAL_SPI_Receive(&hspi2, &spi_buff[1], 0x1U, DRV_TOUT) != HAL_OK) ErrorHandler(DISP_FUNC, SPI_DRV);
    HAL_GPIO_WritePin(DISP_CS_Port, DISP_CS_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(DISP_DC_Port, DISP_DC_Pin, GPIO_PIN_SET);
    if ((spi_buff[1] & (0x1U << 0x7U)) == 0x0U)                             ErrorHandler(DISP_FUNC, SPI_DRV);
}
/*************************************************************************/
/**     D I S P L A Y       D A T E   &   T I M E     I N F O           **/
/*************************************************************************/
void DISP_DateTime(void)
{
    char disp_buff[24];
    static uint32_t temp_day = 0x0U;
    static uint32_t delay_timer = 0x0U;
    
    if ((HAL_GetTick() - delay_timer) >= 999U)
    {
        delay_timer = HAL_GetTick(); 
        HAL_RTC_GetTime(&hrtc, &rtime, RTC_FORMAT_BCD); // should be only time update
        HAL_RTC_GetDate(&hrtc, &rdate, RTC_FORMAT_BCD); // should be only date update
        if (IsRtcTimeValid()) unix_rtc = rtc2unix (&rtime, &rdate);  // rtc to unix 
        /*************************************************************************/
        /**  B A C K U P   R T C   D A T E   D A I L Y   A N D   R E F R E S H  **/  
        /*************************************************************************/
        if(temp_day != rdate.Date)
        {   /* refresh display screen daily to avoid bad pixels*/
            DISP_RefreshSet();
            temp_day = rdate.Date;
            if (IsRtcTimeValid())
            {   /* backup rtc date only if rtc time valid*/
                HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR2, rdate.Date);
                HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR3, rdate.Month);
                HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR4, rdate.WeekDay);
                HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR5, rdate.Year);
            }
        }
        /*************************************************************************/
        /**     D I S P L A Y   V A L I D      D A T E     &    T I M E         **/  
        /*************************************************************************/
        if      (!IsRtcTimeValid()) unix_rtc = 0x0U; // get back with valid rtc time
        else if (ROOM_Status < ROOM_UNUSABLE)   // skeep alarm image
        {
            HEX2STR(disp_buff,      &rdate.Date);
            disp_buff[2] = '.';
            HEX2STR(&disp_buff[3],  &rdate.Month);
            disp_buff[5] = '.';
            disp_buff[6] = '2';
            disp_buff[7] = '0';
            HEX2STR(&disp_buff[8],  &rdate.Year);
            disp_buff[10] = '.';	
            disp_buff[11] = ' ';
            HEX2STR(&disp_buff[12], &rtime.Hours);
            disp_buff[14] = ':';
            HEX2STR(&disp_buff[15], &rtime.Minutes);
            disp_buff[17] = ':';
            HEX2STR(&disp_buff[18], &rtime.Seconds);
            back_color = BLACK;
            pixel_color = WHITE;
            
            if (disp_rot & 0x1U) 
            {
                font_size = BIG_FONT;
                DISP_PrintString(0x0U, Line8_B, disp_buff);
            }
            else
            {
                font_size = MIDDLE_FONT;
                DISP_PrintString(0x0U, 290U, disp_buff);
            }                     
        }
    }
}
/*************************************************************************/
/**     D I S P L A Y       T E M P E R A T U R E       I N F O         **/
/*************************************************************************/
void DISP_Temperature(void)
{
    char dbuf[8], len;
    static uint32_t mv_tmr = 0x0U;
    
    if ((HAL_GetTick() - mv_tmr) >= 1234U)
    {
        mv_tmr = HAL_GetTick(); 
        if ((IsTempLuxRTActive()    // wait for room thermostat,
        ||   IsTempSenNTCActive()   // or room controller NTC,  
        ||   IsTempSenDS18Active()) // or Dalass temperature sensor validation
        &&  (ROOM_Status < ROOM_UNUSABLE)) // don't display temperature over alarm image */
        {   // fill buffer with space characters to erase previous
            mem_set(dbuf, ' ', 0x7U); // text from display screen
            font_size = MIDDLE_FONT;
            pixel_color = YELLOW;
            back_color = BLACK;
            len = 0x0U;
            if (temp_mv) dbuf[len++] = '+';
            Int2Str(&dbuf[len], temp_mv, 0x0U);
            len = strlen(dbuf);
            dbuf[len] = 0x5EU;
            dbuf[len + 0x1U] = 'C';
            dbuf[7] = 0x0U; // null terminate
            if(disp_rot & 0x1U) DISP_PrintString(0x5U, 188U, dbuf);
            else                DISP_PrintString(0x5U, 260U, dbuf);
        }
    }
}
/*************************************************************************/
/**     D I S P L A Y   C A R D     U S E R     N A M E     T E X T     **/
/*************************************************************************/
void DISP_CardUserName(void)
{
	uint32_t t = 0x0U;
    uint32_t c = 0x0U;
    uint32_t n = 0x0U;
    uint8_t dspbuf[34];
    uint32_t xpos, ypos;
    if (disp_rot & 0x1U) xpos = 130U, ypos = 146U;
    else  xpos = 90U, ypos = 240U;
    font_size   = BIG_FONT;                         // set display 
    back_color  = BLACK;                            // text
    pixel_color = WHITE;                            // style
    n = (rc522_rx_buff[0] & 0x80U);                 // get male marker
    rc522_rx_buff[0] &=  0x7FU;                     // clear male marker
    t = GetSize(rc522_tx_buff);                     // get name string size
    c = GetSize(rc522_rx_buff);                     // get surname string size
    if (sCard.user_group == USERGRP_GUEST)  // write prefix text for male/female
    {                                               // if nothing to display
        if (!t && !c)   return;                     // get back
        else if (n)                                 DISP_PrintString(xpos, ypos, " Mr.");     // guest card user is mister
        else                                        DISP_PrintString(xpos, ypos, "Mrs.");     // guest card user is madam         
    }
    else if (sCard.user_group == USERGRP_MAID)      DISP_PrintString(xpos - 30U, ypos, "SOBARICA"); // handmaid card user
    else if (sCard.user_group == USERGRP_SERVICER)  DISP_PrintString(xpos - 30U, ypos, "SERVISER"); // service card user
	ZEROFILL(dspbuf, COUNTOF(dspbuf));
    if (t)                                          // is any
    {
        mem_copy(dspbuf, rc522_tx_buff, t);         // copy name string if any
        dspbuf[t] = ' ';                            // insert space after name
    }
    t = GetSize(dspbuf); 
    dspbuf[t] = 0x0U;
    if (c) mem_copy(&dspbuf[t], rc522_rx_buff, c);  // copy surname string if any
    n =  (disp_width / (0x4U + (font_size * 0x4U)));    // max. letters in text line
    t = GetSize(dspbuf);                            // char to display in buffer 
    dspbuf[t] = 0x0U; 
    if(n < t)                                       // if more char to display 
    {                                               // reduce buffer to max. 
        dspbuf[n - 0x1U] = '.';                       // with dot on end
        dspbuf[n] = 0x0U;                            // nul terminate string
        t = n;                                      // set char to display count
    }                                               // align text to middle 
    c = ((n - t) / 0x2U);                             // free char spaces
    c *= (0x4U + (font_size * 0x4U));                   // position of first letter
    DISP_PrintString(c, ypos + 36U, (char*)dspbuf); // display card user name
}
/*************************************************************************/
/**     S E T           D I S P L A Y       O R I E N T A T I O N       **/
/*************************************************************************/
void DISP_SetOrientation(void)
{
    DISP_WriteRegister(ILI9341_MEMORY_ACCESS_CONTROL);
    
    switch(disp_rot)
    {
        case 3:
            DISP_WriteByte(0xE8);
            disp_width = LCD_W;
            disp_high = LCD_H;
            break;
        case 2:
            DISP_WriteByte(0x88);
            disp_width = LCD_H;
            disp_high = LCD_W;
            break;
        case 1:
            DISP_WriteByte(0x28);
            disp_width = LCD_W;
            disp_high = LCD_H;
            break;
        case 0:
        default:
            DISP_WriteByte(0x48);
            disp_width = LCD_H;
            disp_high = LCD_W;
            break;
    }
}
/*************************************************************************/
/**     D I S P L A Y       R G B 5 6 5         I M A G E               **/
/*************************************************************************/
static void DISP_Image(uint8_t selected)
{
	uint32_t image_size;
	uint32_t char_cnt = 0x0U;
    uint32_t flash_address;
    uint8_t dsp_buff[DISP_BSIZE];
	
	flash_address = ((selected - 0x1U) * 0x00030000U);
	
	if(selected == MINIBAR_IMAGE)
	{
		image_size = IMAGE_MINIBAR_SIZE;
		DISP_AddressSet(IMAGE_MINIBAR_POSITION);
	}
	else 
    {
        image_size = IMAGE_SIZE;
        DISP_AddressSet(0x0U, 0x0U, disp_width - 0x1U, disp_high - 0x1U);
    }
	
	DISP_WriteRegister(ILI9341_MEMORY_WRITE);
	HAL_GPIO_WritePin(DISP_DC_Port, DISP_DC_Pin, GPIO_PIN_SET);
    
    while(image_size)
	{
        if(image_size >= DISP_BSIZE) char_cnt = DISP_BSIZE;
		else char_cnt = image_size;
        
		FLASH_ReadPage(flash_address, dsp_buff, char_cnt);
		HAL_GPIO_WritePin(DISP_CS_Port, DISP_CS_Pin, GPIO_PIN_RESET);
		if (HAL_SPI_Transmit(&hspi2, dsp_buff, char_cnt, DRV_TOUT) != HAL_OK) ErrorHandler(DISP_FUNC, SPI_DRV);
		HAL_GPIO_WritePin(DISP_CS_Port, DISP_CS_Pin, GPIO_PIN_SET);
		flash_address += char_cnt;
        image_size -= char_cnt;
	}
}
/*************************************************************************/
/**      F I L L     D I S P L A Y    W I T H     C O L O R             **/
/*************************************************************************/
static void DISP_Clear(uint16_t color)
{
	uint32_t i = 0x0U;
    uint8_t hi = ((color >> 0x8U) & 0xFFU);
    uint8_t lo = (color & 0xFFU);
    DISP_AddressSet(0x0U, 0x0U, disp_width - 0x1U, disp_high - 0x1U);
	DISP_WriteRegister(ILI9341_MEMORY_WRITE);
    HAL_GPIO_WritePin(DISP_DC_Port, DISP_DC_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(DISP_CS_Port, DISP_CS_Pin, GPIO_PIN_RESET);
    i = disp_high * (disp_width * 0x2U);
    do
    {
        while(((hspi2.Instance->SR) & SPI_FLAG_TXE) != SPI_FLAG_TXE)
        {
        }
        *((__IO uint8_t*)&hspi2.Instance->DR) = hi;
        while(((hspi2.Instance->SR) & SPI_FLAG_TXE) != SPI_FLAG_TXE)
        {
        }  
        *((__IO uint8_t*)&hspi2.Instance->DR) = lo;
    }
    while(i--);
    
    /* Wait until the bus is ready before releasing Chip select */ 
    while(((hspi2.Instance->SR) & SPI_FLAG_BSY) != RESET)
    {
    }
    HAL_GPIO_WritePin(DISP_CS_Port, DISP_CS_Pin, GPIO_PIN_SET);
    DISP_WriteRegister(ILI9341_NOP);
}
/**
  * @brief
  * @param
  * @retval
  */
static void DISP_AddressSet(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    DISP_WriteRegister(ILI9341_COLUMN_ADDR_SET);
    DISP_WriteInt(x1);
    DISP_WriteInt(x2);
    DISP_WriteRegister(ILI9341_PAGE_ADDR_SET);
    DISP_WriteInt(y1);
    DISP_WriteInt(y2);
}
/**
  * @brief
  * @param
  * @retval
  */
static void DISP_WriteRegister(uint8_t data)
{
    HAL_GPIO_WritePin(DISP_DC_Port, DISP_DC_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DISP_CS_Port, DISP_CS_Pin, GPIO_PIN_RESET);
    if (HAL_SPI_Transmit(&hspi2, &data, 0x1U, DRV_TOUT) != HAL_OK) ErrorHandler(DISP_FUNC, SPI_DRV);
    HAL_GPIO_WritePin(DISP_CS_Port, DISP_CS_Pin, GPIO_PIN_SET); 
}
/**
  * @brief
  * @param
  * @retval
  */
static void DISP_PrintCharacter(uint16_t x, uint16_t y, uint8_t num)
{
    uint32_t temp, pos, n, t;
    uint16_t colortemp = pixel_color;
    uint32_t inc_y;
    uint32_t font_address;
    uint32_t inc_x =  (0x4U + (font_size * 0x4U));
    if      (font_size == SMALL_FONT)   inc_y = 16U, font_address = RC_SML_FONT_ADDR;
    else if (font_size == MIDDLE_FONT)  inc_y = 20U, font_address = RC_MID_FONT_ADDR;
    else if (font_size == BIG_FONT)     inc_y = 26U, font_address = RC_BIG_FONT_ADDR;
    else return;

    n = num - ' ';
    
    DISP_AddressSet(x, y, (x + inc_x - 0x1U), (y  + inc_y - 0x1U)); 
    DISP_WriteRegister(ILI9341_MEMORY_WRITE);
    
    for (pos = 0x0U; pos < inc_y; pos++)
    {
        if (font_size == SMALL_FONT) temp = FLASH_ReadByte ((n * inc_y + pos)      + font_address);  
        else                         temp = FLASH_ReadInt (((n * inc_y + pos) * 2) + font_address);
        
        for(t = 0x0U; t < inc_x; t++)
        {
            pixel_color = back_color;
            if(font_size == SMALL_FONT) 
            {
                if (temp & (0x1U << t))  pixel_color = colortemp;
            }
            else if(temp & (0x1U << 15U)) pixel_color = colortemp;            
            DISP_WriteInt(pixel_color);    
            if(font_size != SMALL_FONT) temp <<= 0x1U;                 
        }
    }
    pixel_color = colortemp;	
}
/**
  * @brief
  * @param
  * @retval
  */
static void DISP_PrintNumber(uint16_t x, uint16_t y, uint32_t num, uint8_t len)
{
    uint32_t t, temp, enshow = 0x0U;
    uint32_t inc_x =  (0x4U + (font_size * 0x4U));
    
	for(t = 0x0U; t < len; t++)
	{
		temp =(num / BaseToPower(10U, len - t - 0x1U)) %10U;
        
		if((enshow == 0x0U) && (t < (len - 0x1U)))
		{
			if(temp == 0x0U)
			{
				DISP_PrintCharacter((x + inc_x * t), y, ' ');
				continue;
			}
			else 
			{
                enshow = 0x1U;
            }
		}
        
        DISP_PrintCharacter((x + inc_x * t), y, (temp + '0'));
	}
}
/**
  * @brief
  * @param
  * @retval
  */
static void DISP_PrintString(uint16_t x, uint16_t y, const char *p)
{            
    uint32_t inc_y;
    uint32_t inc_x =  (0x4U + (font_size * 0x4U));
    if      (font_size == SMALL_FONT)   inc_y = 16U;
    else if (font_size == MIDDLE_FONT)  inc_y = 20U;
    else if (font_size == BIG_FONT)     inc_y = 26U;
    
    while(*p)
	{        
        if(x > (disp_width - inc_x))
        {
            x = 0x0U;
            y += inc_y;
        }

        if(y > (disp_high - inc_y)) 
        {
            x = 0x0U;
            y = 0x0U;
        }
        
        DISP_PrintCharacter(x, y, *p);
        x += inc_x;
        ++p;
    }
}
/**
  * @brief
  * @param
  * @retval
  */
static void DISP_WriteByte(uint8_t data)
{
	HAL_GPIO_WritePin(DISP_DC_Port, DISP_DC_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(DISP_CS_Port, DISP_CS_Pin, GPIO_PIN_RESET);
    if (HAL_SPI_Transmit(&hspi2, &data, 0x1U, DRV_TOUT) != HAL_OK) ErrorHandler(DISP_FUNC, SPI_DRV);
    HAL_GPIO_WritePin(DISP_CS_Port, DISP_CS_Pin, GPIO_PIN_SET);
}
/**
  * @brief
  * @param
  * @retval
  */
static void DISP_WriteInt(uint16_t data)
{
	HAL_GPIO_WritePin(DISP_DC_Port, DISP_DC_Pin, GPIO_PIN_SET);
    spi_buff[0] = data >> 8;
    spi_buff[1] = data;
    HAL_GPIO_WritePin(DISP_CS_Port, DISP_CS_Pin, GPIO_PIN_RESET);
    if (HAL_SPI_Transmit(&hspi2, spi_buff, 0x2U, DRV_TOUT) != HAL_OK) ErrorHandler(DISP_FUNC, SPI_DRV);
    HAL_GPIO_WritePin(DISP_CS_Port, DISP_CS_Pin, GPIO_PIN_SET);
}
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
