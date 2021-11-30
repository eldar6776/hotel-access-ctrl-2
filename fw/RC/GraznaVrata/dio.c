/**
 ******************************************************************************
 * File Name          : dio_interface.c
 * Date               : 28/02/2016 23:16:19
 * Description        :  digital in/out and capacitive sensor  modul
 ******************************************************************************
 */
 
 
/* Include  ------------------------------------------------------------------*/
#include "display.h"
#include "logger.h"
#include "eeprom.h"
#include "rc522.h"
#include "rs485.h"
#include "owire.h"
#include "room.h"
#include "main.h"
#include "dio.h"


#if (__DIO_INTERFACE_H__ != FW_BUILD)
    #error "doi header version mismatch"
#endif


/* Imported Types  -----------------------------------------------------------*/
/* Imported Variables --------------------------------------------------------*/
/* Imported Functions    -----------------------------------------------------*/
/* Private Variables  --------------------------------------------------------*/
__IO uint32_t dio_flags;
__IO uint32_t din_0_7;          // digital input after debouncing mask with config buffer for application digital inputs 
__IO uint32_t din_state;        // digital input real time update states input to debouncing
__IO uint32_t din_cap_sen;      // capacitive taster sensors state
__IO uint32_t dout_0_7;         // digital output buffer mask with config buffer to actuate real ouputs 
__IO uint32_t dout_0_7_rem;     // digital output state forcing from remote, not valid after new power cycle 
uint8_t din_cfg[8];             // digital inputs configuration saved and restored from eeprom
uint8_t dout_cfg[8];            // digital output configuration saved and restored from eeprom 
uint8_t buzzer_volume;
uint8_t doorlock_force;
uint8_t hc595_dout;
/* Private Define  -----------------------------------------------------------*/
#define INPUT_DEBOUNCE_CNT  						100U    // number of input state check 100
#define FAST_INPUT_DEBOUNCE_CNT  					50U     // number of input state check 50
#define SLOW_INPUT_DEBOUNCE_CNT  					10000U  // number of input state check 50
#define ENTRY_DOOR_MAX_CYCLES						60U     // check entry door 60 time, then write log
//#define WATER_FLOOD_MAX_CYCLES                    60U     // check water flood sensor 60 time, then write log
#define ENTRY_DOOR_CHECK_CYCLE_TIME                 1111U   // 1000ms cycle period
//#define WATER_FLOOD_CHECK_CYCLE_TIME              1111U   // ~1000ms cycle period
#define DIO_PROCESS_TIME							5432U   // 5s temp  regulator cycle time
#define CAP_SW_ACTIV_TIME						    1234U   // 1 s handmaid capacitive switch activ time
#define MENU_BUTTON_TIME				            56U     // when cap switch used as service menu button
#define DOOR_LOCK_COIL_PULSE_CYCLES					10U     // 10 times on - off cycles for door lock coil
#define DOOR_LOCK_COIL_PULSE_DURATION				250U    // 250 ms door lock coil duty period
#define DOOR_LOCK_MAX_CYCLE_TIME					5112U   // 5 sec. max door lock on time
#define DOOR_BELL_LED_ACTIV_TOGGLE_TIME             200U
#define HANDMAID_LED_ACTIV_TOGGLE_TIME              200U
#define CAP1293_PRODUCT_ID							((uint8_t)0x6fU)
#define CAP1293_VENDOR_ID							((uint8_t)0x5dU)
#define CAP1293_WRITE								((uint8_t)0x50U)
#define CAP1293_READ								((uint8_t)0x51U)
/***********************************************************************
**	 	C A P 1 2 9 3 	  	 R E G I S T E R		A D D E S S E 
***********************************************************************/
#define CAP1293_MAIN_CONTROL_REG					((uint8_t)0x00U)
#define CAP1293_GENERAL_STATUS_REG					((uint8_t)0x02U)
#define CAP1293_SENSOR_INPUT_STATUS_REG				((uint8_t)0x03U)
#define CAP1293_NOISE_FLAG_STATUS_REG				((uint8_t)0x0aU)
#define CAP1293_SENSOR_1_INPUT_DELTA_COUNT_REG		((uint8_t)0x10U)
#define CAP1293_SENSOR_2_INPUT_DELTA_COUNT_REG		((uint8_t)0x11U)
#define CAP1293_SENSOR_3_INPUT_DELTA_COUNT_REG		((uint8_t)0x12U)
#define CAP1293_SENSITIVITY_CONTROL_REG				((uint8_t)0x1fU)
#define CAP1293_CONFIGURATION_REG					((uint8_t)0x20U)
#define CAP1293_SENSOR_INPUT_ENABLE_REG				((uint8_t)0x21U)
#define CAP1293_SENSOR_INPUT_CONFIGURATION_REG		((uint8_t)0x22U)
#define CAP1293_SENSOR_INPUT_CONFIGURATION_2_REG	((uint8_t)0x23U)
#define CAP1293_AVERAGING_AND_SAMPLING_CONFIG_REG	((uint8_t)0x24U)
#define CAP1293_CALIBRATION_ACTIVATE_AND_STATUS_REG	((uint8_t)0x26U)
#define CAP1293_INTERRUPT_ENABLE_REG				((uint8_t)0x27U)
#define CAP1293_REPEAT_RATE_ENABLE_REG				((uint8_t)0x28U)
#define CAP1293_SINGLE_GUARD_ENABLE_REG				((uint8_t)0x29U)
#define CAP1293_MULTIPLE_TOUCH_CONFIGURATION_REG	((uint8_t)0x2aU)
#define CAP1293_MULTIPLE_TOUCH_PATTERN_CONFIG_REG	((uint8_t)0x2bU)
#define CAP1293_MULTIPLE_TOUCH_PATTERN_REG			((uint8_t)0x2dU)
#define CAP1293_BASE_COUNT_OF_LIMIT_REG				((uint8_t)0x2eU)
#define CAP1293_RECALIBRATION_CONFIGURATION_REG		((uint8_t)0x2fU)
#define CAP1293_SENSOR_INPUT_1_TRESHOLD_REG			((uint8_t)0x30U)
#define CAP1293_SENSOR_INPUT_2_TRESHOLD_REG			((uint8_t)0x31U)
#define CAP1293_SENSOR_INPUT_3_TRESHOLD_REG			((uint8_t)0x32U)
#define CAP1293_SENSOR_INPUT_NOISE_TRESHOLD_REG		((uint8_t)0x38U)
#define CAP1293_STANDBY_CHANNEL_REG					((uint8_t)0x40U)
#define CAP1293_STANDBY_CONFIGURATION_REG			((uint8_t)0x41U)
#define CAP1293_STANDBY_SENSITIVITY_REG				((uint8_t)0x42U)
#define CAP1293_STANDBY_TRESHOLD_REG				((uint8_t)0x43U)
#define CAP1293_CONFIGURATION_2_REG					((uint8_t)0x44U)
#define CAP1293_SENSOR_INPUT_1_BASE_COUNT_REG		((uint8_t)0x50U)
#define CAP1293_SENSOR_INPUT_2_BASE_COUNT_REG		((uint8_t)0x51U)
#define CAP1293_SENSOR_INPUT_3_BASE_COUNT_REG		((uint8_t)0x52U)
#define CAP1293_POWER_BUTTON_REG					((uint8_t)0x60U)
#define CAP1293_POWER_BUTTON_CONFIGURATION_REG		((uint8_t)0x61U)
#define CAP1293_CALIBRATION_SENSITIVITY_CONFIG_REG	((uint8_t)0x80U)
#define CAP1293_SENSOR_INPUT_1_CALIBRATION_REG		((uint8_t)0xb1U)
#define CAP1293_SENSOR_INPUT_2_CALIBRATION_REG		((uint8_t)0xb2U)
#define CAP1293_SENSOR_INPUT_3_CALIBRATION_REG		((uint8_t)0xb3U)
#define CAP1293_SENSOR_INPUT_CALIBRATION_LSB_REG	((uint8_t)0xb9U)
#define CAP1293_PRODUCT_ID_REG						((uint8_t)0xfdU)
#define CAP1293_MANUFACTURER_ID_REG					((uint8_t)0xfeU)
#define CAP1293_REVISION_REG						((uint8_t)0xffU)
/* Private Macro   -----------------------------------------------------------*/
/* Private Function Prototype  -----------------------------------------------*/
static void DIN_Service(void);
static void CAP1293_Init(void);
static uint8_t CAP1293_ReadRegister(uint8_t register_address);
static void CAP1293_WriteRegister(uint8_t register_address, uint8_t register_data);
/* Program Code  -------------------------------------------------------------*/
/**
  * @brief
  * @param
  * @retval
  */
