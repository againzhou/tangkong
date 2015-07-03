
/// ----------------------------------------------------------------------------
/// File Name          : mc_close_low.c
/// Description        : 闭环电机的闭环控制低层部分
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

#define  MCL_MODULE        /// 模块定义, 用区分内外部定义


/// Includes --------------------------------------------------------------------------------------------
#include  <ucos_ii.h>               /// OS核的头文件
#include  "mc_base.h"
//#include  "mc_open.h"


/// Private define --------------------------------------------------------------------------------------

#define     MCL_TQ_RUN          BSP_MOT_TQ_20


#define     MCL_E_MAX_TQ        (BSP_ENC_N_T_PR / MCB_POLE_PAIRS / 4)       /// 最大力矩位置, 编码器单位
#define     MCL_E_COMP_F        5                                           /// 正转补偿, 编码器单位
#define     MCL_E_COMP_B        (-4)                                        /// 反转补偿, 编码器单位
#define     MCL_E_MAX_TQ_F      (MCL_E_MAX_TQ + MCL_E_COMP_F)               /// 正转最大力矩位置, 编码器单位
#define     MCL_E_MAX_TQ_B      (MCL_E_MAX_TQ + MCL_E_COMP_B)               /// 反转最大力矩位置, 编码器单位

#define     MCL_ENDURE_DIF      (BSP_ENC_N_T_PR / 40)                       /// 控制上允许的误差


/// for speed fast
#define     MCL_OddClkSet()     (GPIOD->BSRR    =   0x01 <<  9)
#define     MCL_OddClkReset()   (GPIOD->BRR     =   0x01 <<  9)

#define     MCL_EvenClkSet()    (GPIOD->BSRR    =   0x01 << 15)
#define     MCL_EvenClkReset()  (GPIOD->BRR     =   0x01 << 15)

#define     MCL_OddDirSetInc()  (GPIOB->BSRR    =   0x01 << 14)
#define     MCL_OddDirSetDec()  (GPIOB->BRR     =   0x01 << 14)

#define     MCL_EvenDirSetInc() (GPIOD->BSRR    =   0x01 << 12)
#define     MCL_EvenDirSetDec() (GPIOD->BRR     =   0x01 << 12)



/// Private typedef -------------------------------------------------------------------------------------
/// Private macro ---------------------------------------------------------------------------------------
/// Private function prototypes -------------------------------------------------------------------------
/// Private variables -----------------------------------------------------------------------------------

/// variables -------------------------------------------------------------------------------------------

s32     MCL_MNowR   =   0;      /// 被控电机当前圈数
u16     MCL_MNowE   =   0;      /// 被控电机当前方向 (编码器)





/// Private consts --------------------------------------------------------------------------------------



/// Private functions ---------------------------------------------------------
            
            static  s8      MCL_GetDeltaP   (BSP_ID_TypeDef id);
            static  void    MCL_DecRE       (s32 *r, u16 *e);
            static  void    MCL_IncRE       (s32 *r, u16 *e);










