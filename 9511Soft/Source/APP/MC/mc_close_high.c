
/// ----------------------------------------------------------------------------
/// File Name          : mc_close_high.c
/// Description        : 闭环电机的闭环控制高层部分
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

#define  MCH_MODULE        /// 模块定义, 用区分内外部定义


/// Includes --------------------------------------------------------------------------------------------
#include  <ucos_ii.h>               /// OS核的头文件
#include  "mc_base.h"
#include  "mc_close_low.h"


/// Private define --------------------------------------------------------------------------------------
/// Private typedef -------------------------------------------------------------------------------------
/// Private macro ---------------------------------------------------------------------------------------
/// Private function prototypes -------------------------------------------------------------------------
/// Private variables -----------------------------------------------------------------------------------

/// E=0~1599,R=0; E=1600~3199,R=1; E=-1600~-1,R=-1

static  s32     MCH_MDecR   =   0;      /// 被控电机负方向极限圈数
static  u16     MCH_MDecE   =   0;      /// 被控电机负方向极限方向 (编码器)
static  s32     MCH_MIncR   =   0;      /// 被控电机正方向极限圈数
static  u16     MCH_MIncE   =   0;      /// 被控电机正方向极限方向 (编码器)


/// variables -------------------------------------------------------------------------------------------

s32     MCH_MStarR  =   0;      ///  D或 H轴开始圈数
u16     MCH_MStarE  =   0;      ///  D或 H轴开始方向 (编码器)
s32     MCH_MStopR  =   0;      ///  D或 H轴结束圈数
u16     MCH_MStopE  =   0;      ///  D或 H轴结束方向 (编码器)
s32     MCH_MBackR  =   0;      ///  H轴折返点圈数
u16     MCH_MBackE  =   0;      ///  H轴折返点方向 (编码器)





/// Private consts --------------------------------------------------------------------------------------

/// Private functions ---------------------------------------------------------
            
            static  void    MCH_DecRE       (s32 *r, u16 *e);
            static  void    MCH_IncRE       (s32 *r, u16 *e);




            
/// -----------------------------------------------------------------------------------------------------
/// @brief  D 或 H 电机轴目标位置数据清零, 回原点
/// @param  :   None
/// @retval :   None
/// -----------------------------------------------------------------------------------------------------
void  MCH_SetMOrg (void)
{
    MCH_MStarR  =   0;      ///  D或 H轴电机开始圈数
    MCH_MStarE  =   0;      ///  D或 H轴电机开始方向 (编码器)
    MCH_MBackR  =   0;      ///  H轴电机返回点圈数
    MCH_MBackE  =   0;      ///  H轴电机返回点开始方向 (编码器)
    MCH_MStopR  =   0;      ///  D或 H轴结束圈数
    MCH_MStopE  =   0;      ///  D或 H轴结束方向 (编码器)   
    
    MCH_MDecR   =   0;      /// 被控电机负方向极限圈数
    MCH_MDecE   =   0;      /// 被控电机负方向极限方向 (编码器)
    MCH_MIncR   =   0;      /// 被控电机正方向极限圈数
    MCH_MIncE   =   0;      /// 被控电机正方向极限方向 (编码器)
    
    MCL_SetMOrg();
}

/// --------    闭环控制 -- 目标位置控制相关    --------

void  MCH_CloseEnter (BSP_MOT_TQ_TypeDef tq, u16 StartTiming)
{
    BSP_MOTC_TQSet(BSP_ID_ODD , tq);                            /// 设置电流
    BSP_MOTC_TQSet(BSP_ID_EVEN, tq);
    
    BSP_MOTC_ClkSet(BSP_ID_ODD , Bit_RESET);                    /// 设置脉冲电平为低
    BSP_MOTC_ClkSet(BSP_ID_EVEN, Bit_RESET);
    
    BSP_CCR_MC_CLOSE    =   BSP_TM8M_Get() + StartTiming;       /// 设置启动时机
    TIM_ClearITPendingBit(BSP_TMR_8M, BSP_FLAG_CC_MC_CLOSE);    /// 清中断
    TIM_ITConfig(BSP_TMR_8M, BSP_FLAG_CC_MC_CLOSE, ENABLE);     /// 打开中断
}