void DIO_Init(void)
{
	LEDBlueOn();
	UserCapSwPanelOn();
    PowerContactorOn();
	DOUT_Service();
	CAP1293_Init();
}
/**
  * @brief
  * @param
  * @retval
  */
void DIO_Service(void) 
{
	uint8_t rd_reg;
    static uint8_t dout_0_7_mem = 0x0U;
    static uint32_t doorlock_timer = 0x0U;
    
    if (IsRS485_UpdateActiv()) return;
	if (eComState == COM_PACKET_RECEIVED) RS485_Service();
	/** ============================================================================*/
	/**		F O R C E   D I G I T A L   O U T P U T    F R O M  	C O M M A N D 	*/
	/** ============================================================================*/
    if      (dout_0_7_rem & 0x100U)
	{
		if(!(dout_0_7_rem & 0x200U))
		{
			dout_0_7_mem = dout_0_7;
			dout_0_7_rem |= 0x200U;
		}
		dout_0_7 = (dout_0_7_rem & 0xFFU);		
        if      ((dout_0_7_rem & 0x40U) && !(dout_0_7_rem & 0x1000U))
		{
			dout_0_7_rem |= 0x1000U;
            doorlock_timer = HAL_GetTick();
		}
		else if(!(dout_0_7_rem & 0x40U))
		{
			dout_0_7_rem &= 0xEFFFU;
		}
		else if ((dout_0_7_rem & 0x40U) && (dout_0_7_rem & 0x1000U))
		{
			if((HAL_GetTick() - doorlock_timer) >= DOOR_LOCK_MAX_CYCLE_TIME) dout_0_7 &= 0xBFU;
		}
	}
	else if (dout_0_7_rem & 0x200U)
	{
		dout_0_7 = dout_0_7_mem;
		dout_0_7_rem = 0x0U;
	}	
	/** ============================================================================*/                                                                         
	/**		R E L O A D 	C A P A C I T I V E		S E N S O R		S T A T E    	*/                                                                           	
	/** ============================================================================*/
	if (IsCAP1293_Present())
	{
		rd_reg = CAP1293_ReadRegister(CAP1293_SENSOR_INPUT_STATUS_REG);		
		din_cap_sen &= 0xF8U;
		din_cap_sen |= (rd_reg & 0x07U);		
		rd_reg = CAP1293_ReadRegister(CAP1293_MAIN_CONTROL_REG);
		if(rd_reg & (0x1U<<0)) CAP1293_WriteRegister(CAP1293_MAIN_CONTROL_REG, 0);
	}
	/** ============================================================================*/                                                                         
	/**		R E L O A D 	D I G I T A L		I N P U T 		R E G I S T E R    	*/                                                                           	
	/** ============================================================================*/
    if      (din_cfg[0] == 5) din_state &= 0xFE, din_0_7 |= 0x01;
    else if (din_cfg[0] == 4) din_state |= 0x01, din_0_7 &= 0xFE;
    else if (din_cfg[0] == 2)
    {
        if      (HAL_GPIO_ReadPin(DIN_0_Port, DIN_0_Pin) == GPIO_PIN_SET)   din_state |= 0x1U;
        else if (HAL_GPIO_ReadPin(DIN_0_Port, DIN_0_Pin) == GPIO_PIN_RESET) din_state &= 0xFE;
    }
    
    if      (din_cfg[1] == 5) din_state |= 0x02, din_0_7 &= 0xFD;
    else if (din_cfg[1] == 4) din_state &= 0xFD, din_0_7 |= 0x02;
    else if (din_cfg[1] == 2)
    {
        if      (HAL_GPIO_ReadPin(DIN_1_Port, DIN_1_Pin) == GPIO_PIN_SET)   din_state |= 0x02;
        else if (HAL_GPIO_ReadPin(DIN_1_Port, DIN_1_Pin) == GPIO_PIN_RESET) din_state &= 0xFD;
    }
    
    if      (din_cfg[2] == 5) din_state &= 0xFB, din_0_7 |= 0x04;
    else if (din_cfg[2] == 4) din_state |= 0x04, din_0_7 &= 0xFB;
    else if (din_cfg[2] == 2)
    {
        if      (HAL_GPIO_ReadPin(DIN_2_Port, DIN_2_Pin) == GPIO_PIN_SET)   din_state |= 0x04;
        else if (HAL_GPIO_ReadPin(DIN_2_Port, DIN_2_Pin) == GPIO_PIN_RESET) din_state &= 0xFB;
    }        
    if      (din_cfg[3] == 5) din_state |= 0x08, din_0_7 &= 0xF7;
    else if (din_cfg[3] == 4) din_state &= 0xF7, din_0_7 |= 0x08;
    else if (din_cfg[3] == 2)
    {
        if      (HAL_GPIO_ReadPin(DIN_3_Port, DIN_3_Pin) == GPIO_PIN_SET)   din_state |= 0x08;
        else if (HAL_GPIO_ReadPin(DIN_3_Port, DIN_3_Pin) == GPIO_PIN_RESET) din_state &= 0xF7;
    }
    
    if      (din_cfg[4] == 5) din_state |= 0x10, din_0_7 &= 0xEF;
    else if (din_cfg[4] == 4) din_state &= 0xEF, din_0_7 |= 0x10;
    else if (din_cfg[4] == 2)
    {
        if      (HAL_GPIO_ReadPin(DIN_4_Port, DIN_4_Pin) == GPIO_PIN_SET)   din_state |= 0x10;
        else if (HAL_GPIO_ReadPin(DIN_4_Port, DIN_4_Pin) == GPIO_PIN_RESET) din_state &= 0xEF;
    }
    
    if      (din_cfg[5] == 5) din_state &= 0xDF, din_0_7 |= 0x20;
    else if (din_cfg[5] == 4) din_state |= 0x20, din_0_7 &= 0xDF;
    else if (din_cfg[5] == 2)
    {
        if      (HAL_GPIO_ReadPin(DIN_5_Port, DIN_5_Pin) == GPIO_PIN_SET)   din_state |= 0x20;
        else if (HAL_GPIO_ReadPin(DIN_5_Port, DIN_5_Pin) == GPIO_PIN_RESET) din_state &= 0xDF;
    }   
    if      (din_cfg[6] == 5) din_state |= 0x40, din_0_7 &= 0xBF;
    else if (din_cfg[6] == 4) din_state &= 0xBF, din_0_7 |= 0x40;
    else if (din_cfg[6] == 2)
    {
        if      (HAL_GPIO_ReadPin(DIN_6_Port, DIN_6_Pin) == GPIO_PIN_SET)   din_state |= 0x40;
        else if (HAL_GPIO_ReadPin(DIN_6_Port, DIN_6_Pin) == GPIO_PIN_RESET) din_state &= 0xBF;
    }
    
    if      (din_cfg[7] == 5) din_state |= 0x80, din_0_7 &= 0x7F;
    else if (din_cfg[7] == 4) din_state &= 0x7F, din_0_7 |= 0x80;
    else if (din_cfg[7] == 2)
    {
        if      (HAL_GPIO_ReadPin(DIN_7_Port, DIN_7_Pin) == GPIO_PIN_SET)   din_state |= 0x80;
        else if (HAL_GPIO_ReadPin(DIN_7_Port, DIN_7_Pin) == GPIO_PIN_RESET) din_state &= 0x7F;
    }
    /** ============================================================================*/                                                                         
	/**			         C A L L         D I O 		    D R I V E R S  		        */                                                                           	
	/** ============================================================================*/
	DIN_Service();	
	DOUT_Service();
    DOUT_Doorlock();
    DOUT_Buzzer();
}
/**
  * @brief
  * @param
  * @retval
  */
