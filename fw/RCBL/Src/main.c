/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : W25Q64 SPI FLASH BOOTLOADER
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"


#if (__MAIN_H__ != FW_BUILD)
    #error "main header version mismatch"
#endif

#ifndef ROOM_CONTROLLER
    #error "room controller not selected for application in common.h"
#endif

#ifndef BOOTLOADER
    #error "bootloader not selected for application type in common.h"
#endif

/* Private typedefs ----------------------------------------------------------*/
typedef  void (*pFunction)(void);
IWDG_HandleTypeDef hiwdg;
I2C_HandleTypeDef hi2c1;
SPI_HandleTypeDef hspi2;
CRC_HandleTypeDef hcrc;
/* Private variables ---------------------------------------------------------*/
pFunction JumpToApplication;
uint32_t jump_address;
uint8_t dout = 0U;


#define LED_HandmaidGreen_On()        	(dout |= (1U << 0U))
#define LED_HandmaidGreen_Off()       	(dout &= (~(1U << 0U)))
#define LED_HandmaidGreen_Toggle()    	((dout & (1U << 0U)) ? LED_HandmaidGreen_Off() : LED_HandmaidGreen_On())
#define LED_RoomStatusRed_On()      	(dout |= (1U << 1U))
#define LED_RoomStatusRed_Off()     	(dout &= (~(1U << 1U)))
#define LED_RoomStatusRed_Toggle() 		((dout & (1U << 1U)) ? LED_RoomStatusRed_Off() : LED_RoomStatusRed_On())
#define LED_DoorBellBlue_On()     		(dout |= (1U << 2U))
#define LED_DoorBellBlue_Off()    		(dout &= (~(1U << 2U)))
#define LED_DoorBellBlue_Toggle()		((dout & (1U << 2U)) ? LED_DoorBellBlue_Off() : LED_DoorBellBlue_On())
#define LED_RfidReaderWhite_On()        (dout |= (1U << 3U))
#define LED_RfidReaderWhite_Off()       (dout &= (~(1U << 3U)))
#define LED_RfidReaderWhite_Toggle()    ((dout & (1U << 3U)) ? LED_RfidReaderWhite_Off() : LED_RfidReaderWhite_On())
#define DOUT_3_SetHigh()				(dout |= (1U << 4U))
#define DOUT_3_SetLow()					(dout &= (~(1U << 4U)))
#define Get_DOUT_3_State()				(dout &  (1U << 4U))
#define DOUT_4_SetHigh()				(dout |= (1U << 5U))
#define DOUT_4_SetLow()					(dout &= (~(1U << 5U)))
#define Get_DOUT_4_State()				(dout &  (1U << 5U))
#define DISPLAY_RST_SetHigh()			(dout |= (1U << 6U))
#define DISPLAY_RST_SetLow()			(dout &= (~(1U << 6U)))
#define RC522_RST_SetHigh()				(dout |= (1U << 7U))
#define RC522_RST_SetLow()				(dout &= (~(1U << 7U)))

