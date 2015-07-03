
/// ----------------------------------------------------------------------------
/// File Name          : moto.c
/// Description        : 开环电机任务
///     剪线齿轮共14齿, m = 1, d = mz = 14mm, 周长 = 14pi =  约44mm
///     如果剪线电机与原来相反, 调整宏定义 MOTO_CUT_IN 和 MOTO_CUT_OUT
///     针高电机同上
/// ----------------------------------------------------------------------------
/// History:

/// y11/m01/d18     v0.05   chenyong    modify
/// 增加中断函数 MOTO_IRQandClr(), 并做相应改动

/// y10/m12/d22     v0.04   chenyong    modify
/// 1   包括 com.h, 因为调用其函数判断能否微动
/// 2   对剪线和针高电机方向, 针高位置值, 增加PC924条件编译
///     去掉全局变量 MOTO_HghRelativity[] 和 MOTO_HghPotVal[]
///     增加函数 MOTO_HghPotGetAve(), 因为PC924的电位器与PC9511相反
/// 3   增加函数 MOTO_CutGetIsActing() 和 MOTO_HghGetIsActing()
///     因为使用 MOTO_CutGetState() 或 MOTO_HghGetState() 会把错误状态清除, 导致不报错
/// 4   修改函数 MOTO_Start(), 使电机运动前脉冲电平为低
/// 5   修改函数 MOTO_HghStepUntil(), 使其提前降速, 慢速到位
/// 6   修改函数 MOTO_HghChange(), 不在多走16步
/// 7   修改函数 MOTO_Hgh(), 加入微动功能

/// y10/m11/d30     v0.02   chenyong    modify
/// 1   修改 MOTO_DU_TO_POT 的宏定义, 加入强制转换为 u32
/// 2   原来1°范围认为到位减速停车, 现在改为1.5°
/// 3   增加 MOTO_HghTestIsUp() 函数, 减少报错用
/// 4   增加#define MOTO_HGH_POS_UP 10

/// y10/m08/d03     v0.01   chenyong    setup





#ifndef  MOTO_H
#define  MOTO_H

/// EXTERNS --------------------------------------------------------------------
#ifdef   MOTO_MODULE
    #define  MOTO_EXT               /// 不定义为 "外部", 本模块内
#else
    #define  MOTO_EXT  extern       /// 定义为 "外部"
#endif


#include  <ucos_ii.h>               /// OS核的头文件
#include  <bsp.h>

//  #ifndef  CFG_H      /// 没有统一配置

/// USER DEFINED CONSTANTS ------------------------------------------------------------------------------

#define     MOTO_HGH_POS_NUM        11                      /// 分为11档针高: 0 ~ 9档 和 脱离档
#define     MOTO_HGH_POS_ERR        MOTO_HGH_POS_NUM        /// 针高位置异常 不在0~10档内
#define     MOTO_HGH_POS_UP         10                      /// 针高10表示机头提升


/// Exported types -------------------------------------------------------------

/// 开环电机的状态, 目前没有按位使用, 只是按枚举使用
///     7       6       5       4       3       2       1       0
///   sensor (blank)   pro  overtime (blank) reverse (blank) running
///
///     bit7        对于针高电机无效
///     bit2        对于剪线电机无效
///     bit6,3,1    保留reserve

/// 开环电机的状态, 优先级由高到低
typedef enum
{
    MOTO_ST_ACTING  =   0,          /// 动作进行中
                                    /// 以下为静止状态
    MOTO_ST_PRO,                    /// 电机故障导致动作停止
    MOTO_ST_OVTIME,                 /// 动作超时导致动作停止(一般为传感器坏)
    MOTO_ST_CUTLOOPER,              /// 剪刀碰到环梭, 环梭没有落下
    MOTO_ST_REVERSE,                /// 电机反转导致动作停止
//    MOTO_ST_POTERR,                 /// 电机转而其电位器没有变化
    MOTO_ST_OK,                     /// 静止无故障 或 动作完成
} MOTO_ST_TypeDef;

/// 剪线任务使用的命令标志类型
typedef enum
{	MOTO_CUT_POS0   =   0,          /// 针高0
    MOTO_CUT_POS1   ,          /// 针高1
    MOTO_CUT_POS2   ,
    MOTO_CUT_POS3   ,
    MOTO_CUT_POS4   ,
    MOTO_CUT_POS5   ,
    MOTO_CUT_POS6   ,
    MOTO_CUT_POS7   ,
    MOTO_CUT_POS8   ,
    MOTO_CUT_POS9   ,
	MOTO_CUT_D		,
    MOTO_CUT_H      ,          /// 针高10, 即脱离位置
    MOTO_CUT_BACK   ,          /// 收回
    MOTO_CUT_HALF   ,          /// 半出
    MOTO_CUT_FULL   ,          /// 完整剪线
	MOTO_CUT_O,
	MOTO_CUT_NONE   ,          /// 完整剪线
	

} MOTO_CUT_CMD_TypeDef;

