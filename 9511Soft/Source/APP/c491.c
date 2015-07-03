
/// ----------------------------------------------------------------------------
/// File Name          : c491.c
/// Description        : 491ͨѶģ��(ȫ˫��ͨѶ)
///     
///     ��Ϊʹ�� OS ��Դ, ���Է���APP��. ��ײ㺯����Ӳ�����, ���� BSP ��
/// ----------------------------------------------------------------------------
/// History:

/// y10/m07/d22     v0.01   chenyong    setup

/// y10/m09/d02     v0.02   chenyong    modify
/// Э�鲿����ͷ�ļ��Ƶ�COMģ��





#define  C491_MODULE     /// ģ�鶨��, ���������ⲿ����


/// Includes --------------------------------------------------------------------------------------------
//#include  <app_cfg.h>

#include  "c491.h"
#include  <bsp.h>
#include  <ucos_ii.h>
//#include  <stm32f10x.h>




/// Private define --------------------------------------------------------------------------------------
#define     C491_R_BUF_SIZE_POWER       5                                   /// ���ջ�������С��ָ��
#define     C491_R_BUF_SIZE             (1 << C491_R_BUF_SIZE_POWER)        /// ���ջ�������С
#define     C491_R_BUF_SIZE_MASK        (C491_R_BUF_SIZE - 1)               /// ���ջ�������С������

/// Private variables -----------------------------------------------------------------------------------
static  u16         C491_R_Buf[C491_R_BUF_SIZE];        /// ���ջ�����(Ϊ�����Ż�, C491_R_BUF_SIZEӦΪ2����)
static  u32         C491_R_BufInIx;                     /// ���ݷ��뵽������ ʹ�õ��������� дǰΪ��
static  u32         C491_R_BufOutIx;                    /// ���ݴӻ�����ȡ�� ʹ�õ��������� ��ǰΪ�ǿ�
static  u32         C491_R_N;                           /// װ���뵫δ�������������, ������������Ч����

static  u16         C491_T_Buf[C491_T_BUF_SIZE];        /// ���ڷ���, ����ͬ��
static  u32         C491_T_BufOutIx;                    /// ���ڷ���, ����ͬ��
static  u32         C491_T_N;                           /// ���ڷ���, ����ͬ��

static  OS_EVENT*   C491_R_SemPtr;                      /// ���ڽ��յ��ź���ָ��, ���ڹ����������
static  OS_EVENT*   C491_T_SemPtr;                      /// ���ڷ��͵��ź���ָ��, ���ڹ����������

/// Private function prototypes -------------------------------------------------------------------------





