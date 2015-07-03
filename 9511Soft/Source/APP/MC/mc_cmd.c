
/// ----------------------------------------------------------------------------
/// File Name          : mc_cmd.c
/// Description        : 闭环电机的命令控制部分
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

#define  MCC_MODULE        /// 模块定义, 用区分内外部定义


/// Includes --------------------------------------------------------------------------------------------
#include  <ucos_ii.h>               /// OS核的头文件
#include  "mc_base.h"
#include  "mc_open.h"
#include  "mc_close_low.h"
#include  "mc_close_high.h"


/// Private define --------------------------------------------------------------------------------------

#define     MCC_V_NUM_PR        400                                         /// 一整圈对应的命令值
#define     MCC_V_HLF_PR        (MCC_V_NUM_PR >> 1)                         /// 半圈对应的命令值

#define     MCC_V_HOFFSET       (-(MCC_V_NUM_PR / 8))                       /// 正常偏移值为 -r/8

#define     MCC_N_HM_GEAR       64                                          /// H轴电机齿轮齿数
#define     MCC_N_H_GEAR        16                                          /// H轴(Looper)齿轮齿数
#define     MCC_N_COLOR_GEAR    6                                           /// 换色时H轴电机齿轮转过的齿数
#define     MCC_N_LOOPER        6                                           /// Looper 数量, 即换色数

#define     MCC_D_M_RATIO       1                                           /// D轴机械传动比
#define     MCC_H_M_RATIO       (MCC_N_HM_GEAR / MCC_N_H_GEAR)              /// H轴机械传动比
#define     MCC_V_TO_E(v)       ((v) * BSP_ENC_N_T_PR / MCC_V_NUM_PR)       /// 方向命令转成编码器位置


/// 定义找H轴针位原点方式是正负方向就近找   !!! 注释掉表示单向找针位原点
#define     MCC_H_ORG_IS_PN     1

/// 求H轴各针位原点和穿线点对应的转动齿数
#if MCC_H_ORG_IS_PN
    /// n % m, 并使结果在[-m/2, m/2)范围内      !!! 要求n和m都是正数
    #define GET_PN_MOD(n, m)    ((((n) + (m) / 2) % (m)) - (m) / 2)

    #define MCC_GET_N_G_H_O(c)  (GET_PN_MOD((c) * MCC_N_COLOR_GEAR, MCC_N_H_GEAR))
    #define MCC_GET_N_G_H_T(c)  (GET_PN_MOD((c) * MCC_N_COLOR_GEAR + MCC_N_H_GEAR / 2, MCC_N_H_GEAR))
#else
    #define MCC_GET_N_G_H_O(c)  ((c) * MCC_N_COLOR_GEAR % MCC_N_H_GEAR)
    #define MCC_GET_N_G_H_T(c)  (((c) * MCC_N_COLOR_GEAR + MCC_N_H_GEAR / 2) % MCC_N_H_GEAR)
#endif

/// 各针位时, 求从HM原点转到H轴原点相对的命令值
#define     MCC_GET_V_H_O(c)    (MCC_GET_N_G_H_O(c) * MCC_V_NUM_PR / MCC_N_H_GEAR)
/// 各针位时, 求从HM原点转到H轴穿线点相对的命令值
#define     MCC_GET_V_H_T(c)    (MCC_GET_N_G_H_T(c) * MCC_V_NUM_PR / MCC_N_H_GEAR)
///// 各针位时, 求从HM原点转到H轴原点相对的HM轴的值
//#define     MCC_GET_E_H_O(c)    (MCC_GET_N_G_H_O(c) * BSP_ENC_N_T_PR / MCC_N_HM_GEAR)