void DOUT_Service(void)
{	
    /** ============================================================================*/                                                                         
	/**			S E T 		D I G I T A L	  O U T P U T 		D R I V E R   		*/                                                                           	
	/** ============================================================================*/
    if      ( dout_cfg[0] == 0x5U) HAL_GPIO_WritePin(DOUT_0_Port, DOUT_0_Pin, GPIO_PIN_RESET), dout_0_7 &=(~(0x1U<<0));     /* DOUT0 output direct control          */
    else if ( dout_cfg[0] == 0x4U) HAL_GPIO_WritePin(DOUT_0_Port, DOUT_0_Pin, GPIO_PIN_SET),   dout_0_7 |=  (0x1U<<0);      /* pin 30 prema Edinovoj skici          */
    else if ( dout_cfg[0] == 0x3U){}                                                                                        /* dummy , do nothing                   */
    else if ((dout_cfg[0] == 0x2U) &&  (dout_0_7 & (1U << 0)))  HAL_GPIO_WritePin(DOUT_0_Port, DOUT_0_Pin, GPIO_PIN_SET);   /* DOUT0 output direct control          */ 
	else if ((dout_cfg[0] == 0x2U) &&(!(dout_0_7 & (1U << 0)))) HAL_GPIO_WritePin(DOUT_0_Port, DOUT_0_Pin, GPIO_PIN_RESET); /* pin 30 prema Edinovoj skici          */
	
    if      ( dout_cfg[1] == 0x5U) HAL_GPIO_WritePin(DOUT_1_Port, DOUT_1_Pin, GPIO_PIN_RESET), dout_0_7 &=(~(0x1U<<1));     /* DOUT1 output direct control          */
    else if ( dout_cfg[1] == 0x4U) HAL_GPIO_WritePin(DOUT_1_Port, DOUT_1_Pin, GPIO_PIN_SET),   dout_0_7 |=  (0x1U<<1);      /* pin 28 prema Edinovoj skici          */
    else if ( dout_cfg[1] == 0x3U){}                                                                                        /* dummy , do nothing                   */
    else if ((dout_cfg[1] == 0x2U) &&  (dout_0_7 & (1U << 1)))  HAL_GPIO_WritePin(DOUT_1_Port, DOUT_1_Pin, GPIO_PIN_SET);   /* DOUT1 output direct control          */ 
	else if ((dout_cfg[1] == 0x2U) &&(!(dout_0_7 & (1U << 1)))) HAL_GPIO_WritePin(DOUT_1_Port, DOUT_1_Pin, GPIO_PIN_RESET); /* pin 28 prema Edinovoj skici          */
	
    if      ( dout_cfg[2] == 0x5U) HAL_GPIO_WritePin(DOUT_2_Port, DOUT_2_Pin, GPIO_PIN_RESET), dout_0_7 &= 0xF3;            /* DOUT2 output direct control          */
    else if ( dout_cfg[2] == 0x4U) HAL_GPIO_WritePin(DOUT_2_Port, DOUT_2_Pin, GPIO_PIN_SET),   dout_0_7 |= 0x0C;            /* pinovi 26 i 24 prema Edinovoj skici  */
    else if ( dout_cfg[2] == 0x3U){}                                                                                        /* dummy , do nothing                   */
    else if ((dout_cfg[2] == 0x2U) &&  (dout_0_7 & (1U << 2)))  HAL_GPIO_WritePin(DOUT_2_Port, DOUT_2_Pin, GPIO_PIN_SET);   /* DOUT2 output direct control          */ 
	else if ((dout_cfg[2] == 0x2U) &&(!(dout_0_7 & (1U << 2)))) HAL_GPIO_WritePin(DOUT_2_Port, DOUT_2_Pin, GPIO_PIN_RESET); /* pinovi 26 i 24 prema Edinovoj skici  */
	
    if      ( dout_cfg[3] == 0x5U) HAL_GPIO_WritePin(DOUT_2_Port, DOUT_2_Pin, GPIO_PIN_RESET), dout_0_7 &= 0xF3;            /* DOUT2 output direct control          */
    else if ( dout_cfg[3] == 0x4U) HAL_GPIO_WritePin(DOUT_2_Port, DOUT_2_Pin, GPIO_PIN_SET),   dout_0_7 |= 0x0C;            /* pinovi 26 i 24 prema Edinovoj skici  */
    else if ( dout_cfg[3] == 0x3U){}                                                                                        /* dummy , do nothing                   */
    else if ((dout_cfg[3] == 0x2U) &&  (dout_0_7 & (1U << 3)))  HAL_GPIO_WritePin(DOUT_2_Port, DOUT_2_Pin, GPIO_PIN_SET);   /* DOUT2 output direct control          */ 
	else if ((dout_cfg[3] == 0x2U) &&(!(dout_0_7 & (1U << 3)))) HAL_GPIO_WritePin(DOUT_2_Port, DOUT_2_Pin, GPIO_PIN_RESET); /* pinovi 26 i 24 prema Edinovoj skici  */
    
    if      ( dout_cfg[4] == 0x5U) DOUT_3_SetLow(), dout_0_7 &=(~(0x1U<<4));        /* DOUT3 output shift register control  */
    else if ( dout_cfg[4] == 0x4U) DOUT_3_SetHigh(),dout_0_7 |=  (0x1U<<4);         /* DOUT3 output shift register control  */
    else if ( dout_cfg[4] == 0x3U){}                                                /* dummy , do nothing                   */
    else if ((dout_cfg[4] == 0x2U) &&  (dout_0_7 & (1U << 4)))  DOUT_3_SetHigh();   /* DOUT3 serial shift register  pin 4   */
	else if ((dout_cfg[4] == 0x2U) &&(!(dout_0_7 & (1U << 4)))) DOUT_3_SetLow();    /* pin 22 prema Edinovoj skici          */

    if      ( dout_cfg[5] == 0x5U) DOUT_4_SetLow(), dout_0_7 &=(~(0x1U<<5));        /* DOUT4 output shift register control  */
    else if ( dout_cfg[5] == 0x4U) DOUT_4_SetHigh(),dout_0_7 |=  (0x1U<<5);         /* DOUT4 output shift register control  */
    else if ( dout_cfg[5] == 0x3U){}                                                /* dummy , do nothing                   */
    else if ((dout_cfg[5] == 0x2U) &&  (dout_0_7 & (1U << 5)))  DOUT_4_SetHigh();   /* DOUT4 serial shift register  pin 5   */
	else if ((dout_cfg[5] == 0x2U) &&(!(dout_0_7 & (1U << 5)))) DOUT_4_SetLow();    /* pin 20 prema Edinovoj skici          */
        
    /* refresh hc595 serial shift register          */
	if (HAL_SPI_Transmit(&hspi2, &hc595_dout, 0x1U, 10U) != HAL_OK) ErrorHandler(DIO_FUNC, SPI_DRV);	
	HAL_GPIO_WritePin(SHIFT_CLK_Port, SHIFT_CLK_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SHIFT_CLK_Port, SHIFT_CLK_Pin, GPIO_PIN_SET);	
}
/**
  * @brief
  * @param
  * @retval
  */
