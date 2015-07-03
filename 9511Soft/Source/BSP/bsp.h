
/// ----------------------------------------------------------------------------
/// File Name          : bsp.h
/// Description        : 板级支持包的头文件
/// ----------------------------------------------------------------------------
/// History:

/// y11/m01/d18     v0.07   chenyong    modify
/// 整理对 PC924 的判断 或 条件编译
/// 在BSP_NVIC_Cfg()中, 限制严格, 避免中断误入
/// 对于一般函数, 限制宽松, 只对 MOTC(闭环电机) 条件编译, 其他不都编译

/// y10/m12/d24     v0.07   chenyong    modify
/// 增加函数 BSP_MOTO_ClkGet() 和 BSP_MOTC_ClkGet()

/// y10/m12/d23     v0.06   chenyong    modify
/// 整理对 PC924 的判断 或 条件编译

/// y10/m12/d03     v0.05   chenyong    modify
/// 更改编码器相关内容

/// y10/m11/d30     v0.04   chenyong    modify
/// 更改函数 BSP_POT_GetOne(), 原来固定取单元0的值, 改为依次取值

/// y10/m09/d10     v0.03   chenyong    modify
/// 更改为PC924条件编译只在BSP_Judge924()函数内有效
/// 其他情况通过全局变量判断板类型
/// 整理

/// y10/m07/d12     v0.02   chenyong    modify
/// 使用io模块. io 与 MPU 密切相关; bsp 与功能密切相关

/// y10/m07/d05     v0.01   chenyong    setup





/// Define to prevent recursive inclusion --------------------------------------
#ifndef BSP_H
#define BSP_H


/// EXTERNS --------------------------------------------------------------------
#ifdef   BSP_MODULE
    #define  BSP_EXT                /// 不定义为 "外部", 本模块内
#else
    #define  BSP_EXT  extern        /// 定义为 "外部"
#endif


/// Includes -------------------------------------------------------------------
#include  <stm32f10x.h>



/// Exported macro -------------------------------------------------------------

#define     BSP_LED_ID_N                2
#define     BSP_DEVICE_ID_N             2

#define     BSP_DIP_N_BITS              10

#define     BSP_ENC_N_A_PR              400                     /// 编码器A或B相脉冲每圈个数
#define     BSP_ENC_N_T_PR              (BSP_ENC_N_A_PR * 4)    /// 编码器每圈输入定时器脉冲个数
#define     BSP_ENC_N_T_PHR             (BSP_ENC_N_T_PR / 2)    /// 编码器每半圈输入定时器脉冲个数

/// ----     491    ----
#define     BSP_USART_491               USART1
#define     BSP_USART_491_IRQHandler    USART1_IRQHandler

/// ----   8MTimer  ----
    #define     BSP_TMR_8M              TIM1                        /// 定时器, 主要用于125nS为单位的定时
    #define     BSP_TMR_8M_CLK          8000000                     /// 定时器时钟为8MHz
    #define     BSP_TMR_8M_IRQHandler   TIM1_CC_IRQHandler
    
    #define     BSP_CC_MC_CLOSE         1                           /// 闭环电机的闭环控制定时
    #define     BSP_CC_MC_OPEN          2                           /// 闭环电机的开环控制定时
    #define     BSP_CC_CLK_ODD          3                           /// 使用cc3, 用于奇数定时脉冲
    #define     BSP_CC_CLK_EVEN         4                           /// 使用cc4, 用于偶数定时脉冲
    
//#if     (BSP_CC_CLK_ODD == BSP_CC_CLK_EVEN)
//    #error  "BSP_CC_CLK_ODD and BSP_CC_CLK_EVEN use the same cc !"
//#endif

#if     BSP_CC_MC_CLOSE == 1
    #define     BSP_FLAG_CC_MC_CLOSE    TIM_FLAG_CC1
    #define     BSP_CCR_MC_CLOSE        (BSP_TMR_8M->CCR1)
#elif   BSP_CC_MC_CLOSE == 2
    #define     BSP_FLAG_CC_MC_CLOSE    TIM_FLAG_CC2
    #define     BSP_CCR_MC_CLOSE        (BSP_TMR_8M->CCR2)
#elif   BSP_CC_MC_CLOSE == 3
    #define     BSP_FLAG_CC_MC_CLOSE    TIM_FLAG_CC3
    #define     BSP_CCR_MC_CLOSE        (BSP_TMR_8M->CCR3)
#elif   BSP_CC_MC_CLOSE == 4
    #define     BSP_FLAG_CC_MC_CLOSE    TIM_FLAG_CC4
    #define     BSP_CCR_MC_CLOSE        (BSP_TMR_8M->CCR4)