void  MCL_SetMOrg (void)
{
    MCL_MNowR   =   0;      /// 被控电机当前圈数
    MCL_MNowE   =   0;      /// 被控电机当前方向 (编码器)
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  根据编码器和目标位置, 计算脉冲位置增量
/// @param  id    : The ID of close_loop motor (the odd or even)
/// @retval : 脉冲位置增量
///        -1   -   脉冲位置减少, 发负向脉冲
///         0   -   脉冲位置不变, 不发脉冲
///         1   -   脉冲位置增加, 发正向脉冲
/// -----------------------------------------------------------------------------------------------------
static  s8  MCL_GetDeltaP (BSP_ID_TypeDef id)
{
    s32         d_pe;
    s32         d_cp;
    s8          delta;
#if OS_CRITICAL_METHOD == 3
    OS_CPU_SR   cpu_sr  =   0;
#endif
    
    
    OS_ENTER_CRITICAL();
    
    /// 脉冲位置与实际位置(编码器值)的差值
//    d_pe    =   (MCB_PulseR[id] - MCC_R[id]) * BSP_ENC_N_T_PR + MCB_PulseE[id] - BSP_ENC_CntGet(id);
    d_pe    =   MCB_PulseE[id] - BSP_ENC_CntGet(id);
    if (d_pe >= BSP_ENC_N_T_PHR) {
        d_pe   -=   BSP_ENC_N_T_PR;
    } else if (d_pe < -BSP_ENC_N_T_PHR) {
        d_pe   +=   BSP_ENC_N_T_PR;
    } else {
        ;
    }
    /// 命令位置与脉冲位置的差值
    d_cp    =   (MCL_MNowR - MCB_PulseR[id]) * BSP_ENC_N_T_PR + MCL_MNowE - MCB_PulseE[id];
    
    OS_EXIT_CRITICAL();
    
    if (d_cp < 0) {
        if (d_pe > -MCL_E_MAX_TQ) {             /// 没有到达最大力矩角度
            delta   =  -1;                      /// 脉冲位置向命令位置靠近
        } else if (d_pe == -MCL_E_MAX_TQ) {     /// 到达最大力矩角度
            delta   =   0;                      /// 脉冲位置不变
        } else {                                /// 超过最大力矩角度
            delta   =   1;                      /// 脉冲位置向实际位置靠近
        }
    } else if (d_cp > 0) {
        if (d_pe < MCL_E_MAX_TQ) {              /// 没有到达最大力矩角度
            delta   =   1;                      /// 脉冲位置向命令位置靠近
        } else if (d_pe == MCL_E_MAX_TQ) {      /// 到达最大力矩角度
            delta   =   0;                      /// 脉冲位置不变
        } else {                                /// 超过最大力矩角度
            delta   =  -1;                      /// 脉冲位置向实际位置靠近
        }
    } else {
        if (d_pe < -MCL_E_MAX_TQ) {             /// 超过最大力矩角度
            delta   =   1;                      /// 脉冲位置向实际位置靠近
        } else if (d_pe > MCL_E_MAX_TQ) {       /// 超过最大力矩角度
            delta   =  -1;                      /// 脉冲位置向实际位置靠近
        } else {                                /// 没有超过最大力矩角度
            delta   =   0;                      /// 脉冲位置不变
        }
    }
    
    return  delta;
}


/// -----------------------------------------------------------------------------------------------------
/// @brief  编码器值减1, 如果卷绕则圈数减1
///         由于内部函数, 所以不检查参数范围
/// @param  r   :   存放圈数的地址, *e不判断范围, 应在[0, BSP_ENC_N_T_PR)内,
///         e   :   存放编码器值的地址, *r也不判断范围
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
static  void  MCL_DecRE (s32 *r, u16 *e)
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
static  void  MCL_IncRE (s32 *r, u16 *e)
{
    if ((*e) < BSP_ENC_N_T_PR - 1) {
        (*e)++;
    } else {
        (*e)    =   0;
        (*r)++;
    }
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  闭环运动控制的定时中断服务函数
///         方向信号的建立需要早于脉冲信号, 脉冲宽度需要大于8uS, 即频率小于60K.
/// @param  None
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
void  MCL_IRQ (void)
{
    static  s8      DeltaOdd    =   0;
    static  s8      DeltaEven   =   0;
    
    
    if (DeltaOdd == 0) {
        MCL_OddClkReset();
    } else {
        MCL_OddClkSet();
    }
    
    if (DeltaEven == 0) {
        MCL_EvenClkReset();
    } else {
        MCL_EvenClkSet();
    }
    
    if (DeltaOdd == 0) {
        DeltaOdd    =   MCL_GetDeltaP(BSP_ID_ODD);
        if (DeltaOdd < 0) {
            MCL_OddDirSetDec();
        } else if (DeltaOdd > 0) {
            MCL_OddDirSetInc();
        }
    } else {
        if (DeltaOdd < 0) {
            MCL_DecRE(&MCB_PulseR[BSP_ID_ODD], &MCB_PulseE[BSP_ID_ODD]);
        } else {
            MCL_IncRE(&MCB_PulseR[BSP_ID_ODD], &MCB_PulseE[BSP_ID_ODD]);
        }
        DeltaOdd    =   0;
    }
    
    if (DeltaEven == 0) {
        DeltaEven   =   MCL_GetDeltaP(BSP_ID_EVEN);
        if (DeltaEven < 0) {
            MCL_EvenDirSetDec();
        } else if (DeltaEven > 0) {
            MCL_EvenDirSetInc();
        }
    } else {
        if (DeltaEven < 0) {
            MCL_DecRE(&MCB_PulseR[BSP_ID_EVEN], &MCB_PulseE[BSP_ID_EVEN]);
        } else {
            MCL_IncRE(&MCB_PulseR[BSP_ID_EVEN], &MCB_PulseE[BSP_ID_EVEN]);
        }
        DeltaEven   =   0;
    }
    
    BSP_CCR_MC_CLOSE    =   BSP_TM8M_Get() + BSP_TMR_8M_CLK * 3 / 1000000;
//    BSP_CCR_MC_CLOSE    =   BSP_TMR_8M_CLK * 20 / 1000000;
}




#endif

/// End of file -----------------------------------------------------------------------------------------
