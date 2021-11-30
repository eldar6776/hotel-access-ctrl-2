/**
 ******************************************************************************
 * File Name          : display.c
 * Date               : 17.2.2019
 * Description        : GUI Display Module
 ******************************************************************************
 *
 *
 ******************************************************************************
 */
 
 
/* Include  ------------------------------------------------------------------*/
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


#if (__DISP_H__ != FW_BUILD)
    #error "display header version mismatch"
#endif

/* Private Define ------------------------------------------------------------*/
#define GUI_REFRESH_TIME                100U    // refresh gui 10 time in second
#define DATE_TIME_REFRESH_TIME          1000U   // refresh date & time info every 1 sec. 
#define SCRNSVR_TOUT                    10000U  // 10 sec timeout increment to set display in low brigntnes after last touch event
#define SETTINGS_MENU_ENABLE_TIME       4321U    // press and holde upper left corrner for period to enter setup menu
#define WFC_TOUT                        8765U   // 9 sec. weather display timeout   
#define BUTTON_RESPONSE_TIME            678U    // button response delay till all onewire device update state
#define DISP_IMG_TIME_MULTI             30000U  // 30 sec. min time increment for image display time * 255 max
#define SETTINGS_MENU_TIMEOUT           59000U  // 1 min. settings menu timeout
#define WFC_CHECK_TOUT                  (SECONDS_PER_HOUR * 1000U) // check weather forecast data validity every hour
//eng   display image 1- 5
//ger   display image 11-15
//fra   display image 21-25
//arab  display image 31-35
//china display image 41-45
//jap   display image 51-55
//ita   display image 61-65
//tur   display image 71-75
//slov  display image 81-85

#define CLR_DARK_BLUE           GUI_MAKE_COLOR(0x613600)
#define CLR_LIGHT_BLUE          GUI_MAKE_COLOR(0xaa7d67)
#define CLR_BLUE                GUI_MAKE_COLOR(0x855a41)
#define CLR_LEMON               GUI_MAKE_COLOR(0x00d6d3)

#define BTN_INC_X0              220
#define BTN_INC_Y0              90
#define BTN_DEC_X0              3
#define BTN_DEC_Y0              87
#define BTN_DND_X0              330
#define BTN_DND_Y0              0
#define BTN_CMD_X0              330
#define BTN_CMD_Y0              90
#define BTN_SOS_X0              330
#define BTN_SOS_Y0              180
#define BTN_DOOR_X0             25
#define BTN_DOOR_Y0             210
#define BTN_OK_X0               300
#define BTN_OK_Y0               205
#define BTN_SETTINGS_X0         0
#define BTN_SETTINGS_Y0         0
#define SP_H_POS                200
#define SP_V_POS                150
#define CLOCK_H_POS             240
#define CLOCK_V_POS             136

#define BTN_INC_X1              (BTN_INC_X0 + 115)
#define BTN_INC_Y1              (BTN_INC_Y0 + 160)
#define BTN_DEC_X1              (BTN_DEC_X0 + 115)
#define BTN_DEC_Y1              (BTN_DEC_Y0 + 140)
#define BTN_DND_X1              (BTN_DND_X0 + 150)
#define BTN_DND_Y1              (BTN_DND_Y0 + 90)
#define BTN_CMD_X1              (BTN_CMD_X0 + 150)
#define BTN_CMD_Y1              (BTN_CMD_Y0 + 90)
#define BTN_SOS_X1              (BTN_SOS_X0 + 150)
#define BTN_SOS_Y1              (BTN_SOS_Y0 + 90)
#define BTN_DOOR_X1             (BTN_DOOR_X0 + 155)
#define BTN_DOOR_Y1             (BTN_DOOR_Y0 + 50)
#define BTN_OK_X1               (BTN_OK_X0  + 133)
#define BTN_OK_Y1               (BTN_OK_Y0  + 55)
#define BTN_SETTINGS_X1         (BTN_SETTINGS_X0 + 120)
#define BTN_SETTINGS_Y1         (BTN_SETTINGS_Y0 + 80)

#define GUI_ID_BUTTON_Dnd   			                    0x800
#define GUI_ID_BUTTON_Sos   			                    0x801
#define GUI_ID_BUTTON_Maid   			                    0x802
#define GUI_ID_BUTTON_Ok                                    0x803
#define GUI_ID_BUTTON_DoorOpen                              0x804
#define GUI_ID_BUTTON_Next                                  0x805
#define GUI_ID_BUTTON_Weather                               0x806    
#define GUI_ID_BUTTON_RFSenDelete                           0x807
#define GUI_ID_BUTTON_RFSenAdd                              0x808
#define GUI_ID_BUTTON_RFSenUp                               0x809
#define GUI_ID_BUTTON_RFSenDown                             0x80A

#define GUI_ID_SPINBOX_AmbientNtcOffset                     0x821
#define GUI_ID_SPINBOX_OneWireInterfaceAddress              0x825
#define GUI_ID_SPINBOX_ThermostatMaxSetpointTemperature     0x82a
#define GUI_ID_SPINBOX_ThermostatMinSetpointTemperature     0x82b
#define GUI_ID_SPINBOX_DisplayHighBrightness                0x82e
#define GUI_ID_SPINBOX_DisplayLowBrightness                 0x82f
#define GUI_ID_SPINBOX_ScrnsvrTimeout                       0x830
#define GUI_ID_SPINBOX_ScrnsvrEnableHour                    0x831
#define GUI_ID_SPINBOX_ScrnsvrDisableHour                   0x832
#define GUI_ID_SPINBOX_ScrnsvrClockColour                   0x833
#define GUI_ID_SPINBOX_ScrnsvrLogoClockColour               0x834
#define GUI_ID_SPINBOX_Hour                                 0x835
#define GUI_ID_SPINBOX_Minute                               0x836
#define GUI_ID_SPINBOX_Day                                  0x887
#define GUI_ID_SPINBOX_Month                                0x838
#define GUI_ID_SPINBOX_Year                                 0x839
#define GUI_ID_CHECK_Scrnsvr                                0x853
#define GUI_ID_CHECK_ScrnsvrClock                           0x854
#define GUI_ID_CHECK_ScrnsvrLogoClock                       0x855

#define GUI_ID_EDIT_RFSen                                   0x860
/* Private Type --------------------------------------------------------------*/
BUTTON_Handle   hBUTTON_Dnd;
BUTTON_Handle   hButtonSosReset;
BUTTON_Handle   hBUTTON_Maid;
BUTTON_Handle   hBUTTON_Increase;
BUTTON_Handle   hBUTTON_Decrease;
BUTTON_Handle   hBUTTON_Ok;
BUTTON_Handle   hBUTTON_DoorOpen;
BUTTON_Handle   hBUTTON_Next;
BUTTON_Handle   hBUTTON_Forecast;
BUTTON_Handle   hBUTTON_RFSenDelete;
BUTTON_Handle   hBUTTON_RFSenAdd;
BUTTON_Handle   hBUTTON_RFSenUp;
BUTTON_Handle   hBUTTON_RFSenDown;
SPINBOX_Handle  hSPNBX_AmbientNtcOffset;                    //  ambient measured temperature value manual offset
SPINBOX_Handle  hSPNBX_OneWireInterfaceAddress;             //  one wire bus interface address
SPINBOX_Handle  hSPNBX_ThermostatMaxSetpointTemperature;    //  set thermostat user maximum setpoint value
SPINBOX_Handle  hSPNBX_ThermostatMinSetpointTemperature;    //  set thermostat user minimum setpoint value
SPINBOX_Handle  hSPNBX_DisplayHighBrightness;               //  lcd display backlight led brightness level for activ user interface (high level)
SPINBOX_Handle  hSPNBX_DisplayLowBrightness;                //  lcd display backlight led brightness level for activ screensaver (low level)
SPINBOX_Handle  hSPNBX_ScrnsvrTimeout;                      //  start screensaver (value x 10 s) after last touch event or disable screensaver for 0
SPINBOX_Handle  hSPNBX_ScrnsvrEnableHour;                   //  when to start display big digital clock for unused thermostat display 
SPINBOX_Handle  hSPNBX_ScrnsvrDisableHour;                  //  when to stop display big digital clock but to just dimm thermostat user interfaca
SPINBOX_Handle  hSPNBX_ScrnsvrClockColour;                  //  set colour for full display screensaver digital clock digits 
SPINBOX_Handle  hSPNBX_ScrnsvrSemiClkColor;                 //  set colour for user logo size and place embedded digital clock
SPINBOX_Handle  hSPNBX_Hour;
SPINBOX_Handle  hSPNBX_Minute;
SPINBOX_Handle  hSPNBX_Day;
SPINBOX_Handle  hSPNBX_Month;
SPINBOX_Handle  hSPNBX_Year;
CHECKBOX_Handle hCHKBX_ScrnsvrClock;                        //  select full display screensaver digital clock 
CHECKBOX_Handle hCHKBX_ScrnsvrLogoClock;                    //  select user logo size screensaver digital clock
EDIT_Handle     hEDIT_RFSen;
GUI_HMEM hQR_Code;   // QR CODE

FORECAST_DayTypeDef FORECAST_Day[5] =                       //  weather forecast for 5 days
{
    { 1, 1, 0, 0, 0},
    { 2, 2, 0, 0, 0},
    { 3, 3, 0, 0, 0},
    { 4, 4, 0, 0, 0},
    { 5, 5, 0, 0, 0}
};


GUI_RECT ForecastFrame[5] =                                 //  weather forecast 5 day frames display position
{
	{  5, 120,  65, 260},
	{ 70, 120, 130, 260},
	{135, 120, 195, 260},
	{200, 120, 260, 260},
	{265, 120, 325, 260}
};


static char * _apDays[] =                                   //  weather forecast days string
{
	"0", 
    "MON", 
    "TUE", 
    "WED", 
    "THU", 
    "FRI", 
    "SAT", 
    "SUN"
};

static uint32_t clk_clrs[COLOR_BSIZE] =                     //  selectable screensaver clock colours
{
    GUI_GRAY,       
    GUI_RED,        
    GUI_BLUE,       
    GUI_GREEN,      
    GUI_CYAN,       
    GUI_MAGENTA,        
    GUI_YELLOW,     
    GUI_LIGHTGRAY,  
    GUI_LIGHTRED,   
    GUI_LIGHTBLUE,  
    GUI_LIGHTGREEN, 
    GUI_LIGHTCYAN,  
    GUI_LIGHTMAGENTA,  
    GUI_LIGHTYELLOW, 
    GUI_DARKGRAY,   
    GUI_DARKRED,    
    GUI_DARKBLUE,   
    GUI_DARKGREEN,  
    GUI_DARKCYAN,   
    GUI_DARKMAGENTA,    
    GUI_DARKYELLOW,
    GUI_WHITE,      
    GUI_BROWN,      
    GUI_ORANGE,     
    CLR_DARK_BLUE,  
    CLR_LIGHT_BLUE, 
    CLR_BLUE,           
    CLR_LEMON

};
/* Private Variable ----------------------------------------------------------*/
uint32_t disp_fl;
uint32_t thst_fl;
uint8_t disp_img_id, disp_img_time, last_img_id;
uint8_t scrnsvr_ena_hour, scrnsvr_dis_hour;
uint8_t scrnsvr_tout, scrnsvr_clk_clr, scrnsvr_semiclk_clr;
uint8_t disp_low_bcklght, disp_high_bcklght;
int8_t wfc_buff[WFC_BSIZE];
uint16_t rfsen[RFSEN_BSIZE];

int16_t room_temp;
int16_t room_ntc_temp;
int8_t  ntc_offset;
uint8_t thst_sp;
uint8_t thst_dif;
uint8_t thst_cfg;
uint8_t thst_min_sp;
uint8_t thst_max_sp;
uint8_t rfsen_sel;

static uint8_t btn_settings;
static uint8_t btn_ok_state, btn_opendoor_state;
static uint8_t btn_dnd_state, btn_dnd_old_state;
static uint8_t btn_sos_state;
static uint8_t btn_maid_state, btn_maid_old_state;
static uint8_t btn_increase_state, btn_increase_old_state;
static uint8_t btn_decrease_state, btn_decrease_old_state;
static uint32_t disp_sreensvr_tmr = 0U;
/* Private Macro -------------------------------------------------------------*/
/* Private Function Prototype ------------------------------------------------*/
static void DISP_DateTime(void);
static void DISP_SaveRFSen(void);
static void DISP_SaveSetPoint(void);
static void DISP_SaveSettings(void);
static void DISP_ResetScrnsvr(void);
static void PID_Hook(GUI_PID_STATE* pState);
static void DISP_TemperatureSetPoint(void);
static void DISP_DrawScreen(uint8_t img_id);
static void DISP_Temperature(int16_t value);
static void DISP_CreateSettings1Screen(void);
static void DISP_CreateSettings2Screen(void);
static void DISP_DeleteSettings1Screen(void);
static void DISP_DeleteSettings2Screen(void);
static uint8_t DISP_LoadWFC(int8_t* buff);
static uint8_t DISP_SaveWFC(int8_t* buff);
static uint8_t DISP_EnableSettingsMenu(uint8_t btn);
static void DISP_CreateWFCScreen (int8_t* buff);
static void DISP_InitWFCFrame(FORECAST_DayTypeDef* fday);
/* Program Code  -------------------------------------------------------------*/
/**
  * @brief
  * @param
  * @retval
  */
void DISP_Init(void)
{
    GUI_Init();
    GUI_PID_SetHook(PID_Hook);
    WM_MULTIBUF_Enable(1);
    GUI_UC_SetEncodeUTF8();
    disp_img_id = 0x0U;
    ScreenInitSet();
}
/**
  * @brief
  * @param
  * @retval
  */