void DOUT_Doorlock(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    /** ============================================================================*/                                                                         
	/**    S E T    D O O R L O C K    P W M    O R    G P I O    O U T P U T       */                                                                           	
	/** ============================================================================*/
    if      (dout_cfg[6] == 0x5U) // force to off state
    {
        if (doorlock_force == 10U)
        {
            if(HAL_TIM_PWM_GetState(&htim3) != HAL_TIM_STATE_RESET)
            {
                HAL_TIM_PWM_DeInit(&htim3);
                GPIO_InitStruct.Pin = DOORLOCK_PWM_Pin;
                GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
                GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
                HAL_GPIO_Init(DOORLOCK_PWM_Port, &GPIO_InitStruct);
            }
            HAL_GPIO_WritePin(DOORLOCK_PWM_Port, DOORLOCK_PWM_Pin, GPIO_PIN_RESET);
            
        }
        else if (HAL_TIM_PWM_GetState(&htim3) == HAL_TIM_STATE_RESET)
        {
            MX_TIM3_Init();
            HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
            __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0x0U);
        } 
        dout_0_7 &=(~(0x1U<<6));
    }
    else if (dout_cfg[6] == 0x4U) // force to on state
    {
        if (doorlock_force == 10U)
        {
            if(HAL_TIM_PWM_GetState(&htim3) != HAL_TIM_STATE_RESET)
            {
                HAL_TIM_PWM_DeInit(&htim3);
                GPIO_InitStruct.Pin = DOORLOCK_PWM_Pin;
                GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
                GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
                HAL_GPIO_Init(DOORLOCK_PWM_Port, &GPIO_InitStruct);
            }
        }
        else if (HAL_TIM_PWM_GetState(&htim3) == HAL_TIM_STATE_RESET) MX_TIM3_Init();
        
        if(doorlock_force == 10U) HAL_GPIO_WritePin(DOORLOCK_PWM_Port, DOORLOCK_PWM_Pin, GPIO_PIN_SET);
        else
        {
            __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, doorlock_force);
            HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
        }
        dout_0_7 |=  (0x1U<<6);
    }
    else if (dout_cfg[6] == 0x2U) // set only if output enabled in config
    {
        if (doorlock_force == 10U)
        {
            if(HAL_TIM_PWM_GetState(&htim3) != HAL_TIM_STATE_RESET)
            {
                HAL_TIM_PWM_DeInit(&htim3);
                GPIO_InitStruct.Pin = DOORLOCK_PWM_Pin;
                GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
                GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
                HAL_GPIO_Init(DOORLOCK_PWM_Port, &GPIO_InitStruct);
            }
        }
        else if (HAL_TIM_PWM_GetState(&htim3) == HAL_TIM_STATE_RESET) MX_TIM3_Init();

        if (dout_0_7 & (0x1U << 6))
        {
            if(doorlock_force == 10U) HAL_GPIO_WritePin(DOORLOCK_PWM_Port, DOORLOCK_PWM_Pin, GPIO_PIN_SET);
            else
            {
                __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, doorlock_force);
                HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
                
            }
        }
        else
        {
            if (doorlock_force == 10U)  HAL_GPIO_WritePin(DOORLOCK_PWM_Port, DOORLOCK_PWM_Pin, GPIO_PIN_RESET);
            else
            {
                HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
                __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0x0U);
            }
        }        
    }
}
/**
  * @brief
  * @param
  * @retval
  */
