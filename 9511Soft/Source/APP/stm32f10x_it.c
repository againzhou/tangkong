
/// ----------------------------------------------------------------------------
/// File Name          : stm32f10x_it.c
/// Description        : �жϷ�����Դ�ļ�
/// ----------------------------------------------------------------------------
/// History:

/// y11/m01/d18     v0.03   chenyong    modify
/// ��������жϺ��뺯�� MOTO_IRQandClr()

/// y10/m12/d23     v0.02   chenyong    modify
/// ����� PC924 ���ж� �� ��������

/// y10/m07         v0.01   chenyong    setup

/// "stm32f10x_it.h" �� ST �ṩ, �����޸�

/// ��ϵͳ���жϺ����м��� OS �ķ���, ��Ҫʹ��"������˳��ٽ���"����, 
/// ������������ cpu_a.asm �� os_cpu_a.asm �ж���, ����������ͬ, �����ֲ�ͬ
/// ԭ��ʹ�� cpu_a.asm, ���ڸ��� Ports \ os_cpu_a.asm �е���������:
///     OS_CPU_SR_Save() �� OS_CPU_SR_Restore()

/// OS/CPU Ŀ¼���ļ� cpu_a.asm �� cpu_c.c �������Բ���,
/// ���ж���ز����ɿ⺯���ṩ, ֻ��û�� �жϼ��ع��� �� CLZ(ͳ�����������).



/**
  ******************************************************************************
  * @file    Project/Template/stm32f10x_it.c
  * @author  MCD Application Team
  * @version V3.1.2
  * @date    09/28/2009
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2009 STMicroelectronics</center></h2>
  */ 

/* Includes ------------------------------------------------------------------*/
#include    "stm32f10x_it.h"
#include    <ucos_ii.h>
//#include    <os_cpu.h>          /// cy  091119  added
#include    <bsp.h>
#include    <c491.h>
#include    <moto.h>

#ifdef  PC924
//#include    <mc_cmd.h>
#include    <mc_base.h>
#include    <mc_open.h>
#include    <mc_close_low.h>
#include    <mc_close_high.h>
#endif


/** @addtogroup Template_Project
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/// cy 101004   a
/// -----------------------------------------------------------------------------------------------------
/// @brief wait for a time (usually less than 1ms, otherwise by os)
/// @param time : 1 ��Ӧ 1ms @72Hz
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
static  void  IrqDelayMs (u32 time)
{
    vu32  i;
    vu32  j;
    
    
    for (j = time; j != 0; j--) {
        for (i = 8000; i != 0; i--) {
            ;
        }
    }
}

static  void  DispErrNo (u32 n)
{
    vu32  i;
    
    
    BSP_LED_Set(0, BSP_LED_ON);

    BSP_LED_Set(1, BSP_LED_ON);
    IrqDelayMs(250);
        
    BSP_LED_Toggle(0);
    BSP_LED_Toggle(1);
    IrqDelayMs(250);
        
    for (i = 0; i < n; i++) {
        BSP_LED_Toggle(1);
        IrqDelayMs(250);
        BSP_LED_Toggle(1);
        IrqDelayMs(250);
    }
}
/// cy 101004   a


/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
    /// cy 101004   a
    for ( ; ; ) {
        DispErrNo(1);
    }
    /// cy 101004   a
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
    /// cy 101004   m
//  /* Go to infinite loop when Hard Fault exception occurs */
//  while (1)
//  {
//  }

    for ( ; ; ) {
        DispErrNo(2);
    }
    /// cy 101004   m
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
    /// cy 101004   m
//  /* Go to infinite loop when Memory Manage exception occurs */
//  while (1)
//  {
//  }

    for ( ; ; ) {
        DispErrNo(3);
    }
    /// cy 101004   m
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
    /// cy 101004   m
//  /* Go to infinite loop when Bus Fault exception occurs */
//  while (1)
//  {
//  }

    for ( ; ; ) {
        DispErrNo(4);
    }
    /// cy 101004   m
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
    /// cy 101004   m
//  /* Go to infinite loop when Usage Fault exception occurs */
//  while (1)
//  {
//  }

    for ( ; ; ) {
        DispErrNo(5);
    }
    /// cy 101004   m
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
    /// cy 101004   a
    for ( ; ; ) {
        DispErrNo(6);
    }
    /// cy 101004   a
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
    /// cy 101004   a
    for ( ; ; ) {
        DispErrNo(7);
    }
    /// cy 101004   a
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/// ----------------------------------------------------------------------------
/// @brief  This function handles PPP interrupt request.
///         This is a templet only!
/// @param  None
/// @retval None
/// ----------------------------------------------------------------------------
void PPP_IRQHandler(void)
{
//#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
//    CPU_SR         cpu_sr;
//#endif
//    
//    
//    CPU_CRITICAL_ENTER();       /// Tell uC/OS-II that we are starting an ISR
//    OSIntNesting++;
//    CPU_CRITICAL_EXIT();
//    
//    /// call the actual IRQ function
//    /// ......
//
//    OSIntExit();                /// Tell uC/OS-II that we are leaving the ISR
    
#if (OS_CRITICAL_METHOD == 3)
    OS_CPU_SR       cpu_sr;
#endif
    
    
    OS_ENTER_CRITICAL();        /// Tell uC/OS-II that we are starting an ISR
    OSIntNesting++;
    OS_EXIT_CRITICAL();
    
    /// call the actual IRQ function
    /// ......

    OSIntExit();                /// Tell uC/OS-II that we are leaving the ISR
}