//#define     MCC_GET_N_G_H_O(c)  ((c) * MCC_N_COLOR_GEAR % MCC_N_H_GEAR)
//#define     MCC_GET_NP_G_H_O(c) (MCC_GET_N_G_H_O(c))
//#define     MCC_GET_NN_G_H_O(c) (MCC_GET_N_G_H_O(c) - MCC_N_H_GEAR)
//#define     MCC_GET_VP_H_O(c)   (MCC_GET_NP_G_H_O(c) * MCC_V_NUM_PR / MCC_N_H_GEAR)
//#define     MCC_GET_VN_H_O(c)   (MCC_GET_NN_G_H_O(c) * MCC_V_NUM_PR / MCC_N_H_GEAR)
//#define     MCC_GET_EP_HM_O(c)  (MCC_GET_NP_G_H_O(c) * BSP_ENC_N_T_PR / MCC_N_HM_GEAR)
//#define     MCC_GET_EN_HM_O(c)  (MCC_GET_NN_G_H_O(c) * BSP_ENC_N_T_PR / MCC_N_HM_GEAR)
//
//#define     MCC_GET_V_H_T(c)    ((MCC_GET_VP_H_O(c) + MCC_GET_VN_H_O(c)) / 2)
//#define     MCC_GET_E_HM_T(c)   ((MCC_GET_EP_HM_O(c) + MCC_GET_EN_HM_O(c)) / 2)
//
//#define     MCC_GET_N_G_H_T(c)  (((c) * MCC_N_COLOR_GEAR + MCC_N_H_GEAR / 2) % MCC_N_H_GEAR)
//#define     MCC_GET_NP_G_H_T(c) (MCC_GET_N_G_H_T(c))
//#define     MCC_GET_NN_G_H_T(c) (MCC_GET_N_G_H_T(c) - MCC_N_H_GEAR)
//#define     MCC_GET_VP_H_T(c)   (MCC_GET_NP_G_H_T(c) * MCC_V_NUM_PR / MCC_N_H_GEAR)
//#define     MCC_GET_VN_H_T(c)   (MCC_GET_NN_G_H_T(c) * MCC_V_NUM_PR / MCC_N_H_GEAR)
//#define     MCC_GET_EP_HM_T(c)  (MCC_GET_NP_G_H_T(c) * BSP_ENC_N_T_PR / MCC_N_HM_GEAR)
//#define     MCC_GET_EN_HM_T(c)  (MCC_GET_NN_G_H_T(c) * BSP_ENC_N_T_PR / MCC_N_HM_GEAR)








/// Private typedef -------------------------------------------------------------------------------------
/// Private macro ---------------------------------------------------------------------------------------
/// Private function prototypes -------------------------------------------------------------------------
/// Private variables -----------------------------------------------------------------------------------

/// --------    cmd    --------
static  s32     MCC_DAimR   =   0;      ///  D轴目标圈数
static  u16     MCC_DAimV   =   0;      ///  D轴目标方向
static  s16     MCC_DDifV   =   0;      ///  D轴方向差值


static  s8      MCC_THCompV =   0;      /// T轴打环补偿命令值
static  u8      MCC_ColNow  =   0;      /// 当前针位数
                                        /// V轴由于环梭偏转而对应的命令值
static  s16     MCC_OffSetV =   MCC_V_HOFFSET;

static  bool    MCC_HIsN45  =   FALSE;  /// H轴偏移-45°,即刺绣状态,H轴对D和T轴有偏移角

                                        /// 以下三者只能有一个为TRUE
//static  bool    MCC_IsEmb   =   FALSE;  /// 刺绣状态    H轴对D和T轴有偏移角
//static  bool    MCC_IsOrg   =   FALSE;  /// 原点状态    VDH三轴归零
//static  bool    MCC_IsThd   =   FALSE;  /// 穿线点状态  VDH三轴与零位差半圈






/// Private consts --------------------------------------------------------------------------------------

static  const  s16      MCC_THOrgV[MCC_N_LOOPER]    =   {
    MCC_GET_V_H_O(0),
    MCC_GET_V_H_O(1),
    MCC_GET_V_H_O(2),
    MCC_GET_V_H_O(3),
    MCC_GET_V_H_O(4),
    MCC_GET_V_H_O(5)
};


/// Private functions ---------------------------------------------------------

            static  void    MCC_DeltaRE     (s32 *r, u16 *e, s16 e_delta);

            
            static  void    MCC_UpdateD     (u8 IsNeg, u8 direction);
            static  void    MCC_UpdateDM    (void);
            static  void    MCC_UpdateHM    (void);

            static  void    MCC_DtoDM       (s32 *mr, u16 *me, s32 r, u16 v);
            static  void    MCC_HtoHM       (s32 *mr, u16 *me, s32 r, u16 v);


            
            


            
            



/// --------    通讯数据相关    --------

