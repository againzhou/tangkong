
/// ----------------------------------------------------------------------------
/// File Name          : mc_open.c
/// Description        : 闭环电机的开环控制部分
///
///     D   --  表示 D轴, 即针背方向, 单位为1/400圈
///             (毛巾绣时指向线迹终点方向, 链式绣时指向线迹起点方向)
///     DM  --  表示 D轴电机轴, 单位为编码器精度, 即1/1600圈
///     H   --  表示 H轴, 即环梭孔方向, 单位为1/400圈
///             (找原点或穿线点时, H轴与D轴重合; 刺绣时, H轴相对D轴偏转-1/8r)
///     HM  --  表示 H轴电机轴, 单位为编码器精度, 即1/1600圈
///             (a针位HM与H轴原点重合, b~f针位不重合)
///     M   --  表示被控电机轴, 根据上下机头板确定 D或 H, 用于目标控制.
///
///     R   --  表示圈数, 单位为"圈"
///     V   --  表示D 或H 轴方向, 单位为"1/400圈", 通讯命令即按此单.
///     E   --  表示DM或HM轴方向, 单位为编码器分辨率, 即"1/1600圈".
///
///     闭环电机有两种控制方式: 开环方式 和 闭环方式
///     对于D轴, 有以下动作:  
///         1 找原点        (开环)  即找机械原点, 找到后M和T轴变量清零      
///         2 转到某方向    (闭环)  闭环控制, 一般绣作时使用
///         3 找穿线点      (混合)  由1和2组合完成, 两个动作方向最好相反
///     对于H轴, 相对复杂, 所有环轴联动的动作需要闭环控制, 有以下动作:
///         1 找原点        (混合)  找机械原点后, 然后找针位原点. 设置为非偏移状态
///         2 找穿线点      (混合)  找机械原点后, 然后找针位穿线点. 设置为非偏移状态
///         3 偏移动作      (闭环)  负方向转45°. 设置为偏移状态 (偏移状态才能刺绣)
///         4 打环动作      (闭环)  H1 和 H2, 更新T轴变量, 并根据T轴变量计算M轴变量
///         5 晃环动作      (开环)  正负半个齿左右晃动
///         6 换色动作      (开环)  只是更新当前针位 和 M轴变量, T轴变量不更新
/// ----------------------------------------------------------------------------

/// History:

/// y10/m01/d17     v0.02   chenyong    modify
/// 从模块mcc分离出此模块

/// y10/m11/d08     v0.01   chenyong    setup



#ifdef  PC924

#define  MCO_MODULE        /// 模块定义, 用区分内外部定义


/// Includes --------------------------------------------------------------------------------------------
#include  <ucos_ii.h>               /// OS核的头文件
#include  "mc_base.h"


/// !!!
extern        s32     MCL_MNowR;      /// 被控电机当前圈数
extern        u16     MCL_MNowE;      /// 被控电机当前方向 (编码器)

/// Private define --------------------------------------------------------------------------------------

#define     MCO_TQ_RUN          BSP_MOT_TQ_50
#define     MCO_TQ_STOP         BSP_MOT_TQ_20

#define     MCO_TAB_N           32

#define     MCO_E_HALF_ELE      (BSP_ENC_N_T_PR / MCB_POLE_PAIRS / 2)       /// 电角度的一半, 编码器单位

/// Private typedef -------------------------------------------------------------------------------------
/// Private macro ---------------------------------------------------------------------------------------
/// Private function prototypes -------------------------------------------------------------------------
/// Private variables -----------------------------------------------------------------------------------

static  OS_EVENT*   MCO_StopSemPtr;     /// 开环控制结束信号量指针

static  bool    MCO_IsToOrg;            /// 开环模式中, 找原点方式, 否则为位置方式

static  BitAction   MCO_ClkSta;         /// 记录脉冲电平的高低状态

static  u16     MCO_ClkNum;             /// 找原点超时数, 或者位置步数

/// 开环速度定时表数据索引
static  u8      MCO_SpdIMax =   0;      /// 对应最大开环速度
static  u8      MCO_SpdIMin =   0;      /// 对应最小开环速度
static  u16     MCO_SpdINow =   0;      /// 对应当前开环速度