void  BSP_USART_491_IRQHandler (void)
{
#if (OS_CRITICAL_METHOD == 3)
    OS_CPU_SR       cpu_sr;
#endif
    
    
    OS_ENTER_CRITICAL();        /// Tell uC/OS-II that we are starting an ISR
    OSIntNesting++;
    OS_EXIT_CRITICAL();
    
    
    if (USART_GetITStatus(BSP_USART_491, USART_IT_RXNE) != RESET) {
        C491_IrqToRxBuf(USART_ReceiveData(BSP_USART_491));
    }
    
//    if (USART_GetITStatus(BSP_USART_491, USART_IT_TC  ) != RESET) {
//        USART_ClearITPendingBit(BSP_USART_491, USART_IT_TC);
//        BSP_491_DeSet(DISABLE);
//        OSSemPost(C491_T_SemPtr);
//    }
    if (USART_GetITStatus(BSP_USART_491, USART_IT_TC) != RESET) {   
        
//        if (C491_TxIndex < C491_TxNum) {
//			USART_SendData(BSP_USART_491, C491_TxBuf[C491_TxIndex++]);
//		} else {
////        USART_ClearITPendingBit(BSP_USART_491, USART_IT_TC);  /// ��Ҫ����
//            /// �ɲ�������ж�, ֻ�ر��ж�, �Ա��Ժ�������
//            USART_ITConfig(BSP_USART_491, USART_IT_TC, DISABLE);
//            BSP_491_DeSet(DISABLE);
//        }
        C491_IrqTx();
    }
    
    
    OSIntExit();                /// Tell uC/OS-II that we are leaving the ISR
}


void  BSP_TMR_8M_IRQHandler (void)
{
#if (OS_CRITICAL_METHOD == 3)
    OS_CPU_SR       cpu_sr;
#endif
//    BitAction       st;
//    u8              i;
    
    
#ifdef  PC924
    if (TIM_GetITStatus(BSP_TMR_8M, BSP_FLAG_CC_MC_CLOSE) != RESET) {   /// �ջ����ƶ�ʱ�Ƚ��ж�, ���ɹر�����
        TIM_ClearITPendingBit(BSP_TMR_8M, BSP_FLAG_CC_MC_CLOSE);        /// ���жϱ�־
        
        MCL_IRQ();
        
    } else {                                                            /// �������ƶ�ʱ�Ƚ��ж�, ���Թر�����
        
        OS_ENTER_CRITICAL();        /// Tell uC/OS-II that we are starting an ISR
        OSIntNesting++;
        OS_EXIT_CRITICAL();
        
        if (TIM_GetITStatus(BSP_TMR_8M, BSP_FLAG_CC_MC_OPEN) != RESET) {
            TIM_ClearITPendingBit(BSP_TMR_8M, BSP_FLAG_CC_MC_OPEN);     /// ���жϱ�־
            
            MCO_IRQ();                                                  /// �ջ�������������õĶ�ʱ�ж�
            
        } else {
            
            MOTO_IRQandClr();                                           /// ���������ʱ�ж�, �������־λ
            
        }
        
        OSIntExit();                /// Tell uC/OS-II that we are leaving the ISR
    }
    
#else
    OS_ENTER_CRITICAL();            /// Tell uC/OS-II that we are starting an ISR
    OSIntNesting++;
    OS_EXIT_CRITICAL();
    
    MOTO_IRQandClr();                                                   /// ���������ʱ�ж�, �������־λ
        
    OSIntExit();                    /// Tell uC/OS-II that we are leaving the ISR
#endif
    
}


#ifdef  PC924