void DISP_Service(void)
{
    char sentxt[16];
    static enum
    {
        DISP_THERMOSTAT  = ((uint8_t)0x0U),
        DISP_FORECAST    = ((uint8_t)0x1U),
        DISP_MESSAGE     = ((uint8_t)0x2U),
        DISP_ERROR       = ((uint8_t)0x3U),
        DISP_SETTINGS_1  = ((uint8_t)0x4U),
        DISP_SETTINGS_2  = ((uint8_t)0x5U)          
        
    }DISP_Screen = DISP_THERMOSTAT;
    
    static uint32_t thflag = 0x0U;
    static uint32_t disp_tmr = 0x0U;
    static uint32_t disp_rtc_tmr = 0x0U;
    static uint32_t disp_img_tmr = 0x0U;
    static uint32_t button_enable_tmr = 0x0U;
    static uint32_t wfc_tmr = 0x0U;
    /** ==========================================================================*/
	/**    D R A W     D I S P L A Y	G U I	O N	   T I M E R    E V E N T     */
	/** ==========================================================================*/
	if((HAL_GetTick() - disp_tmr) >= GUI_REFRESH_TIME) // regular call for gui redraw, with forecast timer increment inside
	{
        wfc_tmr += GUI_REFRESH_TIME;
		disp_tmr = HAL_GetTick();
		GUI_Exec();
	}
	else return;
    /** ============================================================================*/
	/**		    C H E C K       O P E R A T I N G       C O N D I T I O N S   	    */
	/** ============================================================================*/
    if      ((!IsExtSwClosed() || !ExtSwRemoteClosed()) && ((thflag & 0x1U) == 0x0U))   // allways start power cycle with thermostat disable 
    {
        thflag |= 0x01U;
        DISP_FaultSet();
    }
    else if (IsExtSwClosed() && ExtSwRemoteClosed() && ((thflag & 0x1U) != 0x0U))  // enable thermostat if external switch closed
    {
        thflag &= 0xFEU;
        DISP_GuiSet();
    }
    
    else if (IsTempRegEnabled()  && ((thflag & 0x2U) == 0x0U))   // enable thermostat if remote hvac contacter closed
    {
        thflag |= 0x02U;
        DISP_GuiSet();
    }
    else if (!IsTempRegEnabled() && ((thflag & 0x2U) != 0x0U))   // disable thermostat if remote hvac contacter open
    {
        thflag &= 0xFDU;
        DISP_FaultSet();
    }
    /** ==========================================================================*/
	/**     C H E C K   N E W   W E A T H E R   F O R E C A S T   D A T A         */
	/** ==========================================================================*/
    if      (IsWFC_UpdateActiv()) // check for new received weather forecast
    {
        WFC_UpdateReset();
        
        if (DISP_SaveWFC(wfc_buff) == 0x0U) // save new data to eeprom, function fail if any of data invalid
        {
            if (!IsWFC_ValidActiv()) // if data test passed, show weather button icon and enable forecast display
            {
                WFC_ValidSet();
                ButtonUpdateSet();
            }
        }
    }
    else if (wfc_tmr > WFC_CHECK_TOUT) // check hourly is actual weather forecast data obsolate
    {
        wfc_tmr = 0x0U;
        
        if(DISP_LoadWFC(wfc_buff) == 0x0U) // load forecast from eeprom, function fail if any of data invalid
        {
            if(!IsWFC_ValidActiv()) // if data test passed, show weather button icon and enable forecast display
            {
                WFC_ValidSet();
                ButtonUpdateSet();
            }
        }
        else if(IsWFC_ValidActiv()) // if data test failed, disable forecast display and remove button icon
        {
            WFC_ValidReset();
            ButtonUpdateSet();
        }
    }
    /** ==========================================================================*/
	/**             D I S P L A Y           S C R E E N S A V E R                 */
	/** ==========================================================================*/
    if      (!IsScrnsvrActiv()) // display backlight dimmed after unused period timeout
    {   
        if((HAL_GetTick() - disp_sreensvr_tmr) >= (scrnsvr_tout * SCRNSVR_TOUT))
        {
            ScrnsvrSet();
            ScrnsvrInitReset();
            DISP_SetBrightnes(disp_low_bcklght);
        }
        /** ==========================================================================*/
        /**     C H E C K   S E T T I N G S   M E N U   H I D D E N   B U T T O N     */
        /** ==========================================================================*/
        else if (DISP_EnableSettingsMenu(btn_settings) == 1)   // function return 1 after continous call with 1 for preset time
        {
            disp_img_id = 30U;
            disp_img_time = 0U;
            buzz_sig_time = 0U;
            buzz_sig_id = 0U;
            DISP_UpdateSet();
            ScreenInitSet();
        }
    }
    /** ==========================================================================*/
    /**     R E D R A W    D I S P L A Y    O N    B U T T O N    U P D A T E     */
    /** ==========================================================================*/
    else if (DISP_Screen == DISP_THERMOSTAT)   // and thermostat user interface activ
    {
        if (IsButtonUpdateActiv()) DISP_UpdateSet(); // reset screensaver and redraw buttons
    }
    /** ==========================================================================*/
	/**         N E W       D I S P L A Y       I M A G E       R E Q U E S T     */
	/** ==========================================================================*/
    if (IsDISP_UpdateActiv() && (DISP_Screen < DISP_SETTINGS_1)) // display new gui screen 
    {
        DISP_UpdateReset();
        DISP_ResetScrnsvr(); 
        /************************************/
        /* SET ONEWIRE FLAGS AND SEND EVENT */
        /************************************/
        if(IsDISP_GuiActiv() || IsDISP_FaultActiv()) // two most used display requests: thermostat gui and external switch open user fault
        {
            DISP_FaultReset();
            DISP_GuiReset();
//            DispRemoteSet();
            
            if(IsButtonOkActiv() || IsButtonOpenActiv()) // to prevent update loop when display message confirmed with button
            {
//                DispRemoteReset();   // cancel image update request to one wire device, 
//                SkeepNextDisplayImageSet();     // skeep next image update from one wire device,
                ButtonRemoteSet();     // and inform one wire device for button state change
            }
        }
        /************************************/
        /*  DISPLAY     NEW     SCREEN      */
        /************************************/
        DISP_DrawScreen(disp_img_id);        // redraw display layers
        /************************************/
        /*          SET     TIMERS          */
        /************************************/
        disp_img_tmr      = HAL_GetTick();    // displayed image time is in  x 30 sec. increments
        button_enable_tmr = HAL_GetTick();    // start minimal time delay to send new state to onewire device
        /************************************/
        /*       SET    BUZZER    STATE     */
        /************************************/
        if(buzz_sig_id != 0U)BUZZER_SignalOn(); // start buzzer if defined signal mode
        /************************************/
        /*      SET    NEW  SCREEN  ID      */
        /************************************/
        if      (disp_img_id == 0U)  DISP_Screen = DISP_THERMOSTAT,   ScreenInitSet();
        else if (disp_img_id == 10U) DISP_Screen = DISP_ERROR,        last_img_id = 10U;
        else if (disp_img_id == 20U) DISP_Screen = DISP_FORECAST,     DISP_CreateWFCScreen(wfc_buff);
        else if (disp_img_id == 30U) DISP_Screen = DISP_SETTINGS_1,   DISP_CreateSettings1Screen();
        else                         DISP_Screen = DISP_MESSAGE;
    }
    /** ==========================================================================*/
	/**       S E R V I C E       U S E R       I N P U T       E V E N T S       */
	/** ==========================================================================*/
    if      (DISP_Screen == DISP_THERMOSTAT) // thermostat user interface
	{
        /** ==========================================================================*/
        /**     S C R E E N     I N I T I A L I Z A T I O N     R E Q U E S T         */
        /** ==========================================================================*/
        if(IsScreenInitActiv())
        {
            ScreenInitReset();              // reset init enable flag
            last_img_id = 0U;             // reset last image
            DISP_DateTime();             // show clock time
            ButtonUpdateSet();         // enable button redraw
            DISP_TemperatureSetPoint();  // show setpoint temperature          
        }
        /** ==========================================================================*/
        /**     C H E C K       U S E R     B U T T O N S    N E W       E V E N T    */
        /** ==========================================================================*/
        if ((HAL_GetTick() - button_enable_tmr) >= BUTTON_RESPONSE_TIME)
        {
            /************************************/
            /*      DND BUTTON STATE CHANGED    */
            /************************************/
            if      (btn_dnd_state && !btn_dnd_old_state)         
            {
                btn_dnd_old_state = 0x1U;
                if(!IsButtonDndActiv())
                {
                    ButtonDndSet();
                    ButtonCallReset();
                }
                else ButtonDndReset();
                ButtonRemoteSet();
                ButtonUpdateSet();
                SetpointUpdateSet(); // save new button state
            }
            else if (!btn_dnd_state && btn_dnd_old_state)  btn_dnd_old_state = 0x0U;
            /************************************/
            /*  CALLMAID BUTTON STATE CHANGED   */
            /************************************/        
            if      (btn_maid_state && !btn_maid_old_state)
            {
                btn_maid_old_state = 0x1U;                
                if(!IsButtonCallActiv())
                {
                    ButtonCallSet();
                    ButtonDndReset();
                }
                else ButtonCallReset();
                ButtonRemoteSet();
                ButtonUpdateSet();
                SetpointUpdateSet(); // save new button state
            }
            else if (!btn_maid_state && btn_maid_old_state) btn_maid_old_state = 0x0U;
            /************************************/
            /*      SOS BUTTON STATE CHANGED    */
            /************************************/ 
            if (IsButtonSosActiv() && btn_sos_state)
            {
                ButtonSosReset();
                ButtonUpdateSet();
                ButtonRemoteSet();
                SetpointUpdateSet(); // save new button state
                BUZZER_SignalOff();
            }
            /************************************/
            /*  WEATHER BUTTON STATE CHANGED    */
            /************************************/
            else if (IsWFC_ValidActiv() && btn_sos_state)
            {
                GUI_SelectLayer(0);
                GUI_SetBkColor(GUI_BLACK); 
                GUI_Clear();
                GUI_SelectLayer(1);
                GUI_SetBkColor(GUI_TRANSPARENT); 
                GUI_Clear();
                disp_img_id = 20U;
                DISP_UpdateSet();
            }
        }
        /************************************/
        /*      SETPOINT  VALUE  INCREASED  */
        /************************************/ 
        if      ( btn_increase_state && !btn_increase_old_state)
		{
			btn_increase_old_state = 1U;
            
            if (thst_sp < thst_max_sp) 
            {
                ++thst_sp;
                ButtonRemoteSet();
                SetpointUpdateSet();
                BUZZER_StartSignal(BUZZER_CLICK);
            }
		}
		else if (!btn_increase_state &&  btn_increase_old_state) btn_increase_old_state = 0x0U;
        /************************************/
        /*      SETPOINT  VALUE  DECREASED  */
        /************************************/ 
        if      ( btn_decrease_state && !btn_decrease_old_state)
		{
			btn_decrease_old_state = 1U;
            
            if (thst_sp > thst_min_sp) 
            {
                --thst_sp;
                ButtonRemoteSet();
                SetpointUpdateSet();
                BUZZER_StartSignal(BUZZER_CLICK);
            }
		}
		else if (!btn_decrease_state &&  btn_decrease_old_state) btn_decrease_old_state = 0x0U;
        /** ==========================================================================*/
        /**   R E W R I T E   A N D   S A V E   N E W   S E T P O I N T   V A L U E   */
        /** ==========================================================================*/
        if (IsSetpointUpdateActiv()) // temperature setpoint changed online or gui
        {
            SetpointUpdateReset();
            DISP_TemperatureSetPoint();
            DISP_SaveSetPoint();
        }
        /** ==========================================================================*/
        /**         R E D R A W     R O M       S T A T U S      B U T T O N S        */
        /** ==========================================================================*/
        if (IsButtonUpdateActiv())  
        {
            ButtonUpdateReset();
            button_enable_tmr = HAL_GetTick();
            if      (IsButtonSosActiv())    BUZZER_StartSignal(BUZZER_SHORT);
            else                            BUZZER_StartSignal(BUZZER_CLICK);
            if      (IsButtonDndActiv())    GUI_DrawBitmap(&bmbtn_dnd_1,    BTN_DND_X0, BTN_DND_Y0);
            else                            GUI_DrawBitmap(&bmbtn_dnd_0,    BTN_DND_X0, BTN_DND_Y0);
            if      (IsButtonCallActiv())   GUI_DrawBitmap(&bmbtn_maid_1,   BTN_CMD_X0, BTN_CMD_Y0);
            else                            GUI_DrawBitmap(&bmbtn_maid_0,   BTN_CMD_X0, BTN_CMD_Y0);
            if      (IsButtonSosActiv())    GUI_DrawBitmap(&bmbtn_rst_sos_1,BTN_SOS_X0 + 20, BTN_SOS_Y0);
            else if (IsWFC_ValidActiv())    GUI_DrawBitmap(&bmbtn_weather,  BTN_SOS_X0, BTN_SOS_Y0);
            else                            GUI_ClearRect (BTN_SOS_X0,      BTN_SOS_Y0, BTN_SOS_X1, BTN_SOS_Y1);
        }
        /** ==========================================================================*/
        /**             W R I T E    D A T E    &    T I M E    S C R E E N           */
        /** ==========================================================================*/
        if ((HAL_GetTick() - disp_rtc_tmr) >= DATE_TIME_REFRESH_TIME) // screansaver clock time update 
        {
            disp_rtc_tmr = HAL_GetTick();
            DISP_DateTime();
        }
    }
    else if (DISP_Screen == DISP_FORECAST)   // weather forecast screen
    {
        if ((HAL_GetTick() - disp_img_tmr) >= WFC_TOUT) // exit screen on timeout
        {
            DISP_GuiSet(); // set display image id to user gui
        }
    }
    else if (DISP_Screen == DISP_MESSAGE)    // room guest display message
	{
        if((HAL_GetTick() - button_enable_tmr) >= BUTTON_RESPONSE_TIME) // enable user button after small delay
        {
            /************************************/
            /*      CHECK BUTTON OK STATE       */
            /************************************/
            if(btn_ok_state) // if enbled button to quit message
            {
                if(last_img_id == 10U) DISP_FaultSet();// check previous state and return to error screen
                else DISP_GuiSet();                     // or clear all display thermostat GUI
                BUZZER_StartSignal(BUZZER_CLICK);       // buzzer is set after display macro, or will be oweritten by macro call
                ButtonOkSet();                          // rise flag for interrupt driven onewire communication to send this event
                BtnOkPresed();
                ButtonRemoteSet();
            }
            /************************************/
            /*    CHECK BUTTON OPENDOOR STATE   */
            /************************************/
            if(btn_opendoor_state) // if enabled button to activate door lock and quit message
            {
                if(last_img_id == 10U) DISP_FaultSet(); // check previous state and return to error screen
                else DISP_GuiSet(); // or clear all display thermostat GUI
                BUZZER_StartSignal(BUZZER_CLICK); // buzzer is set after display macro, or will be oweritten by macro call
                ButtonOpenSet(); // rise flag for interrupt driven onewire communication to send this event
                ButtonRemoteSet();
            }
        }
        /************************************/
        /*      CHECK FOR MESSAGE TIMEOUT   */
        /************************************/
        if(disp_img_time != 0U) // if defined message timer in multiply * 30 sec, wait for timer timeout and exit message
        {
            if((HAL_GetTick() - disp_img_tmr) >= (disp_img_time * DISP_IMG_TIME_MULTI))
            {
                if(last_img_id == 10U) DISP_FaultSet();
                else DISP_GuiSet();
                BUZZER_StartSignal(BUZZER_CLICK); // buzzer set after display set macro, or will be oweritten by macro call
                BtnOkRelease();
            }
        }
    }    
    else if (DISP_Screen == DISP_ERROR)      // display error message to inform user about misuse and conditions
	{
        /************************************/
        /*      CHECK FOR MESSAGE TIMEOUT   */
        /************************************/
        if(disp_img_time != 0U) // if defined message timer in multiply * 30 sec, wait for timer timeout and exit message
        {
            if((HAL_GetTick() - disp_img_tmr) >= (disp_img_time * DISP_IMG_TIME_MULTI))
            {
                DISP_GuiSet();
            }
        }
    }
    else if (DISP_Screen == DISP_SETTINGS_1) // setup menu 1
    {
        /** ==========================================================================*/
        /**     S C R E E N     I N I T I A L I Z A T I O N     R E Q U E S T         */
        /** ==========================================================================*/
        thst_max_sp     = SPINBOX_GetValue(hSPNBX_ThermostatMaxSetpointTemperature);
        thst_min_sp     = SPINBOX_GetValue(hSPNBX_ThermostatMinSetpointTemperature);
        ntc_offset      = SPINBOX_GetValue(hSPNBX_AmbientNtcOffset);
        if (ow_ifa  != SPINBOX_GetValue(hSPNBX_OneWireInterfaceAddress))
        {
            ow_ifa = SPINBOX_GetValue(hSPNBX_OneWireInterfaceAddress);
            OW_Init();
        }
        /************************************/
        /*    RADIO SENSOR CONFIG STATE     */
        /************************************/
        if      (BUTTON_IsPressed(hBUTTON_RFSenUp)      && ((thflag & 0x4U) == 0x0U))
        {
            thflag |= 0x4U; // prevent touch event loop
            DISP_RFSenSet();
            if (rfsen_sel < (RFSEN_BSIZE - 0x1U))
            {
                if (rfsen[rfsen_sel+1U]) ++rfsen_sel;
            }
        }
        else if (!BUTTON_IsPressed(hBUTTON_RFSenUp)     && ((thflag & 0x4U) != 0x0U)) thflag &= 0xFBU;
            
        if      (BUTTON_IsPressed(hBUTTON_RFSenDown)    && ((thflag & 0x8U) == 0x0U))
        {
            thflag |= 0x8U; // prevent touch event loop
            DISP_RFSenSet();
            if (rfsen_sel) --rfsen_sel;
        }
        else if (!BUTTON_IsPressed(hBUTTON_RFSenDown)   && ((thflag & 0x8U) != 0x0U)) thflag &= 0xF7U;
        
        if      (BUTTON_IsPressed(hBUTTON_RFSenDelete)  && ((thflag & 0x10U) == 0x0U))
        {   // check is selected sensor exist
            thflag |= 0x10U;    // prevent touch event loop
            
            if (rfsen[rfsen_sel]) 
            {   
                if (rfsen_sel < RFSEN_BSIZE-1U)
                {
                    memmove(&rfsen[rfsen_sel], &rfsen[rfsen_sel+1U], (RFSEN_BSIZE-rfsen_sel-1U)*2U);
                }
                rfsen[RFSEN_BSIZE-1U] = 0U;
                EDIT_SetText(hEDIT_RFSen, "DELETED");
                GUI_Delay(500);
                DISP_RFSenSet();
                RFSenNewSet();
            }
        }
        else if (!BUTTON_IsPressed(hBUTTON_RFSenDelete) && ((thflag & 0x10U) != 0x0U)) thflag &= 0xEFU;
        
        if      (BUTTON_IsPressed(hBUTTON_RFSenAdd)     && ((thflag & 0x20U) == 0x0U))
        {   // selected next empty index
            thflag |= 0x20U; // prevent touch event loop
            while (rfsen[rfsen_sel] && (rfsen_sel < RFSEN_BSIZE)) ++rfsen_sel;
            if (rfsen_sel >= RFSEN_BSIZE) EDIT_SetText(hEDIT_RFSen, "LIST FULL");
            else 
            {
                EDIT_SetText(hEDIT_RFSen, "WAIT EVENT");
                RFSenAddSet();
            }
        }
        else if (!BUTTON_IsPressed(hBUTTON_RFSenAdd)    && ((thflag & 0x20U) != 0x0U)) thflag &= 0xDFU;
        
        /* SAVE VARIABLE TO EPPROM */
        if (IsRFSenNewActiv())
        {   // callback to save new address list
            RFSenNewReset(); // run once
            DISP_SaveRFSen();
        }
        
        /* WRITE SENSOR TYPE AND ADDRESS */
        if (IsDISP_RFSenActiv())
        {   // calback flag to write sensor address
            DISP_RFSenReset();  // run once
            ZEROFILL(sentxt, COUNTOF(sentxt));  // clear text buffer
            if (rfsen[rfsen_sel])
            {
                if (rfsen[rfsen_sel] & (0x1U << 15U))   mem_cpy(sentxt, "NO/SOS ", 7); // write sensor type
                else                                    mem_cpy(sentxt, "SWITCH ", 7); // to easy sesor reckognize
                Int2Str (&sentxt[7], (rfsen[rfsen_sel] & 0x7FFFU), 0);  // insert sensor address after type 
                EDIT_SetText(hEDIT_RFSen, sentxt);  // write formated text inside sensor edit box
            }
        }
        /************************************/
        /*    ROOM TEMPERATURE NTC SENSOR   */
        /************************************/
        if      (!IsNtcValidActiv())
        {
            if(!IsNtcUpdateActiv())
            {
                NtcUpdateSet();
                GUI_ClearRect(10, 220, 200, 110);
                GUI_GotoXY(20, 240);
                GUI_SetColor(GUI_YELLOW);
                GUI_SetFont(GUI_FONT_24_1);
                GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
                GUI_DispString("ERROR");                
            }
        }
        else if (IsRoomTempUpdateActiv())
        {
            RoomTempUpdateReset();
            NtcUpdateReset();
            GUI_ClearRect(10, 220, 200, 260);
            GUI_GotoXY(20, 240);
            GUI_SetColor(GUI_YELLOW);
            GUI_SetFont(GUI_FONT_24_1);
            GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
            DISP_Temperature(room_temp);
        }
        /************************************/
        /*      CHECK BUTTON OK STATE       */
        /************************************/
        if(BUTTON_IsPressed(hBUTTON_Ok)) 
        {
            DISP_SaveSettings();
            DISP_DeleteSettings1Screen();
            DISP_Screen = DISP_THERMOSTAT; // set display swith state to user gui
            if(last_img_id == 10U) DISP_FaultSet();
            else DISP_GuiSet();
        }
        /************************************/
        /*      CHECK BUTTON NEXT STATE     */
        /************************************/
        else if(BUTTON_IsPressed(hBUTTON_Next)) 
        {
            disp_img_id = 30U;
            NtcUpdateReset();
            RoomTempUpdateSet();
            DISP_DeleteSettings1Screen();
            DISP_CreateSettings2Screen();
            DISP_Screen = DISP_SETTINGS_2;
        }
        /************************************/
        /*      CHECK FOR MENU TIMEOUT      */
        /************************************/
        if(disp_img_tmr != disp_sreensvr_tmr) // if defined message timer in multiply * 30 sec, wait for timer timeout and exit message
        {
            disp_img_tmr = disp_sreensvr_tmr;
        }
        else if((HAL_GetTick() - disp_img_tmr) >= SETTINGS_MENU_TIMEOUT)
        {
            DISP_DeleteSettings1Screen();
            DISP_Screen = DISP_THERMOSTAT; // set display swith state to user gui
            if(last_img_id == 10U) DISP_FaultSet();
            else DISP_GuiSet();
        }
    }
    else if (DISP_Screen == DISP_SETTINGS_2) // setup menu 2
    { 
        disp_high_bcklght   = SPINBOX_GetValue(hSPNBX_DisplayHighBrightness);
        disp_low_bcklght    = SPINBOX_GetValue(hSPNBX_DisplayLowBrightness);
        scrnsvr_tout        = SPINBOX_GetValue(hSPNBX_ScrnsvrTimeout);
        scrnsvr_ena_hour    = SPINBOX_GetValue(hSPNBX_ScrnsvrEnableHour);
        scrnsvr_dis_hour    = SPINBOX_GetValue(hSPNBX_ScrnsvrDisableHour);
        
        if (CHECKBOX_GetState(hCHKBX_ScrnsvrClock) == 1)        ScrnsvrClkSet();
        else ScrnsvrClkReset();
        
        if (CHECKBOX_GetState(hCHKBX_ScrnsvrLogoClock) == 1)    ScrnsvrSemiClkSet();
        else ScrnsvrSemiClkReset();
        
        if (rtctm.Hours             != Dec2Bcd(SPINBOX_GetValue(hSPNBX_Hour)))
        {
            rtctm.Hours = Dec2Bcd(SPINBOX_GetValue(hSPNBX_Hour));
            HAL_RTC_SetTime(&hrtc, &rtctm, RTC_FORMAT_BCD);
            RtcValidSet();
        }
        
        if (rtctm.Minutes           != Dec2Bcd(SPINBOX_GetValue(hSPNBX_Minute)))
        {
            rtctm.Minutes = Dec2Bcd(SPINBOX_GetValue(hSPNBX_Minute));
            HAL_RTC_SetTime(&hrtc, &rtctm, RTC_FORMAT_BCD);
            RtcValidSet();
        }
        
        if (rtcdt.Date              != Dec2Bcd(SPINBOX_GetValue(hSPNBX_Day)))
        {
            rtcdt.Date = Dec2Bcd(SPINBOX_GetValue(hSPNBX_Day));
            HAL_RTC_SetDate(&hrtc, &rtcdt, RTC_FORMAT_BCD);
            RtcValidSet();
        }
        
        if (rtcdt.Month             != Dec2Bcd(SPINBOX_GetValue(hSPNBX_Month)))
        {
            rtcdt.Month = Dec2Bcd(SPINBOX_GetValue(hSPNBX_Month));
            HAL_RTC_SetDate(&hrtc, &rtcdt, RTC_FORMAT_BCD);
            RtcValidSet();
        }
        
        if (rtcdt.Year              != Dec2Bcd(SPINBOX_GetValue(hSPNBX_Year) - 2000))
        {
            rtcdt.Year = Dec2Bcd(SPINBOX_GetValue(hSPNBX_Year) - 2000);
            HAL_RTC_SetDate(&hrtc, &rtcdt, RTC_FORMAT_BCD);
            RtcValidSet();
        }
        
        if (scrnsvr_clk_clr       != SPINBOX_GetValue(hSPNBX_ScrnsvrClockColour))
        {
            scrnsvr_clk_clr = SPINBOX_GetValue(hSPNBX_ScrnsvrClockColour);
            GUI_SetColor(clk_clrs[scrnsvr_clk_clr]);
            GUI_FillRect(340, 51, 430, 59);
        }
        
        if (scrnsvr_semiclk_clr   != SPINBOX_GetValue(hSPNBX_ScrnsvrSemiClkColor))
        {
            scrnsvr_semiclk_clr = SPINBOX_GetValue(hSPNBX_ScrnsvrSemiClkColor);
            GUI_SetColor(clk_clrs[scrnsvr_semiclk_clr]);
            GUI_FillRect(340, 91, 430, 99);
        }
        /************************************/
        /*      CHECK BUTTON OK STATE       */
        /************************************/
        if(BUTTON_IsPressed(hBUTTON_Ok)) 
        {
            DISP_SaveSettings();
            DISP_DeleteSettings2Screen();
            DISP_Screen = DISP_THERMOSTAT; // set display swith state to user gui
            if(last_img_id == 10U) DISP_FaultSet();
            else DISP_GuiSet();
        }
        /************************************/
        /*      CHECK BUTTON NEXT STATE     */
        /************************************/        
        else if(BUTTON_IsPressed(hBUTTON_Next)) 
        {
            disp_img_id = 30U;
            DISP_DeleteSettings2Screen();
            DISP_CreateSettings1Screen();
            DISP_Screen = DISP_SETTINGS_1;
        }
        /************************************/
        /*      CHECK FOR MENU TIMEOUT      */
        /************************************/
        if(disp_img_tmr != disp_sreensvr_tmr) // if defined message timer in multiply * 30 sec, wait for timer timeout and exit message
        {
            disp_img_tmr = disp_sreensvr_tmr;
        }
        else if((HAL_GetTick() - disp_img_tmr) >= SETTINGS_MENU_TIMEOUT)
        {
            DISP_DeleteSettings2Screen();
            DISP_Screen = DISP_THERMOSTAT; // set display swith state to user gui
            if(last_img_id == 10U) DISP_FaultSet();
            else DISP_GuiSet();
        }
    }
}
void DISP_RFSensor(uint16_t addr, uint8_t stat)
{
    uint8_t idx = 0;
    // wait for sensor event in settings menu
    if (IsRFSenCfgActiv())
    {   // get in while setting menu 1 activ
        if (IsRFSenAddActiv())  
        {   // add only new sensor 
            RFSenAddReset(); // prevent loop
            rfsen[rfsen_sel] = addr; // save received sensor address
            idx = rfsen_sel; // copy new sensor address index
            while (idx)      // search loop using copy of index
            {                // to preserve user selected address index value
                --idx;       // check is received seensor address new, and if address allready in list, 
                if  (rfsen[idx] == addr) rfsen[rfsen_sel] = 0; // clear last saved from list   
            }
            if      (rfsen[rfsen_sel])  RFSenNewSet(), DISP_RFSenSet(); // save new address to list, and write sensor info
            if     (!rfsen[rfsen_sel])  EDIT_SetText(hEDIT_RFSen, "INCLUDED"); // sensor address exist
            else if (stat == 0x21)  rfsen[rfsen_sel] |= (0x1<<15); // set msbit if sos sensor
            else if((stat == 0x10)||(stat == 0x11)) DISP_RFSenSet(); // to easy sesor reckognize
        }
    }
    else
    {   // search received sensor address in associated sensor list
        idx = 0x0; // reset address index
        while (rfsen[idx] && (idx < RFSEN_BSIZE))  
        {   // exclude msb bit from addresse, bit is used to mark sensor type
            if ((addr & 0x7FFF) == (rfsen[idx] & 0x7FFF))
            {   // valid sensor event is received, set system state
                idx = RFSEN_BSIZE; // exit search loop
                if      (stat == 0x10)  // RF. switch NC event
                {
                    ExtSwClosed();
                    ButtonRemoteSet();
                }
                else if (stat == 0x11)  // RF. switch NO event
                {
                    ExtSwOpen();
                    ButtonRemoteSet();
                }
                else if (stat == 0x21)  // SOS sensor NC event
                {
                    ButtonSosSet();
                    ButtonUpdateSet();
                    SetpointUpdateSet(); // save new button state
                }
            }
            else ++idx; // keep loop live
        }
    }
}