/// -----------------------------------------------------------------------------------------------------
/// @brief  更新 D轴相关变量. 绝对方向命令转换为D 轴相关数据
/// @param  IsNeg       :   绝对方向命令是否为负值
///         direction   :   绝对方向命令值
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
static  void  MCC_UpdateD (u8 IsNeg, u8 direction)
{
    u16     abs;
    s16     dif;
    
    
    /// 把正负命令值(方向)转成正值, 范围为[0, MCC_V_NUM_PR)
    if (IsNeg != 0) {
        if (direction != 0) {
            abs =   MCC_V_NUM_PR - direction;
        } else {
            abs =   0;
        }
    } else {
        abs =   direction;
    }
    
    /// 计算差值, 使其在(-MCC_V_NUM_PR, MCC_V_NUM_PR)区间内. 前提是MCC_DAimV在[0, MCC_V_NUM_PR)内
    dif =   abs - MCC_DAimV;
    
    /// 计算"方向差值", 使其在[-MCC_V_HLF_PR, MCC_V_HLF_PR)区间内
    /// 并且更新 "圈数" 和 "方向数值" 
    if (dif < -MCC_V_HLF_PR) {
        MCC_DDifV    =   dif + MCC_V_NUM_PR;
        MCC_DAimR++;
    } else if (dif == -MCC_V_HLF_PR) {
        if (MCC_DDifV < 0) {
            MCC_DDifV   =   MCC_V_HLF_PR;
            MCC_DAimR++;
        } else {
            ;
        }
    } else if (dif < MCC_V_HLF_PR) {
        MCC_DDifV    =   dif;
    } else if (dif == MCC_V_HLF_PR) {
        if (MCC_DDifV > 0) {
            MCC_DDifV   =  -MCC_V_HLF_PR;
            MCC_DAimR--;
        } else {
            ;
        }
    } else {
        MCC_DDifV    =   dif - MCC_V_NUM_PR;
        MCC_DAimR--;
    }
    MCC_DAimV   =   abs;
}


/// --------    基本数据函数    --------


static  void  MCC_DeltaRE (s32 *r, u16 *e, s16 e_delta)
{
    s32     sum;        /// 和
    s16     quo;        /// 商
    s16     rem;        /// 余数
    
    
    sum =   *e + e_delta;
    quo =   sum / BSP_ENC_N_T_PR;
    rem =   sum % BSP_ENC_N_T_PR;
    if (rem < 0) {
        rem +=   BSP_ENC_N_T_PR;
        quo--;
    } else {
        ;
    }
    (*e)    =   rem;
    (*r)   +=   quo;
}


/// --------    闭环控制 -- 目标位置数据相关    --------