/// -----------------------------------------------------------------------------------------------------
/// @brief  491 communication initialization function. 
///         must be called before calling any other of the user accessible functions.
///         ģ���ʼ��
/// @param  None
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
void  C491_Init (void)
{
    C491_R_N        =   0;                                  /// ���������ݳ�ʼ��
    C491_R_BufInIx  =   0;
    C491_R_BufOutIx =   0;
    C491_R_SemPtr   =   OSSemCreate(0);                     /// ��ʼ������ʹ�õ��ź���
    
    C491_T_N        =   0;                                  /// ���������ݳ�ʼ��
    C491_T_BufOutIx =   0;
    C491_T_SemPtr   =   OSSemCreate(1);                     /// ��ʼ������ʹ�õ��ź���
    
    BSP_491_Init();                                         /// ��ʼ��IO���ж�
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  This function inserts a data into the Rx buffer
///         װ�����ݵ����ջ�����, �����жϺ�������
/// @param  data : ���ڽ��յ�����
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
void  C491_IrqToRxBuf (u16 data)
{
#if OS_CRITICAL_METHOD == 3                             /// Allocate storage for CPU status register
    OS_CPU_SR  cpu_sr = 0;
#endif
    
    
    OS_ENTER_CRITICAL();                                /// �����ٽ���
    if (C491_R_N < C491_R_BUF_SIZE) {                   /// ��֤������û�����
        C491_R_N++;                                     /// ��������
        C491_R_Buf[C491_R_BufInIx++] = data;            /// װ�ص�������, λ�ú���
        C491_R_BufInIx &= C491_R_BUF_SIZE_MASK;         /// ָ����ƴ���, �����ж�, ʹ֮�޴���ʱ�䲨��
        OS_EXIT_CRITICAL();                             /// �˳��ٽ���
        OSSemPost(C491_R_SemPtr);                       /// �����ź���
    } else {                                            /// ��������, ���ݶ�ʧ
        OS_EXIT_CRITICAL();                             /// �˳��ٽ���
    }
}

///// -----------------------------------------------------------------------------------------------------
///// @brief This function checks to see if a key was pressed
/////        �жϻ��������Ƿ���δ����ֵ
///// @param None
///// @retval TRUE  : if a key is     pressed
///// @retval FALSE : if a key is not pressed
///// -----------------------------------------------------------------------------------------------------
//bool  C491_R_Hit (void)
//{
//#if OS_CRITICAL_METHOD == 3                             /// Allocate storage for CPU status register
//    OS_CPU_SR  cpu_sr = 0;
//#endif
//    bool    hit;
//
//
//    OS_ENTER_CRITICAL();
//    hit = (bool)(C491_R_N > 0) ? TRUE : FALSE;
//    OS_EXIT_CRITICAL();
//    return (hit);
//}
//
///// -----------------------------------------------------------------------------------------------------
///// @brief This function clears the keyboard buffer
/////        �������������
///// @param None
///// @retval : None
///// -----------------------------------------------------------------------------------------------------
//void  C491_R_Flush (void)
//{
//    while (C491_R_Hit()) {                                 /// ֻҪ������δ�� ...
//        C491_R_GetOne(0);                                  /// ... ������ֵ, ��ȥ
//    }
//}

/// -----------------------------------------------------------------------------------------------------
/// @brief  Get a data from the Rx buffer.
/// @param  WaitTicks : is the amount of time it will wait (in number of ticks) for a data to be
///                    received.  A timeout of '0' means that the caller is willing to wait forever for
///                    a data to be received.
/// @retval :   - !C491_R_OVRETIME_NODATA   is the data received.
///             -  C491_R_OVRETIME_NODATA   indicates that there is no data in the buffer within the
///                                         specified timeout
/// -----------------------------------------------------------------------------------------------------
u16  C491_ReceiveOne (u16 WaitTicks)
{
#if OS_CRITICAL_METHOD == 3                             /// Allocate storage for CPU status register
    OS_CPU_SR  cpu_sr = 0;
#endif
    u16   data;
    u8    err;
    
    
    OSSemPend(C491_R_SemPtr, WaitTicks, &err);          /// �����ź�, �ȴ� WaitTicks ��ʱ
    OS_ENTER_CRITICAL();                                /// �����ٽ���
    if (C491_R_N > 0) {                                 /// ���ݻ������ǿ�
        C491_R_N--;
        data = C491_R_Buf[C491_R_BufOutIx++];           /// ȡ������, λ�ú���
        C491_R_BufOutIx &= C491_R_BUF_SIZE_MASK;        /// ָ����ƴ���, �����ж�, ʹ֮�޴���ʱ�䲨��
        OS_EXIT_CRITICAL();                             /// �˳��ٽ���
        return (data);                                  /// ��������
    } else {
        OS_EXIT_CRITICAL();                             /// �˳��ٽ���
        return (C491_R_OVRETIME_NODATA);                /// ����δȡ������, ��ʱ
    }
}


/// -----------------------------------------------------------------------------------------------------
/// @brief  This function send a data in USART IRQ
///         ����һ��buffer�ڵ�����, �����жϺ�������
/// @param  data : ���ڽ��յ�����
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
void  C491_IrqTx (void)
{
    if (C491_T_BufOutIx < C491_T_N) {
        USART_SendData(BSP_USART_491, C491_T_Buf[C491_T_BufOutIx++]);
    } else {
//    USART_ClearITPendingBit(BSP_USART_491, USART_IT_TC);
      /// �ɲ�������ж�, ֻ�ر��ж�, �Ա��Ժ�������
      USART_ITConfig(BSP_USART_491, USART_IT_TC, DISABLE);
      BSP_491_DeSet(DISABLE);
      OSSemPost(C491_T_SemPtr);                       /// �����ź���
    }   
}


/// -----------------------------------------------------------------------------------------------------
/// @brief  Send a set of data.
/// @param  n         : is the number of data to be sent.
///         buf       : data array.
///         WaitTicks : is the amount of time it will wait (in number of ticks) for sending 
///                     a set of data.  A timeout of '0' means that the caller is willing to wait 
///                     forever for sending a set of data.
///// @retval :   The status of error
/// -----------------------------------------------------------------------------------------------------
u8  C491_SendBuf (u32 n, u16 buf[], u16 WaitTicks)
{
    u8      err;
    u32     i;
    
    
    OSSemPend(C491_T_SemPtr, WaitTicks, &err);          /// �ȴ� WaitTicks ��ʱ
    if (err == OS_ERR_NONE) {
        BSP_491_DeSet(ENABLE);                          /// ����ʹ��
        for (i = 0; i < n; i++) {
            C491_T_Buf[i]   =   buf[i];
        }
        C491_T_N        =   n;
        C491_T_BufOutIx =   0;
        USART_ITConfig(BSP_USART_491, USART_IT_TC, ENABLE);
    }
    return  err;
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  Send a data.
/// @param  data      : one data to be sent.
/// @param  WaitTicks : is the amount of time it will wait (in number of ticks) for sending a data.
///                     A timeout of '0' means that the caller is willing to wait forever for
///                     sending a data.
/// @retval :   The status of error
/// -----------------------------------------------------------------------------------------------------
u8  C491_SendOne (u16 data, u16 WaitTicks)
{
    return  C491_SendBuf(1, &data, WaitTicks);
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  Send data.
/// @param  n   :   is the number of data to be sent.
///         buf :   data array.
/// @retval :   None
/// -----------------------------------------------------------------------------------------------------
void  C491_Send (u32 n, u16 buf[])
{
    u32     i;
    
    
    BSP_491_DeSet(ENABLE);
    for (i = 0; i < n; i++) {
        C491_T_Buf[i]   =   buf[i];
    }
    C491_T_N        =   n;
    C491_T_BufOutIx =   0;
    USART_ITConfig(BSP_USART_491, USART_IT_TC, ENABLE);
}


/// End of file -----------------------------------------------------------------------------------------