/**
  * @brief  Display coustom color+size+type text in coustom color+size+position room   
  * @param  0roomcolour,1+2roomx0pos,3+4roomy0pos,5+6roomx1pos,7+8roomy1pos,9txtcolour,10txtfont,11txthalign,12txtvalign,,13msgtxt
  * @retval disp_img_id = 50
  */
uint8_t DISP_Message(uint8_t *buff)
{
    int rect_x0, rect_y0, rect_x1, rect_y1;
    //
    //  check all parameter value
    //
    rect_x0 = ((buff[1] << 8) & 0xFF00U) + (buff[2] & 0xFFU);
    rect_y0 = ((buff[3] << 8) & 0xFF00U) + (buff[4] & 0xFFU);
    rect_x1 = ((buff[5] << 8) & 0xFF00U) + (buff[6] & 0xFFU);
    rect_y1 = ((buff[7] << 8) & 0xFF00U) + (buff[8] & 0xFFU);    
    if      (buff[0] > 25U) return NAK; // room color wrong
    else if (rect_x0 > DISP_XSIZE) return NAK; // message room  x0 position wrong
    else if (rect_y0 > DISP_YSIZE) return NAK; // message room  y0 position wrong
    else if (rect_x1 > DISP_XSIZE) return NAK; // message room  x1 position wrong
    else if (rect_y1 > DISP_YSIZE) return NAK; // message room  y1 position wrong
    else if (buff[9] > 25) return NAK; // font color wrong
    else if (buff[10]> 12) return NAK; // font size wrong
    else if (buff[11]>  4) return NAK; // text horisontal align wrong
    else if (buff[12]>  4) return NAK; // text vertical align wrong
    else if (GetSize(&buff[13]) == 0U) return NAK; // message text empty
    GUI_SetTextMode(GUI_TM_TRANS);
    //
    //  draw room
    //
    if      (buff[0] == 0U)  GUI_SetColor(GUI_BLUE);
    else if (buff[0] == 1U)  GUI_SetColor(GUI_GREEN);
    else if (buff[0] == 2U)  GUI_SetColor(GUI_RED);
    else if (buff[0] == 3U)  GUI_SetColor(GUI_CYAN);
    else if (buff[0] == 4U)  GUI_SetColor(GUI_MAGENTA);
    else if (buff[0] == 5U)  GUI_SetColor(GUI_YELLOW);
    else if (buff[0] == 6U)  GUI_SetColor(GUI_LIGHTBLUE);
    else if (buff[0] == 7U)  GUI_SetColor(GUI_LIGHTGREEN);
    else if (buff[0] == 8U)  GUI_SetColor(GUI_LIGHTRED);
    else if (buff[0] == 9U)  GUI_SetColor(GUI_LIGHTCYAN);
    else if (buff[0] == 10U) GUI_SetColor(GUI_LIGHTMAGENTA);
    else if (buff[0] == 11U) GUI_SetColor(GUI_LIGHTYELLOW);
    else if (buff[0] == 12U) GUI_SetColor(GUI_DARKBLUE);
    else if (buff[0] == 13U) GUI_SetColor(GUI_DARKGREEN);
    else if (buff[0] == 14U) GUI_SetColor(GUI_DARKRED);
    else if (buff[0] == 15U) GUI_SetColor(GUI_DARKCYAN);
    else if (buff[0] == 16U) GUI_SetColor(GUI_DARKMAGENTA);
    else if (buff[0] == 17U) GUI_SetColor(GUI_DARKYELLOW);
    else if (buff[0] == 18U) GUI_SetColor(GUI_WHITE);
    else if (buff[0] == 19U) GUI_SetColor(GUI_LIGHTGRAY);
    else if (buff[0] == 20U) GUI_SetColor(GUI_GRAY);
    else if (buff[0] == 21U) GUI_SetColor(GUI_DARKGRAY);
    else if (buff[0] == 22U) GUI_SetColor(GUI_BLACK);
    else if (buff[0] == 23U) GUI_SetColor(GUI_BROWN);
    else if (buff[0] == 24U) GUI_SetColor(GUI_ORANGE);
    else if (buff[0] == 25U) GUI_SetColor(GUI_TRANSPARENT);
    GUI_RECT Rect = {rect_x0, rect_y0, rect_x1, rect_y1};
    GUI_FillRectEx(&Rect);
    //
    //  write text
    //
    int hal, val;
    if      (buff[9] == 0U)  GUI_SetColor(GUI_BLUE);
    else if (buff[9] == 1U)  GUI_SetColor(GUI_GREEN);
    else if (buff[9] == 2U)  GUI_SetColor(GUI_RED);
    else if (buff[9] == 3U)  GUI_SetColor(GUI_CYAN);
    else if (buff[9] == 4U)  GUI_SetColor(GUI_MAGENTA);
    else if (buff[9] == 5U)  GUI_SetColor(GUI_YELLOW);
    else if (buff[9] == 6U)  GUI_SetColor(GUI_LIGHTBLUE);
    else if (buff[9] == 7U)  GUI_SetColor(GUI_LIGHTGREEN);
    else if (buff[9] == 8U)  GUI_SetColor(GUI_LIGHTRED);
    else if (buff[9] == 9U)  GUI_SetColor(GUI_LIGHTCYAN);
    else if (buff[9] == 10U) GUI_SetColor(GUI_LIGHTMAGENTA);
    else if (buff[9] == 11U) GUI_SetColor(GUI_LIGHTYELLOW);
    else if (buff[9] == 12U) GUI_SetColor(GUI_DARKBLUE);
    else if (buff[9] == 13U) GUI_SetColor(GUI_DARKGREEN);
    else if (buff[9] == 14U) GUI_SetColor(GUI_DARKRED);
    else if (buff[9] == 15U) GUI_SetColor(GUI_DARKCYAN);
    else if (buff[9] == 16U) GUI_SetColor(GUI_DARKMAGENTA);
    else if (buff[9] == 17U) GUI_SetColor(GUI_DARKYELLOW);
    else if (buff[9] == 18U) GUI_SetColor(GUI_WHITE);
    else if (buff[9] == 19U) GUI_SetColor(GUI_LIGHTGRAY);
    else if (buff[9] == 20U) GUI_SetColor(GUI_GRAY);
    else if (buff[9] == 21U) GUI_SetColor(GUI_DARKGRAY);
    else if (buff[9] == 22U) GUI_SetColor(GUI_BLACK);
    else if (buff[9] == 23U) GUI_SetColor(GUI_BROWN);
    else if (buff[9] == 24U) GUI_SetColor(GUI_ORANGE);
    else if (buff[9] == 25U) GUI_SetColor(GUI_TRANSPARENT);
    if      (buff[10]== 0U)  GUI_SetFont(GUI_FONT_8_1);
    else if (buff[10]== 1U)  GUI_SetFont(GUI_FONT_10_1);
    else if (buff[10]== 2U)  GUI_SetFont(GUI_FONT_13_1);
    else if (buff[10]== 3U)  GUI_SetFont(GUI_FONT_13B_1);
    else if (buff[10]== 4U)  GUI_SetFont(GUI_FONT_16_1);
    else if (buff[10]== 5U)  GUI_SetFont(GUI_FONT_16B_1);
    else if (buff[10]== 6U)  GUI_SetFont(GUI_FONT_20_1);
    else if (buff[10]== 7U)  GUI_SetFont(GUI_FONT_20B_1);
    else if (buff[10]== 8U)  GUI_SetFont(GUI_FONT_24_1);
    else if (buff[10]== 9U)  GUI_SetFont(GUI_FONT_24B_1);
    else if (buff[10]== 10U) GUI_SetFont(GUI_FONT_32_1);
    else if (buff[10]== 11U) GUI_SetFont(GUI_FONT_32B_1);
    if      (buff[11]== 0U)  hal = GUI_TA_LEFT;
    else if (buff[11]== 1U)  hal = GUI_TA_HORIZONTAL;
    else if (buff[11]== 2U)  hal = GUI_TA_RIGHT;
    else if (buff[11]== 3U)  hal = GUI_TA_CENTER;
    else if (buff[11]== 4U)  hal = GUI_TA_HCENTER;
    if      (buff[12]== 0U)  val = GUI_TA_TOP;
    else if (buff[12]== 1U)  val = GUI_TA_VERTICAL;
    else if (buff[12]== 2U)  val = GUI_TA_BOTTOM;
    else if (buff[12]== 3U)  val = GUI_TA_BASELINE;
    else if (buff[12]== 4U)  val = GUI_TA_VCENTER;
    GUI_DispStringInRectWrap((char*)&buff[13], &Rect, hal | val, GUI_WRAPMODE_WORD);
    return ACK; // parameter error
}
/**
  * @brief
  * @param
  * @retval
  */