/// -----------------------------------------------------------------------------------------------------
/// @brief  D 轴命令数据转换为D 轴电机位置数据
/// @param  mr  :   存放电机位置圈数数据    (输出)
///         me  :   存放电机位置编码器数据  (输出)
///         r   :   命令圈数数据            (输入)
///         v   :   命令位置数据            (输入)
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
static  void  MCC_DtoDM (s32 *mr, u16 *me, s32 r, u16 v)
{
#if MCC_D_M_RATIO > 1
    s32     quo;        /// 商
    s8      rem;        /// 余数
#endif
    
    
#if MCC_D_M_RATIO > 1
    #error "D_M_RATIO != 1"
    quo =   r / MCC_H_M_RATIO;
    rem =   r % MCC_H_M_RATIO;
    if (rem < 0) {
        rem +=   MCC_H_M_RATIO;
        quo--;
    } else {
        ;
    }
    *mr =   quo;
    *me =   (MCC_V_TO_E(v_col) + rem * BSP_ENC_N_T_PR) / MCC_H_M_RATIO;
    
#elif MCC_D_M_RATIO == 1
    *mr =   r;
    *me =   MCC_V_TO_E(v);
    
#else
    #error "D_M_RATIO != 1"
#endif
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  H 轴命令数据转换为H 轴电机位置数据
/// @param  mr  :   存放电机位置圈数数据    (输出)
///         me  :   存放电机位置编码器数据  (输出)
///         r   :   命令圈数数据            (输入)
///         v   :   命令位置数据            (输入)
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
static  void  MCC_HtoHM (s32 *mr, u16 *me, s32 r, u16 v)
{
    s32     r_col;
    s16     v_col;
#if MCC_H_M_RATIO > 1
    s32     quo;        /// 商
    s8      rem;        /// 余数
#endif
    
    
    v_col   =   v + MCC_THOrgV[MCC_ColNow];
//    v_col   =   v + MCC_HOrgV;
    if (v_col < 0) {
        v_col  +=   MCC_V_NUM_PR;
        r_col   =   r - 1;
    } else if (v_col < MCC_V_NUM_PR) {
        r_col   =   r;
    } else {
        v_col  -=   MCC_V_NUM_PR;
        r_col   =   r + 1;
    }
    
#if MCC_H_M_RATIO > 1
    quo =   r_col / MCC_H_M_RATIO;
    rem =   r_col % MCC_H_M_RATIO;
    if (rem < 0) {
        rem +=   MCC_H_M_RATIO;
        quo--;
    } else {
        ;
    }
    *mr =   quo;
    *me =   (MCC_V_TO_E(v_col) + rem * BSP_ENC_N_T_PR) / MCC_H_M_RATIO;
    
#elif MCC_H_M_RATIO == 1
    #error "H_M_RATIO should be larger than 1"
    *mr =   r_col;
    *me =   MCC_V_TO_E(v_col);
    
#else
    #error "H_M_RATIO should be larger than 1"
#endif
}


/// -----------------------------------------------------------------------------------------------------
/// @brief  更新D 轴电机相关变量. D轴数据转换为D 轴电机数据. 用于上机头板
/// @param  None
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
static  void  MCC_UpdateDM (void)
{
    MCH_MStarR  =   MCH_MStopR;
    MCH_MStarE  =   MCH_MStopE;
//    MCH_MBackR  =   MCH_MStarR;
//    MCH_MBackE  =   MCH_MStarE;
    
    MCC_DtoDM(&MCH_MStopR, &MCH_MStopE, MCC_DAimR, MCC_DAimV);
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  更新H 轴电机相关变量. D轴数据转换为H 轴电机数据. 用于下机头板
/// @param  None
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
static  void  MCC_UpdateHM (void)
{
    s16     hv1;
    s32     hr1;
    s16     hv2;
    s32     hr2;
    
    
    hv1 =   MCC_DAimV + MCC_THCompV;
    hv2 =   MCC_DAimV;
    if (MCC_HIsN45 != FALSE) {
        hv1    +=   MCC_OffSetV;
        hv2    +=   MCC_OffSetV;
    } else {
        ;
    }
    
    /// MCC_THCompV范围是[-2r/40, 7r/40], MCC_OffSetV范围是[-r/4, 0), 所以hv1范围是[-12r/40, 47r/40)
    if (hv1 < 0) {
        hr1     =   MCC_DAimR + 1 - 1;         /// +1 for h1
        hv1    +=   MCC_V_NUM_PR;
    } else if (hv1 < MCC_V_NUM_PR) {
        hr1     =   MCC_DAimR + 1;
    } else {
        hr1     =   MCC_DAimR + 1 + 1;
        hv1    -=   MCC_V_NUM_PR;
    }
    
    /// MCC_OffSetV范围是[-r/4, 0), 所以hv1范围是[-r/4, r)
    if (hv2 < 0) {
        hr2     =   MCC_DAimR - 1;
        hv2    +=   MCC_V_NUM_PR;
    } else {
        hr2     =   MCC_DAimR;
    }
    
    MCH_MStarR  =   MCH_MStopR;
    MCH_MStarE  =   MCH_MStopE;
    
    MCC_HtoHM(&MCH_MBackR, &MCH_MBackE, hr1, hv1);
    MCC_HtoHM(&MCH_MStopR, &MCH_MStopE, hr2, hv2);
}





/// --------    动作  包括开环和闭环控制    --------
        
/// -----------------------------------------------------------------------------------------------------
/// @brief  D轴找原点
/// @param  :   None
/// @retval :   TRUE    -   成功
///             FALSE   -   失败, 超时
/// -----------------------------------------------------------------------------------------------------
bool  MCC_DToOrg (void)
{
    MCH_CloseExit();
    
    if (MCO_Org() != FALSE) {               /// 找电机原点
        
        MCH_SetMOrg();
                                            /// 设置D 轴数据
        MCC_DAimR   =   0;
        MCC_DAimV   =   0;
        
        MCC_UpdateDM();                     /// DM轴数据根据 D轴数据更新
        
        MCH_CloseMov1(8, 10);               /// 闭环运动
        
        return  TRUE;
        
    } else {
        return  FALSE;
    }
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  D轴找穿线点
/// @param  :   None
/// @retval :   TRUE    -   成功
///             FALSE   -   失败, 超时
/// -----------------------------------------------------------------------------------------------------
bool  MCC_DToThd (void)
{
    MCH_CloseExit();
    
    if (MCO_Org() != FALSE) {               /// 找电机原点
        
        MCH_SetMOrg();
                                            /// 设置D 轴数据
        if (MCL_MNowE < BSP_ENC_N_T_PHR) {
            MCC_DAimR   =   0;
        } else if (MCL_MNowE > BSP_ENC_N_T_PHR) {
            MCC_DAimR   =  -1;
        } else {
            if (MCL_MNowR < 0) {
                MCC_DAimR   =  -1;
            } else {
                MCC_DAimR   =   0;
            }
        }
        MCC_DAimV   =   MCC_V_HLF_PR;
        
        MCC_UpdateDM();                     /// DM轴数据根据 D轴数据更新
        
        MCH_CloseMov1(8, 10);               /// 闭环运动
        
        return  TRUE;
        
    } else {
        return  FALSE;
    }
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  H轴找原点
/// @param  color   :   指定的颜色(针位)号
/// @retval :   TRUE    -   成功
///             FALSE   -   失败, 超时
/// -----------------------------------------------------------------------------------------------------
bool  MCC_HToOrg (u8 color)
{
    if (color < MCC_N_LOOPER) {
        
        MCC_ColNow  =   color;              /// 记录当前颜色(环梭)号
        MCC_HIsN45  =   FALSE;              /// 没有偏移, 非刺绣状态

        MCH_CloseExit();
        
        if (MCO_Org() != FALSE) {           /// 找电机原点
            
            MCH_SetMOrg();
                                            /// 设置D 轴数据
            MCC_DAimR   =   0;
            MCC_DAimV   =   0;
            
            MCC_UpdateHM();                 /// HM轴数据根据 D轴数据更新
            
            MCH_CloseMov1(8, 10);           /// 闭环运动
            
            return  TRUE;
            
        } else {
            return  FALSE;
        }
        
    } else {
        return  FALSE;
    }
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  H轴找穿线点
/// @param  color   :   指定的颜色(针位)号
/// @retval :   TRUE    -   成功
///             FALSE   -   失败, 超时
/// -----------------------------------------------------------------------------------------------------
bool  MCC_HToThd (u8 color)
{
    if (color < MCC_N_LOOPER) {
        
        MCC_ColNow  =   color;              /// 记录当前颜色(环梭)号
        MCC_HIsN45  =   FALSE;              /// 没有偏移, 非刺绣状态
        
        MCH_CloseExit();
        
        if (MCO_Org() != FALSE) {           /// 找电机原点
            
            MCH_SetMOrg();
                                            /// 设置D 轴数据
            if (MCC_THOrgV[color] < 0) {
                MCC_DAimR   =   0;
            } else if (MCC_THOrgV[color] > 0) {
                MCC_DAimR   =  -1;
            } else {
                if (MCL_MNowE < BSP_ENC_N_T_PHR) {
                    MCC_DAimR   =   0;
                } else if (MCL_MNowE > BSP_ENC_N_T_PHR) {
                    MCC_DAimR   =  -1;
                } else {
                    if (MCL_MNowR < 0) {
                        MCC_DAimR   =  -1;
                    } else {
                        MCC_DAimR   =   0;
                    }
                }
            }
            MCC_DAimV   =   MCC_V_HLF_PR;
            
            MCC_UpdateHM();                 /// HM轴数据根据 D轴数据更新
            
            MCH_CloseMov1(8, 10);           /// 闭环运动
            
            return  TRUE;
            
        } else {
            return  FALSE;
        }
        
    } else {
        return  FALSE;
    }
}


/// -----------------------------------------------------------------------------------------------------
/// @brief  H轴晃动, 开环
/// @param  dir :   开始晃动的方向
///         n   :   晃动的单向距离
///         ts  :   晃动次数
/// @retval :   None
/// -----------------------------------------------------------------------------------------------------
void  MCC_HShake (BSP_MOT_DIR_TypeDef dir, u16 n, u8 ts)
{
#define MCC_SHAKE_SPD_MIN   8
#define MCC_SHAKE_SPD_MAX   13

    u8      i;
    
    
    MCH_CloseExit();
    
//    MCO_Mov(dir, n * 2, 13, MCO_TAB_N - 1);
    MCO_Mov(dir, n, MCC_SHAKE_SPD_MIN, MCC_SHAKE_SPD_MAX);
    
    for (i = ts; i != 0; i--) {
        if (dir == MCB_DIR_DEC) {
            dir =   MCB_DIR_INC;
        } else {
            dir =   MCB_DIR_DEC;
        }
        
        MCO_Mov(dir, 2 * n, MCC_SHAKE_SPD_MIN, MCC_SHAKE_SPD_MAX);
        OSTimeDlyHMSM(0, 0, 0, 150);
    }
    
    if (dir == MCB_DIR_DEC) {
        dir =   MCB_DIR_INC;
    } else {
        dir =   MCB_DIR_DEC;
    }
    MCO_Mov(dir, n, MCC_SHAKE_SPD_MIN, MCC_SHAKE_SPD_MAX);
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  H轴晃动, 开环
/// @param  to_col  :   换色目的号
/// @retval :   None
/// -----------------------------------------------------------------------------------------------------
void  MCC_HToColor (u8 to_col)
{
    BSP_MOT_DIR_TypeDef     dir;
    u16                     dif;
    s8                      sgn;
    
    
    if (to_col < MCC_N_LOOPER) {
        MCH_CloseExit();
        
        if (to_col == MCC_ColNow) {
            return;
        } else if (to_col < MCC_ColNow) {
            sgn =  -1;
            dir =   MCB_DIR_DEC;
            dif =   MCC_ColNow - to_col;
        } else {
            sgn =   1;
            dir =   MCB_DIR_INC;
            dif =   to_col - MCC_ColNow;
        }
        dif *=  BSP_ENC_N_T_PR * MCC_N_COLOR_GEAR / MCC_N_HM_GEAR;
        MCO_Mov(dir, dif, 0, 0);
        
        MCC_DeltaRE (&MCL_MNowR, &MCL_MNowE, dif * sgn);
    }
}




void  MCC_Init (void)
{
//    MCC_TStarR  =   0;      ///  T轴开始圈数
//    MCC_TStarV  =   0;      ///  T轴开始方向

//    MCC_TStopR  =   0;      ///  T轴结束圈数
//    MCC_TStopV  =   0;      ///  T轴结束方向
//    MCC_TDifV   =   0;      ///  T轴方向差值
    MCC_DAimR   =   0;      ///  D轴目标圈数
    MCC_DAimV   =   0;      ///  D轴目标方向
    MCC_DDifV   =   0;      ///  D轴方向差值
    
//    MCC_DStarR  =   0;      ///  D轴开始圈数
//    MCC_DStarE  =   0;      ///  D轴开始方向 (编码器)
//    MCC_DStopR  =   0;      ///  D轴结束圈数
//    MCC_DStopE  =   0;      ///  D轴结束方向 (编码器)
//    
//    MCC_HStarR  =   0;      ///  H轴开始圈数
//    MCC_HStarE  =   0;      ///  H轴开始方向 (编码器)
//    MCC_HStopR  =   0;      ///  H轴结束圈数
//    MCC_HStopE  =   0;      ///  H轴结束方向 (编码器)    
    
//    MCH_MStarR  =   0;      ///  D或 H轴开始圈数
//    MCH_MStarE  =   0;      ///  D或 H轴开始方向 (编码器)
//    MCH_MStopR  =   0;      ///  D或 H轴结束圈数
//    MCH_MStopE  =   0;      ///  D或 H轴结束方向 (编码器)   
//    
//    MCL_MNowR   =   0;      /// 被控电机当前圈数
//    MCL_MNowE   =   0;      /// 被控电机当前方向 (编码器)
//    
//    MCH_MDecR   =   0;      /// 被控电机负方向极限圈数
//    MCH_MDecE   =   0;      /// 被控电机负方向极限方向 (编码器)
//    MCH_MIncR   =   0;      /// 被控电机正方向极限圈数
//    MCH_MIncE   =   0;      /// 被控电机正方向极限方向 (编码器)
    MCH_SetMOrg();
    
    MCC_THCompV =   0;      /// T轴打环补偿命令值
    MCC_ColNow  =   0;      /// 当前针位数
                            /// V轴由于环梭偏转而对应的命令值
    MCC_OffSetV =   MCC_V_HOFFSET;
    
//    MCO_StopSemPtr  =   OSSemCreate(0);
    MCO_Init();
    
//    BSP_ENC_IndexEnable();
}



/// -----------------------------------------------------------------------------------------------------
/// @brief  打环角度补偿
/// @param  Comp    :   补偿档位值, 范围是[0,9], 2为不补偿, 0~1为减少, 3~9为增加.
///                     每档为10/400r,即9°. 范围是[-2r/40,7r/40]
/// @retval :   None
/// -----------------------------------------------------------------------------------------------------
void  MCC_ComSetHAngComp (u8 Comp)
{
    if (Comp < 10) {
        MCC_THCompV =   (Comp - 2) * 10;
    } else {
        MCC_THCompV =   0;
    }
}


/// -----------------------------------------------------------------------------------------------------
/// @brief  起绣前H轴的偏移动作
/// @param  b1  :   通讯命令数据1, 表示偏移方向符号和对应主轴速度
///         b2  :   通讯命令数据2, 表示偏移绝对值.
/// @retval :   None
/// -----------------------------------------------------------------------------------------------------
void  MCC_ComHOffset (u8 b1, u8 b2)
{
    u8      spd_d10;
    
    
                                                                /// 记录偏移值
    if ((b1 >> 7) && (b2 < MCC_V_NUM_PR / 4) && (b2 != 0)) {    /// 有效范围为(-r/4, 0)
        MCC_OffSetV =  -b2;
    } else {
        MCC_OffSetV =   MCC_V_HOFFSET;                          /// !不一定用默认值, 也可使用原值
    }
    
    MCC_HIsN45   =   TRUE;                                      /// 设为刺绣状态, 偏移状态
    
    spd_d10 =   b1 & (~(1 << 7));                               /// 模拟主轴速度
    
    MCC_UpdateHM();                                             /// HM轴数据根据 D轴数据更新
    
    MCH_CloseMov1(spd_d10, 10);                                 /// 闭环运动
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  绣作命令, 用于 D轴
/// @param  b1  :   通讯命令数据1, 表示D轴方向符号和对应主轴速度
///         b2  :   通讯命令数据2, 表示D轴方向绝对值.
/// @retval :   None
/// -----------------------------------------------------------------------------------------------------
void  MCC_ComD (u8 b1, u8 b2)
{
    u8      spd_d10;
    
    
    spd_d10 =   b1 & (~(1 << 7));           /// 调速. 如果超出服务, 则不更新, 保持原状态.(BSP层有限制)
    
    MCC_UpdateD((b1 >> 7) & 0x01, b2);
    MCC_UpdateDM();
    
    MCH_CloseMov1(spd_d10, 10);             /// 闭环运动
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  绣作命令, 用于 H轴
/// @param  b1  :   通讯命令数据1, 表示D轴方向符号和对应主轴速度
///         b2  :   通讯命令数据2, 表示D轴方向绝对值.
/// @retval :   None
/// -----------------------------------------------------------------------------------------------------
void  MCC_ComH (u8 b1, u8 b2)
{
    u8      spd_d10;
    
    
    MCC_HIsN45   =   TRUE;                      /// 设为刺绣状态, 偏移状态
    
    spd_d10 =   b1 & (~(1 << 7));               /// 调速. 如果超出服务, 则不更新, 保持原状态.(BSP层有限制)
    
    MCC_UpdateD((b1 >> 7) & 0x01, b2);
    MCC_UpdateHM();
    
    MCH_CloseMov2(spd_d10, 10, 4010);           /// 闭环运动
}











#endif

/// End of file -----------------------------------------------------------------------------------------