void DOUT_Buzzer(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
	/** ============================================================================*/                                                                         
	/**      S E T    B U Z Z E R    P W M    O R    G P I O    O U T P U T         */                                                                           	
	/** ============================================================================*/  
    if      (dout_cfg[7] == 0x5U) // force to off state
    {
        if (buzzer_volume == 10U)
        {
            if(HAL_TIM_PWM_GetState(&htim1) != HAL_TIM_STATE_RESET)
            {
                HAL_TIM_PWM_DeInit(&htim1);
                GPIO_InitStruct.Pin = SOUND_PWM_Pin;
                GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
                GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
                HAL_GPIO_Init(SOUND_PWM_Port, &GPIO_InitStruct);
            }
            HAL_GPIO_WritePin(SOUND_PWM_Port, SOUND_PWM_Pin, GPIO_PIN_RESET);
        }
        else if (HAL_TIM_PWM_GetState(&htim1) == HAL_TIM_STATE_RESET)
        {
            MX_TIM1_Init();
            HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0x0U);
        }
        dout_0_7 &=(~(0x1U<<7)); 
    }
    else if (dout_cfg[7] == 0x4U) // force to on state
    {
        if (buzzer_volume == 10U)
        {
            if(HAL_TIM_PWM_GetState(&htim1) != HAL_TIM_STATE_RESET)
            {
                HAL_TIM_PWM_DeInit(&htim1);
                GPIO_InitStruct.Pin = SOUND_PWM_Pin;
                GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
                GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
                HAL_GPIO_Init(SOUND_PWM_Port, &GPIO_InitStruct);
            }
        }
        else if (HAL_TIM_PWM_GetState(&htim1) == HAL_TIM_STATE_RESET) MX_TIM1_Init();
        
        if (buzzer_volume == 10U) HAL_GPIO_WritePin(SOUND_PWM_Port, SOUND_PWM_Pin, GPIO_PIN_SET);
        else
        {
            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, buzzer_volume);
            HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
        }
        dout_0_7 |=  (0x1U<<7);
    }
    else if (dout_cfg[7] == 0x2U) // set only if output enabled in config
    {
        if (buzzer_volume == 10U)
        {
            if(HAL_TIM_PWM_GetState(&htim1) != HAL_TIM_STATE_RESET)
            {
                HAL_TIM_PWM_DeInit(&htim1);
                GPIO_InitStruct.Pin = SOUND_PWM_Pin;
                GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
                GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
                HAL_GPIO_Init(SOUND_PWM_Port, &GPIO_InitStruct);
            }
        }
        else if(HAL_TIM_PWM_GetState(&htim1) == HAL_TIM_STATE_RESET) MX_TIM1_Init();

        if (dout_0_7 & (0x1U << 7))
        {
            if(buzzer_volume == 10U) HAL_GPIO_WritePin(SOUND_PWM_Port, SOUND_PWM_Pin, GPIO_PIN_SET);
            else
            {
                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, buzzer_volume);
                HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
            }
        }
        else 
        {
            if (buzzer_volume == 10U) HAL_GPIO_WritePin(SOUND_PWM_Port, SOUND_PWM_Pin, GPIO_PIN_RESET);
            else
            {
                HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
                 __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0x0U);
            } 
        }        
    }
}
/**
  * @brief
  * @param
  * @retval
  */