uint8_t DISP_QR_Code(void)
{
    uint16_t rd = QRC_DSIZE;
    uint8_t qr_code[QRC_BSIZE];
//    int rect_x0, rect_y0, rect_x1, rect_y1;
//    //
//    //  check all parameter value
//    //
//    rect_x0 = ((buff[1] << 8) & 0xFF00U) + (buff[2] & 0xFFU);
//    rect_y0 = ((buff[3] << 8) & 0xFF00U) + (buff[4] & 0xFFU);
//    rect_x1 = ((buff[5] << 8) & 0xFF00U) + (buff[6] & 0xFFU);
//    rect_y1 = ((buff[7] << 8) & 0xFF00U) + (buff[8] & 0xFFU);  
//    GUI_RECT Rect = {rect_x0, rect_y0, rect_x1, rect_y1};
//    GUI_SetColor(GUI_WHITE);
//    GUI_FillRectEx(&Rect);
    
    ZEROFILL(qr_code, COUNTOF(qr_code));
    EE_ReadBuffer(qr_code, EE_QR_CODE, &rd);
    GUI_SelectLayer(0);
    GUI_SetBkColor(GUI_WHITE);
    GUI_Clear();
    GUI_SelectLayer(1);
    GUI_Clear();
    hQR_Code = GUI_QR_Create((char*) qr_code, 4, GUI_QR_ECLEVEL_L, 0);
    GUI_MULTIBUF_BeginEx(1);
    GUI_QR_Draw(hQR_Code, 50, 40);
//    GUI_QR_Draw(hQR_Code, rect_x0, rect_y0);
    GUI_MULTIBUF_EndEx(1);
    return 0x0U;
}
/**
  * @brief  Display Backlight LED brightnes control
  * @param  brightnes_high_level
  * @retval disp_ sreensvr_tmr loaded with system_tmr value
  */
void DISP_SetBrightnes(uint8_t val)
{
    if      (val < DISP_BRGHT_MIN) val = DISP_BRGHT_MIN;
    else if (val > DISP_BRGHT_MAX) val = DISP_BRGHT_MAX;
    
    __HAL_TIM_SET_COMPARE(&htim9, TIM_CHANNEL_1, (uint16_t) (val * 10U));
}
/**
  * @brief  Display Screen Redraw
  * @param  new image id
  * @retval new activ screen
  */
static void DISP_DrawScreen(uint8_t img_id)
{
    GUI_SelectLayer(0);
    GUI_Clear();
    GUI_MULTIBUF_BeginEx(0);

    if      (img_id == 0U) GUI_BMP_Draw(&disp_00, 0, 0); //  gui = thermostat 
    else if (img_id == 1U) GUI_BMP_Draw(&disp_01, 0, 0); //  eng user message 1- 5
    else if (img_id == 2U) GUI_BMP_Draw(&disp_02, 0, 0);
    else if (img_id == 3U) GUI_BMP_Draw(&disp_03, 0, 0);
    else if (img_id == 4U) GUI_BMP_Draw(&disp_04, 0, 0);
    else if (img_id == 5U) GUI_BMP_Draw(&disp_05, 0, 0);
    
    else if (img_id == 10U) GUI_BMP_Draw(&disp_10, 0, 0); //  gui = user fault message
    else if (img_id == 11U) GUI_BMP_Draw(&disp_11, 0, 0); //  ger user message 11-15
    else if (img_id == 12U) GUI_BMP_Draw(&disp_12, 0, 0);
    else if (img_id == 13U) GUI_BMP_Draw(&disp_13, 0, 0);
    else if (img_id == 14U) GUI_BMP_Draw(&disp_14, 0, 0);
    else if (img_id == 15U) GUI_BMP_Draw(&disp_15, 0, 0);

    else if (img_id == 20U) ;  // nothing to draw on this layer   //  gui = weather forecast
    else if (img_id == 21U) GUI_BMP_Draw(&disp_21, 0, 0); //  fra user message 21-25
    else if (img_id == 22U) GUI_BMP_Draw(&disp_22, 0, 0);
    else if (img_id == 23U) GUI_BMP_Draw(&disp_23, 0, 0);
    else if (img_id == 24U) GUI_BMP_Draw(&disp_24, 0, 0);
    else if (img_id == 25U) GUI_BMP_Draw(&disp_25, 0, 0);

    else if (img_id == 30U);    // nothing to draw on this layer   //  gui = settings menu
    else if (img_id == 31U);    //GUI_BMP_Draw(&disp_31, 0, 0); //  arab user message 31-35
    else if (img_id == 32U);    //GUI_BMP_Draw(&disp_32, 0, 0);
    else if (img_id == 33U);    //GUI_BMP_Draw(&disp_33, 0, 0);
    else if (img_id == 34U);    //GUI_BMP_Draw(&disp_34, 0, 0);
    else if (img_id == 35U);    //GUI_BMP_Draw(&disp_35, 0, 0);

    else if (img_id == 40U);    // nothing to draw on this layer   //  gui = qr code
    else if (img_id == 41U);    //GUI_BMP_Draw(&disp_41, 0, 0); //  chn user message 41-45
    else if (img_id == 42U);    // GUI_BMP_Draw(&disp_42, 0, 0);  
    else if (img_id == 43U);    // GUI_BMP_Draw(&disp_43, 0, 0);
    else if (img_id == 44U);    // GUI_BMP_Draw(&disp_44, 0, 0);
    else if (img_id == 45U);    // GUI_BMP_Draw(&disp_45, 0, 0);

    else if (img_id == 50U);    // nothing to draw on this layer   //  gui = coustom message
    else if (img_id == 51U);    //GUI_BMP_Draw(&disp_51, 0, 0); //  jap user message 51-55
    else if (img_id == 52U);    //GUI_BMP_Draw(&disp_52, 0, 0);
    else if (img_id == 53U);    //GUI_BMP_Draw(&disp_53, 0, 0);
    else if (img_id == 54U);    //GUI_BMP_Draw(&disp_54, 0, 0);
    else if (img_id == 55U);    //GUI_BMP_Draw(&disp_55, 0, 0);

    else if (img_id == 60U);// GUI_PNG_Draw(&logo, logo_size, 0, 0);                                //  user loogo
    else if (img_id == 61U) GUI_BMP_Draw(&disp_61, 0, 0); //  ita user message 61-65
    else if (img_id == 62U) GUI_BMP_Draw(&disp_62, 0, 0);
    else if (img_id == 63U) GUI_BMP_Draw(&disp_63, 0, 0);
    else if (img_id == 64U) GUI_BMP_Draw(&disp_64, 0, 0);
    else if (img_id == 65U) GUI_BMP_Draw(&disp_65, 0, 0);

    else if (img_id == 70U);
    else if (img_id == 71U) GUI_BMP_Draw(&disp_71, 0, 0); //  tur user message 71-75
    else if (img_id == 72U) GUI_BMP_Draw(&disp_72, 0, 0);
    else if (img_id == 73U) GUI_BMP_Draw(&disp_73, 0, 0);
    else if (img_id == 74U) GUI_BMP_Draw(&disp_74, 0, 0);
    else if (img_id == 75U) GUI_BMP_Draw(&disp_75, 0, 0);

//    else if (img_id == 80U);
//    else if (img_id == 81U) GUI_BMP_Draw(&disp_81, 0, 0); //  slo user message 81-85
//    else if (img_id == 82U) GUI_BMP_Draw(&disp_82, 0, 0);
//    else if (img_id == 83U) GUI_BMP_Draw(&disp_83, 0, 0);
//    else if (img_id == 84U) GUI_BMP_Draw(&disp_84, 0, 0);
//    else if (img_id == 85U) GUI_BMP_Draw(&disp_85, 0, 0);

    GUI_MULTIBUF_EndEx(0);
    GUI_SelectLayer(1);
    GUI_SetBkColor(GUI_TRANSPARENT); 
    GUI_Clear();
    GUI_MULTIBUF_BeginEx(1);
    if ((img_id  % 10U) != 0U)  GUI_DrawBitmap(&bmbtn_ok, BTN_OK_X0, BTN_OK_Y0); // draw button ok for image id with units different from 0 (1,2,3..9,11,12...)
    if ((img_id  % 10U) == 1U)  GUI_DrawBitmap(&bmbtn_door_open, BTN_DOOR_X0, BTN_DOOR_Y0); // draw button door open for image id with units of 1 (1,11,21...)     
    GUI_MULTIBUF_EndEx(1);
}
/**
  * @brief  Display Date and Time in deifferent font size colour and position
  * @param  Flags: IsRtcValidActiv, IsScrnsvrActiv, BUTTON_Dnd, BUTTON_CallMaid,
            ButtonSosReset, IsScrnsvrSemiClkActiv, IsScrnsvrClkActiv
  * @retval None
  */