#define EE_FW_UPDATE_BYTE_CNT           ((uint16_t)0x0021)		// firmware update byte count i2c eeprom address 
#define EE_FW_UPDATE_STATUS             ((uint16_t)0x0025)		// firmware update status i2c eeprom address 
/* Private function prototypes -----------------------------------------------*/
static void HW_DeInit       (void);
static void FLASH_Init      (void);
static void HC595_Load      (void);
static void MX_CRC_Init     (void);
static void MX_SPI2_Init    (void);
static void MX_GPIO_Init    (void);
static void MX_I2C1_Init    (void);
static void MX_IWDG_Init    (void);
static void SysClk_Config   (void);
static void ApplicationExe  (void);
static void LED_RedBlink    (uint8_t cnt);
static void EEPROM_WrUpdStat(uint8_t status);
static uint32_t FLASH_Erase (uint32_t StartSector);
static void FLASH_Copy      (uint32_t addr, uint32_t size);
static void FLASH_Read      (uint32_t addr, uint8_t  *data, uint16_t size);
static uint32_t FLASH_Write (uint32_t addr, uint32_t *data, uint32_t size);
/* Program code   ------------------------------------------------------------*/
int main(void)
{
    uint8_t buf[8], status;
    uint32_t image_size;
    HAL_Init();
    SysClk_Config();
#ifdef	USE_WATCHDOG
	MX_IWDG_Init();
#endif
    MX_CRC_Init();
    MX_GPIO_Init();
    FLASH_Init();    
	MX_I2C1_Init();
	MX_SPI2_Init();
	/**
    *   jump around
    */	
    while (1)
	{
		LED_HandmaidGreen_On();
        HC595_Load();
		DelayMs(500U);
        
		if (HAL_I2C_IsDeviceReady (&hi2c1, I2CEE_ADDR, 1000U, 1000U) != HAL_OK)
		{
			LED_RedBlink(1);
			HW_DeInit();
			HAL_FLASH_OB_Launch();
		}
        
		buf[0] = EE_FW_UPDATE_STATUS >> 8;
		buf[1] = EE_FW_UPDATE_STATUS & 0xFFU;
		HAL_I2C_Master_Transmit (&hi2c1, I2CEE_ADDR, buf, 0x2U, I2CEE_TIMEOUT);
		HAL_I2C_Master_Receive  (&hi2c1, I2CEE_ADDR, &status, 0x1U, I2CEE_TIMEOUT);
#ifdef	USE_WATCHDOG
		HAL_IWDG_Refresh(&hiwdg);
#endif        
		if      (status == BLDR_CMD_RUN)
		{
			buf[0] = EE_FW_UPDATE_BYTE_CNT >> 8;
			buf[1] = EE_FW_UPDATE_BYTE_CNT & 0xFFU;
			HAL_I2C_Master_Transmit (&hi2c1, I2CEE_ADDR, buf, 0x2U, I2CEE_TIMEOUT);
			HAL_I2C_Master_Receive  (&hi2c1, I2CEE_ADDR, &buf[2], 0x4U, I2CEE_TIMEOUT);
			image_size = (buf[2] << 24)|(buf[3] << 16)|(buf[4] << 8)|buf[5];
            EEPROM_WrUpdStat(BLDR_UPD_OK);
            
            if ((image_size > RC_APPL_SIZE) || (image_size == 0x0U))
            {
                LED_RedBlink(2);
                HW_DeInit();
                HAL_FLASH_OB_Launch();
            }    
			FLASH_Erase(RC_APPL_ADDR);
			FLASH_Copy(RC_NEW_APPL_ADDR, image_size);
		}
		else if (status == BLDR_UPD_OK)
		{
            LED_RedBlink(3);
			FLASH_Erase(RC_APPL_ADDR);
            FLASH_Copy(RC_BKP_APPL_ADDR, RC_APPL_SIZE);
            EEPROM_WrUpdStat(BLDR_UPD_ERR);
		}
		else if (status == BLDR_UPD_ERR)
		{
			EEPROM_WrUpdStat(BLDR_CMD_RUN);
			LED_HandmaidGreen_Off();
			
			while(1)
			{
                LED_RedBlink(1);
			}			
		}
		ApplicationExe();
    }  
}
/**
  * Initializes the Global MSP.
  */
void HAL_MspInit(void)
{
	__HAL_RCC_AFIO_CLK_ENABLE();

	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
	__HAL_AFIO_REMAP_SWJ_NOJTAG();
}


void MX_IWDG_Init(void)
{
    hiwdg.Instance = IWDG;
    hiwdg.Init.Prescaler = IWDG_PRESCALER_64;
    hiwdg.Init.Reload = 4095U;
    HAL_IWDG_Init(&hiwdg);
}


void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	/* Peripheral clock enable */
	__SPI2_CLK_ENABLE();

	/**SPI1 GPIO Configuration    
	PB13     ------> SPI2_SCK
	PB14     ------> SPI2_MISO
	PB15     ------> SPI2_MOSI 
	*/
	GPIO_InitStruct.Pin = SPI2_SCK_Pin|SPI2_MOSI_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(SPI2_MOSI_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = SPI2_MISO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(SPI2_MISO_Port, &GPIO_InitStruct);
  
}