#endif

#if     BSP_CC_MC_OPEN == 1
    #define     BSP_FLAG_CC_MC_OPEN     TIM_FLAG_CC1
    #define     BSP_CCR_MC_OPEN         (BSP_TMR_8M->CCR1)
#elif   BSP_CC_MC_OPEN == 2
    #define     BSP_FLAG_CC_MC_OPEN     TIM_FLAG_CC2
    #define     BSP_CCR_MC_OPEN         (BSP_TMR_8M->CCR2)
#elif   BSP_CC_MC_OPEN == 3
    #define     BSP_FLAG_CC_MC_OPEN     TIM_FLAG_CC3
    #define     BSP_CCR_MC_OPEN         (BSP_TMR_8M->CCR3)
#elif   BSP_CC_MC_OPEN == 4
    #define     BSP_FLAG_CC_MC_OPEN     TIM_FLAG_CC4
    #define     BSP_CCR_MC_OPEN         (BSP_TMR_8M->CCR4)
#endif

#if     BSP_CC_CLK_ODD == 1
    #define     BSP_FLAG_CC_CLK_ODD     TIM_FLAG_CC1
    #define     BSP_CCR_CLK_ODD         (BSP_TMR_8M->CCR1)
#elif   BSP_CC_CLK_ODD == 2
    #define     BSP_FLAG_CC_CLK_ODD     TIM_FLAG_CC2
    #define     BSP_CCR_CLK_ODD         (BSP_TMR_8M->CCR2)
#elif   BSP_CC_CLK_ODD == 3
    #define     BSP_FLAG_CC_CLK_ODD     TIM_FLAG_CC3
    #define     BSP_CCR_CLK_ODD         (BSP_TMR_8M->CCR3)
#elif   BSP_CC_CLK_ODD == 4
    #define     BSP_FLAG_CC_CLK_ODD     TIM_FLAG_CC4
    #define     BSP_CCR_CLK_ODD         (BSP_TMR_8M->CCR4)
#endif

#if     BSP_CC_CLK_EVEN == 1
    #define     BSP_FLAG_CC_CLK_EVEN    TIM_FLAG_CC1
    #define     BSP_CCR_CLK_EVEN        (BSP_TMR_8M->CCR1)
#elif   BSP_CC_CLK_EVEN == 2
    #define     BSP_FLAG_CC_CLK_EVEN    TIM_FLAG_CC2
    #define     BSP_CCR_CLK_EVEN        (BSP_TMR_8M->CCR2)
#elif   BSP_CC_CLK_EVEN == 3
    #define     BSP_FLAG_CC_CLK_EVEN    TIM_FLAG_CC3
    #define     BSP_CCR_CLK_EVEN        (BSP_TMR_8M->CCR3)
#elif   BSP_CC_CLK_EVEN == 4
    #define     BSP_FLAG_CC_CLK_EVEN    TIM_FLAG_CC4
    #define     BSP_CCR_CLK_EVEN        (BSP_TMR_8M->CCR4)
#endif

///// 闭环电机控制方案暂定为, 只使用一个中断
///// 使用Timer3的溢出中断, 暂时不使用比较中断, 减少中断内的操作。
///// ----   MC_Timer  ----
//    #define     BSP_TMR_MC              TIM3                        /// 定时器, 主要用于闭环电机控制
//    #define     BSP_TMR_MC_T            10                          /// 闭环控制周期(uS), 主要取决于脉宽
//    #define     BSP_TMR_MC_F            (1000000 / BSP_TMR_MC_T)    /// 闭环控制频率
////    #define     BSP_TMR_MC_CLK          1000000                     /// 定时器时钟为1MHz
//    #define     BSP_TMR_MC_IRQHandler   TIM3_IRQHandler
//    
////    #define     BSP_FLAG_CC_MOTC        TIM_FLAG_CC1
////    #define     BSP_CCR_MOTC            (BSP_TMR_MC->CCR1)
//    #define     BSP_FLAG_CC_MOTC        TIM_FLAG_Update

///// 闭环电机的命令控制使用的中断, 使用Timer3的比较中断1
///// ----   MCC_Timer  ----
//    #define     BSP_TMR_MCC             TIM3                        /// 定时器3, 主要用于闭环电机的命令控制
//    #define     BSP_TMR_MC_IRQHandler   TIM3_IRQHandler
//    
//    #define     BSP_FLAG_CC_MCC         TIM_FLAG_CC1
//    #define     BSP_CCR_MCC             (BSP_TMR_MCC->CCR1)