/// 针高任务收到的任务类型
typedef enum
{
    MOTO_HGH_POS0   =   0,          /// 针高0
    MOTO_HGH_POS1   =   1,          /// 针高1
    MOTO_HGH_POS2   =   2,
    MOTO_HGH_POS3   =   3,
    MOTO_HGH_POS4   =   4,
    MOTO_HGH_POS5   =   5,
    MOTO_HGH_POS6   =   6,
    MOTO_HGH_POS7   =   7,
    MOTO_HGH_POS8   =   8,
    MOTO_HGH_POS9   =   9,
    MOTO_HGH_POS10  =  10,          /// 针高10, 即脱离位置
} MOTO_HGH_CMD_TypeDef;
typedef enum
{
    HOT_POS_DOWN_OVERR  =   0,          /// 动作进行中
                                    /// 以下为静止状态
    HOT_POS_DOWN,                    /// 电机故障导致动作停止
    HOT_POS_MID,                 /// 动作超时导致动作停止(一般为传感器坏)
    HOT_POS_UP,              /// 剪刀碰到环梭, 环梭没有落下
    HOT_POS_UP_OVERR,                /// 电机反转导致动作停止

} HOT_POS_TypeDef;
///// 任务发送的回复信息类型
//typedef enum
//{
//    MOTO_RES_FINISH =   0,      /// 完成
//    MOTO_RES_TIMEOUT,           /// 超时
//} MOTO_RES_TypeDef;

/// 开环电机的使用情况
typedef enum
{
    MOTO_USE_HGH   =   0,       /// 用于针高
    MOTO_USE_CUT,               /// 用于剪线
} MOTO_USE_TypeDef;

///// 电机步进脉冲控制结构
//typedef  struct
//{
//    u32         step;
//    u16         (*GetSpdFun)(u8*, s8);
//    u8          gear;
//    s8          acc;
//    BitAction   clk;
//} MOTO_STEP_CTRL_TypeDef;


/// Global variables -----------------------------------------------------------
//MOTO_EXT    u8          MOTO_StepDeltaOdd;
//MOTO_EXT    u8          MOTO_StepDeltaEven;
//MOTO_EXT    u32         MOTO_ClkNumOdd;
//MOTO_EXT    u32         MOTO_ClkNumEven;
////MOTO_EXT    u32         MOTO_StepSumOdd;
////MOTO_EXT    u32         MOTO_StepSumEven;
//MOTO_EXT    u16         MOTO_TimingOdd;
//MOTO_EXT    u16         MOTO_TimingEven;

/// for 中断
//MOTO_EXT    BitAction                   MOTO_StepState[BSP_DEVICE_ID_N];
//MOTO_EXT    u16                         MOTO_Timing[BSP_DEVICE_ID_N];

/// for 其他任务
//MOTO_EXT    MOTO_CUT_CMD_TypeDef    MOTO_CutCmd[BSP_DEVICE_ID_N];
//MOTO_EXT    MOTO_HGH_CMD_TypeDef    MOTO_HghCmd[BSP_DEVICE_ID_N];


//MOTO_EXT    OS_EVENT*                   MOTO_SemIrqPtr[BSP_DEVICE_ID_N];
////MOTO_EXT    OS_EVENT*                   MOTO_SemCmdPtr[BSP_DEVICE_ID_N];      /// cy 100909

//MOTO_EXT    const       uint16_t        MOTO_FlagCC[BSP_DEVICE_ID_N];
//MOTO_EXT    volatile    uint16_t* const MOTO_CCR[BSP_DEVICE_ID_N];

/// FUNCTION PROTOTYPES ---------------------------------------------------------------------------------

void                MOTO_Init               (MOTO_USE_TypeDef odd, MOTO_USE_TypeDef even);
//void                MOTO_CutSetSpdEngine    (u8             engine);
void                MOTO_SetSpdEngine       (MOTO_USE_TypeDef use, u8 engine);

void                MOTO_CutSetFullAngle    (u8             n10du);
void                MOTO_CutSetHalfAngle    (u8             n10du);
MOTO_ST_TypeDef     MOTO_CutGetState        (BSP_ID_TypeDef id);
bool                MOTO_CutGetIsActing     (BSP_ID_TypeDef id);
void                MOTO_CutSetCmd          (BSP_ID_TypeDef id, MOTO_CUT_CMD_TypeDef cmd);

u8                  MOTO_HghGetHeight       (BSP_ID_TypeDef id);
bool                MOTO_HghTestIsUp        (BSP_ID_TypeDef id, u8 time);
MOTO_ST_TypeDef     MOTO_HghGetState        (BSP_ID_TypeDef id);
bool                MOTO_HghGetIsActing     (BSP_ID_TypeDef id);
void                MOTO_HghSetCmd          (BSP_ID_TypeDef id, MOTO_HGH_CMD_TypeDef cmd);

void                MOTO_IRQandClr          (void);
void                MOTO_TKSetFullAngle    (u8             n10du);
void                MOTO_TKSetHalfAngle    (u8             n10du);
HOT_POS_TypeDef     HOT_POS_Get();


#endif