void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	if(hi2c->Instance==I2C1)
	{
        /* Peripheral clock enable */
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_AFIO_CLK_ENABLE();
        __HAL_AFIO_REMAP_I2C1_ENABLE();
        /**I2C1 GPIO Configuration    
        PB8     ------> I2C1_SCL
        PB9     ------> I2C1_SDA 
        */
        GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);


        /* Enable Eval_I2Cx clock */
        __HAL_RCC_I2C1_CLK_ENABLE();

        /* Add delay related to RCC workaround */
        while (READ_BIT(RCC->APB1ENR, RCC_APB1ENR_I2C1EN) != RCC_APB1ENR_I2C1EN) {};

        /* Force the I2C Periheral Clock Reset */  
        __HAL_RCC_I2C1_FORCE_RESET();
          
        /* Release the I2C Periheral Clock Reset */  
        __HAL_RCC_I2C1_RELEASE_RESET(); 
	}
}
static void SysClk_Config   (void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_PeriphCLKInitTypeDef PeriphClkInit;
	
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSEState = RCC_HSI_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
	RCC_OscInitStruct.LSEState = RCC_LSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
	HAL_RCC_OscConfig(&RCC_OscInitStruct);

	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
							  |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
	HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);

	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
	PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

	/* SysTick_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}


static void MX_GPIO_Init    (void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    __GPIOA_CLK_ENABLE();
    __GPIOB_CLK_ENABLE();
	/**
	*	preset gpio output pin
	*/
    HAL_GPIO_WritePin(GPIOA, DISPLAY_LED_PWM_Pin|DOUT_0_Pin|DOUT_1_Pin|DOUT_2_Pin|DOORLOCK_PWM_Pin|
                             SOUND_PWM_Pin|RS485_DIR_Pin|SHIFT_CLK_Pin, GPIO_PIN_RESET);	
    HAL_GPIO_WritePin(GPIOB, DISPLAY_CS_Pin|FLASH_CS_Pin, GPIO_PIN_SET);

    GPIO_InitStruct.Pin = 	DISPLAY_LED_PWM_Pin |
                            DOUT_0_Pin |
                            DOUT_1_Pin |
                            DOUT_2_Pin |
                            DOORLOCK_PWM_Pin |
                            SOUND_PWM_Pin |
                            RS485_DIR_Pin |
                            SHIFT_CLK_Pin;
    GPIO_InitStruct.Mode = 	GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GPIOA, 	&GPIO_InitStruct);
	
    GPIO_InitStruct.Pin = 	DISPLAY_CS_Pin |
                            FLASH_CS_Pin;
    GPIO_InitStruct.Mode = 	GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GPIOB, 	&GPIO_InitStruct);
}


static void MX_I2C1_Init    (void)
{
    if(HAL_I2C_GetState(&hi2c1) == HAL_I2C_STATE_RESET)
    {
        hi2c1.Instance = I2C1;
        hi2c1.Init.ClockSpeed = 400000U;
        hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
        hi2c1.Init.OwnAddress1 = 0U;
        hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
        hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
        hi2c1.Init.OwnAddress2 = 0U;
        hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
        hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
        if(HAL_I2C_Init(&hi2c1) != HAL_OK) while(1) LED_RedBlink(1); 
    }
}


static void MX_SPI2_Init    (void)
{
    hspi2.Instance = SPI2;
    hspi2.Init.Mode = SPI_MODE_MASTER;
    hspi2.Init.Direction = SPI_DIRECTION_2LINES;
    hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi2.Init.NSS = SPI_NSS_SOFT;
    hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
    hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi2.Init.TIMode = SPI_TIMODE_DISABLED;
    hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
    hspi2.Init.CRCPolynomial = 10U;
    HAL_SPI_Init(&hspi2);
}


static void HC595_Load      (void)
{
    HAL_SPI_Transmit(&hspi2, &dout, 1, 10);	
    HAL_GPIO_WritePin(SHIFT_CLK_Port, SHIFT_CLK_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SHIFT_CLK_Port, SHIFT_CLK_Pin, GPIO_PIN_SET);
}


static void ApplicationExe  (void)
{
    if (((*(__IO uint32_t*)RC_APPL_ADDR) & 0x2FFE0000U) == 0x20000000U)
    {
        HW_DeInit();
		jump_address = *(__IO uint32_t*) (RC_APPL_ADDR + 4U);
		JumpToApplication = (pFunction) jump_address;
		__set_MSP(*(__IO uint32_t*) RC_APPL_ADDR);
		JumpToApplication();
    }
}


static void HW_DeInit       (void)
{
    HAL_CRC_DeInit(&hcrc);
    HAL_GPIO_DeInit(GPIOA, DISPLAY_LED_PWM_Pin|DOUT_0_Pin|DOUT_1_Pin|DOUT_2_Pin|DOORLOCK_PWM_Pin|SOUND_PWM_Pin|RS485_DIR_Pin|SHIFT_CLK_Pin);
    HAL_GPIO_DeInit(GPIOB, DISPLAY_CS_Pin|FLASH_CS_Pin);
    __I2C1_CLK_DISABLE();
		HAL_GPIO_DeInit(I2C1_SDA_Port, I2C1_SCL_Pin|I2C1_SDA_Pin);
    __SPI2_CLK_DISABLE();
	HAL_GPIO_DeInit(GPIOB, SPI2_SCK_Pin|SPI2_MISO_Pin|SPI2_MOSI_Pin);
    HAL_DeInit();
#ifdef	USE_WATCHDOG
		HAL_IWDG_Refresh(&hiwdg);
#endif  
}    