static void DIN_Service(void)
{
    static uint32_t garage_time = 0U;
    static uint32_t garage_timer = 0U;
	static uint32_t din_0_timer = 0x0U;
	static uint32_t din_1_timer = 0x0U;
	static uint32_t din_2_timer = 0x0U;
	static uint32_t din_3_timer = 0x0U;
	static uint32_t din_4_timer = 0x0U;
	static uint32_t din_5_timer = 0x0U;
	static uint32_t din_6_timer = 0x0U;
	static uint32_t din_7_timer = 0x0U;
    static uint32_t entry_door_tmr = 0x0U;
    static uint32_t entry_door_pcnt = 0x0U;
    static uint32_t handmaid_sw_time = 0x0U;
    static uint32_t handmaid_sw_timer = 0x0U;
    static uint32_t door_bell_sw_time = 0x0U;
    static uint32_t door_bell_sw_timer = 0x0U;
    static uint32_t door_bell_signal_timer = 0x0U;
    static uint32_t handmaid_signal_timer = 0x0U;
    static uint32_t old_state = 0x0U;
    //
	// CARD STACKER STATE
    //
    if (din_cfg[0] == 3)
    {   /* skipp processing of disabled digital input 0 */
    } 
	else if ((din_state & 0x01) && (!(din_0_7 & 0x01)))	
	{   /*  debounce digital input 0 on state */
		if(++din_0_timer >= SLOW_INPUT_DEBOUNCE_CNT)
		{        
			din_0_timer = 0;    // reset bit 0 debounce counter
			din_0_7 |= 0x01;    // change input register bit 0 state
			if (ROOM_Status != ROOM_CLEANING_RUN) UserCapSwPanelOn();
            if (!IsPowerContactorActiv())
            {
                PowerContactorOn();
                LogEvent.log_event = CARD_STACKER_ON;
                LOGGER_Write();
                StatUpdSet();
            }
            LEDRedOn(); 
		}   
	} 
	else if ((!(din_state & 0x01)) && ( din_0_7 & 0x01))
	{   /*  debounce digital input 0 off state */
		if(++din_0_timer >= SLOW_INPUT_DEBOUNCE_CNT)
		{        
            din_0_timer = 0;           // reset bit 0 debounce counter
            din_0_7 &= 0xFE;    // change input register bit 0 state
            if (IsPowerContactorActiv())
            {
                PowerContactorOff();
                LogEvent.log_event = CARD_STACKER_OFF;
                TempRegDisable();
                LOGGER_Write();
                StatUpdSet();
            }
            LEDRedOff();
		}
	} 
	else din_0_timer = 0; // reset bit 0 debounce counter 
    //
    // SOS ALARM TRIGGER SWITCH STATE
    //
    if (din_cfg[1] == 3)
    {   /* skipp processing of disabled digital input 1 */
    } 
	else if ((din_state & 0x02) && (!(din_0_7 & 0x02)))	
	{   /*  debounce digital input 1 on state */     
		if(++din_1_timer >= FAST_INPUT_DEBOUNCE_CNT)
		{        
			din_1_timer = 0; 	// reset bit 1 debounce counter
			din_0_7 |= 0x02;    // change input register bit 1 state 
		}       
	} 
	else if ((!(din_state & 0x02)) && (din_0_7 & 0x02))
	{   /*  debounce digital input 1 off state */ 
		if(++din_1_timer >= FAST_INPUT_DEBOUNCE_CNT)
		{        
			din_1_timer = 0; 	// reset bit 1 debounce counter
			din_0_7 &= 0xFD;    // change input register bit 1 state
			if (ROOM_Status != ROOM_SOS_ALARM)
            {
                ROOM_Status = ROOM_SOS_ALARM;
                StatUpdSet(); 
            }           
		}
	} 
	else din_1_timer = 0U; // reset bit 1 debounce counter
    //
	// SOS ALARM RESET SWITCH STATE
    //
    if (din_cfg[2] == 3)
    {   /* skipp processing of disabled digital input 2 */
    } 
	else if ((din_state & 0x04) && (!(din_0_7 & 0x04)))	
	{   /*  debounce digital input 2 on state */ 
		if(++din_2_timer >= FAST_INPUT_DEBOUNCE_CNT)
		{
			din_2_timer = 0; 	// reset bit 2 debounce counter
			din_0_7 |= 0x04;    // change input register bit 2 state
			if (ROOM_Status == ROOM_SOS_ALARM) 
            {
                ROOM_Status = ROOM_RESET_ALARM;
                StatUpdSet();
            }                
		}
	} 
	else if((!(din_state  & 0x04)) && (din_0_7 & 0x04))
	{   /*  debounce digital input 2 off state */ 
		if(++din_2_timer >= FAST_INPUT_DEBOUNCE_CNT)
		{
			din_2_timer = 0; 	        // reset bit 2 debounce counter
			din_0_7 &= 0xFB;    // change input register bit 2 state
		}
	} 
	else din_2_timer = 0; // reset bit 2 debounce counter       
    //
    // HANDMAID CALL STATE
    //
	if (din_cfg[3] == 3)
    {   /* skipp processing of disabled digital input 3 */
    } 
	else if ((din_state & 0x08) && (!(din_0_7 & 0x08)))	
	{   /*  debounce digital input 3 on state */ 
		if(++din_3_timer >= INPUT_DEBOUNCE_CNT)
		{
			din_3_timer = 0; 	// reset bit 3 debounce counter
			din_0_7 |= 0x08;    // change input register bit 3 state
			if (IsCallMaidActiv())
			{
				CallMaidReset();
				LogEvent.log_event = HANDMAID_SWITCH_OFF;
				LOGGER_Write();
                StatUpdSet();
			}
            LEDGreenOff();
		}  
	} 
	else if ((!(din_state & 0x08)) && (din_0_7 & 0x08))
	{   /*  debounce digital input 3 off state */ 
		if(++din_3_timer >= INPUT_DEBOUNCE_CNT)
		{        
			din_3_timer = 0; 	        // reset bit 3 debounce counter
			din_0_7 &= 0xF7;    // change input register bit 3 state
			
            if (IsDonNotDisturbActiv())
            {
                DoNotDisturb_Off();
                DISP_DndImgDelete();
                LogEvent.log_event = DO_NOT_DISTURB_SWITCH_OFF;
                LOGGER_Write();
                StatUpdSet();
            }
            
            if (!IsCallMaidActiv())
			{
                CallMaidSet();
                LogEvent.log_event = HANDMAID_SWITCH_ON;
                LOGGER_Write();
                StatUpdSet();
            }
			LEDGreenOn();
		}          
	} 
	else din_3_timer = 0; // reset bit 3 debounce counter
    //
    // MINIBAR SENSOR STATE
    //
	if (din_cfg[4] == 3)
    {   /* skipp processing of disabled digital input 4 */
    } 
	else if ((din_state & 0x10) && (!(din_0_7 & 0x10)))	
	{   /*  debounce digital input 4 on state */ 
		if(++din_4_timer >= FAST_INPUT_DEBOUNCE_CNT)
		{        
			din_4_timer = 0; 	// reset bit 4 debounce counter
			din_0_7 |= 0x10;    // change input register bit 4 state
            garage_time = 30000 + garage_timer;
		}        
	} 
	else if ((!(din_state & 0x10)) && (din_0_7 & 0x10))
	{   /*  debounce digital input 4 off state */ 
		if(++din_4_timer >= FAST_INPUT_DEBOUNCE_CNT)
		{        
			din_4_timer = 0;    // reset bit 4 debounce counter
			din_0_7 &= 0xEF;    // change input register bit 4 state
            garage_time = 0;
        }            
	} 
	else din_4_timer = 0;     	// reset bit 4 debounce counter
    //
    // BALCONY DOOR SWITCH STATE
    //
	if (din_cfg[5] == 3)
    {   /* skipp processing of disabled digital input 5 */
    } 
	else if ((din_state & 0x20) && (!(din_0_7 & 0x20)))	
	{   /*  debounce digital input 5 on state */ 
		if(++din_5_timer >= FAST_INPUT_DEBOUNCE_CNT)
		{
			din_5_timer = 0;    // reset bit 5 debounce counter
			din_0_7 |= 0x20;    // change input register bit 5 state
            garage_time = 30000 + garage_timer;
        }        
	} 
	else if ((!(din_state & 0x20)) && (din_0_7 & 0x20))
	{   /*  debounce digital input 5 off state */ 
		if(++din_5_timer >= FAST_INPUT_DEBOUNCE_CNT)
		{        
			din_5_timer = 0;    // reset bit 5 debounce counter
			din_0_7 &= 0xDF;    // change input register bit 5 state
		}        
	} 
	else din_5_timer = 0; // reset bit 5 debounce counter
    //
    // DO NOT DISTURB SWITCH STATE
    //
	if (din_cfg[6] == 3)
    {   /* skipp processing of disabled digital input 6 */
    } 
	else if ((  din_state & 0x40) && (!(din_0_7 & 0x40)))	
	{   /*  debounce digital input 6 on state */ 
		if(++din_6_timer >= INPUT_DEBOUNCE_CNT)
		{        
			din_6_timer = 0; 		// reset bit 6 debounce counter
			din_0_7 |= 0x40;    // change input register bit 6 state 
			if (IsDonNotDisturbActiv())
			{
				DoNotDisturb_Off();
				DISP_DndImgDelete();
				LogEvent.log_event = DO_NOT_DISTURB_SWITCH_OFF;
				LOGGER_Write();
                StatUpdSet();
			}		
		}        
	} 
	else if ((!(din_state & 0x40)) && (din_0_7 & 0x40))
	{   /*  debounce digital input 6 off state */ 
 		if(++din_6_timer >= INPUT_DEBOUNCE_CNT)
		{        
			din_6_timer = 0; 	// reset bit 6 debounce counter
			din_0_7 &= 0xBF;    	// change input register bit 6 state
            if (IsCallMaidActiv())
			{
				CallMaidReset();
				LogEvent.log_event = HANDMAID_SWITCH_OFF;
				LOGGER_Write();
                LEDGreenOff();
                StatUpdSet();
			}
            
            if (!IsDonNotDisturbActiv())
			{
                DoNotDisturb_On();
                LogEvent.log_event = DO_NOT_DISTURB_SWITCH_ON;
                LOGGER_Write();
                DISP_DndImg();
                StatUpdSet();
            }
        }       
	} 
	else din_6_timer = 0; // reset bit 6 debounce counter
    //
    // ENTRY DOOR SWITCH 
    // WATER FLOOD SENSOR
    //
	if (din_cfg[7] == 3)
    {   /* skipp processing of disabled digital input 7 */
    } 
	else if ((  din_state & 0x80) && (!(din_0_7 & 0x80)))	
	{   /*  debounce digital input 7 on state */ 
		if(++din_7_timer >= FAST_INPUT_DEBOUNCE_CNT)
		{
			din_7_timer = 0; // reset bit 7 debounce counter
			din_0_7 |= 0x80;    // change input register bit 7 state
			if (!IsEntryDoorOpen() && IsEntryDoorSwEnable())
            {
                EntryDoorOpenSet();
                LogEvent.log_event = ENTRY_DOOR_OPENED;
                LOGGER_Write();     
            }
            StatUpdSet();
		}	
	} 
	else if ((!(din_state & 0x80)) && (din_0_7 & 0x80))
	{   /*  debounce digital input 7 off state */ 
		if(++din_7_timer >= FAST_INPUT_DEBOUNCE_CNT)
		{
			din_7_timer = 0; // reset bit 7 debounce counter
			din_0_7 &= 0x7F;    // change input register bit 7 state
            EntryDoorSwEnable();
            if (IsEntryDoorOpen())
            {
                EntryDoorOpenReset();
                LogEvent.log_event = ENTRY_DOOR_CLOSED;
                LOGGER_Write();
            }
            StatUpdSet();
		}
	} 
	else  din_7_timer = 0; // reset bit 7 debounce counter
	//
    // ENTRY DOOR SWITCH OR WATER FLOOD SENSOR
    // ALARM REPETITION
    //
    if (IsEntryDoorOpen())
	{
		if((HAL_GetTick() - entry_door_tmr) >= ENTRY_DOOR_CHECK_CYCLE_TIME)
		{
			entry_door_tmr = HAL_GetTick();
			if(++entry_door_pcnt >= ENTRY_DOOR_MAX_CYCLES)
			{
				entry_door_pcnt = 0;
//                din_cap_sen |= (0x1U << 2); // DoorBellSwitchActiv
                LogEvent.log_event = ENTRY_DOOR_NOT_CLOSED;
				LOGGER_Write();
			}
		}
	}
	else
	{
		entry_door_pcnt = 0;
		entry_door_tmr = HAL_GetTick();
	}
    /**
    * 
    *   DOOR BELL TASTER STATE
    *
	*   check door bell taster state and if pressed
	*	activate output for defined time period and
	*	wait for releaseed switch state for new cycle
	*/
	if (IsDoorBellSwitchActiv() && !IsRightButtonActiv())   
	{
		RightButtonSet();
        door_bell_sw_timer = HAL_GetTick();
        door_bell_signal_timer = HAL_GetTick();
        
		if(!IsDonNotDisturbActiv() && (menu.state == IDLE) && (jrnl_mod < 0x2U))
		{
            DoorBell_On();
            BUZZ_State = BUZZ_DOOR_BELL;
            door_bell_sw_time = CAP_SW_ACTIV_TIME;
			LogEvent.log_event = DOOR_BELL_ACTIVE;
			LOGGER_Write();
            StatUpdSet();
		}
		else
		{
            door_bell_sw_time = MENU_BUTTON_TIME;
            menu.event = RIGHT_TASTER_PRESSED;
            jrnl_mod = 0x0U;
		}
	}
	else if ((HAL_GetTick() - door_bell_sw_timer) >= door_bell_sw_time)
	{ 
		if(!(dout_0_7_rem & (0x1U << 0x3U))) 
        {
            DoorBell_Off();
        }
		
		if(IsRightButtonActiv())			
		{
            LEDBlueOn();
			RightButtonReset();
		}
	}
    else if (IsRightButtonActiv())
    {
        if((HAL_GetTick() - door_bell_signal_timer) >= DOOR_BELL_LED_ACTIV_TOGGLE_TIME)
        {
            door_bell_signal_timer = HAL_GetTick();
            if (IsLEDBlueActiv())   LEDBlueOff();
            else LEDBlueOn();
        }
    }
	//
    //  HANDMAID TASTER STATE
    //
	if (IsHandmaidSwitchActiv() && !IsLeftButtonActiv())
	{
		LeftButtonSet();
		handmaid_sw_timer = HAL_GetTick();
        handmaid_signal_timer = HAL_GetTick();
        if(IsLEDGreenActiv()) old_state = 0x1U;
        else old_state = 0x0U;
        StatUpdSet();
        
		if((menu.state == PENDING) || (jrnl_mod == 0x1U))
		{
            if(menu.state == PENDING) menu.state = PRINT;
            else if (jrnl_mod == 0x1U) ++jrnl_mod;
			BUZZ_State = BUZZ_DOOR_BELL;
            handmaid_sw_time = CAP_SW_ACTIV_TIME;
		}
		else
		{
            handmaid_sw_time = MENU_BUTTON_TIME;
            menu.event = LEFT_TASTER_PRESSED;
            if(jrnl_mod == 0x3U) jrnl_mod = 0x4U;
            else if(jrnl_mod == 0x5U) jrnl_mod = 0x2U;
		}
	}
    else if ((HAL_GetTick() - handmaid_sw_timer) > handmaid_sw_time) 
    {
        if(IsLeftButtonActiv())
        {
            LeftButtonReset();
            if(old_state == 0x0U) LEDGreenOff();
            else LEDGreenOn();
        }
	}
    else if (IsLeftButtonActiv())
    {
        if((HAL_GetTick() - handmaid_signal_timer) >= HANDMAID_LED_ACTIV_TOGGLE_TIME)
        {
            handmaid_signal_timer = HAL_GetTick();
            if (IsLEDGreenActiv())  LEDGreenOff();
            else LEDGreenOn();
        }
    }
    //
    //  GARAGE DOOR TIMER
    //
    if (garage_time) // if timer timeout set, timer is activated
    {
        if ((HAL_GetTick() - garage_timer) >= garage_time)
        {
            DoorLockCoil_On();
            BUZZ_State = BUZZ_CARD_VALID;
            RC522_ExtendDoorlockTimeSet();
            RC522_ClearData();
        }          
    }
    else garage_timer = HAL_GetTick(); // keep update with system clock
}
/**
  * @brief
  * @param
  * @retval
  */