/// ----   ENC_Timer  ----
    #define     BSP_TMR_E_ODD           TIM2                        /// Encoder_ODD
    #define     BSP_TMR_E_EVEN          TIM4                        /// Encoder_EVEN
    
    #define     BSP_TMR_E_ODD_IRQHandler    TIM2_IRQHandler         /// Encoder_ODD  irq
    #define     BSP_TMR_E_EVEN_IRQHandler   TIM4_IRQHandler         /// Encoder_EVEN irq
    
/// ----   Encoder Index  ----
    #define     BSP_EXIT_LINE_I_E_ODD   EXTI_Line2                  /// Index_Encoder_ODD
    #define     BSP_EXIT_LINE_I_E_EVEN  EXTI_Line8                  /// Index_Encoder_EVEN
    
    #define     BSP_I_E_ODD_IRQHandler  EXTI2_IRQHandler            /// Index_Encoder_ODD  irq
    #define     BSP_I_E_EVEN_IRQHandler EXTI9_5_IRQHandler          /// Index_Encoder_EVEN irq

/// 模拟计数的定时器, 使用Timer3
/// ----   CNT_Timer  ----
    #define     BSP_TMR_CNT             TIM3
    #define     BSP_TMR_CNT_IRQHandler  TIM3_IRQHandler
    
    #define     BSP_CC_MCC_DEC          1                           /// cc1用于闭环电机的位置命令减少
    #define     BSP_CC_MCC_INC          2                           /// cc2用于闭环电机的位置命令增加
    
#if     (BSP_CC_MCC_DEC == BSP_CC_MCC_INC)
    #error  "BSP_CC_MCC_DEC and BSP_CC_MCC_INC use the same cc !"
#endif

#if     BSP_CC_MCC_DEC == 1
    #define     BSP_FLAG_CC_MCC_DEC     TIM_FLAG_CC1
    #define     BSP_CCR_MCC_DEC         (BSP_TMR_CNT->CCR1)
#elif   BSP_CC_MCC_DEC == 2
    #define     BSP_FLAG_CC_MCC_DEC     TIM_FLAG_CC2
    #define     BSP_CCR_MCC_DEC         (BSP_TMR_CNT->CCR2)
#elif   BSP_CC_MCC_DEC == 3
    #define     BSP_FLAG_CC_MCC_DEC     TIM_FLAG_CC3
    #define     BSP_CCR_MCC_DEC         (BSP_TMR_CNT->CCR3)
#elif   BSP_CC_MCC_DEC == 4
    #define     BSP_FLAG_CC_MCC_DEC     TIM_FLAG_CC4
    #define     BSP_CCR_MCC_DEC         (BSP_TMR_CNT->CCR4)
#endif

#if     BSP_CC_MCC_INC == 1
    #define     BSP_FLAG_CC_MCC_INC     TIM_FLAG_CC1
    #define     BSP_CCR_MCC_INC         (BSP_TMR_CNT->CCR1)
#elif   BSP_CC_MCC_INC == 2
    #define     BSP_FLAG_CC_MCC_INC     TIM_FLAG_CC2
    #define     BSP_CCR_MCC_INC         (BSP_TMR_CNT->CCR2)
#elif   BSP_CC_MCC_INC == 3
    #define     BSP_FLAG_CC_MCC_INC     TIM_FLAG_CC3
    #define     BSP_CCR_MCC_INC         (BSP_TMR_CNT->CCR3)
#elif   BSP_CC_MCC_INC == 4
    #define     BSP_FLAG_CC_MCC_INC     TIM_FLAG_CC4
    #define     BSP_CCR_MCC_INC         (BSP_TMR_CNT->CCR4)
#endif

    #define     BSP_COUNTER_PR          8000                        /// 主轴一圈模拟计数为8000
    
    #define     BSP_SPD_D10_MAX         100                         /// 最大速度 (单位10rpm)
    #define     BSP_SPD_D10_MIN         8                           /// 最小速度, 保证分频数为u16,(单位10rpm)
                                                                    /// 速度数量
    #define     BSP_SPD_NUM             (BSP_SPD_D10_MAX - BSP_SPD_D10_MIN + 1)

   	#define CONFIRM_MAINCONTRAL_ADDR1   0x0801f800
	#define CONFIRM_MAINCONTRAL_ADDR2   0x0801f400
	#define CONFIRM_9511_ID_ADDR       0x0801fc00
	#define CONFIRM_STM32_ID_ADDR	   0x1FFFF7E8
	#define CONFIRM_FASH_ID_ADDR	   0x1FFFF7E0//

/// Exported types -------------------------------------------------------------