static void DISP_DateTime(void)
{
    char dbuf[32];
    static uint8_t old_min = 60U, old_day = 0x0U;
    if(!IsRtcValidActiv()) return; // nothing to display untill system rtc validated
    HAL_RTC_GetTime(&hrtc, &rtctm, RTC_FORMAT_BCD);
    HAL_RTC_GetDate(&hrtc, &rtcdt, RTC_FORMAT_BCD);
    /************************************/
    /*   CHECK IS SCREENSAVER ENABLED   */
    /************************************/ 
    if      (scrnsvr_ena_hour >= scrnsvr_dis_hour)
    {
        if      (Bcd2Dec(rtctm.Hours) >= scrnsvr_ena_hour) ScrnsvrEnable();
        else if (Bcd2Dec(rtctm.Hours)  < scrnsvr_dis_hour) ScrnsvrEnable();
        else if (IsScrnsvrEnabled()) ScrnsvrDisable(), DISP_UpdateSet();
    }
    else if (scrnsvr_ena_hour < scrnsvr_dis_hour)
    {
        if      ((Bcd2Dec(rtctm.Hours) < scrnsvr_dis_hour) && (Bcd2Dec(rtctm.Hours) >= scrnsvr_ena_hour)) ScrnsvrEnable();
        else if (IsScrnsvrEnabled()) ScrnsvrDisable(), DISP_UpdateSet();
    }
    /************************************/
    /*      DISPLAY  DATE  &  TIME      */
    /************************************/ 
    if      (IsScrnsvrActiv() && IsScrnsvrEnabled() && (IsButtonDndActiv() || IsButtonCallActiv() || IsButtonSosActiv()) && IsScrnsvrSemiClkActiv())
    { 
        if(!IsScrnsvrInitActiv())
        {
            ScrnsvrInitSet();
            GUI_SelectLayer(0);
            GUI_Clear();
            GUI_SetColor(GUI_BLACK);
            GUI_FillRect(0, 0, 330, 80);
//            GUI_MULTIBUF_BeginEx(0);
//            GUI_BMP_Draw(&imgBackground, 0, 0);
//            GUI_MULTIBUF_EndEx(0);
            GUI_SelectLayer(1);
            GUI_SetBkColor(GUI_TRANSPARENT); 
            GUI_ClearRect(0, 220, 100, 270);
            rtctm.Seconds = 0x00;
        }
        
        GUI_SetColor(clk_clrs[scrnsvr_semiclk_clr]);
        GUI_SetFont(GUI_FONT_D80);
        GUI_SetTextAlign(GUI_TA_RIGHT);
        GUI_MULTIBUF_BeginEx(1);
        GUI_ClearRect(150, 0, 190, 100);
        if(rtctm.Seconds & 0x01U) GUI_DispCharAt(':', 150, 0);
        else if(rtctm.Seconds == 0x00U)
        {
            GUI_ClearRect(0, 0, 330, 80);
            GUI_DispHexAt(rtctm.Hours, 130, 0, 2);
            GUI_SetTextAlign(GUI_TA_LEFT);
            GUI_DispHexAt(rtctm.Minutes, 190, 0, 2);
        }
        
        GUI_MULTIBUF_EndEx(1);
    }
    else if (IsScrnsvrActiv() && IsScrnsvrEnabled() && !IsButtonDndActiv() && !IsButtonCallActiv() && !IsButtonSosActiv() && IsScrnsvrClkActiv())
    {
        if(!IsScrnsvrInitActiv() || (old_day != rtcdt.WeekDay))
        {
            ScrnsvrInitSet();
            GUI_SelectLayer(0);
            GUI_Clear();
//            GUI_MULTIBUF_BeginEx(0);
//            GUI_BMP_Draw(&imgBackground, 0, 0);
//            GUI_MULTIBUF_EndEx(0);
            GUI_SelectLayer(1);
            GUI_SetBkColor(GUI_TRANSPARENT); 
            GUI_Clear();
            old_min = 60U;
            old_day = rtcdt.WeekDay;
        }
        
        HEX2STR(dbuf, &rtctm.Hours);
        if(rtctm.Seconds & 0x01U) dbuf[2] = ':';
        else dbuf[2] = ' ';
        HEX2STR(&dbuf[3], &rtctm.Minutes);
        GUI_GotoXY(CLOCK_H_POS, CLOCK_V_POS);
        GUI_SetColor(clk_clrs[scrnsvr_clk_clr]);
        GUI_SetFont(GUI_FONT_D80);
        GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
        GUI_MULTIBUF_BeginEx(1);
        GUI_ClearRect(0, 80, 480, 192);
        GUI_DispString(dbuf);
        if      (rtcdt.WeekDay == 0x00) rtcdt.WeekDay = 0x07U;
        if      (rtcdt.WeekDay == 0x01) memcpy(dbuf,    "  Monday  ", 10);
        else if (rtcdt.WeekDay == 0x02) memcpy(dbuf,    " Tuestday ", 10);
        else if (rtcdt.WeekDay == 0x03) memcpy(dbuf,    "Wednesday ", 10);
        else if (rtcdt.WeekDay == 0x04) memcpy(dbuf,    "Thurstday ", 10);
        else if (rtcdt.WeekDay == 0x05) memcpy(dbuf,    "  Friday  ", 10);
        else if (rtcdt.WeekDay == 0x06) memcpy(dbuf,    " Saturday ", 10);
        else if (rtcdt.WeekDay == 0x07) memcpy(dbuf,    "  Sunday  ", 10);
        HEX2STR(&dbuf[10], &rtcdt.Date);
        if      (rtcdt.Month == 0x01) memcpy(&dbuf[12], ". January ", 10);		
        else if (rtcdt.Month == 0x02) memcpy(&dbuf[12], ". February", 10);
        else if (rtcdt.Month == 0x03) memcpy(&dbuf[12], ".  March  ", 10);
        else if (rtcdt.Month == 0x04) memcpy(&dbuf[12], ".  April  ", 10);
        else if (rtcdt.Month == 0x05) memcpy(&dbuf[12], ".   May   ", 10);
        else if (rtcdt.Month == 0x06) memcpy(&dbuf[12], ".   June  ", 10);
        else if (rtcdt.Month == 0x07) memcpy(&dbuf[12], ".   July  ", 10);
        else if (rtcdt.Month == 0x08) memcpy(&dbuf[12], ". August  ", 10);
        else if (rtcdt.Month == 0x09) memcpy(&dbuf[12], ".September", 10);
        else if (rtcdt.Month == 0x10) memcpy(&dbuf[12], ". Oktober ", 10);
        else if (rtcdt.Month == 0x11) memcpy(&dbuf[12], ". November", 10);
        else if (rtcdt.Month == 0x12) memcpy(&dbuf[12], ". December", 10);
        memcpy(&dbuf[22], " 20", 3U);
        HEX2STR(&dbuf[25], &rtcdt.Year);
        dbuf[27] = '.';
        dbuf[28] = NUL;
        GUI_SetFont(GUI_FONT_24B_1);
        GUI_GotoXY(CLOCK_H_POS, CLOCK_V_POS + 70U);
        GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
        GUI_DispString(dbuf);
        GUI_MULTIBUF_EndEx(1); 
    }
    else if(old_min != rtctm.Minutes) // thermostat gui clock
    {
        old_min = rtctm.Minutes;
        HEX2STR(dbuf, &rtctm.Hours);
        dbuf[2] = ':';
        HEX2STR(&dbuf[3], &rtctm.Minutes);
        GUI_SetFont(GUI_FONT_32_1);
        GUI_SetColor(GUI_WHITE);
        GUI_SetTextMode(GUI_TM_TRANS);
        GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
        GUI_GotoXY(5, 245);
        GUI_MULTIBUF_BeginEx(1);
        GUI_ClearRect(0, 220, 100, 270);
        GUI_DispString(dbuf);
        GUI_MULTIBUF_EndEx(1);
    }
}
/**
  * @brief  Display Backlight LED brightnes control
  * @param  brightnes_high_level
  * @retval disp_ sreensvr_tmr loaded with system_tmr value
  */
static void DISP_ResetScrnsvr(void)
{
    /**
    *   if screensaver activ mode digital clock request display redraw 
    *   or just set backlight to high 
    **/
    if(IsScrnsvrActiv() && IsScrnsvrEnabled() && (IsScrnsvrSemiClkActiv() || IsScrnsvrClkActiv())) 
    {
        DISP_UpdateSet();
    }
    
    ScrnsvrReset();
    ScrnsvrInitReset();
    disp_sreensvr_tmr = HAL_GetTick();
    DISP_SetBrightnes(disp_high_bcklght);
}
/**
  * @brief
  * @param
  * @retval
  */
static void DISP_TemperatureSetPoint(void)
{
    GUI_MULTIBUF_BeginEx(1);
    GUI_ClearRect(SP_H_POS - 5, SP_V_POS - 5, SP_H_POS + 120, SP_V_POS + 85);
    GUI_SetColor(GUI_WHITE);
    GUI_SetFont(GUI_FONT_D48);
    GUI_SetTextMode(GUI_TM_NORMAL);
    GUI_SetTextAlign(GUI_TA_RIGHT);
    GUI_GotoXY(SP_H_POS, SP_V_POS);
    GUI_DispDec(thst_sp , 2);
    GUI_MULTIBUF_EndEx(1);
}
/**
  * @brief
  * @param
  * @retval
  */
static void DISP_Temperature(int16_t value)
{
    GUI_DispSDecShift(value, 5, 1);
    GUI_DispString("°C");
}
/**
  * @brief
  * @param
  * @retval
  */
static uint8_t DISP_LoadWFC(int8_t *buff)
{
    RTC_t dt;
    uint32_t wfc_time_stamp;
    int8_t wfc_buf[WFC_DSIZE];
    
    if (!IsRtcValidActiv()) return 0x1U;
    RTC_GetDateTime(&dt, RTC_FORMAT_BIN);
    if (EE_ReadData(EE_ADDR, EE_WFORECAST, (uint8_t*)wfc_buf, WFC_DSIZE) != 0x0U) return 0x1U;
    wfc_time_stamp = (((wfc_buf[0] << 24) & 0xFF000000U)|((wfc_buf[1] << 16) & 0x00FF0000U)|
                      ((wfc_buf[2] <<  8) & 0x0000FF00U)| (wfc_buf[3]        & 0x000000FFU));
    if ((wfc_time_stamp == 0x0U) || (wfc_time_stamp == 0xFFFFFFFFU))  return 0x1U;
    if (wfc_time_stamp > (dt.unix + SECONDS_PER_DAY))  return 0x1U;
    if (dt.unix > (wfc_time_stamp + SECONDS_PER_DAY))  return 0x1U;
    if ((wfc_buf[4]  < -60) || (wfc_buf[4]  > 60)) return 0x1U;  // BYTE4: 	ACTUAL TEMPERATURE		int8  -60 ~ +60
    if ((wfc_buf[5]  <   1) || (wfc_buf[5]  >  7)) return 0x1U;  // BYTE5:  ACTUAL CLOUDNESS		uint8 	 1 ~ 7
    if ((wfc_buf[6]  < -60) || (wfc_buf[6]  > 60)) return 0x1U;  // BYTE6:  DAY + 1 HI TEMPERATURE	int8  -60 ~ +60
    if ((wfc_buf[7]  < -60) || (wfc_buf[7]  > 60)) return 0x1U;  // BYTE7:  DAY + 1 LO TEMPERATURE	int8  -60 ~ +60
    if ((wfc_buf[8]  <   1) || (wfc_buf[8]  >  7)) return 0x1U;  // BYTE8:  DAY + 1 CLOUDS			uint8	 1 ~ 7
    if ((wfc_buf[9]  < -60) || (wfc_buf[9]  > 60)) return 0x1U;  // BYTE9:  DAY + 2 HI TEMPERATURE	int8  -60 ~ +60
    if ((wfc_buf[10] < -60) || (wfc_buf[10] > 60)) return 0x1U;  // BYTE10: DAY + 2 LO TEMPERATURE	int8  -60 ~ +60
    if ((wfc_buf[11] <   1) || (wfc_buf[11] >  7)) return 0x1U;  // BYTE11:	DAY + 2 CLOUDS			uint8	 1 ~ 7
    if ((wfc_buf[12] < -60) || (wfc_buf[12] > 60)) return 0x1U;  // BYTE12:	DAY + 3 HI TEMPERATURE	int8  -60 ~ +60
    if ((wfc_buf[13] < -60) || (wfc_buf[13] > 60)) return 0x1U;  // BYTE13:	DAY + 3 LO TEMPERATURE	int8  -60 ~ +60
    if ((wfc_buf[14] <   1) || (wfc_buf[14] >  7)) return 0x1U;  // BYTE14:	DAY + 3 CLOUDS			uint8	 1 ~ 7
    if ((wfc_buf[15] < -60) || (wfc_buf[15] > 60)) return 0x1U;  // BYTE15:	DAY + 4 HI TEMPERATURE	int8  -60 ~ +60
    if ((wfc_buf[16] < -60) || (wfc_buf[16] > 60)) return 0x1U;  // BYTE16:	DAY + 4 LO TEMPERATURE	int8  -60 ~ +60
    if ((wfc_buf[17] <   1) || (wfc_buf[17] >  7)) return 0x1U;  // BYTE17:	DAY + 4 CLOUDS			uint8	 1 ~ 7
    if ((wfc_buf[18] < -60) || (wfc_buf[18] > 60)) return 0x1U;  // BYTE18:	DAY + 5 HI TEMPERATURE	int8  -60 ~ +60
    if ((wfc_buf[19] < -60) || (wfc_buf[19] > 60)) return 0x1U;  // BYTE19:	DAY + 5 LO TEMPERATURE	int8  -60 ~ +60
    if ((wfc_buf[20] <   1) || (wfc_buf[20] >  7)) return 0x1U;  // BYTE20:	DAY + 5 CLOUDS			uint8	 1 ~ 7
    memcpy(buff, wfc_buf, WFC_DSIZE);
    return 0x0U;
}
/**
  * @brief
  * @param
  * @retval
  */