static  u8      MCO_AccUp   =   1;      /// 开环升速加速度
static  u8      MCO_AccDn   =   1;      /// 开环减速加速度


/// Private consts --------------------------------------------------------------------------------------

static  const   u16     MCO_ClkTimD2[MCO_TAB_N]    =   {
    32000,  28800,  25920,  23328,  20995,  18896,  17006,  15306,
    13775,  12397,  11158,  10042,   9038,   8134,   7321,   6589,
     5930,   5337,   4803,   4323,   3890,   3501,   3151,   2836,
     2553,   2297,   2068,   1861,   1675,   1507,   1357,   1221
};


/// Private functions ---------------------------------------------------------





void  MCO_Init (void)
{
    MCO_StopSemPtr  =   OSSemCreate(0);
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  开环控制方式的定时中断服务函数
///         分两种情况: 一是运动到位后退出, 二是找到原点后退出
/// @param  None
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
void  MCO_IRQ (void)
{
    BitAction       st_odd;
    BitAction       st_even;
    u32             ceiling;
    
    
    BSP_CCR_MC_OPEN +=  MCO_ClkTimD2[MCO_SpdINow];                  /// 设置下个中断时刻
    
    if (MCO_ClkSta == Bit_RESET) {                                  /// 如果是低电平上升沿时刻
        MCO_ClkSta  =   Bit_SET;                                    /// 设置为高电平状态
        
        if (MCO_ClkNum != 0) {                                      /// 如果没有超时或到位
            MCO_ClkNum--;
            if (MCO_IsToOrg != FALSE) {                             /// 找原点模式
                if (MCB_OrgIs0[BSP_ID_ODD] == FALSE) {              /// 如果奇数头没有找到原点
                    st_odd  =   Bit_SET;                            /// 设为高电平
                } else {                                            /// 如果奇数头已经找到原点
                    st_odd  =   Bit_RESET;                          /// 设为低电平
                }
                if (MCB_OrgIs0[BSP_ID_EVEN] == FALSE) {             /// 如果偶数头没有找到原点
                    st_even =   Bit_SET;                            /// 设为高电平
                } else {                                            /// 如果偶数头已经找到原点
                    st_even =   Bit_RESET;                          /// 设为低电平
                }
            } else {                                                /// 开环位置控制模式
                st_odd  =   Bit_SET;
                st_even =   Bit_SET;
            }
        } else {                                                    /// 如果已经到达目标位置或超时
            st_odd  =   Bit_RESET;
            st_even =   Bit_RESET;
        }
        
        BSP_MOTC_ClkSet(BSP_ID_ODD , st_odd);                       /// 设置脉冲电平
        BSP_MOTC_ClkSet(BSP_ID_EVEN, st_even);
        
        if ((st_odd == Bit_RESET) && (st_even == Bit_RESET)) {      /// 无需发脉冲
            TIM_ITConfig(BSP_TMR_8M, BSP_FLAG_CC_MC_OPEN, DISABLE); /// 关闭中断
            OSSemPost(MCO_StopSemPtr);                              /// 发送信号量
        } else {
            ;
        }
        
    } else {                                                        /// 如果是高电平下降沿时刻
        MCO_ClkSta  =   Bit_RESET;                                  /// 设置为低电平阶段
        
        BSP_MOTC_ClkSet(BSP_ID_ODD , Bit_RESET);                    /// 设置脉冲电平为低
        BSP_MOTC_ClkSet(BSP_ID_EVEN, Bit_RESET);
        
        ceiling     =   MCO_ClkNum * MCO_AccDn + MCO_SpdIMin;       /// 计算到位前减速时的限速
        if (ceiling > MCO_SpdIMax) {                                /// 最高限速限制
            ceiling =   MCO_SpdIMax;
        }
        
        MCO_SpdINow   +=   MCO_AccUp;                               /// 启动加速时速度
        if (MCO_SpdINow > ceiling) {                                /// 限速
            MCO_SpdINow    =   ceiling;
        }
    }
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  电机开环运动, 只是相对运动, 并且恒速
///         一般用于换色, 晃环, 找原点时的偏移运动
/// @param  dir     :   运动的方向
///         enc     :   运动的路程, 单位是编码器线数
///         spd_min :   最小运动速度, 0 ~ MCO_TAB_N - 1, 保证 spd_min <= spd_max
///         spd_max :   最大运动速度, 0 ~ MCO_TAB_N - 1, 保证 spd_min <= spd_max
/// @retval :   None
/// -----------------------------------------------------------------------------------------------------
void  MCO_Mov (BSP_MOT_DIR_TypeDef dir, u16 enc, u8 spd_min, u8 spd_max)
{
    u8      err;
    
    /// !!!         参数范围
    
    
    MCO_IsToOrg     =   FALSE;                                  /// 设置为开环模式, 非找原点模式
    
    MCO_SpdIMin     =   spd_min;                                /// 设置最小速度
    MCO_SpdIMax     =   spd_max;                                /// 设置最大速度
    MCO_SpdINow     =   spd_min;                                /// 设置当前速度
    
    BSP_MOTC_TQSet(BSP_ID_ODD , MCO_TQ_RUN);                    /// 设置开环电流
    BSP_MOTC_TQSet(BSP_ID_EVEN, MCO_TQ_RUN);
    
    BSP_MOTC_ClkSet(BSP_ID_ODD , Bit_RESET);                    /// 设置脉冲信号电平为低
    BSP_MOTC_ClkSet(BSP_ID_EVEN, Bit_RESET);
    MCO_ClkSta      =   Bit_RESET;                              /// 记录脉冲电平为低
    
    BSP_MOTC_DirSet(BSP_ID_ODD , dir);                          /// 设置方向
    BSP_MOTC_DirSet(BSP_ID_EVEN, dir);
    
    MCO_ClkNum      =   enc * MCB_P_DIV_E;                      /// 设置运动的脉冲数
    
    BSP_CCR_MC_OPEN =   BSP_TM8M_Get() + spd_min;               /// 设置启动时机
    
    TIM_ClearITPendingBit(BSP_TMR_8M, BSP_FLAG_CC_MC_OPEN);     /// 清中断
    TIM_ITConfig(BSP_TMR_8M, BSP_FLAG_CC_MC_OPEN, ENABLE);      /// 打开中断
    
    OSSemPend(MCO_StopSemPtr, 0, &err);                         /// 等待运动结束
    
    BSP_MOTC_TQSet(BSP_ID_ODD , BSP_MOT_TQ_20);                 /// 设置为小电流
    BSP_MOTC_TQSet(BSP_ID_EVEN, BSP_MOT_TQ_20);
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  电机找原点, 即编码器原点
///         0位, 计数器0, 脉冲计数(电角度)三者对齐
/// @param  :   None
/// @retval :   TRUE    -   成功
///             FALSE   -   失败, 超时
/// -----------------------------------------------------------------------------------------------------
bool  MCO_Org (void)
{
    BSP_MOT_DIR_TypeDef  dir;
    u8      i;
    u8      j;
    u16     enc;
    u8      err;
    
    
//    MCH_CloseExit();
    
    /// 至少一个电机找过原点, 位置可知, 即不是首次找原点
    if ((MCB_OrgIs0[BSP_ID_ODD] != FALSE) || (MCB_OrgIs0[BSP_ID_EVEN] != FALSE)) {
                                                                /// 当前位置在原点附近
        if ((MCL_MNowE <= MCO_E_HALF_ELE) || (MCL_MNowE >= BSP_ENC_N_T_PR - MCO_E_HALF_ELE)) {
            MCO_Mov(MCB_DIR_INC, MCO_E_HALF_ELE * 2, 13, 29);
            OSTimeDlyHMSM(0, 0, 0, 100);
            dir =   MCB_DIR_DEC;
        } else if (MCL_MNowE < BSP_ENC_N_T_PHR) {
            dir =   MCB_DIR_DEC;
        } else if (MCL_MNowE > BSP_ENC_N_T_PHR) {
            dir =   MCB_DIR_INC;
        } else {
            if (MCL_MNowR < 0) {
                dir =   MCB_DIR_INC;
            } else {
                dir =   MCB_DIR_DEC;
            }
        }
    } else {                                                    /// 首次找原点按负方向运动
        dir =   MCB_DIR_DEC;
    }
    
    MCO_IsToOrg             =   TRUE;                           /// 设置需要找原点
    MCB_OrgIs0[BSP_ID_ODD]  =   FALSE;
    MCB_OrgIs0[BSP_ID_EVEN] =   FALSE;
    
    BSP_MOTC_TQSet(BSP_ID_ODD , MCO_TQ_RUN);                    /// 设置电流
    BSP_MOTC_TQSet(BSP_ID_EVEN, MCO_TQ_RUN);
    
    BSP_MOTC_DirSet(BSP_ID_ODD , dir);                          /// 设置方向
    BSP_MOTC_DirSet(BSP_ID_EVEN, dir);
    
    BSP_MOTC_ClkSet(BSP_ID_ODD , Bit_RESET);                    /// 设置脉冲信号电平为低
    BSP_MOTC_ClkSet(BSP_ID_EVEN, Bit_RESET);
    MCO_ClkSta      =   Bit_RESET;                              /// 记录脉冲电平为低
    
    MCO_ClkNum      =   MCB_N_P_PR * 55 / 50;                   /// 设置找原点超时脉冲数
    
    MCO_SpdIMin     =   13;                                     /// 设置最小速度
    MCO_SpdIMax     =   29;                                     /// 设置最大速度
    MCO_SpdINow     =   MCO_SpdIMin;                            /// 设置当前速度
    
    BSP_CCR_MC_OPEN =   BSP_TM8M_Get() + MCO_SpdINow;           /// 设置启动时机
    
    TIM_ClearITPendingBit(BSP_TMR_8M, BSP_FLAG_CC_MC_OPEN);     /// 清中断
    TIM_ITConfig(BSP_TMR_8M, BSP_FLAG_CC_MC_OPEN, ENABLE);      /// 打开中断
    
    OSSemPend(MCO_StopSemPtr, 0, &err);                         /// 等待运动结束
    
    BSP_MOTC_TQSet(BSP_ID_ODD , BSP_MOT_TQ_20);                 /// 设置小电流
    BSP_MOTC_TQSet(BSP_ID_EVEN, BSP_MOT_TQ_20);
    
    if (MCO_ClkNum != 0) {                                      /// 没有超时
        /// 以下为精确找原点
        /// !!! 应使用小电流找原点, 小电流定位准, 并与闭环电流状态一致.
        /// !!! 切换电流可能使位置改变 !!!
        OSTimeDlyHMSM(0, 0, 0, 200);                            /// 根据情况调整, 一般越大越好, >=100
        
        for (i = 0; i < BSP_DEVICE_ID_N; i++) {
            enc =   BSP_ENC_CntGet((BSP_ID_TypeDef)i);
            if (enc < BSP_ENC_N_T_PHR) {
                dir =   MCB_DIR_DEC;
            } else {
                dir =   MCB_DIR_INC;
                enc =   BSP_ENC_N_T_PR - enc;
            }
            
            BSP_MOTC_DirSet((BSP_ID_TypeDef)i, dir);
            for (j = enc * MCB_P_DIV_E; j != 0; j--) {
                OSTimeDlyHMSM(0, 0, 0, 1);
                BSP_MOTC_ClkSet((BSP_ID_TypeDef)i, Bit_SET);
                OSTimeDlyHMSM(0, 0, 0, 1);
                BSP_MOTC_ClkSet((BSP_ID_TypeDef)i, Bit_RESET);
            }
            
            MCB_PulseR[i]    =   0;                             /// 电机脉冲(电角度)数据清零
            MCB_PulseE[i]    =   0;
        }
        
//        MCH_SetMOrg();
        
        return  TRUE;                                           /// 返回找原点正确
        
    } else {
        return  FALSE;                                          /// 返回找原点错误, 即超时
    }
}




#endif

/// End of file -----------------------------------------------------------------------------------------