static void CAP1293_Init(void)
{
	uint8_t vendor_id, product_id, reg_wr[2];
	
	vendor_id = 0U;
	product_id = 0U;
    
	reg_wr[0] = CAP1293_PRODUCT_ID_REG;
	if(HAL_I2C_Master_Transmit(&hi2c1, CAP1293_WRITE, reg_wr, 1U, DRV_TOUT) != HAL_OK)       ErrorHandler(CAP_FUNC, I2C_DRV);
	if(HAL_I2C_Master_Receive(&hi2c1, CAP1293_READ, &product_id, 1U, DRV_TOUT) != HAL_OK)    ErrorHandler(CAP_FUNC, I2C_DRV);
	reg_wr[0] = CAP1293_MANUFACTURER_ID_REG;
	if(HAL_I2C_Master_Transmit(&hi2c1, CAP1293_WRITE, reg_wr, 1U, DRV_TOUT) != HAL_OK)       ErrorHandler(CAP_FUNC, I2C_DRV);
	if(HAL_I2C_Master_Receive(&hi2c1, CAP1293_READ, &vendor_id, 1U, DRV_TOUT) != HAL_OK)     ErrorHandler(CAP_FUNC, I2C_DRV);
	
	if((product_id == CAP1293_PRODUCT_ID) && (vendor_id == CAP1293_VENDOR_ID)) 
	{
		CAP1293_SensorPresent();
		CAP1293_WriteRegister(CAP1293_MULTIPLE_TOUCH_CONFIGURATION_REG, 0U);
		CAP1293_WriteRegister(CAP1293_SENSOR_INPUT_ENABLE_REG, 0x05U);
		CAP1293_WriteRegister(CAP1293_INTERRUPT_ENABLE_REG, 0x05U);
        CAP1293_WriteRegister(CAP1293_CONFIGURATION_REG, 0x28U);
		CAP1293_WriteRegister(CAP1293_CONFIGURATION_2_REG, 0x40U);
		CAP1293_WriteRegister(CAP1293_REPEAT_RATE_ENABLE_REG, 0U);
		CAP1293_WriteRegister(CAP1293_SINGLE_GUARD_ENABLE_REG, 0x05U);
		CAP1293_WriteRegister(CAP1293_SENSITIVITY_CONTROL_REG, 0x0fU);
		CAP1293_WriteRegister(CAP1293_CALIBRATION_SENSITIVITY_CONFIG_REG, 0U);
		CAP1293_WriteRegister(CAP1293_MAIN_CONTROL_REG, 0U);
	}
	else CAP1293_SensorNotPresent();
}
/**
  * @brief
  * @param
  * @retval
  */
static uint8_t CAP1293_ReadRegister(uint8_t register_address)
{
	uint8_t ret_val;
	
	if(HAL_I2C_Master_Transmit(&hi2c1, CAP1293_WRITE, &register_address, 1U, DRV_TOUT) != HAL_OK)    ErrorHandler(CAP_FUNC, I2C_DRV);
	if(HAL_I2C_Master_Receive(&hi2c1, CAP1293_READ, &ret_val, 1U, DRV_TOUT) != HAL_OK)               ErrorHandler(CAP_FUNC, I2C_DRV);
	return(ret_val);
}
/**
  * @brief
  * @param
  * @retval
  */
static void CAP1293_WriteRegister(uint8_t register_address, uint8_t register_data)
{
	uint8_t reg_val[2];
	
	reg_val[0] = register_address;
	reg_val[1] = register_data;
	if(HAL_I2C_Master_Transmit(&hi2c1, CAP1293_WRITE, reg_val, 2U, DRV_TOUT) != HAL_OK)              ErrorHandler(CAP_FUNC, I2C_DRV);
}
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