/// !!! ���Բ�֪ͨϵͳ, �����жϲ�Ƕ��, ��������ж����ȼ����
void  BSP_TMR_CNT_IRQHandler (void)
{
#if (OS_CRITICAL_METHOD == 3)
    OS_CPU_SR       cpu_sr;
#endif
    
    
    OS_ENTER_CRITICAL();        /// Tell uC/OS-II that we are starting an ISR
    OSIntNesting++;
    OS_EXIT_CRITICAL();
    
    
    if (TIM_GetITStatus(BSP_TMR_CNT, BSP_FLAG_CC_MCC_DEC) != RESET) {       /// ����Ǽ���λ�õĶ�ʱ�Ƚ��ж�
        TIM_ClearITPendingBit(BSP_TMR_CNT, BSP_FLAG_CC_MCC_DEC);            /// ���ж�
        
        MCH_AimDecIRQ();
    }
    
    if (TIM_GetITStatus(BSP_TMR_CNT, BSP_FLAG_CC_MCC_INC) != RESET) {       /// ���������λ�õĶ�ʱ�Ƚ��ж�
        TIM_ClearITPendingBit(BSP_TMR_CNT, BSP_FLAG_CC_MCC_INC);            /// ���ж�
        
        MCH_AimIncIRQ();
    }
    
    OSIntExit();                /// Tell uC/OS-II that we are leaving the ISR
}

/// !!! ���Բ�֪ͨϵͳ, �����жϲ�Ƕ��, ��������ж����ȼ����
void  BSP_I_E_ODD_IRQHandler (void)
{
    if(EXTI_GetITStatus(BSP_EXIT_LINE_I_E_ODD) != RESET) {
        
//        /// ��¼������������ֵ
//        MCC_OrgEnc[BSP_ID_ODD]  =   BSP_ENC_CntGet(BSP_ID_ODD);
//        
//        if (MCC_OrgIs0[BSP_ID_ODD] == FALSE) {      /// û���ҵ�ԭ��
//            BSP_ENC_CntSet(BSP_ID_ODD, 0);          /// ����������
////            MCC_R[BSP_ID_ODD]       =   0;          /// ��Ȧ������
//            MCC_OrgIs0[BSP_ID_ODD]  =   TRUE;       /// �Ѿ��ҵ�ԭ��, ����λʱ����������
//        }
        MCB_OrgIRQ(BSP_ID_ODD);
        
        /// Clear the Odd Encoder Index EXTI line pending bit
        EXTI_ClearITPendingBit(BSP_EXIT_LINE_I_E_ODD);
    }
}

/// !!! ���Բ�֪ͨϵͳ, �����жϲ�Ƕ��, ��������ж����ȼ����
void  BSP_I_E_EVEN_IRQHandler (void)
{
    if(EXTI_GetITStatus(BSP_EXIT_LINE_I_E_EVEN) != RESET) {
        
//        /// ��¼������������ֵ
//        MCC_OrgEnc[BSP_ID_EVEN] =   BSP_ENC_CntGet(BSP_ID_EVEN);
//        
//        if (MCC_OrgIs0[BSP_ID_EVEN] == FALSE) {     /// û���ҵ�ԭ��
//            BSP_ENC_CntSet(BSP_ID_EVEN, 0);         /// ����������
////            MCC_R[BSP_ID_EVEN]      =   0;          /// ��Ȧ������
//            MCC_OrgIs0[BSP_ID_EVEN] =   TRUE;       /// �Ѿ��ҵ�ԭ��, ����λʱ����������
//        }
        MCB_OrgIRQ(BSP_ID_EVEN);
        
        /// Clear the Odd Encoder Index EXTI line pending bit
        EXTI_ClearITPendingBit(BSP_EXIT_LINE_I_E_EVEN);
    }
}

///// !!! ���Բ�֪ͨϵͳ, �����жϲ�Ƕ��, ��������ж����ȼ����
//void  BSP_TMR_E_ODD_IRQHandler (void)
//{
//    if (TIM_GetITStatus(BSP_TMR_E_ODD, TIM_IT_Update) != RESET) {       /// ����Ǹ����ж�
//        TIM_ClearITPendingBit(BSP_TMR_E_ODD, TIM_IT_Update);            /// ���ж�
//        
//        if (BSP_ENC_CntGet(BSP_ID_ODD) == 0) {
//            MCC_R[BSP_ID_ODD]++;
//        } else {
//            MCC_R[BSP_ID_ODD]--;
//        }
//    }
//}
//
///// !!! ���Բ�֪ͨϵͳ, �����жϲ�Ƕ��, ��������ж����ȼ����
//void  BSP_TMR_E_EVEN_IRQHandler (void)
//{
//    if (TIM_GetITStatus(BSP_TMR_E_EVEN, TIM_IT_Update) != RESET) {      /// ����Ǹ����ж�
//        TIM_ClearITPendingBit(BSP_TMR_E_EVEN, TIM_IT_Update);           /// ���ж�
//        
//        if (BSP_ENC_CntGet(BSP_ID_EVEN) == 0) {
//            MCC_R[BSP_ID_EVEN]++;
//        } else {
//            MCC_R[BSP_ID_EVEN]--;
//        }
//    }
//}

#endif



/**
  * @}
  */ 


/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