void  MCH_CloseExit (void)
{
    TIM_ITConfig(BSP_TMR_8M, BSP_FLAG_CC_MC_CLOSE, DISABLE);    /// 关闭中断
    
    BSP_MOTC_ClkSet(BSP_ID_ODD , Bit_RESET);                    /// 设置脉冲电平为低
    BSP_MOTC_ClkSet(BSP_ID_EVEN, Bit_RESET);
    
    BSP_MOTC_TQSet(BSP_ID_ODD , BSP_MOT_TQ_20);                 /// 设置为小电流
    BSP_MOTC_TQSet(BSP_ID_EVEN, BSP_MOT_TQ_20);
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  闭环单向运动, 即单向发出微步指令, 用于D轴或H轴闭环运动控制
///         由于内部函数, 所以不检查参数范围
/// @param  spd_d10 :   主轴速度值的1/10, 单位是 10rpm
///         cnt     :   开始运动的时间，  单位是主轴模拟编码器线数
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
void  MCH_CloseMov1 (u8 spd_d10, u16 cnt)
{
#if OS_CRITICAL_METHOD == 3
    OS_CPU_SR  cpu_sr = 0;
#endif
    s32     dif;
    
    
    BSP_TM_CNT_SpdSet(spd_d10);
    
    TIM_ITConfig(BSP_TMR_CNT, BSP_FLAG_CC_MCC_DEC, DISABLE);        /// 关闭模拟计数减位置中断
    TIM_ITConfig(BSP_TMR_CNT, BSP_FLAG_CC_MCC_INC, DISABLE);        /// 关闭模拟计数增位置中断
        
    dif =   (MCH_MStopR - MCH_MStarR) * BSP_ENC_N_T_PR + MCH_MStopE - MCH_MStarE;
        
    OS_ENTER_CRITICAL();
    MCL_MNowR   =   MCH_MStarR;
    MCL_MNowE   =   MCH_MStarE;
    
    if (dif < 0) {
        MCH_MDecR   =   MCH_MStopR;
        MCH_MDecE   =   MCH_MStopE;
        MCH_MIncR   =   MCH_MStarR;
        MCH_MIncE   =   MCH_MStarE;
        OS_EXIT_CRITICAL();
        
        BSP_CCR_MCC_DEC =   BSP_TM_CNT_Get() + cnt;                 /// 设置启动时机
        TIM_ClearITPendingBit(BSP_TMR_CNT, BSP_FLAG_CC_MCC_DEC);    /// 清中断
        TIM_ITConfig(BSP_TMR_CNT, BSP_FLAG_CC_MCC_DEC, ENABLE);     /// 打开中断
        
    } else if (dif == 0) {
        OS_EXIT_CRITICAL();
        
    } else {
        MCH_MDecR   =   MCH_MStarR;
        MCH_MDecE   =   MCH_MStarE;
        MCH_MIncR   =   MCH_MStopR;
        MCH_MIncE   =   MCH_MStopE;
        OS_EXIT_CRITICAL();
        
        BSP_CCR_MCC_INC =   BSP_TM_CNT_Get() + cnt;                 /// 设置启动时机
        TIM_ClearITPendingBit(BSP_TMR_CNT, BSP_FLAG_CC_MCC_INC);    /// 清中断
        TIM_ITConfig(BSP_TMR_CNT, BSP_FLAG_CC_MCC_INC, ENABLE);     /// 打开中断
    }
    
    MCH_CloseEnter(BSP_MOT_TQ_20, 100);
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  闭环双向运动, 即发出一方向微步指令后再发出另一方向的微步指令, 用于H轴刺绣闭环运动控制
///         由于内部函数, 所以不检查参数范围
/// @param  spd_d10 :   主轴速度值的1/10, 单位是 10rpm
///         cnt1    :   第一方向开始运动的时间，  单位是主轴模拟编码器线数
///         cnt2    :   另一方向开始运动的时间，  单位是主轴模拟编码器线数
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
void  MCH_CloseMov2 (u8 spd_d10, u16 cnt1, u16 cnt2)
{
#if OS_CRITICAL_METHOD == 3
    OS_CPU_SR  cpu_sr = 0;
#endif
    u16     cnt;
    
    
    BSP_TM_CNT_SpdSet(spd_d10);
    
    TIM_ITConfig(BSP_TMR_CNT, BSP_FLAG_CC_MCC_DEC, DISABLE);        /// 关闭模拟计数减位置中断
    TIM_ITConfig(BSP_TMR_CNT, BSP_FLAG_CC_MCC_INC, DISABLE);        /// 关闭模拟计数增位置中断
    
    OS_ENTER_CRITICAL();
    
    MCL_MNowR   =   MCH_MStarR;
    MCL_MNowE   =   MCH_MStarE;
    
    MCH_MIncR   =   MCH_MBackR;
    MCH_MIncE   =   MCH_MBackE;
    MCH_MDecR   =   MCH_MStopR;
    MCH_MDecE   =   MCH_MStopE;
    
    OS_EXIT_CRITICAL();
        
    cnt         =   BSP_TM_CNT_Get();
    
    BSP_CCR_MCC_INC =   cnt + cnt1;                                 /// 设置启动时机
    TIM_ClearITPendingBit(BSP_TMR_CNT, BSP_FLAG_CC_MCC_INC);        /// 清中断
    TIM_ITConfig(BSP_TMR_CNT, BSP_FLAG_CC_MCC_INC, ENABLE);         /// 打开中断
    
    BSP_CCR_MCC_DEC =   cnt + cnt2;                                 /// 设置启动时机
    TIM_ClearITPendingBit(BSP_TMR_CNT, BSP_FLAG_CC_MCC_DEC);        /// 清中断
    TIM_ITConfig(BSP_TMR_CNT, BSP_FLAG_CC_MCC_DEC, ENABLE);         /// 打开中断
    
    MCH_CloseEnter(BSP_MOT_TQ_20, 100);
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  编码器值减1, 如果卷绕则圈数减1
///         由于内部函数, 所以不检查参数范围
/// @param  r   :   存放圈数的地址, *e不判断范围, 应在[0, BSP_ENC_N_T_PR)内,
///         e   :   存放编码器值的地址, *r也不判断范围
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
static  void  MCH_DecRE (s32 *r, u16 *e)
{
    if ((*e) != 0) {
        (*e)--;
    } else {
        (*e)    =   BSP_ENC_N_T_PR - 1;
        (*r)--;
    }
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  编码器值加1, 如果卷绕则圈数加1
///         由于内部函数, 所以不检查参数范围
/// @param  r   :   存放圈数的地址, *e不判断范围, 应在[0, BSP_ENC_N_T_PR)内,
///         e   :   存放编码器值的地址, *r也不判断范围
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
static  void  MCH_IncRE (s32 *r, u16 *e)
{
    if ((*e) < BSP_ENC_N_T_PR - 1) {
        (*e)++;
    } else {
        (*e)    =   0;
        (*r)++;
    }
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  设置目标位置减少, 在模拟计数中断中调用. for D-, H-(H2)
///         更新目标位置后如果到达最终目标位置, 并没有马上关闭此中断, 而在下个中断中关闭
/// @param  None
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
void  MCH_AimDecIRQ (void)
{
#if OS_CRITICAL_METHOD == 3
    OS_CPU_SR  cpu_sr = 0;
#endif
    
    
    OS_ENTER_CRITICAL();
    
    if ((MCL_MNowR - MCH_MDecR) * BSP_ENC_N_T_PR + MCL_MNowE - MCH_MDecE > 0) {     /// 如果大于最终位置
        MCH_DecRE(&MCL_MNowR, &MCL_MNowE);                                          /// 位置减少
        BSP_CCR_MCC_DEC +=  7;                                                      /// 设置下个更新时间
    } else {
        TIM_ITConfig(BSP_TMR_CNT, BSP_FLAG_CC_MCC_DEC, DISABLE);                    /// 关闭对应中断
    }
    
    OS_EXIT_CRITICAL();
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  设置目标位置增加, 在模拟计数中断中调用. for D+, H+(H1)
///         更新目标位置后如果到达最终目标位置, 并没有马上关闭此中断, 而在下个中断中关闭
/// @param  None
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
void  MCH_AimIncIRQ (void)
{
#if OS_CRITICAL_METHOD == 3
    OS_CPU_SR  cpu_sr = 0;
#endif
    
    
    OS_ENTER_CRITICAL();
    
    if ((MCL_MNowR - MCH_MIncR) * BSP_ENC_N_T_PR + MCL_MNowE - MCH_MIncE < 0) {     /// 如果小于最终位置
        MCH_IncRE(&MCL_MNowR, &MCL_MNowE);                                          /// 位置增加
        BSP_CCR_MCC_INC +=  7;                                                      /// 设置下个更新时间
    } else {
        TIM_ITConfig(BSP_TMR_CNT, BSP_FLAG_CC_MCC_INC, DISABLE);                    /// 关闭对应中断
    }
    
    OS_EXIT_CRITICAL();
}




#endif

/// End of file -----------------------------------------------------------------------------------------
