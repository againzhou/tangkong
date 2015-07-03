
/// ----------------------------------------------------------------------------
/// File Name          : c491.h
/// Description        : 491通讯模块(全双工通讯)
///     
///     因为使用 OS 资源, 所以放在APP层. 其底层函数与硬件相关, 放在 BSP 层 
/// ----------------------------------------------------------------------------
/// History:

/// y10/m07/d22     v0.01   chenyong    setup

/// y10/m09/d02     v0.02   chenyong    modify
/// 协议部分由头文件移到COM模块




/// Define to prevent recursive inclusion --------------------------------------
#ifndef  C491_H
#define  C491_H


/// EXTERNS --------------------------------------------------------------------
#ifdef   C491_MODULE
    #define  C491_EXT                /// 不定义为 "外部", 本模块内
#else
    #define  C491_EXT  extern        /// 定义为 "外部"
#endif


/// Includes -------------------------------------------------------------------
//#include  <os_cpu.h>
//#include  <bsp.h>
#include  <stm32f10x.h>
#include  <ucos_ii.h>



/// Exported macro -------------------------------------------------------------

#define     C491_R_OVRETIME_NODATA      (u16)(-1)       /// bit9~15 不都为0即可

#define     C491_T_BUF_SIZE             32              /// 考虑测试状态，从主控接收的数据可能超过8


// Exported constants ----------------------------------------------------------

/// Global variables -----------------------------------------------------------

/// Exported functions ---------------------------------------------------------

void    C491_Init           (void);                     /// 模块初始化
void    C491_IrqToRxBuf     (u16 data);                 /// 将数据装入buffer
u16     C491_ReceiveOne     (u16 WaitTicks);            /// 从缓冲区中读取数据, 等待时间以系统节拍为单位

void    C491_IrqTx          (void);
u8      C491_SendBuf        (u32 n, u16 buf[], u16 WaitTicks);
u8      C491_SendOne        (u16 data, u16 WaitTicks);  /// 发送一个数据, 等待时间以系统节拍为单位
void    C491_Send           (u32 n, u16 buf[]);





#endif
/// End of file -----------------------------------------------------------------------------------------