static uint8_t DISP_SaveWFC(int8_t *buff)
{
    RTC_t dt;
    uint32_t wfc_time_stamp;
    /**
    *   FULL CHECK OF DATA BEFORE WRITING TO EEPROM
    */
    if (!IsRtcValidActiv()) return 0x1U;
    RTC_GetDateTime(&dt, RTC_FORMAT_BIN);
    wfc_time_stamp = (((buff[0] << 24) & 0xFF000000U)|((buff[1] << 16) & 0x00FF0000U)|
                      ((buff[2] <<  8) & 0x0000FF00U)| (buff[3]        & 0x000000FFU));
    if (!wfc_time_stamp || (wfc_time_stamp == 0xFFFFFFFFU))  return 0x1U;
    if (wfc_time_stamp > (dt.unix + SECONDS_PER_DAY))    return 0x1U;
    if (dt.unix > (wfc_time_stamp + SECONDS_PER_DAY))    return 0x1U;
    if ((buff[4]  < -60) || (buff[4]  > 60)) return 0x1U;  // BYTE4:	ACTUAL TEMPERATURE		int8  -60 ~ +60
    if ((buff[5]  <   1) || (buff[5]  >  7)) return 0x1U;  // BYTE5:	ACTUAL CLOUDNESS		uint8 	 1 ~ 7
    if ((buff[6]  < -60) || (buff[6]  > 60)) return 0x1U;  // BYTE6:	DAY + 1 HI TEMPERATURE	int8  -60 ~ +60
    if ((buff[7]  < -60) || (buff[7]  > 60)) return 0x1U;  // BYTE7:	DAY + 1 LO TEMPERATURE	int8  -60 ~ +60
    if ((buff[8]  <   1) || (buff[8]  >  7)) return 0x1U;  // BYTE8:	DAY + 1 CLOUDS			uint8	 1 ~ 7
    if ((buff[9]  < -60) || (buff[9]  > 60)) return 0x1U;  // BYTE9:	DAY + 2 HI TEMPERATURE	int8  -60 ~ +60
    if ((buff[10] < -60) || (buff[10] > 60)) return 0x1U;  // BYTE10:	DAY + 2 LO TEMPERATURE	int8  -60 ~ +60
    if ((buff[11] <   1) || (buff[11] >  7)) return 0x1U;  // BYTE11:	DAY + 2 CLOUDS			uint8	 1 ~ 7
    if ((buff[12] < -60) || (buff[12] > 60)) return 0x1U;  // BYTE12:	DAY + 3 HI TEMPERATURE	int8  -60 ~ +60
    if ((buff[13] < -60) || (buff[13] > 60)) return 0x1U;  // BYTE13:	DAY + 3 LO TEMPERATURE	int8  -60 ~ +60
    if ((buff[14] <   1) || (buff[14] >  7)) return 0x1U;  // BYTE14:	DAY + 3 CLOUDS			uint8	 1 ~ 7
    if ((buff[15] < -60) || (buff[15] > 60)) return 0x1U;  // BYTE15:	DAY + 4 HI TEMPERATURE	int8  -60 ~ +60
    if ((buff[16] < -60) || (buff[16] > 60)) return 0x1U;  // BYTE16:	DAY + 4 LO TEMPERATURE	int8  -60 ~ +60
    if ((buff[17] <   1) || (buff[17] >  7)) return 0x1U;  // BYTE17:	DAY + 4 CLOUDS			uint8	 1 ~ 7
    if ((buff[18] < -60) || (buff[18] > 60)) return 0x1U;  // BYTE18:	DAY + 5 HI TEMPERATURE	int8  -60 ~ +60
    if ((buff[19] < -60) || (buff[19] > 60)) return 0x1U;  // BYTE19:	DAY + 5 LO TEMPERATURE	int8  -60 ~ +60
    if ((buff[20] <   1) || (buff[20] >  7)) return 0x1U;  // BYTE20:	DAY + 5 CLOUDS			uint8	 1 ~ 7
    if(EE_WriteData(EE_ADDR, EE_WFORECAST, (uint8_t *)buff, WFC_DSIZE) != 0x0U) return 0x1U;
    return 0x0U;
}
/**
  * @brief
  * @param
  * @retval
  */
static void DISP_InitWFCFrame(FORECAST_DayTypeDef *fday)
{
        char dsp[8];
        uint32_t tp;
        //
        //  drav rounded frame for one forecast day
        //
		GUI_SetColor((0x62ul << 24) | GUI_DARKGRAY);
		GUI_FillRoundedRect(    ForecastFrame[fday->index].x0 + 1U, 
                                ForecastFrame[fday->index].y0 + 1U,
								ForecastFrame[fday->index].x1 - 1U, 
								ForecastFrame[fday->index].y1 - 1U, 10U);
                    
        GUI_SetColor(GUI_WHITE);            
		GUI_SetFont(GUI_FONT_24B_1);
		GUI_SetTextMode(GUI_TM_TRANS);
		ForecastFrame[fday->index].y0 += 5U;
		GUI_DispStringInRect(_apDays[fday->week_day], &ForecastFrame[fday->index], GUI_TA_TOP|GUI_TA_HCENTER);
		ForecastFrame[fday->index].y0 -= 5U;
        
        switch(fday->cloudness)
        {
            case 1:
                GUI_DrawBitmap(&bmclear_sky_icon, 
                (ForecastFrame[fday->index].x0 + 2U), 
                (ForecastFrame[fday->index].y0 + 30U));
                break;
            case 2:
                GUI_DrawBitmap(&bmfew_clouds_icon, 
                (ForecastFrame[fday->index].x0 + 2U), 
                (ForecastFrame[fday->index].y0 + 30U));
                break;
            case 3:
                GUI_DrawBitmap(&bmscattered_clouds_icon, 
                (ForecastFrame[fday->index].x0 + 2U), 
                (ForecastFrame[fday->index].y0 + 30U));
                break;
            case 4:
                GUI_DrawBitmap(&bmrain_icon, 
                (ForecastFrame[fday->index].x0 + 2U), 
                (ForecastFrame[fday->index].y0 + 30U));
                break;
            case 5:
                GUI_DrawBitmap(&bmshower_rain_icon,
                (ForecastFrame[fday->index].x0 + 2U), 
                (ForecastFrame[fday->index].y0 + 30U));
                break;
            case 6:
                GUI_DrawBitmap(&bmthunderstorm_icon, 
                (ForecastFrame[fday->index].x0 + 2U), 
                (ForecastFrame[fday->index].y0 + 30U));
                break;
            case 7:
                GUI_DrawBitmap(&bmsnow_icon, 
                (ForecastFrame[fday->index].x0 + 2U), 
                (ForecastFrame[fday->index].y0 + 30U));
                break;
        }
        
		GUI_SetColor(GUI_RED);
        GUI_SetFont(GUI_FONT_20_1);
		GUI_SetTextMode(GUI_TM_TRANS);
		GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
		GUI_GotoXY((ForecastFrame[fday->index].x0 + 
					((ForecastFrame[fday->index].x1 - 
					ForecastFrame[fday->index].x0) / 2U)), 
					(ForecastFrame[fday->index].y0 + 90U));

        ZEROFILL(dsp, COUNTOF(dsp));
        if(fday->high_temp < 0) 
        {
            Int2Str(dsp, fday->high_temp, 0U);
        }
        else 
        {
            dsp[0] = '+';
            Int2Str(&dsp[1], fday->high_temp, 0U);
        }
        
        tp = strlen(dsp);
        memcpy(&dsp[tp], "°C", 3U);
        GUI_DispString(dsp);
        
        GUI_SetColor(GUI_LIGHTBLUE);
        GUI_SetFont(GUI_FONT_20_1);
        GUI_SetTextMode(GUI_TM_TRANS);
		GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
		GUI_GotoXY((ForecastFrame[fday->index].x0 + 
					((ForecastFrame[fday->index].x1 - 
					ForecastFrame[fday->index].x0) / 2U)), 
					(ForecastFrame[fday->index].y0 + 120U));
        
		ZEROFILL(dsp, COUNTOF(dsp));
        if(fday->low_temp < 0) 
        {
            Int2Str(dsp, fday->low_temp, 0U);
        }
        else 
        {
            dsp[0] = '+';
            Int2Str(&dsp[1], fday->low_temp, 0U);
        }
        
        tp = strlen(dsp);
        memcpy(&dsp[tp], "°C", 3U);
        GUI_DispString(dsp);
}
/**
  * @brief
  * @param
  * @retval
  */
static void DISP_CreateWFCScreen(int8_t *buff)
{
    char disp_buff[16];
    /* 
    *   convert and display clock time
    */
    GUI_GotoXY(400, 150);
    GUI_SetColor(GUI_RED);
    GUI_SetFont(GUI_FONT_32B_1);
    GUI_SetTextMode(GUI_TM_TRANS);
    GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
    ZEROFILL(disp_buff, COUNTOF(disp_buff));
    HEX2STR(disp_buff, &rtctm.Hours);
    disp_buff[2] = ':';
    HEX2STR(&disp_buff[3], &rtctm.Minutes);
    GUI_DispString(disp_buff);
    /* 
    *   convert and display date day
    */
    GUI_GotoXY(400, 185);
    GUI_SetColor(GUI_WHITE);
    GUI_SetFont(GUI_FONT_24_1);
    GUI_SetTextMode(GUI_TM_TRANS);
    GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
    if      (rtcdt.WeekDay == 0x00)   rtcdt.WeekDay = 0x07;
    if      (rtcdt.WeekDay == 0x01)   GUI_DispString("Monday");
    else if (rtcdt.WeekDay == 0x02)   GUI_DispString("Tuestday");
    else if (rtcdt.WeekDay == 0x03)   GUI_DispString("Wednesday");
    else if (rtcdt.WeekDay == 0x04)   GUI_DispString("Thurstday");
    else if (rtcdt.WeekDay == 0x05)   GUI_DispString("Friday");
    else if (rtcdt.WeekDay == 0x06)   GUI_DispString("Saturday");
    else if (rtcdt.WeekDay == 0x07)   GUI_DispString("Sunday");
    /* 
    *   convert and display month and date
    */
    GUI_GotoXY(400, 215);
    GUI_SetColor(GUI_RED);
    GUI_SetFont(GUI_FONT_20B_1);
    GUI_SetTextMode(GUI_TM_TRANS);
    GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
    ZEROFILL(disp_buff, COUNTOF(disp_buff));
    HEX2STR(disp_buff, &rtcdt.Date);
    disp_buff[2] = '.';
    if      (rtcdt.Month == 0x01) memcpy(&disp_buff[3], "January",    7);		
    else if (rtcdt.Month == 0x02) memcpy(&disp_buff[3], "February",   8);
    else if (rtcdt.Month == 0x03) memcpy(&disp_buff[3], "March",      5);
    else if (rtcdt.Month == 0x04) memcpy(&disp_buff[3], "April",      5);
    else if (rtcdt.Month == 0x05) memcpy(&disp_buff[3], "May",        3);
    else if (rtcdt.Month == 0x06) memcpy(&disp_buff[3], "June",       4);
    else if (rtcdt.Month == 0x07) memcpy(&disp_buff[3], "July",       4);
    else if (rtcdt.Month == 0x08) memcpy(&disp_buff[3], "August",     6);
    else if (rtcdt.Month == 0x09) memcpy(&disp_buff[3], "September",  9);
    else if (rtcdt.Month == 0x10) memcpy(&disp_buff[3], "October",    7);
    else if (rtcdt.Month == 0x11) memcpy(&disp_buff[3], "November",   8);
    else if (rtcdt.Month == 0x12) memcpy(&disp_buff[3], "December",   8);
    GUI_DispString(disp_buff);
    /* 
    *   convert and display year
    */
    GUI_GotoXY(400, 245);
    GUI_SetColor(GUI_WHITE);
    GUI_SetFont(GUI_FONT_24B_1);
    GUI_SetTextMode(GUI_TM_TRANS);
    GUI_SetTextAlign(GUI_TA_HCENTER|GUI_TA_VCENTER);
    ZEROFILL(disp_buff, COUNTOF(disp_buff));
    disp_buff[0] = '2';
    disp_buff[1] = '0';
    HEX2STR(&disp_buff[2], &rtcdt.Year);
    GUI_DispString(disp_buff);
    /* 
    *   display city
    */
    GUI_GotoXY(20, 10);
    GUI_SetColor(GUI_WHITE);
    GUI_SetFont(GUI_FONT_32B_1);
    GUI_SetTextMode(GUI_TM_TRANS);
    GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
    GUI_DispString("Sarajevo Today");
    /* 
    *   display city actual temperature
    */
    GUI_GotoXY(20, 50);
    GUI_SetFont(GUI_FONT_32_1);
    ZEROFILL(disp_buff, COUNTOF(disp_buff));
    buff += 4U; // buffer data offset
    
    if(*buff < 0) 
    {
        Int2Str(disp_buff, *buff++, 0U);
    }
    else 
    {
        disp_buff[0] = '+';
        Int2Str(&disp_buff[1], *buff++, 0U);
    }
    
    uint32_t tp = strlen(disp_buff);
    memcpy(&disp_buff[tp], "°C", 3U);
    GUI_DispString(disp_buff);
    /* 
    *   show cloudnes icon
    */
    GUI_GotoXY(120, 50);
    GUI_SetColor(GUI_WHITE);
    GUI_SetFont(GUI_FONT_32_1);
    GUI_SetTextMode(GUI_TM_TRANS);
    GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
    
    switch(*buff++)
    {
        case 1:
            GUI_DispString("Clear Sky");
            GUI_DrawBitmap(&bmclear_sky_img, 320, 0);
            break;
        case 2:
            GUI_DispString("Few Clouds");
            GUI_DrawBitmap(&bmfew_clouds_img, 320, 0);
            break;
        case 3:
            GUI_DispString("Scattered Clouds");
            GUI_DrawBitmap(&bmscattered_clouds_img, 320, 0);
            break;
        case 4:
            GUI_DispString("Rain");
            GUI_DrawBitmap(&bmrain_img, 320, 0);
            break;
        case 5:
            GUI_DispString("Shower Rain");
            GUI_DrawBitmap(&bmshower_rain_img  , 320, 0);
            break;
        case 6:
            GUI_DispString("Thunderstorm");
            GUI_DrawBitmap(&bmthunderstorm_img, 320, 0);
            break;
        case 7:
            GUI_DispString("Snow");
            GUI_DrawBitmap(&bmsnow_img, 320, 0);
            break;
        default:
            GUI_DispString("Data Unavailable");
            break;
    }
    /* 
    *   draw weather forecast for nex 5 days
    */
    for(uint32_t i = 0U; i < 5U; i++)   // init forecast day structure
    {
        FORECAST_Day[i].index = i;
        FORECAST_Day[i].week_day = rtcdt.WeekDay + i + 1U;
        if(FORECAST_Day[i].week_day > 0x07U) FORECAST_Day[i].week_day -= 0x07U;
        FORECAST_Day[i].high_temp   = *buff++;
        FORECAST_Day[i].low_temp    = *buff++;
        FORECAST_Day[i].cloudness   = *buff++;
        DISP_InitWFCFrame(&FORECAST_Day[i]);
    }
}
/**
  * @brief
  * @param
  * @retval
  */