/// 奇偶设备id
typedef enum
{
    BSP_ID_ODD          =   0,
    BSP_ID_EVEN         =   1,
} BSP_ID_TypeDef;

/// LED状态(包括板上和机头开关板上的灯, 如果两者电平不同需要分开定义)
typedef enum
{
    BSP_LED_OFF         =   1,
    BSP_LED_ON          =   0,
} BSP_LED_TypeDef;

/// 机头开关状态
typedef enum
{
    BSP_HD_KEY_OFF      =   0,
    BSP_HD_KEY_ON       =   1,
} BSP_HD_KEY_TypeDef;

/// DIP决定的板的工作类型   0 - 上机头板, 1 - 下机头板
typedef enum {
    BSP_DIP_BD_FUNC_UP  =   0,
    BSP_DIP_BD_FUNC_DN  =   1,
} BSP_DIP_BD_FUNC_TypeDef;

/// 电机状态    错误 - 0, 正常 - 1
typedef enum
{
    BSP_MOT_ST_ERROR    =   0,
    BSP_MOT_ST_NORMAL   =   1,
} BSP_MOT_ST_TypeDef;

/// 电机方向    cw - 0, ccw - 1
typedef enum
{
    BSP_MOT_DIR_CW      =   0,
    BSP_MOT_DIR_CCW     =   1,
} BSP_MOT_DIR_TypeDef;

/// 电机电流    
typedef enum
{
    BSP_MOT_TQ_100      =   0,
    BSP_MOT_TQ_75       =   1,
    BSP_MOT_TQ_50       =   2,
    BSP_MOT_TQ_20       =   3,
} BSP_MOT_TQ_TypeDef;
typedef enum
{
    BSP_MOT_M_0      =   0,
    BSP_MOT_M_1       =   1,
    BSP_MOT_M_2       =   2,
    BSP_MOT_M_3       =   3,
	BSP_MOT_M_4      =    4,
    BSP_MOT_M_5       =   5,
    BSP_MOT_M_6       =   6,
    BSP_MOT_M_7       =   7,
} BSP_MOT_M123_TypeDef;
/// 闭环电机步距    0 - 0.45°, 1 - 0.225°
typedef enum
{
    BSP_MOT_M_P45       =   0,
    BSP_MOT_M_P225      =   1,
} BSP_MOT_M_TypeDef;

/// 闭环电机初始化状态      0 - 初始化状态, 1 - 通常状态
typedef enum
{
    BSP_MOT_IST_INITIAL =   0,
    BSP_MOT_IST_NORMAL  =   1,
} BSP_MOT_IST_TypeDef;

/// 传感器开关状态(没有定义On或Off, 因为对于pnp或npn是不同的)
typedef enum
{
    BSP_SWITCH_HIGH     =   0,
    BSP_SWITCH_LOW      =   1,
} BSP_SWITCH_TypeDef;

/// 491_RxBuf状态
typedef enum
{
    BSP_491_RXBUF_NORMAL    =   0,
    BSP_491_RXBUF_FULL,
    BSP_491_RXBUF_EMPTY,
} BSP_491_RxBuf_TypeDef;

/// 491_Tx状态
typedef enum
{
    BSP_491_TX_NORMAL   =   0,
    BSP_491_TX_FULL,
} BSP_491_Tx_TypeDef;

/// 给金片板的刺绣状态    
typedef enum
{
    BSP_EMB_ST_OFF      =   0,
    BSP_EMB_ST_NORMAL   =   1,
    BSP_EMB_ST_PATCH    =   2,
} BSP_EMB_ST_TypeDef;

/// 闭环电机步距    0 - 0.45°, 1 - 0.225°

/// SOL状态
typedef enum
{
    BSP_SOL_OFF         =   0,
    BSP_SOL_ON          =   1,
} BSP_SOL_TypeDef;

/// Global variables -----------------------------------------------------------

BSP_EXT     bool                        BSP_Is924;          /// 是否924

BSP_EXT     BSP_MOT_TQ_TypeDef          BSP_DIP_TQ;         /// 设定的电流
BSP_EXT     BSP_DIP_BD_FUNC_TypeDef     BSP_DIP_BDFunc;     /// 板功能
BSP_EXT     u8                          BSP_DIP_9511Addr;   /// 9511板地址
BSP_EXT     u8                          BSP_DIP_924Addr;    ///  924板地址
BSP_EXT		bool    SECURITY_9511_STAT ; //不加密	  TRUE加密
BSP_EXT		bool	MAIN_SECURITY_STAT ; //不加密	  TRUE加密
BSP_EXT     u8 STM32_ID[12];
BSP_EXT     u8 PRO_9511_ID[4];
BSP_EXT     u8 MAIN_Sec_ID[4];