static void FLASH_Init      (void)
{
    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
    HAL_FLASH_Lock();
}


static void MX_CRC_Init     (void)
{
	hcrc.Instance = CRC;
	HAL_CRC_Init(&hcrc);
}


static void LED_RedBlink    (uint8_t cnt)
{
    uint32_t t = cnt * 0x2U;
    LED_RoomStatusRed_Off();
    
    do
    {
        LED_RoomStatusRed_Toggle();
        HC595_Load();
        DelayMs(500U);
    }
    while(t--);
}    



static void EEPROM_WrUpdStat(uint8_t data)
{
    uint8_t eebuff[4];
    eebuff[0] = EE_FW_UPDATE_STATUS >> 8;
    eebuff[1] = EE_FW_UPDATE_STATUS & 0xFFU;
    eebuff[2] = data;
    HAL_I2C_Master_Transmit (&hi2c1, I2CEE_ADDR, eebuff, 3,     I2CEE_TIMEOUT);
    HAL_I2C_IsDeviceReady   (&hi2c1, I2CEE_ADDR, I2CEE_TRIALS,  I2CEE_TIMEOUT);
}


static uint32_t FLASH_Erase (uint32_t addr)
{
    uint32_t PageError = 0x0U;
    uint32_t NbrOfPages = 0x0U;
    FLASH_EraseInitTypeDef pEraseInit;
    HAL_StatusTypeDef status = HAL_OK;
    HAL_FLASH_Unlock();
    NbrOfPages = ((FLASH_END_ADDR - addr) / FLASH_PAGE_SIZE);
    pEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
    pEraseInit.PageAddress = addr;
    pEraseInit.Banks = FLASH_BANK_1;
    pEraseInit.NbPages = NbrOfPages;
    status = HAL_FLASHEx_Erase(&pEraseInit, &PageError);
    HAL_FLASH_Lock();
    if (status != HAL_OK) return FLIF_WR_ERR;
    return FLIF_OK;
}


static void FLASH_Copy      (uint32_t fw_address, uint32_t fw_size)
{
    
    uint32_t bcnt = 0x0U;
    uint8_t buff[FLASH_BUFFER_SIZE];
    uint32_t flash_destination = RC_APPL_ADDR;
    
    while(fw_size)
    {		
        if (fw_size >= FLASH_BUFFER_SIZE) bcnt = FLASH_BUFFER_SIZE;
        else bcnt = fw_size;
        FLASH_Read  (fw_address, buff,  bcnt);
        FLASH_Write (flash_destination, (uint32_t*) buff, (bcnt / 4U));
        flash_destination += bcnt;
        fw_address += bcnt;
        fw_size -= bcnt;
    }
}


static void FLASH_Read      (uint32_t addr, uint8_t *data, uint16_t size)
{
    uint8_t tx[4];
	
    tx[0] = SPI_EE_READ;
    tx[1] = (addr >> 16);
    tx[2] = (addr >> 8);
    tx[3] = (addr & 0xFFU);    
    HAL_GPIO_WritePin (FLASH_CS_Port, FLASH_CS_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit  (&hspi2, tx, 4U, FLASH_TIMEOUT);
    HAL_SPI_Receive   (&hspi2, data, size, FLASH_TIMEOUT);
    HAL_GPIO_WritePin (FLASH_CS_Port, FLASH_CS_Pin, GPIO_PIN_SET);
}


static uint32_t FLASH_Write (uint32_t addr, uint32_t *data, uint32_t size)
{
    uint32_t i;
    
    HAL_FLASH_Unlock();

    for (i = 0U; (i < size) && (addr <= (FLASH_END_ADDR - 4U)); i++)
    {
        if (HAL_FLASH_Program (FLASH_TYPEPROGRAM_WORD, addr, *(uint32_t*)(data + i)) == HAL_OK)      
        {
            if (*(uint32_t*)addr != *(uint32_t*)(data + i))
            {
                return(FLIF_WRCTRL_ERR);
            }
            addr += 4U;
        }
        else return (FLIF_WR_ERR);
    }
    HAL_FLASH_Lock();
    return (FLIF_OK);
}
/***************** (C) COPYRIGHT JUBERA D.O.O. SARAJEVO ***********************/
