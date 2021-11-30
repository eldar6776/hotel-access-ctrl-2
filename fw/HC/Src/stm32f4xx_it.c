/**
  ******************************************************************************
  * @file    Project/STM32F4xx_StdPeriph_Templates/stm32f4xx_it.c 
  * @author  MCD Application Team
  * @version V1.5.0
  * @date    06-March-2015
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_it.h"
#include "GUIDRV_stm32f429i.h"
#include "main.h"
#include "uart.h"
#include "display.h"
#include "hotel_ctrl.h"
#include "GUI.h"


/** @addtogroup STM32F4xx_StdPeriph_Examples
  * @{
  */

/** @addtogroup SysTick_Example
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/
extern volatile GUI_TIMER_TIME OS_TimeMS;
__IO uint32_t sys_tick;
/**
  * @brief  This function handles ltdc global interrupt request.
  * @param  None
  * @retval None
  */


void LTDC_IRQHandler(void)
{
  LTDC_ISR_Handler();
}

/**
  * @brief  This function handles DMA2D global interrupt request.
  * @param  None
  * @retval None
  */
void DMA2D_IRQHandler(void)
{
  DMA2D_ISR_Handler();
}
//void SDIO_IRQHandler(void)
//{
//	

//}

//void DMA2_Stream3_IRQHandler (void)
//{
//  /* Process DMA2 Stream3 or DMA2 Stream6 Interrupt Sources */
// 
//}
inline uint32_t Get_SysTick(void)
{
	return sys_tick;
}


void SysTick_Handler(void)
{
	++OS_TimeMS;
    ++sys_tick;
 
	if(disp_tmr) --disp_tmr;
	if(disp_toutmr) --disp_toutmr;
}

extern void udp_echoclient_connect(void);
/**
  * @brief  This function handles EXTI15_10 exception.
  * @param  None
  * @retval None
  */
void EXTI15_10_IRQHandler(void) 
{

	 if(EXTI_GetITStatus(EXTI_Line11) != RESET)
  { 		
	 // udp_echoclient_connect();
    EXTI_ClearITPendingBit(EXTI_Line11);                   /* Clear the EXTI line  pending bit */
  }
	
}

void EXTI0_IRQHandler(void) 
{

	 if(EXTI_GetITStatus(EXTI_Line0) != RESET)
  { 		
    EXTI_ClearITPendingBit(EXTI_Line0);                   /* Clear the EXTI line  pending bit */
  }
	
}

void DCMI_IRQHandler(void)
{  	 
	//if (DCMI_GetITStatus(DCMI_IT_VSYNC) != RESET)
  if (DCMI_GetITStatus(DCMI_IT_FRAME) != RESET) 		
	{	  
		//DCMI_ClearITPendingBit(DCMI_IT_VSYNC);
//		DCMI_ClearITPendingBit(DCMI_IT_FRAME);
//		CamFrame++;  
	  
	}

}

void DMA2_Stream1_IRQHandler(void) 	 
{

	if(DMA_GetITStatus(DMA2_Stream1, DMA_IT_TCIF1))   
	{	
		DMA_ClearITPendingBit(DMA2_Stream1, DMA_IT_TCIF1); 
	}
}

#ifdef SD_DMA_MODE
	void SD_SDIO_DMA_IRQHANDLER(void)
	{
		SD_ProcessDMAIRQ();  
	} 

#endif
	
/**
  * @brief  This function handles RTC Alarm exception.
  *         used to run update service at preset date & time 
  *         when user activity is minimal 
  */
void RTC_Alarm_IRQHandler(void)
{    
	if(RTC_GetFlagStatus(RTC_FLAG_ALRAF)==SET) 
	{
		RTC_ClearFlag(RTC_FLAG_ALRAF);        	
	}   
	EXTI_ClearITPendingBit(EXTI_Line17);
}



void DMA2_Stream4_IRQHandler(void) 	
{
	/* Test on DMA Stream Transfer Complete interrupt */
	if(DMA_GetITStatus(DMA2_Stream4, DMA_IT_TCIF4))
	{
		
	}
}
/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
	
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
//void SVC_Handler(void)
//{
//}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
//void PendSV_Handler(void)
//{
//}


/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

void USART2_IRQHandler(void)
{
	uint32_t res;
    
	if(USART_GetITStatus(USARTx, USART_IT_RXNE) != RESET)
	{	 	
		res = USART_ReceiveData(USARTx);
        
		if(rxbcnt < RS_BSIZE)
		{
			rx_buff[rxbcnt] = res;
			rxbcnt++;
		} 
	}		
}

//void Uart3_IRQ(void);
void USART3_IRQHandler(void)
{
   // Uart3_IRQ(); 	
}

/**
  * @brief  This function handles CAN1 RX0 request.
  * @param  None
  * @retval None
  */
void CAN1_RX0_IRQHandler(void)
{
//    CAN1_RX0_ISR();
}

/**
  * @brief  This function handles CAN2 RX0 request.
  * @param  None
  * @retval None
  */
void CAN2_RX0_IRQHandler(void)
{
//    CAN2_RX0_ISR();
	
}
/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
