
/// ----------------------------------------------------------------------------
/// File Name          : c491.h
/// Description        : 491ͨѶģ��(ȫ˫��ͨѶ)
///     
///     ��Ϊʹ�� OS ��Դ, ���Է���APP��. ��ײ㺯����Ӳ�����, ���� BSP �� 
/// ----------------------------------------------------------------------------
/// History:

/// y10/m07/d22     v0.01   chenyong    setup

/// y10/m09/d02     v0.02   chenyong    modify
/// Э�鲿����ͷ�ļ��Ƶ�COMģ��




/// Define to prevent recursive inclusion --------------------------------------
#ifndef  C491_H
#define  C491_H


/// EXTERNS --------------------------------------------------------------------
#ifdef   C491_MODULE
    #define  C491_EXT                /// ������Ϊ "�ⲿ", ��ģ����
#else
    #define  C491_EXT  extern        /// ����Ϊ "�ⲿ"
#endif


/// Includes -------------------------------------------------------------------
//#include  <os_cpu.h>
//#include  <bsp.h>
#include  <stm32f10x.h>
#include  <ucos_ii.h>



/// Exported macro -------------------------------------------------------------

#define     C491_R_OVRETIME_NODATA      (u16)(-1)       /// bit9~15 ����Ϊ0����

#define     C491_T_BUF_SIZE             32              /// ���ǲ���״̬�������ؽ��յ����ݿ��ܳ���8


// Exported constants ----------------------------------------------------------

/// Global variables -----------------------------------------------------------

/// Exported functions ---------------------------------------------------------

void    C491_Init           (void);                     /// ģ���ʼ��
void    C491_IrqToRxBuf     (u16 data);                 /// ������װ��buffer
u16     C491_ReceiveOne     (u16 WaitTicks);            /// �ӻ������ж�ȡ����, �ȴ�ʱ����ϵͳ����Ϊ��λ

void    C491_IrqTx          (void);
u8      C491_SendBuf        (u32 n, u16 buf[], u16 WaitTicks);
u8      C491_SendOne        (u16 data, u16 WaitTicks);  /// ����һ������, �ȴ�ʱ����ϵͳ����Ϊ��λ
void    C491_Send           (u32 n, u16 buf[]);





#endif
/// End of file -----------------------------------------------------------------------------------------
