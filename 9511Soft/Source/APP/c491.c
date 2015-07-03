
/// ----------------------------------------------------------------------------
/// File Name          : c491.c
/// Description        : 491通讯模块(全双工通讯)
///     
///     因为使用 OS 资源, 所以放在APP层. 其底层函数与硬件相关, 放在 BSP 层
/// ----------------------------------------------------------------------------
/// History:

/// y10/m07/d22     v0.01   chenyong    setup

/// y10/m09/d02     v0.02   chenyong    modify
/// 协议部分由头文件移到COM模块





#define  C491_MODULE     /// 模块定义, 用区分内外部定义


/// Includes --------------------------------------------------------------------------------------------
//#include  <app_cfg.h>

#include  "c491.h"
#include  <bsp.h>
#include  <ucos_ii.h>
//#include  <stm32f10x.h>




/// Private define --------------------------------------------------------------------------------------
#define     C491_R_BUF_SIZE_POWER       5                                   /// 接收缓冲区大小的指数
#define     C491_R_BUF_SIZE             (1 << C491_R_BUF_SIZE_POWER)        /// 接收缓冲区大小
#define     C491_R_BUF_SIZE_MASK        (C491_R_BUF_SIZE - 1)               /// 接收缓冲区大小的掩码

/// Private variables -----------------------------------------------------------------------------------
static  u16         C491_R_Buf[C491_R_BUF_SIZE];        /// 接收缓冲区(为方便优化, C491_R_BUF_SIZE应为2的幂)
static  u32         C491_R_BufInIx;                     /// 数据放入到缓冲区 使用的数组索引 写前为空
static  u32         C491_R_BufOutIx;                    /// 数据从缓冲区取出 使用的数组索引 读前为非空
static  u32         C491_R_N;                           /// 装载入但未处理的数据数量, 即缓冲区中有效个数

static  u16         C491_T_Buf[C491_T_BUF_SIZE];        /// 用于发送, 意义同上
static  u32         C491_T_BufOutIx;                    /// 用于发送, 意义同上
static  u32         C491_T_N;                           /// 用于发送, 意义同上

static  OS_EVENT*   C491_R_SemPtr;                      /// 用于接收的信号量指针, 用于挂起调用任务
static  OS_EVENT*   C491_T_SemPtr;                      /// 用于发送的信号量指针, 用于挂起调用任务

/// Private function prototypes -------------------------------------------------------------------------





/// -----------------------------------------------------------------------------------------------------
/// @brief  491 communication initialization function. 
///         must be called before calling any other of the user accessible functions.
///         模块初始化
/// @param  None
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
void  C491_Init (void)
{
    C491_R_N        =   0;                                  /// 缓冲区数据初始化
    C491_R_BufInIx  =   0;
    C491_R_BufOutIx =   0;
    C491_R_SemPtr   =   OSSemCreate(0);                     /// 初始化接收使用的信号量
    
    C491_T_N        =   0;                                  /// 缓冲区数据初始化
    C491_T_BufOutIx =   0;
    C491_T_SemPtr   =   OSSemCreate(1);                     /// 初始化发送使用的信号量
    
    BSP_491_Init();                                         /// 初始化IO及中断
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  This function inserts a data into the Rx buffer
///         装载数据到接收缓冲区, 串口中断函数调用
/// @param  data : 串口接收的数据
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
void  C491_IrqToRxBuf (u16 data)
{
#if OS_CRITICAL_METHOD == 3                             /// Allocate storage for CPU status register
    OS_CPU_SR  cpu_sr = 0;
#endif
    
    
    OS_ENTER_CRITICAL();                                /// 进入临界区
    if (C491_R_N < C491_R_BUF_SIZE) {                   /// 保证缓冲区没有溢出
        C491_R_N++;                                     /// 数量增加
        C491_R_Buf[C491_R_BufInIx++] = data;            /// 装载到缓冲区, 位置后移
        C491_R_BufInIx &= C491_R_BUF_SIZE_MASK;         /// 指针卷绕处理, 不用判断, 使之无处理时间波动
        OS_EXIT_CRITICAL();                             /// 退出临界区
        OSSemPost(C491_R_SemPtr);                       /// 发出信号量
    } else {                                            /// 缓冲区满, 数据丢失
        OS_EXIT_CRITICAL();                             /// 退出临界区
    }
}

///// -----------------------------------------------------------------------------------------------------
///// @brief This function checks to see if a key was pressed
/////        判断缓冲区中是否有未读键值
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
/////        清除按键缓冲区
///// @param None
///// @retval : None
///// -----------------------------------------------------------------------------------------------------
//void  C491_R_Flush (void)
//{
//    while (C491_R_Hit()) {                                 /// 只要缓存区未空 ...
//        C491_R_GetOne(0);                                  /// ... 读出键值, 舍去
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
    
    
    OSSemPend(C491_R_SemPtr, WaitTicks, &err);          /// 接收信号, 等待 WaitTicks 后超时
    OS_ENTER_CRITICAL();                                /// 进入临界区
    if (C491_R_N > 0) {                                 /// 数据缓冲区非空
        C491_R_N--;
        data = C491_R_Buf[C491_R_BufOutIx++];           /// 取出数据, 位置后移
        C491_R_BufOutIx &= C491_R_BUF_SIZE_MASK;        /// 指针卷绕处理, 不用判断, 使之无处理时间波动
        OS_EXIT_CRITICAL();                             /// 退出临界区
        return (data);                                  /// 返回数据
    } else {
        OS_EXIT_CRITICAL();                             /// 退出临界区
        return (C491_R_OVRETIME_NODATA);                /// 返回未取得数据, 超时
    }
}


/// -----------------------------------------------------------------------------------------------------
/// @brief  This function send a data in USART IRQ
///         发送一个buffer内的数据, 串口中断函数调用
/// @param  data : 串口接收的数据
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
void  C491_IrqTx (void)
{
    if (C491_T_BufOutIx < C491_T_N) {
        USART_SendData(BSP_USART_491, C491_T_Buf[C491_T_BufOutIx++]);
    } else {
//    USART_ClearITPendingBit(BSP_USART_491, USART_IT_TC);
      /// 可不用清除中断, 只关闭中断, 以便以后发送数据
      USART_ITConfig(BSP_USART_491, USART_IT_TC, DISABLE);
      BSP_491_DeSet(DISABLE);
      OSSemPost(C491_T_SemPtr);                       /// 发出信号量
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
    
    
    OSSemPend(C491_T_SemPtr, WaitTicks, &err);          /// 等待 WaitTicks 后超时
    if (err == OS_ERR_NONE) {
        BSP_491_DeSet(ENABLE);                          /// 发送使能
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