static void DISP_CreateSettings1Screen(void)
{
    RFSenCfgSet();  // settings1 is activ, rf. sensor events not processed
    DISP_RFSenSet(); // init edit box if sensors added to list
    GUI_SelectLayer(0);
    GUI_Clear();
    GUI_SelectLayer(1);
    GUI_SetBkColor(GUI_TRANSPARENT); 
    GUI_Clear();
    GUI_MULTIBUF_BeginEx(1);
    GUI_SetColor(GUI_WHITE);
    GUI_SetFont(GUI_FONT_13_1);
    GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
    
    GUI_GotoXY(10, 4);
    GUI_DispString("SETPOINT LIMITS");
    GUI_DrawHLine(12, 5, 190);
    hSPNBX_ThermostatMaxSetpointTemperature = SPINBOX_CreateEx(10, 20, 110, 40, 0, WM_CF_SHOW, GUI_ID_SPINBOX_ThermostatMaxSetpointTemperature, 15, 40);
    SPINBOX_SetEdge(hSPNBX_ThermostatMaxSetpointTemperature, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hSPNBX_ThermostatMaxSetpointTemperature, thst_max_sp);
    GUI_GotoXY(130, 30);
    GUI_DispString("MAX. USER");
    GUI_GotoXY(130, 42);
    GUI_DispString("SETPOINT");
    GUI_GotoXY(130, 54);
    GUI_DispString("TEMP. x1*C");
    hSPNBX_ThermostatMinSetpointTemperature = SPINBOX_CreateEx(10, 80, 110, 40, 0, WM_CF_SHOW, GUI_ID_SPINBOX_ThermostatMinSetpointTemperature, 15, 40);
    SPINBOX_SetEdge(hSPNBX_ThermostatMinSetpointTemperature, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hSPNBX_ThermostatMinSetpointTemperature, thst_min_sp);
    GUI_GotoXY(130, 80);
    GUI_DispString("MIN. USER");
    GUI_GotoXY(130, 92);
    GUI_DispString("SETPOINT");
    GUI_GotoXY(130, 104);
    GUI_DispString("TEMP. x1*C");
    
    GUI_GotoXY(10, 140);
    GUI_DispString("ROOM NTC OFFSET");
    GUI_DrawHLine(150, 5, 190);
    hSPNBX_AmbientNtcOffset = SPINBOX_CreateEx(10, 160, 110, 40, 0, WM_CF_SHOW, GUI_ID_SPINBOX_AmbientNtcOffset, -100, 100);
    SPINBOX_SetEdge(hSPNBX_AmbientNtcOffset, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hSPNBX_AmbientNtcOffset, ntc_offset);
    GUI_GotoXY(130, 170);
    GUI_DispString("SENSOR");
    GUI_GotoXY(130, 182);
    GUI_DispString("OFFSET");
    GUI_GotoXY(130, 194);
    GUI_DispString("TEMP. x0.1*C");
    
    GUI_GotoXY(210, 4);
    GUI_DispString("ADD/REMOVE RADIO SENSOR TO CONFIGURATION");
    GUI_DrawHLine(12, 205, 470);
    hEDIT_RFSen = EDIT_CreateEx(210, 20, 110, 40, 0, WM_CF_SHOW, 0, GUI_ID_EDIT_RFSen, 16);
    EDIT_SetFont(hEDIT_RFSen, GUI_FONT_16_1);
    hBUTTON_RFSenDown = BUTTON_Create(210, 80, 50, 40, GUI_ID_BUTTON_RFSenDown, WM_CF_SHOW);
    BUTTON_SetText(hBUTTON_RFSenDown, "DN-");
    
    hBUTTON_RFSenUp = BUTTON_Create(270, 80, 50, 40, GUI_ID_BUTTON_RFSenUp, WM_CF_SHOW);
    BUTTON_SetText(hBUTTON_RFSenUp, "UP+");
    
    GUI_GotoXY(210, 140);
    GUI_DispString("ONEWIRE INTERFACE ADDR.");
    GUI_DrawHLine(150, 205, 350);
    hSPNBX_OneWireInterfaceAddress = SPINBOX_CreateEx(210, 160, 110, 40, 0, WM_CF_SHOW, GUI_ID_SPINBOX_OneWireInterfaceAddress, 1, 9);
    SPINBOX_SetEdge(hSPNBX_OneWireInterfaceAddress, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hSPNBX_OneWireInterfaceAddress, ow_ifa);
    
    hBUTTON_RFSenDelete = BUTTON_Create(370, 20, 100, 40, GUI_ID_BUTTON_RFSenDelete, WM_CF_SHOW);
    BUTTON_SetText(hBUTTON_RFSenDelete, "DELETE SENSOR");
    
    hBUTTON_RFSenAdd = BUTTON_Create(370, 80, 100, 40, GUI_ID_BUTTON_RFSenAdd, WM_CF_SHOW);
    BUTTON_SetText(hBUTTON_RFSenAdd, "ADD SENSOR");
    
    hBUTTON_Next = BUTTON_Create(370, 160, 100, 40, GUI_ID_BUTTON_Next, WM_CF_SHOW);
    BUTTON_SetText(hBUTTON_Next, "NEXT");
    
    hBUTTON_Ok = BUTTON_Create(370, 220, 100, 40, GUI_ID_BUTTON_Ok, WM_CF_SHOW);
    BUTTON_SetText(hBUTTON_Ok, "OK");
    
    GUI_MULTIBUF_EndEx(1);
}
/**
  * @brief
  * @param
  * @retval
  */
static void DISP_CreateSettings2Screen(void)
{
    GUI_SelectLayer(0);
    GUI_Clear();
    GUI_SelectLayer(1);
    GUI_SetBkColor(GUI_TRANSPARENT); 
    GUI_Clear();
    GUI_MULTIBUF_BeginEx(1);
    HAL_RTC_GetTime(&hrtc, &rtctm, RTC_FORMAT_BCD);
    HAL_RTC_GetDate(&hrtc, &rtcdt, RTC_FORMAT_BCD);
    hSPNBX_DisplayHighBrightness = SPINBOX_CreateEx(10, 20, 90, 30, 0, WM_CF_SHOW, GUI_ID_SPINBOX_DisplayHighBrightness, 1, 90);
    SPINBOX_SetEdge(hSPNBX_DisplayHighBrightness, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hSPNBX_DisplayHighBrightness, disp_high_bcklght);
    hSPNBX_DisplayLowBrightness = SPINBOX_CreateEx(10, 60, 90, 30, 0, WM_CF_SHOW, GUI_ID_SPINBOX_DisplayLowBrightness, 1, 90);
    SPINBOX_SetEdge(hSPNBX_DisplayLowBrightness, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hSPNBX_DisplayLowBrightness, disp_low_bcklght);
    hSPNBX_ScrnsvrTimeout = SPINBOX_CreateEx(10, 130, 90, 30, 0, WM_CF_SHOW, GUI_ID_SPINBOX_ScrnsvrTimeout, 1, 240);
    SPINBOX_SetEdge(hSPNBX_ScrnsvrTimeout, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hSPNBX_ScrnsvrTimeout, scrnsvr_tout);
    hSPNBX_ScrnsvrEnableHour = SPINBOX_CreateEx(10, 170, 90, 30, 0, WM_CF_SHOW, GUI_ID_SPINBOX_ScrnsvrEnableHour, 0, 23);
    SPINBOX_SetEdge(hSPNBX_ScrnsvrEnableHour, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hSPNBX_ScrnsvrEnableHour, scrnsvr_ena_hour);
    hSPNBX_ScrnsvrDisableHour = SPINBOX_CreateEx(10, 210, 90, 30, 0, WM_CF_SHOW, GUI_ID_SPINBOX_ScrnsvrDisableHour, 0, 23);
    SPINBOX_SetEdge(hSPNBX_ScrnsvrDisableHour, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hSPNBX_ScrnsvrDisableHour, scrnsvr_dis_hour);
    hSPNBX_Hour = SPINBOX_CreateEx(190, 20, 90, 30, 0, WM_CF_SHOW, GUI_ID_SPINBOX_Hour, 0, 23);
    SPINBOX_SetEdge(hSPNBX_Hour, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hSPNBX_Hour, Bcd2Dec(rtctm.Hours));    
    hSPNBX_Minute = SPINBOX_CreateEx(190, 60, 90, 30, 0, WM_CF_SHOW, GUI_ID_SPINBOX_Minute, 0, 59);
    SPINBOX_SetEdge(hSPNBX_Minute, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hSPNBX_Minute, Bcd2Dec(rtctm.Minutes));    
    hSPNBX_Day = SPINBOX_CreateEx(190, 130, 90, 30, 0, WM_CF_SHOW, GUI_ID_SPINBOX_Day, 1, 31);
    SPINBOX_SetEdge(hSPNBX_Day, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hSPNBX_Day, Bcd2Dec(rtcdt.Date));    
    hSPNBX_Month = SPINBOX_CreateEx(190, 170, 90, 30, 0, WM_CF_SHOW, GUI_ID_SPINBOX_Month, 1, 12);
    SPINBOX_SetEdge(hSPNBX_Month, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hSPNBX_Month, Bcd2Dec(rtcdt.Month));    
    hSPNBX_Year = SPINBOX_CreateEx(190, 210, 90, 30, 0, WM_CF_SHOW, GUI_ID_SPINBOX_Year, 2000, 2099);
    SPINBOX_SetEdge(hSPNBX_Year, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hSPNBX_Year, (Bcd2Dec(rtcdt.Year) + 2000));
    hSPNBX_ScrnsvrClockColour = SPINBOX_CreateEx(340, 20, 90, 30, 0, WM_CF_SHOW, GUI_ID_SPINBOX_ScrnsvrClockColour, 1, COLOR_BSIZE);
    SPINBOX_SetEdge(hSPNBX_ScrnsvrClockColour, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hSPNBX_ScrnsvrClockColour, scrnsvr_clk_clr);
    hSPNBX_ScrnsvrSemiClkColor = SPINBOX_CreateEx(340, 60, 90, 30, 0, WM_CF_SHOW, GUI_ID_SPINBOX_ScrnsvrLogoClockColour, 1, COLOR_BSIZE);
    SPINBOX_SetEdge(hSPNBX_ScrnsvrSemiClkColor, SPINBOX_EDGE_CENTER);
    SPINBOX_SetValue(hSPNBX_ScrnsvrSemiClkColor, scrnsvr_semiclk_clr);
    hCHKBX_ScrnsvrClock = CHECKBOX_Create(340, 110, 90, 20, 0, GUI_ID_CHECK_ScrnsvrClock, WM_CF_SHOW);
    CHECKBOX_SetTextColor(hCHKBX_ScrnsvrClock, GUI_GREEN);	
    CHECKBOX_SetText(hCHKBX_ScrnsvrClock, "FULL CLOCK");
    if(IsScrnsvrClkActiv()) CHECKBOX_SetState(hCHKBX_ScrnsvrClock, 1);
    else CHECKBOX_SetState(hCHKBX_ScrnsvrClock, 0);
    hCHKBX_ScrnsvrLogoClock = CHECKBOX_Create(340, 140, 90, 20, 0, GUI_ID_CHECK_ScrnsvrLogoClock, WM_CF_SHOW);
    CHECKBOX_SetTextColor(hCHKBX_ScrnsvrLogoClock, GUI_GREEN);	
    CHECKBOX_SetText(hCHKBX_ScrnsvrLogoClock, "LOGO CLOCK");
    if(IsScrnsvrSemiClkActiv()) CHECKBOX_SetState(hCHKBX_ScrnsvrLogoClock, 1);
    else CHECKBOX_SetState(hCHKBX_ScrnsvrLogoClock, 0);
    hBUTTON_Next = BUTTON_Create(390, 180, 80, 30, GUI_ID_BUTTON_Next, WM_CF_SHOW);
    BUTTON_SetText(hBUTTON_Next, "NEXT");
    hBUTTON_Ok = BUTTON_Create(390, 230, 80, 30, GUI_ID_BUTTON_Ok, WM_CF_SHOW);
    BUTTON_SetText(hBUTTON_Ok, "OK");
    GUI_SetColor(clk_clrs[scrnsvr_clk_clr]);
    GUI_FillRect(340, 51, 430, 59);
    GUI_SetColor(clk_clrs[scrnsvr_semiclk_clr]);
    GUI_FillRect(340, 91, 430, 99);
    GUI_SetColor(GUI_WHITE);
    GUI_SetFont(GUI_FONT_13_1);
    GUI_SetTextAlign(GUI_TA_LEFT|GUI_TA_VCENTER);
    /************************************/
    /* DISPLAY BACKLIGHT LED BRIGHTNESS */
    /************************************/
    GUI_DrawHLine   ( 15,   5, 160);
    GUI_GotoXY      ( 10,   5);
    GUI_DispString  ("DISPLAY BACKLIGHT");
    GUI_GotoXY      (110,  35);
    GUI_DispString  ("HIGH");
    GUI_GotoXY      (110,  75);
    GUI_DispString  ("LOW");
    GUI_DrawHLine   ( 15, 185, 320);
    GUI_GotoXY      (190,   5);
    /************************************/
    /*          SET        TIME         */
    /************************************/
    GUI_DispString  ("SET TIME");
    GUI_GotoXY      (290,  35);
    GUI_DispString  ("HOUR");
    GUI_GotoXY      (290,  75);
    GUI_DispString  ("MINUTE");
    GUI_DrawHLine   ( 15, 335, 475);
    /************************************/
    /*    SET SCREENSAVER CLOCK COOLOR  */
    /************************************/
    GUI_GotoXY      (340,   5);
    GUI_DispString  ("SET COLOR");
    GUI_GotoXY      (440, 26);
    GUI_DispString  ("FULL");
    GUI_GotoXY      (440, 38);
    GUI_DispString  ("CLOCK");
    GUI_GotoXY      (440, 66);
    GUI_DispString  ("LOGO");
    GUI_GotoXY      (440, 78);
    GUI_DispString  ("CLOCK");
    /************************************/
    /*      SCREENSAVER     OPTION      */
    /************************************/ 
    GUI_DrawHLine   (125,   5, 160);
    GUI_GotoXY      ( 10, 115);
    GUI_DispString  ("SCREENSAVER OPTION");
    GUI_GotoXY      (110, 145);
    GUI_DispString  ("TIMEOUT");
    GUI_GotoXY      (110, 176);
    GUI_DispString  ("ENABLE");
    GUI_GotoXY      (110, 188);
    GUI_DispString  ("HOUR");
    GUI_GotoXY      (110, 216);
    GUI_DispString  ("DISABLE");
    GUI_GotoXY      (110, 228);
    GUI_DispString  ("HOUR");
    /************************************/
    /*          SET        DATE         */
    /************************************/
    GUI_DrawHLine   (125, 185, 320);
    GUI_GotoXY      (190, 115);
    GUI_DispString  ("SET DATE");
    GUI_GotoXY      (290, 145);
    GUI_DispString  ("DAY");
    GUI_GotoXY      (290, 185);
    GUI_DispString  ("MONTH");
    GUI_GotoXY      (290, 225);
    GUI_DispString  ("YEAR");
    GUI_MULTIBUF_EndEx(1);
}
/**u
  * @brief
  * @param
  * @retval
  */