/// Exported functions ---------------------------------------------------------

void                    BSP_Init            (void);

u16                     BSP_TM8M_Get        (void);
void                    BSP_TM8M_Delay      (u16 time);

void                    BSP_TM_CNT_SpdSet   (u16 spd_d10);
u16                     BSP_TM_CNT_Get      (void);

/// -------- HMI --------
/// -------- led --------
void                    BSP_LED_Set         (u8 led, BSP_LED_TypeDef st);
void                    BSP_LED_Toggle      (u8 led);
/// -------- Head Board --------
void                    BSP_HD_KeyGet       (BSP_ID_TypeDef id, BSP_HD_KEY_TypeDef *up,
                                                                BSP_HD_KEY_TypeDef *down);
void                    BSP_HD_LedSet       (BSP_ID_TypeDef id, BSP_LED_TypeDef red,
                                                                BSP_LED_TypeDef green);
/// -------- dip --------
u32                     BSP_DIP_Get         (void);

/// -------- EXECUTOR --------
/// -------- solenoid --------
/// 暂无
/// -------- motor (controled by open loop)  --------
BSP_MOT_ST_TypeDef      BSP_MOTO_StatusGet  (BSP_ID_TypeDef id);
BitAction               BSP_MOTO_ClkGet     (BSP_ID_TypeDef id);
void                    BSP_MOTO_ClkSet     (BSP_ID_TypeDef id, BitAction bit);
void                    BSP_MOTO_DirSet     (BSP_ID_TypeDef id, BSP_MOT_DIR_TypeDef dir);
void                    BSP_MOTO_TQSet      (BSP_ID_TypeDef id, BSP_MOT_TQ_TypeDef tq);
void  					BSP_MOTO_MSet (BSP_ID_TypeDef id, BSP_MOT_M123_TypeDef tq);
#ifdef  PC924
/// -------- motor (controled by closed loop) --------
BSP_MOT_ST_TypeDef      BSP_MOTC_StatusGet  (BSP_ID_TypeDef id);
BitAction               BSP_MOTC_ClkGet     (BSP_ID_TypeDef id);
void                    BSP_MOTC_ClkSet     (BSP_ID_TypeDef id, BitAction bit);
BSP_MOT_DIR_TypeDef     BSP_MOTC_DirGet     (BSP_ID_TypeDef id);
void                    BSP_MOTC_DirSet     (BSP_ID_TypeDef id, BSP_MOT_DIR_TypeDef dir);
void                    BSP_MOTC_TQSet      (BSP_ID_TypeDef id, BSP_MOT_TQ_TypeDef tq);
void                    BSP_MOTC_MSet       (BSP_ID_TypeDef id, BSP_MOT_M_TypeDef m);
void                    BSP_MOTC_STSet      (BSP_ID_TypeDef id, BSP_MOT_IST_TypeDef ist);
#endif

/// -------- SENSOR --------
/// -------- encoder --------
void                    BSP_ENC_CntSet      (BSP_ID_TypeDef id, u16 cnt);
u16                     BSP_ENC_CntGet      (BSP_ID_TypeDef id);
/// -------- 24V detect --------
/// 暂无
/// -------- approach switch --------
BSP_SWITCH_TypeDef      BSP_SWI_FiGet       (BSP_ID_TypeDef id);
BSP_SWITCH_TypeDef      BSP_SWI_SeGet       (BSP_ID_TypeDef id);
/// -------- potentiometer --------
u16                     BSP_POT_GetOne      (BSP_ID_TypeDef id);
u16                     BSP_POT_GetAve      (BSP_ID_TypeDef id);

/// -------- COMMUNICATION --------
/// -------- 491 --------
void                    BSP_491_Init        (void);
void                    BSP_491_DeSet       (FunctionalState NewState);
/// -------- sequin --------
void                    BSP_SQN_EmbStSend   (BSP_ID_TypeDef id, BSP_EMB_ST_TypeDef st);
void  					Get_Stm32_ID(void);
void  					Get_9511_ID(void);
void                    BSP_SOL_Set         (BSP_ID_TypeDef id, BSP_SOL_TypeDef st, u16 tmr);
void                    BSP_SOL_TmrProc     (void);
BSP_EXT     bool                        BSP_DIP_SpdIsSlow;  /// 机头电机速度是否慢速 (力矩大)
BSP_EXT     bool                        BSP_DIP_RedIsSpkl;  /// 断线红灯是否闪烁

#endif
/// End of file -----------------------------------------------------------------------------------------
