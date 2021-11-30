/**
 ******************************************************************************
 * File Name          : rs485.c
 * Date               : 28/02/2016 23:16:19
 * Description        : rs485 communication modul
 ******************************************************************************
 *
 *
 ******************************************************************************
 */

#if (__RS485_H__ != FW_BUILD)
    #error "rs485 header version mismatch"
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
/* Imported Types  -----------------------------------------------------------*/
/* Imported Variables --------------------------------------------------------*/
/* Imported Functions    -----------------------------------------------------*/
/* Private Typedef -----------------------------------------------------------*/
/* Private Define  -----------------------------------------------------------*/
#define SENS_ADD_MIN              10       
#define SENS_ADD_MAX              9999
/* Private Variables  --------------------------------------------------------*/
uint8_t rs_bps;
uint32_t rs_flags;
uint8_t rs_ifa[2];
uint8_t rs_gra[2];
uint8_t rs_bra[2];
uint8_t rs_txaddr[2]; // sender address
static uint32_t rs_rxtmr    = 0U;
static uint32_t rs_rxtout   = 0U;
//static uint8_t  rs_init     = 0U;
static uint8_t  rs_rxbuf[RS485_BSIZE];
static uint8_t  rs_txbuf[RS485_BSIZE];
/* Private macros   ----------------------------------------------------------*/
/* Private Function Prototypes -----------------------------------------------*/
static void RS485_SetUsart(uint8_t brate, uint8_t bsize);
/* Program Code  -------------------------------------------------------------*/
/**
* @brief :  init usart interface to rs485 9 bit receiving 
* @param :  and init state to receive packet control block 
* @retval:  wait to receive:
*           packet start address marker SOH or STX  2 byte  (1 x 9 bit)
*           packet receiver address 4 bytes msb + lsb       (2 x 9 bit)
*           packet sender address msb + lsb 4 bytes         (2 x 9 bit)
*           packet lenght msb + lsb 4 bytes                 (2 x 9 bit)
*/
void RS485_Init(void) // init usart to known state
{
    rs_rxtmr = HAL_GetTick();           // take current sys timer
    rs_rxtout = REC_TOUT;               // set timeout for receive to complete
    RS485_SetUsart(BR_115200, WL_8BIT); // init usart to 9 bit mode
}
/**
* @brief  : rs485 service function is  called from every
* @param  : main loop cycle to service rs485 communication
* @retval : receive and send on regulary base 
*/
void RS485_Service(void)
{
    if (IsUpdateActiv()) return;
    else if ((HAL_GetTick() - rs_rxtmr) >= rs_rxtout)
    {
        rs_rxtmr = HAL_GetTick(); // take current sys timer
        rs_rxtout = 12U;
        rs_txbuf[0] = 0x55;
        rs_txbuf[1] = 0xAA;
        rs_txbuf[2] = 0x09;
        rs_txbuf[3] = 0x00;
        rs_txbuf[4] = 0x09;
        RS485_SetUsart(BR_115200, WL_8BIT);         // init usart to 9 bit mode 
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);
        HAL_UART_Transmit(&huart1, rs_txbuf, 5, 10); 
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
        ZEROFILL(rs_rxbuf, 12);
        __HAL_UART_FLUSH_DRREGISTER(&huart1);       // clear data buffers 
        HAL_UART_Receive_IT(&huart1, rs_rxbuf, 11U); // init and start interrupt receive        
    }
}
/**
  * @brief  
  * @param  
  * @retval 
  */
void RS485_RxCpltCallback(void)
{
    static uint8_t  senstat = 0;
    static uint16_t senaddr = 0;

    if((((rs_rxbuf[5]<<8)|rs_rxbuf[4]) > SENS_ADD_MIN)
    && (((rs_rxbuf[5]<<8)|rs_rxbuf[4]) < SENS_ADD_MAX)
    &&  ((rs_rxbuf[6] == 0x10)
    ||   (rs_rxbuf[6] == 0x11)
    ||   (rs_rxbuf[6] == 0x21)))
    {
        if ((senaddr != ((rs_rxbuf[5]<<8)|rs_rxbuf[4])) || (senstat != rs_rxbuf[6]))
        {
            senstat = rs_rxbuf[6];
            senaddr = ((rs_rxbuf[5]<<8)|rs_rxbuf[4]);
            DISP_RFSensor(senaddr, senstat); // call only if new event
        }
    }
}
/**
* @brief : all data send from buffer ?
* @param : what  should one to say   ? well done,   
* @retval: well done, and there will be more..
*/
void RS485_TxCpltCallback(void)
{
}
/**
* @brief : set usart for comunication ower rs485 interface, default to receive 9 bit, 115200bps
* @param : as in onewire interface, 9 bit will be used for addressing and 8 bit for all other 
* @retval: data and controll data exchange untill link is closed. data rate should be negotiable.
*        : and data controll dirrection should be mainteninh in hradware with GPIOA PIN 12
*/
static void RS485_SetUsart(uint8_t brate, uint8_t bsize)
{
    HAL_NVIC_DisableIRQ(USART1_IRQn);
    HAL_UART_DeInit(&huart1);
	huart1.Instance             = USART1;
    huart1.Init.BaudRate        = bps[brate];
    huart1.Init.StopBits        = UART_STOPBITS_1;
	huart1.Init.Mode            = UART_MODE_TX_RX;
    huart1.Init.Parity          = UART_PARITY_NONE;
    huart1.Init.HwFlowCtl       = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling    = UART_OVERSAMPLING_16;
    huart1.Init.OneBitSampling  = UART_ONE_BIT_SAMPLE_DISABLE;
    if      (bsize == WL_9BIT)  huart1.Init.WordLength  = UART_WORDLENGTH_9B;
	else if (bsize == WL_8BIT)  huart1.Init.WordLength  = UART_WORDLENGTH_8B;
	huart1.AdvancedInit.AdvFeatureInit                  = UART_ADVFEATURE_NO_INIT;
    HAL_NVIC_SetPriority(USART1_IRQn, 3, 0);
	HAL_NVIC_EnableIRQ(USART1_IRQn);

    if(bsize == WL_9BIT)
    {
        if (HAL_MultiProcessor_Init(&huart1, STX, UART_WAKEUPMETHOD_ADDRESSMARK) != HAL_OK) ErrorHandler(RS485_FUNC, USART_DRV);
        HAL_MultiProcessor_EnableMuteMode(&huart1);
        HAL_MultiProcessor_EnterMuteMode (&huart1);
    }
    else  if (bsize == WL_8BIT)
    {
        if (HAL_UART_Init(&huart1) != HAL_OK)   ErrorHandler(RS485_FUNC, USART_DRV);
    }
}
/**
* @brief : usart error occured during transfer
* @param : clear error flags and reinit usaart
* @retval: and wait for address mark from master 
*/
void RS485_ErrorCallback(void) 
{
    __HAL_UART_CLEAR_PEFLAG(&huart1);
	__HAL_UART_FLUSH_DRREGISTER(&huart1);
	huart1.ErrorCode = HAL_UART_ERROR_NONE;
}
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