static void DISP_DeleteSettings1Screen(void)
{
    RFSenAddReset();
    RFSenCfgReset();
    RFSenNewReset();
    DISP_RFSenReset();
    WM_DeleteWindow(hBUTTON_Ok);
    WM_DeleteWindow(hBUTTON_Next);
    WM_DeleteWindow(hBUTTON_RFSenDelete);
    WM_DeleteWindow(hBUTTON_RFSenAdd);
    WM_DeleteWindow(hBUTTON_RFSenUp);
    WM_DeleteWindow(hBUTTON_RFSenDown);
    WM_DeleteWindow(hEDIT_RFSen);
    WM_DeleteWindow(hSPNBX_ThermostatMaxSetpointTemperature);
    WM_DeleteWindow(hSPNBX_ThermostatMinSetpointTemperature);
    WM_DeleteWindow(hSPNBX_OneWireInterfaceAddress);
    WM_DeleteWindow(hSPNBX_AmbientNtcOffset);
}
/**
  * @brief
  * @param
  * @retval
  */
static void DISP_DeleteSettings2Screen(void)
{
    WM_DeleteWindow(hBUTTON_Ok);
    WM_DeleteWindow(hBUTTON_Next);
    WM_DeleteWindow(hSPNBX_DisplayHighBrightness);
    WM_DeleteWindow(hSPNBX_DisplayLowBrightness);
    WM_DeleteWindow(hCHKBX_ScrnsvrClock);
    WM_DeleteWindow(hCHKBX_ScrnsvrLogoClock);
    WM_DeleteWindow(hSPNBX_ScrnsvrTimeout);
    WM_DeleteWindow(hSPNBX_ScrnsvrEnableHour);
    WM_DeleteWindow(hSPNBX_ScrnsvrDisableHour);
    WM_DeleteWindow(hSPNBX_Hour);
    WM_DeleteWindow(hSPNBX_Minute);
    WM_DeleteWindow(hSPNBX_Day);
    WM_DeleteWindow(hSPNBX_Month);
    WM_DeleteWindow(hSPNBX_Year);
    WM_DeleteWindow(hSPNBX_ScrnsvrClockColour);
    WM_DeleteWindow(hSPNBX_ScrnsvrSemiClkColor);
}
/**
  * @brief
  * @param
  * @retval
  */
static void PID_Hook(GUI_PID_STATE * pState) 
{
    if (pState->Pressed  == 1U)
    {
        pState->Layer = 1U;
        DISP_ResetScrnsvr();
        
        if ((pState->x >= BTN_SETTINGS_X0) && 
            (pState->y >= BTN_SETTINGS_Y0) && 
            (pState->x < BTN_SETTINGS_X1) && 
            (pState->y < BTN_SETTINGS_Y1)) 
        {
            btn_settings = 1U;
        }
        
        if ((pState->x >= BTN_INC_X0) && 
            (pState->y >= BTN_INC_Y0) && 
            (pState->x < BTN_INC_X1) && 
            (pState->y < BTN_INC_Y1)) 
        {	
            btn_increase_state = 1U;   
        }
        
        if ((pState->x >= BTN_DEC_X0) && 
            (pState->y >= BTN_DEC_Y0) && 
            (pState->x < BTN_DEC_X1) && 
            (pState->y < BTN_DEC_Y1)) 
        {	
            btn_decrease_state = 1U;
        }  

        if ((pState->x >= BTN_DND_X0) && 
            (pState->y >= BTN_DND_Y0) && 
            (pState->x < BTN_DND_X1) && 
            (pState->y < BTN_DND_Y1)) 
        {
            btn_dnd_state = 1U;
        }
           
        if ((pState->x >= BTN_CMD_X0) && 
            (pState->y >= BTN_CMD_Y0) && 
            (pState->x < BTN_CMD_X1) && 
            (pState->y < BTN_CMD_Y1)) 
        {
            btn_maid_state = 1U;
        } 
        
        if ((pState->x >= BTN_SOS_X0) && 
            (pState->y >= BTN_SOS_Y0) && 
            (pState->x < BTN_SOS_X1) && 
            (pState->y < BTN_SOS_Y1)) 
        {
            btn_sos_state = 1U;
        } 
        
        if ((pState->x >= BTN_DOOR_X0) && 
            (pState->y >= BTN_DOOR_Y0 + 20) && 
            (pState->x < BTN_DOOR_X1) && 
            (pState->y < BTN_DOOR_Y1)) 
        {
            btn_opendoor_state = 1U;
        }
           
        if ((pState->x >= BTN_OK_X0) && 
            (pState->y >= BTN_OK_Y0) && 
            (pState->x < BTN_OK_X1) && 
            (pState->y < BTN_OK_Y1)) 
        {
            btn_ok_state = 1U;
        } 
    }
    else
    {
        btn_ok_state = 0U; 
        btn_settings = 0U;
        btn_sos_state = 0U; 
        btn_dnd_state = 0U;
        btn_maid_state = 0U;
        btn_decrease_state = 0U;   
        btn_increase_state = 0U;
        btn_opendoor_state = 0U;
    }
}
/**
  * @brief
  * @param
  * @retval
  */
static uint8_t DISP_EnableSettingsMenu(uint8_t btn)
{
    static uint8_t last_state = 0U;
    static uint32_t menu_tmr = 0U;
    
    if      ((btn == 1U) && (last_state == 0U))
    {
        last_state = 1U;
        menu_tmr = HAL_GetTick(); 
    }
    else if ((btn == 1U) && (last_state == 1U))
    {
        if((HAL_GetTick() - menu_tmr) >= SETTINGS_MENU_ENABLE_TIME)
        {
            last_state = 0U;
            return (1U);
        }
    }
    else if ((btn == 0U) && (last_state == 1U)) last_state = 0U;
    
    return (0U);
}
/**
  * @brief
  * @param
  * @retval
  */
static void DISP_SaveRFSen(void)
{
    uint8_t wbuf[16], rbuf[16], cnt = 100U;
    
    if      (!rfsen[0]||(rfsen[0]==0xFFFFU)) ZEROFILL(rfsen, 8);
    else if (!rfsen[1]||(rfsen[1]==0xFFFFU)) ZEROFILL(&rfsen[1], 7);
    else if (!rfsen[2]||(rfsen[2]==0xFFFFU)) ZEROFILL(&rfsen[2], 6);
    else if (!rfsen[3]||(rfsen[3]==0xFFFFU)) ZEROFILL(&rfsen[3], 5);
    else if (!rfsen[4]||(rfsen[4]==0xFFFFU)) ZEROFILL(&rfsen[4], 4);  
    else if (!rfsen[5]||(rfsen[5]==0xFFFFU)) ZEROFILL(&rfsen[5], 3);
    else if (!rfsen[6]||(rfsen[6]==0xFFFFU)) ZEROFILL(&rfsen[6], 2);
    else if (!rfsen[7]||(rfsen[7]==0xFFFFU)) rfsen[7]=0U;
    
    while (cnt)
    {
        wbuf[0] = (rfsen[0] >> 8);
        wbuf[1] = (rfsen[0] & 0xFFU);
        wbuf[2] = (rfsen[1] >> 8);
        wbuf[3] = (rfsen[1] & 0xFFU);
        wbuf[4] = (rfsen[2] >> 8);
        wbuf[5] = (rfsen[2] & 0xFFU);
        wbuf[6] = (rfsen[3] >> 8);
        wbuf[7] = (rfsen[3] & 0xFFU);
        wbuf[8] = (rfsen[4] >> 8);
        wbuf[9] = (rfsen[4] & 0xFFU);
        wbuf[10]= (rfsen[5] >> 8);
        wbuf[11]= (rfsen[5] & 0xFFU);
        wbuf[12]= (rfsen[6] >> 8);
        wbuf[13]= (rfsen[6] & 0xFFU);
        wbuf[14]= (rfsen[7] >> 8);
        wbuf[15]= (rfsen[7] & 0xFFU);
        HAL_I2C_Mem_Write(&hi2c4, EE_ADDR, EE_RF_SENSOR_1, I2C_MEMADD_SIZE_16BIT, wbuf, 16U, EETOUT);
        HAL_I2C_IsDeviceReady(&hi2c4, EE_ADDR, EE_MAX_TRIALS, EETOUT);   
        ZEROFILL(rbuf,16);
        HAL_I2C_Mem_Read(&hi2c4, EE_ADDR, EE_RF_SENSOR_1, I2C_MEMADD_SIZE_16BIT, rbuf, 16U, EETOUT);
        if ((wbuf[0]==rbuf[0])  &&(wbuf[1]==rbuf[1])    
        &&  (wbuf[2]==rbuf[2])  &&(wbuf[3]==rbuf[3])
        &&  (wbuf[4]==rbuf[4])  &&(wbuf[5]==rbuf[5])    
        &&  (wbuf[6]==rbuf[6])  &&(wbuf[7]==rbuf[7])
        &&  (wbuf[8]==rbuf[8])  &&(wbuf[9]==rbuf[9])    
        &&  (wbuf[10]==rbuf[10])&&(wbuf[11]==rbuf[11])
        &&  (wbuf[12]==rbuf[12])&&(wbuf[13]==rbuf[13])
        &&  (wbuf[14]==rbuf[14])&&(wbuf[15]==rbuf[15])) break;
        --cnt;
    }
    if (!cnt) ErrorHandler(DISP_FUNC, I2C_DRV); // eeprom write fail
}
/**
  * @brief
  * @param
  * @retval
  */
static void DISP_SaveSettings(void)
{
    uint8_t sbuf[18], gbuf[18], cnt = 100U;
    
    while (cnt)
    {
        sbuf[0] = (thst_fl >> 24);
        sbuf[1] = (thst_fl >> 16);
        sbuf[2] = (thst_fl >> 8);
        sbuf[3] = (thst_fl & 0xFFU);
        sbuf[4] = thst_min_sp;
        sbuf[5] = thst_max_sp;
        sbuf[6] = thst_sp;
        sbuf[7] = ntc_offset;
        sbuf[8] = ow_bps; 
        sbuf[9] = ow_ifa;
        sbuf[10]= disp_low_bcklght;
        sbuf[11]= disp_high_bcklght;
        sbuf[12]= scrnsvr_tout;
        sbuf[13]= scrnsvr_ena_hour;
        sbuf[14]= scrnsvr_dis_hour;
        sbuf[15]= scrnsvr_clk_clr;
        sbuf[16]= scrnsvr_semiclk_clr;
        sbuf[17]= sys_stat;
        HAL_I2C_Mem_Write(&hi2c4, EE_ADDR, EE_THST_FLAGS, I2C_MEMADD_SIZE_16BIT, sbuf, 18U, EETOUT);
        HAL_I2C_IsDeviceReady(&hi2c4, EE_ADDR, EE_MAX_TRIALS, EETOUT);   
        ZEROFILL(gbuf,18);
        HAL_I2C_Mem_Read(&hi2c4, EE_ADDR, EE_THST_FLAGS, I2C_MEMADD_SIZE_16BIT, gbuf, 18U, EETOUT);
        if ((sbuf[0]==gbuf[0])  &&(sbuf[1]==gbuf[1])    
        &&  (sbuf[2]==gbuf[2])  &&(sbuf[3]==gbuf[3])
        &&  (sbuf[4]==gbuf[4])  &&(sbuf[5]==gbuf[5])    
        &&  (sbuf[6]==gbuf[6])  &&(sbuf[7]==gbuf[7])
        &&  (sbuf[8]==gbuf[8])  &&(sbuf[9]==gbuf[9])    
        &&  (sbuf[10]==gbuf[10])&&(sbuf[11]==gbuf[11])
        &&  (sbuf[12]==gbuf[12])&&(sbuf[13]==gbuf[13])  
        &&  (sbuf[14]==gbuf[14])&&(sbuf[15]==gbuf[15])
        &&  (sbuf[16]==gbuf[16])&&(sbuf[17]==gbuf[17])) break;
        --cnt;
    }
    if (!cnt) ErrorHandler(DISP_FUNC, I2C_DRV); // eeprom write fail
}
/**
  * @brief
  * @param
  * @retval
  */
static void DISP_SaveSetPoint(void)
{
    uint8_t tbuf[4], rbuf[4], cnt = 100U;
    
    while (cnt)
    {
        tbuf[0] = (thst_fl >> 24);
        tbuf[1] = (thst_fl >> 16);
        tbuf[2] = (thst_fl >> 8);
        tbuf[3] = (thst_fl & 0xFFU);
        HAL_I2C_Mem_Write(&hi2c4, EE_ADDR, EE_THST_FLAGS, I2C_MEMADD_SIZE_16BIT, tbuf, 0x4U, EETOUT);
        HAL_I2C_IsDeviceReady(&hi2c4, EE_ADDR, EE_MAX_TRIALS, EETOUT);   
        ZEROFILL(rbuf,4);
        HAL_I2C_Mem_Read(&hi2c4, EE_ADDR, EE_THST_FLAGS, I2C_MEMADD_SIZE_16BIT, rbuf, 0x4U, EETOUT);
        if ((tbuf[0] == rbuf[0])
        &&  (tbuf[1] == rbuf[1])
        &&  (tbuf[2] == rbuf[2])
        &&  (tbuf[3] == rbuf[3])) break;
        --cnt;
    }
    if (!cnt) ErrorHandler(DISP_FUNC, I2C_DRV); // eeprom write fail
    else cnt = 100U; // data write verified, save next
    while (cnt)
    {    
        tbuf[0] = thst_sp;
        HAL_I2C_Mem_Write(&hi2c4, EE_ADDR, EE_THST_SETPOINT, I2C_MEMADD_SIZE_16BIT, tbuf, 0x1U, EETOUT);
        HAL_I2C_IsDeviceReady(&hi2c4, EE_ADDR, EE_MAX_TRIALS, EETOUT); 
        rbuf[0] = 0x0U; 
        HAL_I2C_Mem_Read(&hi2c4, EE_ADDR, EE_THST_SETPOINT, I2C_MEMADD_SIZE_16BIT, rbuf, 0x1U, EETOUT);
        if (tbuf[0] == rbuf[0]) break;
        --cnt;
    }
    if (!cnt) ErrorHandler(DISP_FUNC, I2C_DRV); // eeprom write fail
}
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/

