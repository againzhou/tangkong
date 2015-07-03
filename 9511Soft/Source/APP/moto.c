
/// ----------------------------------------------------------------------------
/// File Name          : moto.c
/// Description        : 开环电机任务
///     剪线齿轮共14齿, m = 1, d = mz = 14mm, 周长 = 14pi =  约44mm
///     如果剪线电机与原来相反, 调整宏定义 MOTO_CUT_IN 和 MOTO_CUT_OUT
///     针高电机同上
/// ----------------------------------------------------------------------------
/// History:

/// y11/m03/d06     v0.06   chenyong    modify
/// 由于力矩比原9511板小, 所以需要降低运动速度.
/// 将 MOTO_HghSpdEngine 更改为 2, 原来为 3

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

/// y10/m11/d30     v0.03   chenyong    modify
/// 修改10针高范围, 原来为+-6, 改为+-12

/// y10/m11/d30     v0.02   chenyong    modify
/// 1   修改 MOTO_DU_TO_POT 的宏定义, 加入强制转换为 u32
/// 2   原来1°范围认为到位减速停车, 现在改为1.5°
/// 3   增加 MOTO_HghTestIsUp() 函数, 减少报错用
/// 4   增加#define MOTO_HGH_POS_UP 10

/// y10/m08/d03     v0.01   chenyong    setup





#define  MOTO_MODULE     /// 模块定义, 用区分内外部定义


/// Includes --------------------------------------------------------------------------------------------
#include  <bsp.h>
#include  "com.h"
#include  "moto.h"

extern u8 Speed_pare[];
extern u8 HeadUpDown[BSP_DEVICE_ID_N];
extern u8 UpHeadSpeed;
/// Private define --------------------------------------------------------------------------------------

/// 1   通用定义    ------------------------------------------------------------

/// 开环电机转动一圈需要的脉冲数
#define     MOTO_N_P_PR                 1600

/// 度数转成脉冲数
#define     MOTO_DU_TO_PULSE(n)         ((n) * MOTO_N_P_PR / 360)

/// 2   剪线相关定义    --------------------------------------------------------

/// 剪线电机方向
#ifndef  PC924                                              /// for PC9511
    #define     MOTO_CUT_IN             BSP_MOT_DIR_CCW
    #define     MOTO_CUT_OUT            BSP_MOT_DIR_CW
#else                                                       /// for PC924
    #define     MOTO_CUT_IN             BSP_MOT_DIR_CCW
    #define     MOTO_CUT_OUT            BSP_MOT_DIR_CW
#endif

/// 剪线到位传感器有效电平
#define     MOTO_CUT_SWI_ON             BSP_SWITCH_LOW
#define     MOTO_CUT_SWI_OFF            BSP_SWITCH_HIGH
//#define     MOTO_CUT_SWI_ON             BSP_SWITCH_HIGH
//#define     MOTO_CUT_SWI_OFF            BSP_SWITCH_LOW


/// 没有剪线命令, 剪线任务空闲状态 (不能与现有命令重复)
#define     MOTO_CUT_CMD_NONE           ((MOTO_CUT_CMD_TypeDef)0)

/// 剪线脉冲速度
#define     MOTO_CUT_SPD_MAX_ENGINE     4
#define     MOTO_CUT_SPD_MAX_GEAR       38


//#define     MOTO_CUT_FULL_STEP_N        1333            /// 默认出刀步数, 从离开接近开关到最长点 300°
//#define     MOTO_CUT_HALF_STEP_N        356             /// 默认半出步数, 从离开接近开关到半出点  80°

#define     MOTO_CUT_LOOPER_N           400             /// 出刀与回刀步数差超过此值时, 则认为环梭没落下
u32     	MOTO_CUT_IN_OVER_N   =      750;            /// 用于检测回刀超时
#define     MOTO_CUT_OUT_OVER_N         750             /// 用于检测出刀超时
//#define     MOTO_CUT_IN_GO_ON_N         100             /// 到位后又继续收回的步数
#define     MOTO_CUT_IN_GO_ON_N         25              /// 到位后又继续收回的步数


/// 3   针高相关定义    --------------------------------------------------------

/// 针高电机方向
#ifndef  PC924                                              /// for PC9511
    #define     MOTO_HGH_DN             BSP_MOT_DIR_CW
    #define     MOTO_HGH_UP             BSP_MOT_DIR_CCW
#else                                                       /// for PC924
    #define     MOTO_HGH_DN             BSP_MOT_DIR_CW
    #define     MOTO_HGH_UP             BSP_MOT_DIR_CCW
#endif


/// 没有针高命令, 剪线任务空闲状态 (不能与现有命令重复)
#define     MOTO_HGH_CMD_NONE           ((MOTO_HGH_CMD_TypeDef)(-1))

/// 针高脉冲速度
#define     MOTO_HGH_SPD_MAX_ENGINE     4
#define     MOTO_HGH_SPD_MAX_GEAR       10

/// 针高角度相关
#define     MOTO_HGH_DU_POT_MAX         320                     /// 针高电位器最大有效度数

#ifndef  PC924                                                  /// for PC9511
    #define     MOTO_HGH_DU_MD0         12.5                    /// 针高0中间位置的度数
    #define     MOTO_HGH_DU_MD1         22.5                    /// 针高1中间位置的度数
    #define     MOTO_HGH_DU_MD2         32.5                    /// ...
    #define     MOTO_HGH_DU_MD3         42.5
    #define     MOTO_HGH_DU_MD4         52.5
    #define     MOTO_HGH_DU_MD5         62.5
    #define     MOTO_HGH_DU_MD6         72.5
    #define     MOTO_HGH_DU_MD7         82.5
    #define     MOTO_HGH_DU_MD8         92.5
    #define     MOTO_HGH_DU_MD9        102.5                    /// 针高9中间位置的度数
    #define     MOTO_HGH_DU_MD10       262.5                    /// 针高10(脱离)中间位置的度数
#else                                                           /// for PC924
    #define     MOTO_HGH_DU_MD0         56.25                   /// 针高0中间位置的度数
    #define     MOTO_HGH_DU_MD1         66.25                   /// 针高1中间位置的度数
    #define     MOTO_HGH_DU_MD2         76.25                   /// ...
    #define     MOTO_HGH_DU_MD3         86.25
    #define     MOTO_HGH_DU_MD4         96.25
    #define     MOTO_HGH_DU_MD5        106.25
    #define     MOTO_HGH_DU_MD6        116.25
    #define     MOTO_HGH_DU_MD7        126.25
    #define     MOTO_HGH_DU_MD8        136.25
    #define     MOTO_HGH_DU_MD9        146.25                   /// 针高9中间位置的度数
    #define     MOTO_HGH_DU_MD10       295                      /// 针高10(脱离)中间位置的度数
#endif

#define     MOTO_HGH_DU_DN0             5                       /// 针高0下限位置的度数
#define     MOTO_HGH_DU_UP9             (MOTO_HGH_DU_MD9 + 5)   /// 针高9上限位置的度数
#define     MOTO_HGH_DU_DN10            (MOTO_HGH_DU_MD10 - 12) /// 针高10下限位置的度数
#define     MOTO_HGH_DU_UP10            (MOTO_HGH_DU_MD10 + 12) /// 针高10上限位置的度数

/// 度数(<MOTO_HGH_DU_POT_MAX)转换为电位器AD值(16位数)
#define     MOTO_DU_TO_POT(n)           ((u32)(((n) * (1 << 16)) / MOTO_HGH_DU_POT_MAX))

/// 以下为对应的电位器值
#define     MOTO_HGH_POT_MD0            (MOTO_DU_TO_POT(MOTO_HGH_DU_MD0))
#define     MOTO_HGH_POT_MD1            (MOTO_DU_TO_POT(MOTO_HGH_DU_MD1))
#define     MOTO_HGH_POT_MD2            (MOTO_DU_TO_POT(MOTO_HGH_DU_MD2))
#define     MOTO_HGH_POT_MD3            (MOTO_DU_TO_POT(MOTO_HGH_DU_MD3))
#define     MOTO_HGH_POT_MD4            (MOTO_DU_TO_POT(MOTO_HGH_DU_MD4))
#define     MOTO_HGH_POT_MD5            (MOTO_DU_TO_POT(MOTO_HGH_DU_MD5))
#define     MOTO_HGH_POT_MD6            (MOTO_DU_TO_POT(MOTO_HGH_DU_MD6))
#define     MOTO_HGH_POT_MD7            (MOTO_DU_TO_POT(MOTO_HGH_DU_MD7))
#define     MOTO_HGH_POT_MD8            (MOTO_DU_TO_POT(MOTO_HGH_DU_MD8))
#define     MOTO_HGH_POT_MD9            (MOTO_DU_TO_POT(MOTO_HGH_DU_MD9))
#define     MOTO_HGH_POT_MD10           (MOTO_DU_TO_POT(MOTO_HGH_DU_MD10))

/// 针高0~9位置无缝连接
#define     MOTO_HGH_POT_DN0            (MOTO_DU_TO_POT(MOTO_HGH_DU_DN0))
#define     MOTO_HGH_POT_UP0            ((MOTO_HGH_POT_MD0 + MOTO_HGH_POT_MD1) / 2)
#define     MOTO_HGH_POT_UP1            ((MOTO_HGH_POT_MD1 + MOTO_HGH_POT_MD2) / 2)
#define     MOTO_HGH_POT_UP2            ((MOTO_HGH_POT_MD2 + MOTO_HGH_POT_MD3) / 2)
#define     MOTO_HGH_POT_UP3            ((MOTO_HGH_POT_MD3 + MOTO_HGH_POT_MD4) / 2)
#define     MOTO_HGH_POT_UP4            ((MOTO_HGH_POT_MD4 + MOTO_HGH_POT_MD5) / 2)
#define     MOTO_HGH_POT_UP5            ((MOTO_HGH_POT_MD5 + MOTO_HGH_POT_MD6) / 2)
#define     MOTO_HGH_POT_UP6            ((MOTO_HGH_POT_MD6 + MOTO_HGH_POT_MD7) / 2)
#define     MOTO_HGH_POT_UP7            ((MOTO_HGH_POT_MD7 + MOTO_HGH_POT_MD8) / 2)
#define     MOTO_HGH_POT_UP8            ((MOTO_HGH_POT_MD8 + MOTO_HGH_POT_MD9) / 2)
#define     MOTO_HGH_POT_UP9            (MOTO_DU_TO_POT(MOTO_HGH_DU_UP9))
#define     MOTO_HGH_POT_DN10           (MOTO_DU_TO_POT(MOTO_HGH_DU_DN10))
#define     MOTO_HGH_POT_UP10           (MOTO_DU_TO_POT(MOTO_HGH_DU_UP10))
/// 错误区
#define     MOTO_HGH_POS_ERR_DN         (MOTO_HGH_POS_NUM + 0)          /// 在小于针高0的错误区
#define     MOTO_HGH_POS_ERR_MD         (MOTO_HGH_POS_NUM + 1)          /// 在针高9和10之间的错误区
#define     MOTO_HGH_POS_ERR_UP         (MOTO_HGH_POS_NUM + 2)          /// 在大于针高10的错误区
#define     MOTO_HGH_POS_FLOAT          (MOTO_HGH_POS_NUM + 3)          /// 电位器浮空状态


/// 每步脉冲对应的AD变化值. 实际机构传动比为1:4.
#define     MOTO_HGH_POT_STEP           ((360 << 16) / MOTO_N_P_PR / MOTO_HGH_DU_POT_MAX / 4)
/// 每步滤波值. 按16倍计算. 工装传动比为1:1, 所以工装滤波为4倍
#define     MOTO_HGH_POT_STEP_FILTER    (MOTO_HGH_POT_STEP * 16)
/// 每100mS滤波值, 按10KHz计算
#define     MOTO_HGH_POT_100MS_FILTER   (MOTO_HGH_POT_STEP * 10000 / 10 / 4)

/// Private typedef -------------------------------------------------------------------------------------

typedef  void MOTO_FUNC_TypeDef (void *);






/// Private macro ---------------------------------------------------------------------------------------
/// Private function prototypes -------------------------------------------------------------------------
/// Private variables -----------------------------------------------------------------------------------

/// 1   通用变量    ------------------------------------------------------------
static  MOTO_FUNC_TypeDef  *MOTO_TaskFunc[BSP_DEVICE_ID_N];                 /// 任务函数
                                                                            /// 动作命令状态
static  MOTO_ST_TypeDef     MOTO_State[BSP_DEVICE_ID_N]     =   {MOTO_ST_OK, MOTO_ST_OK};
static  u32                 MOTO_ClkNum[BSP_DEVICE_ID_N]    =   {0, 0};     /// 脉冲个数即步数
/// cy 100909
//static  u8                  MOTO_SpdGear[BSP_DEVICE_ID_N]   =   {0, 0};   /// 速度档 决定定时间隔
/// cy 100909

static  OS_EVENT*           MOTO_SemIrqPtr[BSP_DEVICE_ID_N];                /// 定时中断通知任务用的信号量
                                                                            /// 脉冲电平状态
static  BitAction           MOTO_StepState[BSP_DEVICE_ID_N] =   {Bit_RESET, Bit_RESET};
static  u16                 MOTO_Timing[BSP_DEVICE_ID_N];                   /// 定时间隔

//static  OS_STK              MOTO_TaskMotoOddStk[APP_TASK_MOTO_STK_SIZE];  /// 奇头开环电机任务栈空间
//static  OS_STK              MOTO_TaskMotoEvenStk[APP_TASK_MOTO_STK_SIZE]; /// 偶头开环电机任务栈空间
                                                                            /// 开环电机任务栈空间
static  OS_STK              MOTO_TaskMotoStk[BSP_DEVICE_ID_N][APP_TASK_MOTO_STK_SIZE];

/// 2   剪线相关变量    --------------------------------------------------------

/// 默认出刀步数, 从离开接近开关到最长点 300°
static  u16                 MOTO_CutFullSteps               =   MOTO_DU_TO_PULSE(300);
/// 默认半出步数, 从离开接近开关到半出点  80°
static  u16                 MOTO_CutHalfSteps               =   40;//MOTO_DU_TO_PULSE(80);

static  u16                 MOTO_CutSpdEngine               =   1;              /// 1.0 time speed
static  u8                  MOTO_CutDownAcc                 =   1;              /// 降速加速度

/// 剪线任务的命令
static  MOTO_CUT_CMD_TypeDef    MOTO_CutCmd[BSP_DEVICE_ID_N]
                                    =   {MOTO_CUT_CMD_NONE, MOTO_CUT_CMD_NONE};

/// 3   针高相关定义    --------------------------------------------------------

//static  u16                 MOTO_HghSpdEngine               =   3;
static  u16                 MOTO_HghSpdEngine               =   0;
static  u8                  MOTO_HghDownAcc                 =   1;              /// 降速加速度

///// 针高电机与电位器方向相关性：
///// 1 - 正相关 电机UP对应电位器值增大; 0 - 可能无电位器 或 间隙大; -1 - 负相关
//static  s8                  MOTO_HghRelativity[BSP_DEVICE_ID_N];
//
//static  u16                 MOTO_HghPotVal[BSP_DEVICE_ID_N];                    /// 针高电位器值

static  u8                  MOTO_HghHeight[BSP_DEVICE_ID_N];                    /// 针高值 0~10,E1~3


/// 针高任务的命令
static  MOTO_HGH_CMD_TypeDef    MOTO_HghCmd[BSP_DEVICE_ID_N]
                                    =   {MOTO_HGH_CMD_NONE, MOTO_HGH_CMD_NONE};

static  OS_EVENT*               MOTO_SemCmdPtr[BSP_DEVICE_ID_N];                /// cy 100909

/// Private consts --------------------------------------------------------------------------------------
/// 开环电机定时中断用的标志
const       uint16_t        MOTO_FlagCC[BSP_DEVICE_ID_N]    =   {BSP_FLAG_CC_CLK_ODD, BSP_FLAG_CC_CLK_EVEN};
/// 开环电机定时用的比较寄存器 (地址是常量)
volatile    uint16_t* const MOTO_CCR[BSP_DEVICE_ID_N]       =   {&BSP_CCR_CLK_ODD , &BSP_CCR_CLK_EVEN};
/// 任务优先级
const       u8              MOTO_TaskPrio[BSP_DEVICE_ID_N]  =   {APP_TASK_MOTO_ODD_PRIO, APP_TASK_MOTO_EVEN_PRIO};


/// Private functions ---------------------------------------------------------
static  void                MOTO_SetSpd         (BSP_ID_TypeDef id, MOTO_USE_TypeDef use, u32 SpdGear);
static  void                MOTO_Start          (BSP_ID_TypeDef         id,
                                                 MOTO_USE_TypeDef       use,
                                                 BSP_MOT_DIR_TypeDef    dir,
                                                 BSP_MOT_TQ_TypeDef     tq,
                                                 u16                    StartTiming);
static  MOTO_ST_TypeDef     MOTO_Step           (BSP_ID_TypeDef id, MOTO_USE_TypeDef use, u32 nStep);
static  void                MOTO_Stop           (BSP_ID_TypeDef id, u16 msTQDown);

static  MOTO_ST_TypeDef     MOTO_CutStepUntil   (BSP_ID_TypeDef id, u32 nStep, BSP_SWITCH_TypeDef swi);
static  MOTO_ST_TypeDef     MOTO_CutIn          (BSP_ID_TypeDef id);
static  MOTO_ST_TypeDef     MOTO_CutOut         (BSP_ID_TypeDef id, u32 nStep);
static  MOTO_ST_TypeDef     MOTO_CutBack        (BSP_ID_TypeDef id);
static  MOTO_ST_TypeDef     MOTO_CutHalf        (BSP_ID_TypeDef id);
static  MOTO_ST_TypeDef     MOTO_CutFull        (BSP_ID_TypeDef id);
static  MOTO_ST_TypeDef     MOTO_CutD        (BSP_ID_TypeDef id);
static  MOTO_ST_TypeDef     MOTO_CutH        (BSP_ID_TypeDef id);
static  MOTO_ST_TypeDef     MOTO_CutO        (BSP_ID_TypeDef id);
static  void                MOTO_Cut            (BSP_ID_TypeDef id);

static  u8                  MOTO_HghGetFromPot  (u16 pot);
static  MOTO_ST_TypeDef     MOTO_HghStepUntil   (BSP_ID_TypeDef id, u32 nStep, u16 value);
static  MOTO_ST_TypeDef     MOTO_HghChange      (BSP_ID_TypeDef id, u8 hgh);
static  void                MOTO_Hgh            (BSP_ID_TypeDef id);


static  void                MOTO_TaskCutOdd     (void *data);
static  void                MOTO_TaskHghOdd     (void *data);
static  void                MOTO_TaskCutEven    (void *data);
static  void                MOTO_TaskHghEven    (void *data);
static  void                MOTO_TaskHghCutNoth    (void *data);
static  MOTO_ST_TypeDef     MOTO_CutOutD (BSP_ID_TypeDef id, u32 nStep);

static  u8                  MOTO_HghGetFromPot  (u16 pot);                                  /// cy 100906
static 	HOT_POS_TypeDef	 HOT_Position;
extern bool                COM_9511TangkongDir;
extern bool                COM_9511TangkongEnable[2];
HOT_POS_TypeDef     HOT_POS_Get()
{
	 return HOT_Position;
}


void  MOTO_Init (MOTO_USE_TypeDef odd, MOTO_USE_TypeDef even)
{
    u32         i;
    
  
	    if (odd == MOTO_USE_CUT) {
        MOTO_TaskFunc[BSP_ID_ODD]   = MOTO_TaskHghCutNoth;
    	} else {
        MOTO_TaskFunc[BSP_ID_ODD]   =   MOTO_TaskCutOdd;
    	}
    	if (even == MOTO_USE_CUT) {
        MOTO_TaskFunc[BSP_ID_EVEN]  =   MOTO_TaskHghCutNoth;
    	} else {
        MOTO_TaskFunc[BSP_ID_EVEN]  =   MOTO_TaskCutEven;
    	}	
	
    
    for (i = 0; i < BSP_DEVICE_ID_N; i++) {
        MOTO_SemIrqPtr[i]   =   OSSemCreate(0);                         /// 初始化中断使用的信号量
        MOTO_SemCmdPtr[i]   =   OSSemCreate(0);                         /// 初始化其他任务使用的信号量
    
        /// 建立奇数头开环电机任务
        OSTaskCreateExt((void (*)(void *)) MOTO_TaskFunc[i],            /// 任务函数入口(指针)
                        (void          * ) 0,                           /// 函数参数指针
                                                                        /// 栈顶指针
                        (OS_STK        * )&MOTO_TaskMotoStk[i][APP_TASK_MOTO_STK_SIZE - 1],
                        (u8              ) MOTO_TaskPrio[i],            /// 优先级
                        (u16             ) MOTO_TaskPrio[i],            /// ID标识, 暂与优先级相同
                        (OS_STK        * )&MOTO_TaskMotoStk[i][0],      /// 栈底指针
                        (u32             ) APP_TASK_MOTO_STK_SIZE,      /// 栈空间大小, 单位为OS_STK
                        (void          * ) 0,                           /// TCB扩展, 附属信息
                                                                        /// 操作信息
                        (u16             )(OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));
    }
}

void  MOTO_SetSpdEngine (MOTO_USE_TypeDef use, u8 engine)
{
    if (use == MOTO_USE_CUT) {
        if (engine < MOTO_CUT_SPD_MAX_ENGINE) {
            MOTO_CutSpdEngine   =   engine;
        } else {
            MOTO_CutSpdEngine   =   MOTO_CUT_SPD_MAX_ENGINE - 1;
        }
    } else if (use == MOTO_USE_HGH) {
        if (engine < MOTO_HGH_SPD_MAX_ENGINE) {
            MOTO_HghSpdEngine   =   engine;
        } else {
            MOTO_HghSpdEngine   =   MOTO_HGH_SPD_MAX_ENGINE - 1;
        }
    }
}

void  MOTO_CutSetFullAngle (u8 n10du)
{
    /// 参数为u8, 不用检查参数范围
    MOTO_CutFullSteps   =   n10du * 10 * MOTO_N_P_PR / 360;
}

void  MOTO_CutSetHalfAngle (u8 n10du)
{
    /// 参数为u8, 不用检查参数范围
    MOTO_CutHalfSteps   =   n10du * 10 * MOTO_N_P_PR / 360;
}
void  MOTO_TKSetFullAngle (u8 n10du)
{
    /// 参数为u8, 不用检查参数范围
    MOTO_CutFullSteps   =   n10du  * MOTO_N_P_PR / 360;
}

void  MOTO_TKSetHalfAngle (u8 n10du)
{
    /// 参数为u8, 不用检查参数范围
    MOTO_CutHalfSteps   =   n10du  * MOTO_N_P_PR / 360;
}
MOTO_ST_TypeDef  MOTO_CutGetState (BSP_ID_TypeDef id)
{
#if OS_CRITICAL_METHOD == 3             /// Allocate storage for CPU status register
    OS_CPU_SR  cpu_sr = 0;
#endif
    MOTO_ST_TypeDef     rt;
    
    
    if ((id >= BSP_DEVICE_ID_N) || (MOTO_CutCmd[id] != MOTO_CUT_CMD_NONE)) {
        return  MOTO_ST_ACTING;
    } else {
        /// 报告错误后, 如果无动作则报告正确. !原来协议要求
        OS_ENTER_CRITICAL();
        rt              =   MOTO_State[id];
        MOTO_State[id]  =   MOTO_ST_OK;
        OS_EXIT_CRITICAL();
        return  rt;
    }
}

bool  MOTO_CutGetIsActing (BSP_ID_TypeDef id)
{
    if (   (id >= BSP_DEVICE_ID_N)
        || (MOTO_CutCmd[id] != MOTO_CUT_CMD_NONE)
        || (MOTO_State[id] == MOTO_ST_ACTING)   ) {
        return  TRUE;
    } else {
        return  FALSE;
    }
}

void  MOTO_CutSetCmd (BSP_ID_TypeDef id, MOTO_CUT_CMD_TypeDef cmd)
{
    if (   (id < BSP_DEVICE_ID_N)
        && ((cmd == MOTO_CUT_BACK) || (cmd == MOTO_CUT_HALF) || (cmd == MOTO_CUT_FULL)|| (cmd == MOTO_CUT_D) || (cmd == MOTO_CUT_H))
        && (MOTO_CutCmd[id] == MOTO_CUT_CMD_NONE)   ) {
        MOTO_CutCmd[id] =   cmd;
        OSSemPost(MOTO_SemCmdPtr[id]);
    }
}

u8  MOTO_HghGetHeight (BSP_ID_TypeDef id)
{
    if ((id < BSP_DEVICE_ID_N) && (MOTO_HghHeight[id] < MOTO_HGH_POS_NUM)) {
        return  MOTO_HghHeight[id];
    } else {
        return  MOTO_HGH_POS_ERR;
    }
}

bool  MOTO_HghTestIsUp (BSP_ID_TypeDef id, u8 time)
{
    u8  i;
    
    
    if ((id < BSP_DEVICE_ID_N) && (time <= 20)) {
        for (i = 0; i < time; i++) {
            if (MOTO_HghGetFromPot(BSP_POT_GetOne(id)) == MOTO_HGH_POS_UP) {
                return  TRUE;
            }
        }
        return  FALSE;
    } else {
        return  TRUE;
    }
}

MOTO_ST_TypeDef  MOTO_HghGetState (BSP_ID_TypeDef id)
{    
#if OS_CRITICAL_METHOD == 3             /// Allocate storage for CPU status register
    OS_CPU_SR  cpu_sr = 0;
#endif
    MOTO_ST_TypeDef     rt;
    
    
    if ((id >= BSP_DEVICE_ID_N) || (MOTO_HghCmd[id] != MOTO_HGH_CMD_NONE)) {
        return  MOTO_ST_ACTING;
    } else {
        /// 报告错误后, 如果无动作则报告正确. !原来协议要求
        OS_ENTER_CRITICAL();
        rt              =   MOTO_State[id];
        MOTO_State[id]  =   MOTO_ST_OK;
        OS_EXIT_CRITICAL();
        return  rt;
    }
}

bool  MOTO_HghGetIsActing (BSP_ID_TypeDef id)
{
    if (   (id >= BSP_DEVICE_ID_N)
        || (MOTO_CutCmd[id] != MOTO_CUT_CMD_NONE)
        || (MOTO_State[id] == MOTO_ST_ACTING)   ) {
        return  TRUE;
    } else {
        return  FALSE;
    }
}

void  MOTO_HghSetCmd (BSP_ID_TypeDef id, MOTO_HGH_CMD_TypeDef cmd)
{
    if (   (id < BSP_DEVICE_ID_N)
        && (cmd < MOTO_HGH_POS_NUM)
        && (MOTO_HghCmd[id] == MOTO_HGH_CMD_NONE)   ) {
        MOTO_HghCmd[id] =   cmd;
        OSSemPost(MOTO_SemCmdPtr[id]);
    }
}



/// 对于剪线电机, 最快定时频率为 8M / 400        = 20 KHz,      最快脉冲频率为10K.
/// 对于针高电机, 最快定时频率为 8M / (1200 / 2) = 13.3 KHz,    最快脉冲频率为6.6K
static  void  MOTO_SetSpd (BSP_ID_TypeDef id, MOTO_USE_TypeDef use, u32 SpdGear)
{
    /// cut motor speed table
    static  const  u16  MOTO_CutSpdTab[MOTO_CUT_SPD_MAX_ENGINE][MOTO_CUT_SPD_MAX_GEAR] = {
                                                                /// 1.5 time
        7215,7213,6927,6642,6358,6078,5799,5523,5251,4983,
        4719,4461,4207,3961,3720,3486,3259,3040,2829,2626,
        2433,2248,2073,1908,1752,1606,1473,1348,1236,1135,
        1045, 967, 901, 847, 804, 774, 756, 750,
                                                                /// 1.2 time
        5772,5770,5541,5313,5086,4862,4639,4418,4201,3986,
        3775,3568,3366,3169,2976,2788,2607,2432,2263,2101,
        1946,1798,1658,1526,1401,1285,1178,1078, 988, 908,
         836, 774, 721, 678, 643, 619, 604, 600,
                                                                /// 1.0 time (old)
        4810,4809,4618,4428,4239,4052,3866,3682,3501,3322,
        3146,2974,2805,2641,2480,2324,2173,2027,1886,1751,
        1622,1499,1382,1272,1168,1071, 982, 899, 824, 757,
         697, 645, 601, 565, 536, 516, 504, 500,
                                                                /// 0.8 time
        3848,3847,3694,3542,3391,3241,3092,2945,2800,2657,
        2516,2379,2244,2112,1984,1859,1738,1621,1508,1400,
        1297,1199,1105,1017, 934, 856, 785, 719, 659, 605,
         557, 516, 480, 452, 428, 412, 403, 400,
    };
    
    static  const  u16  MOTO_HghSpdTab[MOTO_HGH_SPD_MAX_ENGINE][MOTO_HGH_SPD_MAX_GEAR] = {
        40000, 40000, 20000, 16000, 12800, 10000,  7200,  5600,  4400,  4000,
        16000, 16000, 11200,  9200,  7200,  5600,  4000,  2800,  2000,  1600,
        13332, 13332,  7200,  5600,  4000,  2800,  2000,  1600,  1400,  1332,
        13332, 13332,  7200,  5600,  4000,  2800,  2000,  1520,  1360,  1200,
    };
    
    
    if (use == MOTO_USE_CUT) {                  /// 由于原剪线定时器时钟为4MHz, 所以剪线曲线不用除2 
        if (SpdGear < MOTO_CUT_SPD_MAX_GEAR) {
            MOTO_Timing[id]     =   MOTO_CutSpdTab[MOTO_CutSpdEngine][SpdGear]*UpHeadSpeed/100;
        } else {
            MOTO_Timing[id]     =   MOTO_CutSpdTab[MOTO_CutSpdEngine][MOTO_CUT_SPD_MAX_GEAR - 1]*UpHeadSpeed/100;
        }
    } else if (use == MOTO_USE_HGH) {           /// 由于原针高定时器时钟为8MHz, 所以针高曲线除2 
        if (SpdGear < MOTO_HGH_SPD_MAX_GEAR) {
            MOTO_Timing[id]     =   (MOTO_HghSpdTab[MOTO_HghSpdEngine][SpdGear] >> 1);
        } else {
            MOTO_Timing[id]     =   (MOTO_HghSpdTab[MOTO_HghSpdEngine][MOTO_HGH_SPD_MAX_GEAR - 1] >> 1)*UpHeadSpeed/100;
        }
    }
}

static  void  MOTO_Start (BSP_ID_TypeDef        id,
                          MOTO_USE_TypeDef      use,
                          BSP_MOT_DIR_TypeDef   dir,
                          BSP_MOT_TQ_TypeDef    tq,
                          u16                   StartTiming)
{
    MOTO_ClkNum[id]     =   0;                              /// 脉冲累计数清零
    /// 初始化定时中断使用的变量 MOTO_StepState 和 MOTO_Timing
    MOTO_StepState[id]  =   Bit_RESET;                      /// 记录脉冲电平为低
    MOTO_SetSpd(id, use, 0);                                /// 设置初始速度为0档, 并初始化 MOTO_Timing
    
    BSP_MOTO_ClkSet(id, Bit_RESET);                         /// 设置脉冲电平为低
    BSP_MOTO_DirSet(id, dir);                               /// 设置方向
	//    BSP_MOTO_TQSet(id, tq);                                 /// 设置电流
   //	BSP_MOTO_MSet((BSP_ID_TypeDef)id, BSP_MOT_M_5);
    BSP_MOTO_TQSet(id, BSP_MOT_TQ_75);
    *(MOTO_CCR[id]) =   BSP_TM8M_Get() + StartTiming;       /// 设置启动时机
    TIM_ClearITPendingBit(BSP_TMR_8M, MOTO_FlagCC[id]);     /// 清中断
    TIM_ITConfig(BSP_TMR_8M, MOTO_FlagCC[id], ENABLE);      /// 打开中断
}

static  MOTO_ST_TypeDef  MOTO_Step (BSP_ID_TypeDef id, MOTO_USE_TypeDef use, u32 nStep)
{
    u32     i;
    u8      err;
    u8      acc;
    u32     SpdGear;
    
    
    /// 内部函数, 不检查参数范围
    if (use == MOTO_USE_CUT) {                              /// 取得降速加速度
        acc =   MOTO_CutDownAcc;
    } else if (use == MOTO_USE_HGH) {
        acc =   MOTO_HghDownAcc;
    } else {
        acc =   1;
    }
    
    for (i = nStep; i != 0; i--) {
        SpdGear =   ((i - 1) * acc)<<1;
        OSSemPend(MOTO_SemIrqPtr[id], 0, &err);                 /// 等待脉冲下降沿到
        MOTO_ClkNum[id]++;                                      /// 步数增加1
//        MOTO_SetSpd(id, use, MOTO_ClkNum[id]);                  /// 速度随位移增加, acc =  1
//        MOTO_SetSpd(id, use, (i - 1) * acc);                    /// 到位减速
        if (SpdGear > MOTO_ClkNum[id]) {
            SpdGear = MOTO_ClkNum[id]<<2;
        }
        MOTO_SetSpd(id, use, SpdGear);
        
        if (BSP_MOTO_StatusGet(id) == BSP_MOT_ST_ERROR) {
            return  MOTO_ST_PRO;                                /// 电机故障 TB6560错误
        }
    }
    
    return  MOTO_ST_OK;
}

static  void  MOTO_Stop  (BSP_ID_TypeDef    id,
                          u16               msTQDown)
{
    TIM_ITConfig(BSP_TMR_8M, MOTO_FlagCC[id], DISABLE);     /// 关闭中断
    BSP_MOTO_ClkSet(id, Bit_RESET);                         /// 设置脉冲电平为低
    OSTimeDlyHMSM(0, 0, 0, msTQDown);                       /// 等待(ms)
   	BSP_MOTO_TQSet(id, BSP_MOT_TQ_50);
}



static  MOTO_ST_TypeDef  MOTO_CutStepUntil (BSP_ID_TypeDef id, u32 nStep, BSP_SWITCH_TypeDef swi)
{
    u32     i;
    u8      err;
    
    
    /// 内部函数, 不检查参数范围
    for (i = nStep; i != 0; i--) {
        OSSemPend(MOTO_SemIrqPtr[id], 0, &err);                 /// 等待脉冲下降沿到
        MOTO_ClkNum[id]++;                                      /// 步数增加1
        MOTO_SetSpd(id, MOTO_USE_CUT, MOTO_ClkNum[id]);         /// 速度随位移增加, acc =  1
//        MOTO_SetSpd(id, MOTO_USE_CUT, (i - 1) * 2);             /// 到位减速度      acc = -2
        
        if (BSP_MOTO_StatusGet(id) == BSP_MOT_ST_ERROR) {
            return  MOTO_ST_PRO;                                /// 电机故障 TB6560错误
        }
        if (BSP_SWI_FiGet(id) == swi) {                         /// 目标状态
            return  MOTO_ST_ACTING;
        }
    }
    
    return  MOTO_ST_OK;
}

static  MOTO_ST_TypeDef  MOTO_CutIn (BSP_ID_TypeDef id)
{
    MOTO_ST_TypeDef     st;
    
    
    if (BSP_SWI_FiGet(id) != MOTO_CUT_SWI_ON) {                                 /// 没有到位
                                                                                /// 2/1000S后启动
        MOTO_Start(id, MOTO_USE_CUT, MOTO_CUT_IN, BSP_DIP_TQ, BSP_TMR_8M_CLK * 1 / 1000);
        
        st  =   MOTO_CutStepUntil(id, MOTO_CUT_IN_OVER_N, MOTO_CUT_SWI_ON);     /// 回刀, 直到接近开关
        switch (st) {
        case MOTO_ST_PRO:               /// TB6560错误
            break;
        case MOTO_ST_ACTING:            /// 达到接近开关
            st  =   MOTO_ST_OK;
            break;
        case MOTO_ST_OK:                /// 到达指定步数, 即超时
            st  =   MOTO_ST_OVTIME;
            break;
        default:                        /// 其他
            break;
        }
        
        if (st == MOTO_ST_OK) {         /// 如果无异常, 需要可靠回到接近开关处
            st  =   MOTO_Step(id, MOTO_USE_CUT, MOTO_CUT_IN_GO_ON_N);
        }
        
        MOTO_Stop(id, 1);               /// 停止脉冲, 4ms后设置为小电流
        
    } else {
        st  =   MOTO_ST_OK;
    }
    
    return  st;
}

static  MOTO_ST_TypeDef  MOTO_CutOut (BSP_ID_TypeDef id, u32 nStep)
{
    MOTO_ST_TypeDef     st;
    
    
    if (BSP_SWI_FiGet(id) != MOTO_CUT_SWI_ON) {                                 /// 如没有回位, 需回位
        st  =   MOTO_CutIn(id);
    } else {
        st  =   MOTO_ST_OK;
    }
    
    if (st == MOTO_ST_OK) {
                                                                                /// 2/1000S后启动
        MOTO_Start(id, MOTO_USE_CUT, MOTO_CUT_OUT, BSP_DIP_TQ, BSP_TMR_8M_CLK * 1 / 1000);
        
        st  =   MOTO_CutStepUntil(id, MOTO_CUT_OUT_OVER_N, MOTO_CUT_SWI_OFF);   /// 出刀, 直到离开接近开关
        switch (st) {
        case MOTO_ST_PRO:               /// TB6560错误
            break;
        case MOTO_ST_ACTING:            /// 离开接近开关
            st  =   MOTO_ST_OK;
            break;
        case MOTO_ST_OK:                /// 到达指定步数, 即超时
            st  =   MOTO_ST_OVTIME;
            break;
        default:                        /// 其他
            break;
        }
        
        if (st == MOTO_ST_OK) {         /// 如果无异常, 到达目标位置
            st  =   MOTO_Step(id, MOTO_USE_CUT, nStep);
        }
            
        MOTO_Stop(id, 1);               /// 停止脉冲, 4ms后设置为小电流
    }
    
    return  st;    
}
static  MOTO_ST_TypeDef  MOTO_CutOutH (BSP_ID_TypeDef id, u32 nStep)
{
    MOTO_ST_TypeDef     st;  
	st=MOTO_CutOutD(id,nStep);  
    /*if (BSP_SWI_FiGet(id) != MOTO_CUT_SWI_ON) 
	{                                 /// 如没有回位, 需回位
		MOTO_Start(id, MOTO_USE_CUT, MOTO_CUT_OUT, BSP_DIP_TQ, BSP_TMR_8M_CLK * 1 / 1000);
        st  =   MOTO_CutStepUntil(id, MOTO_CUT_OUT_OVER_N, MOTO_CUT_SWI_ON);   /// 出刀, 直到离开接近开关
        switch (st) 
		{
        case MOTO_ST_PRO:               /// TB6560错误
            break;
        case MOTO_ST_ACTING:            /// 离开接近开关
            st  =   MOTO_ST_OK;
            break;
        case MOTO_ST_OK:                /// 到达指定步数, 即超时
            st  =   MOTO_ST_OVTIME;
            break;
        default:                        /// 其他
            break;
        }
		 if (st == MOTO_ST_OK)          /// 如果无异常, 到达目标位置
            st  =   MOTO_Step(id, MOTO_USE_CUT, 10);
            
        MOTO_Stop(id, 1);               /// 停止脉冲, 4ms后设置为小电流
    }
     else 
        st  =   MOTO_ST_OK;
	if (st == MOTO_ST_OVTIME) 
	{
	    MOTO_Start(id, MOTO_USE_CUT, MOTO_CUT_IN, BSP_DIP_TQ, BSP_TMR_8M_CLK * 1 / 1000);        
        st  =   MOTO_CutStepUntil(id, MOTO_CUT_OUT_OVER_N, MOTO_CUT_SWI_ON);   /// 出刀, 直到离开接近开关
        switch (st) 
		{
        case MOTO_ST_PRO:               /// TB6560错误
            break;
        case MOTO_ST_ACTING:            /// 离开接近开关
            st  =   MOTO_ST_OK;
            break;
        case MOTO_ST_OK:                /// 到达指定步数, 即超时
            st  =   MOTO_ST_OVTIME;
            break;
        default:                        /// 其他
            break;
        }
        if (st == MOTO_ST_OK)          /// 如果无异常, 到达目标位置
            st  =   MOTO_Step(id, MOTO_USE_CUT, 200);
        MOTO_Stop(id, 1);               /// 停止脉冲, 4ms后设置为小电流
	} 
	if (st == MOTO_ST_OK) 
	{                                                                              /// 2/1000S后启动
        MOTO_Start(id, MOTO_USE_CUT, MOTO_CUT_IN, BSP_DIP_TQ, BSP_TMR_8M_CLK * 1 / 1000);
        st  =   MOTO_CutStepUntil(id, MOTO_CUT_OUT_OVER_N, MOTO_CUT_SWI_OFF);   /// 出刀, 直到离开接近开关
        switch (st) 
		{
        case MOTO_ST_PRO:               /// TB6560错误
            break;
        case MOTO_ST_ACTING:            /// 离开接近开关
            st  =   MOTO_ST_OK;
            break;
        case MOTO_ST_OK:                /// 到达指定步数, 即超时
            st  =   MOTO_ST_OVTIME;
            break;
        default:                        /// 其他
            break;
        }
        if (st == MOTO_ST_OK)          /// 如果无异常, 到达目标位置
            st  =   MOTO_Step(id, MOTO_USE_CUT, 10);    
        MOTO_Stop(id, 1);               /// 停止脉冲, 4ms后设置为小电流
   }*/
   if (st == MOTO_ST_OK) 
   { 
        MOTO_Start(id, MOTO_USE_CUT, MOTO_CUT_IN, BSP_DIP_TQ, BSP_TMR_8M_CLK * 1 / 1000);
        st  =   MOTO_CutStepUntil(id, 430, MOTO_CUT_SWI_OFF);   /// 出刀, 直到离开接近开关
        switch (st) {
        case MOTO_ST_PRO:               /// TB6560错误
            break;
        case MOTO_ST_ACTING:            /// 离开接近开关
            st  =   MOTO_ST_OK;
            break;
        case MOTO_ST_OK:                /// 到达指定步数, 即超时
            st  =   MOTO_ST_OVTIME;
            break;
        default:                        /// 其他
            break;
        }
        
        if (st == MOTO_ST_OK)          /// 如果无异常, 到达目标位置
            st  =   MOTO_Step(id, MOTO_USE_CUT, 10);     
        MOTO_Stop(id, 1);               /// 停止脉冲, 4ms后设置为小电流
   }
  if (st == MOTO_ST_OK)
  {
		MOTO_Start(id, MOTO_USE_CUT, MOTO_CUT_OUT, BSP_DIP_TQ, BSP_TMR_8M_CLK * 1 / 1000);
        st  =   MOTO_CutStepUntil(id, MOTO_CUT_OUT_OVER_N, MOTO_CUT_SWI_ON);   /// 出刀, 直到离开接近开关
        switch (st) 
		{
        case MOTO_ST_PRO:               /// TB6560错误
            break;
        case MOTO_ST_ACTING:            /// 离开接近开关
            st  =   MOTO_ST_OK;
            break;
        case MOTO_ST_OK:                /// 到达指定步数, 即超时
            st  =   MOTO_ST_OVTIME;
            break;
        default:                        /// 其他
            break;
        }
        if (st == MOTO_ST_OK)          /// 如果无异常, 到达目标位置
            st  =   MOTO_Step(id, MOTO_USE_CUT, 10);            
        MOTO_Stop(id, 1);               /// 停止脉冲, 4ms后设置为小电流
   } 
    if (st == MOTO_ST_OVTIME)
  	{
		if (BSP_SWI_FiGet(id) == MOTO_CUT_SWI_ON)
			st  =   MOTO_ST_OK;		
	}
    return  st;    
}
static  MOTO_ST_TypeDef  MOTO_CutOutO (BSP_ID_TypeDef id, u32 nStep)
{
		MOTO_ST_TypeDef     st;
       if (BSP_SWI_FiGet(id) != MOTO_CUT_SWI_ON) {                                 /// 没有到位
                                                                                /// 2/1000S后启动
        MOTO_Start(id, MOTO_USE_CUT, MOTO_CUT_IN, BSP_DIP_TQ, BSP_TMR_8M_CLK * 1 / 1000);
        
        st  =   MOTO_CutStepUntil(id, 400, MOTO_CUT_SWI_ON);     /// 回刀, 直到接近开关
        switch (st) {
        case MOTO_ST_PRO:               /// TB6560错误
            break;
        case MOTO_ST_ACTING:            /// 达到接近开关
            st  =   MOTO_ST_OK;
            break;
        case MOTO_ST_OK:                /// 到达指定步数, 即超时
            st  =   MOTO_ST_OVTIME;
            break;
        default:                        /// 其他
            break;
        }
        
        if (st == MOTO_ST_OK) 
		{         /// 如果无异常, 需要可靠回到接近开关处
            st  =   MOTO_Step(id, MOTO_USE_CUT, MOTO_CUT_IN_GO_ON_N);
			if (BSP_SWI_FiGet(id) == MOTO_CUT_SWI_ON) 
			{ 
				st  =   MOTO_CutOutD(id, MOTO_CutFullSteps);
			}
		    
        }
        
        MOTO_Stop(id, 1);               /// 停止脉冲, 4ms后设置为小电流
        
    } else {
        st  =   MOTO_ST_OK;
    }
    return  st; 
}  
static  MOTO_ST_TypeDef  MOTO_CutOutD (BSP_ID_TypeDef id, u32 nStep)
{
     MOTO_ST_TypeDef     st;
	if((COM_9511TangkongDir==FALSE)||(COM_9511TangkongEnable[id]==FALSE))
	{
	 if (BSP_SWI_FiGet(id) != MOTO_CUT_SWI_ON) 
	{                                 /// 如没有回位, 需回位
        MOTO_Start(id, MOTO_USE_CUT, MOTO_CUT_IN, BSP_DIP_TQ, BSP_TMR_8M_CLK * 1 / 1000);        
        st  =   MOTO_CutStepUntil(id, MOTO_CUT_OUT_OVER_N, MOTO_CUT_SWI_ON);   /// 出刀, 直到离开接近开关
        switch (st) 
		{
	        case MOTO_ST_PRO:               /// TB6560错误
	            break;
	        case MOTO_ST_ACTING:            /// 离开接近开关
	            st  =   MOTO_ST_OK;
	            break;
	        case MOTO_ST_OK:                /// 到达指定步数, 即超时
	            st  =   MOTO_ST_OVTIME;
	            break;
	        default:                        /// 其他
	            break;
        }
		if (st == MOTO_ST_OK)          /// 如果无异常, 到达目标位置
            st  =   MOTO_Step(id, MOTO_USE_CUT, 10);    
        MOTO_Stop(id, 1);               /// 停止脉冲, 4ms后设置为小电流
    }
    else
	{ 
	   st  =   MOTO_ST_OK;
	  // return;
	}
   }    
/*    if (BSP_SWI_FiGet(id) != MOTO_CUT_SWI_ON) 
	{                                 /// 如没有回位, 需回位
        MOTO_Start(id, MOTO_USE_CUT, MOTO_CUT_IN, BSP_DIP_TQ, BSP_TMR_8M_CLK * 1 / 1000);        
        st  =   MOTO_CutStepUntil(id, MOTO_CUT_OUT_OVER_N, MOTO_CUT_SWI_ON);   /// 出刀, 直到离开接近开关
        switch (st) 
		{
	        case MOTO_ST_PRO:               /// TB6560错误
	            break;
	        case MOTO_ST_ACTING:            /// 离开接近开关
	            st  =   MOTO_ST_OK;
	            break;
	        case MOTO_ST_OK:                /// 到达指定步数, 即超时
	            st  =   MOTO_ST_OVTIME;
	            break;
	        default:                        /// 其他
	            break;
        }
		if (st == MOTO_ST_OK)          /// 如果无异常, 到达目标位置
            st  =   MOTO_Step(id, MOTO_USE_CUT, 10);    
        MOTO_Stop(id, 1);               /// 停止脉冲, 4ms后设置为小电流
    }
    else 
	{
	   st  =   MOTO_ST_OK;
	   return;
	} */
   /* if (st == MOTO_ST_OVTIME) 
	{
		MOTO_Start(id, MOTO_USE_CUT, MOTO_CUT_OUT, BSP_DIP_TQ, BSP_TMR_8M_CLK * 1 / 1000);
        st  =   MOTO_CutStepUntil(id, MOTO_CUT_OUT_OVER_N, MOTO_CUT_SWI_ON);   /// 出刀, 直到离开接近开关
        switch (st) 
		{
        case MOTO_ST_PRO:               /// TB6560错误
            break;
        case MOTO_ST_ACTING:            /// 离开接近开关
            st  =   MOTO_ST_OK;
            break;
        case MOTO_ST_OK:                /// 到达指定步数, 即超时
            st  =   MOTO_ST_OVTIME;
            break;
        default:                        /// 其他
            break;
        }        
        if (st == MOTO_ST_OK)          /// 如果无异常, 到达目标位置
           st  =   MOTO_Step(id, MOTO_USE_CUT, 10);            
        MOTO_Stop(id, 1);               /// 停止脉冲, 4ms后设置为小电流
	} */
   // if (st == MOTO_ST_OK) 
   if (BSP_SWI_FiGet(id) == MOTO_CUT_SWI_ON)
	{                                                                                /// 2/1000S后启动
        MOTO_Start(id, MOTO_USE_CUT, MOTO_CUT_OUT, BSP_DIP_TQ, BSP_TMR_8M_CLK * 1 / 1000);
        st  =   MOTO_CutStepUntil(id, MOTO_CUT_OUT_OVER_N, MOTO_CUT_SWI_OFF);   /// 出刀, 直到离开接近开关
        switch (st) 
		{
        case MOTO_ST_PRO:               /// TB6560错误
            break;
        case MOTO_ST_ACTING:            /// 离开接近开关
            st  =   MOTO_ST_OK;
            break;
        case MOTO_ST_OK:                /// 到达指定步数, 即超时
            st  =   MOTO_ST_OVTIME;
            break;
        default:                        /// 其他
            break;
        }
        if (st == MOTO_ST_OK)          /// 如果无异常, 到达目标位置
            st  =   MOTO_Step(id, MOTO_USE_CUT, 10);    
        MOTO_Stop(id, 1);               /// 停止脉冲, 4ms后设置为小电流
	}
   if (st == MOTO_ST_OVTIME) 
	{                                                                                /// 2/1000S后启动
        MOTO_Start(id, MOTO_USE_CUT, MOTO_CUT_OUT, BSP_DIP_TQ, BSP_TMR_8M_CLK * 1 / 1000);
        st  =   MOTO_CutStepUntil(id, MOTO_CUT_OUT_OVER_N, MOTO_CUT_SWI_OFF);   /// 出刀, 直到离开接近开关
        switch (st) 
		{
        case MOTO_ST_PRO:               /// TB6560错误
            break;
        case MOTO_ST_ACTING:            /// 离开接近开关
            st  =   MOTO_ST_OK;
            break;
        case MOTO_ST_OK:                /// 到达指定步数, 即超时
            st  =   MOTO_ST_OVTIME;
            break;
        default:                        /// 其他
            break;
        }
        if (st == MOTO_ST_OK)          /// 如果无异常, 到达目标位置
            st  =   MOTO_Step(id, MOTO_USE_CUT, 10);    
        MOTO_Stop(id, 1);               /// 停止脉冲, 4ms后设置为小电流
	}
  if (st == MOTO_ST_OK)
   { 
		MOTO_Start(id, MOTO_USE_CUT, MOTO_CUT_IN, BSP_DIP_TQ, BSP_TMR_8M_CLK * 1 / 1000);        
        st  =   MOTO_CutStepUntil(id, MOTO_CUT_OUT_OVER_N, MOTO_CUT_SWI_ON);   /// 出刀, 直到离开接近开关
        switch (st) 
		{
        case MOTO_ST_PRO:               /// TB6560错误
            break;
        case MOTO_ST_ACTING:            /// 离开接近开关
            st  =   MOTO_ST_OK;
            break;
        case MOTO_ST_OK:                /// 到达指定步数, 即超时
            st  =   MOTO_ST_OVTIME;
            break;
        default:                        /// 其他
            break;
        }
        if (st == MOTO_ST_OK)          /// 如果无异常, 到达目标位置
            st  =   MOTO_Step(id, MOTO_USE_CUT, 10);
        MOTO_Stop(id, 1);               /// 停止脉冲, 4ms后设置为小电流
    }
	if(COM_9511TangkongEnable[id]==FALSE)
		COM_9511TangkongEnable[id]=TRUE;    
    return  st;    
}
static  MOTO_ST_TypeDef  MOTO_CutBack (BSP_ID_TypeDef id)
{
    return  MOTO_CutIn(id);
}
extern u8 HeadUpDown_Status[];
static  MOTO_ST_TypeDef  MOTO_CutHalf (BSP_ID_TypeDef id)
{	
	MOTO_ST_TypeDef     st;
    st= MOTO_CutOut(id, MOTO_CutHalfSteps);
	if(st   ==   MOTO_ST_OK)
		HeadUpDown_Status[id]=3;
	return st;
}

/// ! 以后加入变电流75% -> 100%
static  MOTO_ST_TypeDef  MOTO_CutFull (BSP_ID_TypeDef id)
{
    MOTO_ST_TypeDef     st;
    u32                 StepOut;
    u32                 StepIn;   
    st  =   MOTO_CutOut(id, MOTO_CutFullSteps);
    if (st == MOTO_ST_OK) {                         /// 如果无异常, 等待后收回
        StepOut =   MOTO_ClkNum[id];
        st  =   MOTO_CutIn(id);
        if (st == MOTO_ST_OK) {
            StepIn =   MOTO_ClkNum[id];
            if ((StepIn + MOTO_CUT_LOOPER_N) < StepOut) {
                st  =   MOTO_ST_CUTLOOPER;
            }
			HeadUpDown_Status[id]=1;
        }
	  
    }
    return  st;
}
static  MOTO_ST_TypeDef  MOTO_CutO(BSP_ID_TypeDef id)
{
	MOTO_ST_TypeDef     st;
    st  =   MOTO_CutOutO(id, 390);
    return  st;
}
static  MOTO_ST_TypeDef  MOTO_CutD(BSP_ID_TypeDef id)
{
	MOTO_ST_TypeDef     st;
    st  =   MOTO_CutOutD(id, MOTO_CutFullSteps);
	if(st   ==   MOTO_ST_OK)
		HeadUpDown_Status[id]=1;
    return  st;
}

static  MOTO_ST_TypeDef  MOTO_CutH (BSP_ID_TypeDef id)
{
    MOTO_ST_TypeDef     st;
	MOTO_ST_TypeDef     MotoSt[BSP_DEVICE_ID_N];
//	MotoSt[id]   =   MOTO_CutGetState((BSP_ID_TypeDef)id);
	if(HeadUpDown_Status[id]!=2)//&&(MotoSt[id]   ==   MOTO_ST_OK))
	{
    	st  =   MOTO_CutOutH(id, MOTO_CutFullSteps);
	    if(st == MOTO_ST_OK)
			HeadUpDown_Status[id]=2;
	}
	else
		st=MOTO_ST_OK;//MotoSt[id];
    return  st;
}
static  void  MOTO_Cut (BSP_ID_TypeDef id) //任务的执行函数中调用这个函数
{
    u8    err;
    for ( ; ; ) {
        OSSemPend(MOTO_SemCmdPtr[id], 0, &err);             /// 一直等待命令
        
        switch (MOTO_CutCmd[id]) {
        case MOTO_CUT_BACK:
            MOTO_State[id]  =   MOTO_ST_ACTING;             /// 也可不用此句, 由 MOTO_CUT_CMD_NONE 保证
                                                            /// 实际上, 命令后此句之前就是MOTO_ST_ACTING状态
            MOTO_State[id]  =   MOTO_CutBack(id);
            break;
        case MOTO_CUT_HALF:
            MOTO_State[id]  =   MOTO_ST_ACTING;             /// 也可不用此句, 由 MOTO_CUT_CMD_NONE 保证
            MOTO_State[id]  =   MOTO_CutHalf(id);
            break;
        case MOTO_CUT_FULL:
            MOTO_State[id]  =   MOTO_ST_ACTING;             /// 也可不用此句, 由 MOTO_CUT_CMD_NONE 保证
            MOTO_State[id]  =   MOTO_CutFull(id);
            break;
		case MOTO_CUT_H:
			MOTO_State[id]  =   MOTO_ST_ACTING;             /// 也可不用此句, 由 MOTO_CUT_CMD_NONE 保证
            MOTO_State[id]  =   MOTO_CutH(id);
			break;
		case MOTO_CUT_D:
			MOTO_State[id]  =   MOTO_ST_ACTING;             /// 也可不用此句, 由 MOTO_CUT_CMD_NONE 保证
            MOTO_State[id]  =   MOTO_CutD(id);
			break;
		case MOTO_CUT_O:
			MOTO_State[id]  =   MOTO_ST_ACTING;             /// 也可不用此句, 由 MOTO_CUT_CMD_NONE 保证
            MOTO_State[id]  =   MOTO_CutO(id);
			break;
        default:
            break;
        }
        MOTO_CutCmd[id]     =   (MOTO_CUT_CMD_TypeDef)MOTO_CUT_CMD_NONE;
    }
}

static  void  MOTO_TaskCutOdd (void *data)//任务的执行函数
{
    MOTO_Cut(BSP_ID_ODD);
}

static  void  MOTO_TaskCutEven (void *data)
{
    MOTO_Cut(BSP_ID_EVEN);
}
static  void                MOTO_TaskHghCutNoth    (void *data)
{
;
}


static  u16  MOTO_HghPotGetAve (BSP_ID_TypeDef id)
{
#ifndef  PC924                                              /// for PC9511
    return  BSP_POT_GetAve(id);
#else                                                       /// for PC924
    return  (1 << 16) - 1 - BSP_POT_GetAve(id);
#endif

}

static  u8  MOTO_HghGetFromPot (u16 pot)
{
    u8      high;
    
    
    if (pot < MOTO_HGH_POT_DN0) {
        high    =   MOTO_HGH_POS_ERR_DN;
    } else if (pot <= MOTO_HGH_POT_UP0) {
        high    =   0;
    } else if (pot <= MOTO_HGH_POT_UP1) {
        high    =   1;
    } else if (pot <= MOTO_HGH_POT_UP2) {
        high    =   2;
    } else if (pot <= MOTO_HGH_POT_UP3) {
        high    =   3;
    } else if (pot <= MOTO_HGH_POT_UP4) {
        high    =   4;
    } else if (pot <= MOTO_HGH_POT_UP5) {
        high    =   5;
    } else if (pot <= MOTO_HGH_POT_UP6) {
        high    =   6;
    } else if (pot <= MOTO_HGH_POT_UP7) {
        high    =   7;
    } else if (pot <= MOTO_HGH_POT_UP8) {
        high    =   8;
    } else if (pot <= MOTO_HGH_POT_UP9) {
        high    =   9;
    } else if (pot < MOTO_HGH_POT_DN10) {
        high    =   MOTO_HGH_POS_ERR_MD;
    } else if (pot <= MOTO_HGH_POT_UP10) {
        high    =   10;
    } else {
        high    =   MOTO_HGH_POS_ERR_UP;
    }
    
    return  high;
}
static  MOTO_ST_TypeDef  MOTO_HghStepUntil (BSP_ID_TypeDef id, u32 nStep, u16 value)
{
    u32     i;
    u8      err;
    u16     last;
    u16     now;
    s32     dislast;
    s32     disnow;
    u32     rev;
    u32     slow;
    u32     SpdGear;
    
    
    /// 内部函数, 不检查参数范围
    
    now     =   MOTO_HghPotGetAve(id);
    rev     =   0;
    slow    =   MOTO_HGH_SPD_MAX_GEAR;
    
    for (i = nStep; i != 0; i--) {
        OSSemPend(MOTO_SemIrqPtr[id], 0, &err);                 /// 等待脉冲下降沿到
        MOTO_ClkNum[id]++;                                      /// 步数增加1
        SpdGear =   (i - 1) * MOTO_HghDownAcc;
        if (SpdGear > slow) {
            SpdGear =   slow;
        }
        if (SpdGear > MOTO_ClkNum[id]) {
            SpdGear =   MOTO_ClkNum[id];
        }
        MOTO_SetSpd(id, MOTO_USE_HGH, SpdGear);                 /// 速度随位移增加, acc =  1
        
        /// 故障判断
        if (BSP_MOTO_StatusGet(id) == BSP_MOT_ST_ERROR) {
            return  MOTO_ST_PRO;                                /// 电机故障 TB6560错误
        }
        
        last    =   now;
        dislast =   last - value;
        if (dislast < 0) {
            dislast =  -dislast;
        }
        now     =   MOTO_HghPotGetAve(id);
        disnow  =   now - value;
        if (disnow < 0) {
            disnow  =  -disnow;
        }
        /// 反转判断
        if (disnow < dislast) {
            rev =   0;
        } else if (disnow == dislast) {
            ;
        } else {
            rev++;
            if (rev > 100) {
                return  MOTO_ST_REVERSE;
            }
        }
        
        /// 调速
        if (disnow < MOTO_DU_TO_POT(1.5)) {                     /// 停车位置
            MOTO_HghHeight[id]  =   MOTO_HghGetFromPot(now);
            return  MOTO_ST_ACTING;
        } else if (disnow < MOTO_DU_TO_POT(3)) {                /// 减速位置
            if (slow > MOTO_HghDownAcc) {
                slow   -=   MOTO_HghDownAcc;
            } else {
                slow    =   0;
            }
        } else {
            slow    =   MOTO_HGH_SPD_MAX_GEAR;
        }
    }
    
    return  MOTO_ST_OK;
}

static  MOTO_ST_TypeDef  MOTO_HghChange (BSP_ID_TypeDef id, u8 hgh)
{
    MOTO_ST_TypeDef         st;
    BSP_MOT_DIR_TypeDef     dir;
    u8                      HigNow;
    static  const  u16      position[MOTO_HGH_POS_NUM]  =   {
                                MOTO_HGH_POT_MD0,
                                MOTO_HGH_POT_MD1,
                                MOTO_HGH_POT_MD2,
                                MOTO_HGH_POT_MD3,
                                MOTO_HGH_POT_MD4,
                                MOTO_HGH_POT_MD5,
                                MOTO_HGH_POT_MD6,
                                MOTO_HGH_POT_MD7,
                                MOTO_HGH_POT_MD8,
                                MOTO_HGH_POT_MD9,
                                MOTO_HGH_POT_MD10
                            };
    
    
    /// 不检查参数范围
        
    HigNow  =   MOTO_HghHeight[id];
    if (hgh != HigNow) {
        if (    (HigNow == MOTO_HGH_POS_ERR_DN)
            || ((HigNow == MOTO_HGH_POS_ERR_MD) && (hgh == MOTO_HGH_POS_UP))
            || ((hgh - HigNow) > 0)    )  {
            dir =   MOTO_HGH_UP;
        } else {
            dir =   MOTO_HGH_DN;
        }
        
        MOTO_Start(id, MOTO_USE_HGH, dir, BSP_DIP_TQ, BSP_TMR_8M_CLK * 5 / 1000);
        
        st  =   MOTO_HghStepUntil(id, MOTO_N_P_PR * 4, position[hgh]);//大小齿轮的传动比是4：1，小齿轮需要转4圈
        switch (st) {
        case MOTO_ST_PRO:               /// TB6560错误
            break;
        case MOTO_ST_ACTING:            /// 达到目标位置
            st  =   MOTO_ST_OK;
            break;
        case MOTO_ST_OK:                /// 到达指定步数, 即超时
            st  =   MOTO_ST_OVTIME;
            break;
        default:                        /// 其他
            break;
        }
        
        MOTO_Stop(id, 10);              /// 停止脉冲, 10ms后设置为小电流
        
    } else {
        st  =   MOTO_ST_OK;
    }
    
    return  st;
}
static  void  MOTO_Hgh (BSP_ID_TypeDef id)
{
    u16         pot;
    u8          hgh;
    u32         wait;
    u8          err;
    u32         time;
    u32         jog;
    BitAction   st;
    bool        JogEn;
    MOTO_HGH_CMD_TypeDef  	MOTO_HghCmd_Tmp;
    pot                 =   MOTO_HghPotGetAve(id);
    hgh                 =   MOTO_HghGetFromPot(pot);
    MOTO_HghHeight[id]  =   hgh;
    
    JogEn   =   COM_JogIsEn(id);
    wait    =   1;
    for ( ; ; ) {
        OSSemPend(MOTO_SemCmdPtr[id], wait, &err);          /// 等待命令
        
        if (MOTO_HghCmd[id] < MOTO_HGH_POS_NUM) {           /// 有效命令
		if (HeadUpDown[id] ==1) 
			{ 
				HeadUpDown[id]=0;
				MOTO_HghCmd_Tmp=MOTO_HghCmd[id];

				MOTO_HghCmd[id]=(MOTO_HGH_CMD_TypeDef)10;
	            MOTO_State[id]  =   MOTO_ST_ACTING;             /// 也可不用此句, 由 MOTO_HGH_CMD_NONE 保证
	            MOTO_State[id]  =   MOTO_HghChange(id, MOTO_HghCmd[id]);
	            MOTO_HghCmd[id] =   MOTO_HGH_CMD_NONE;
	
				MOTO_HghCmd[id]=MOTO_HghCmd_Tmp;
			}
        MOTO_State[id]  =   MOTO_ST_ACTING;             /// 也可不用此句, 由 MOTO_HGH_CMD_NONE 保证
        MOTO_State[id]  =   MOTO_HghChange(id, MOTO_HghCmd[id]);
        MOTO_HghCmd[id] =   MOTO_HGH_CMD_NONE;
        } else {                                            /// 无效命令 或 超时
            pot =   MOTO_HghPotGetAve(id);                  /// 电位器电压值
            hgh =   MOTO_HghGetFromPot(pot);                /// 转为针高值
            
            if (hgh < MOTO_HGH_POS_NUM) {                   /// 正常值
                MOTO_HghHeight[id]  =   hgh;
                jog     =   8;
                time    =   8;
                wait    =   100;
                
            } else {                                        /// 异常值
                                                            /// 处理查询
                if (time != 0) {
                    time--;
                } else {
                    ;
                }
                                                            /// 处理微动, 方向不变
                JogEn   =   COM_JogIsEn(id);
                if (JogEn && (jog != 0) && (time == 0)) {
                    if (st == Bit_RESET) {
                        st  =   Bit_SET;
                    } else {
                        st  =   Bit_RESET;
                        jog--;
                    }
                } else {
                    st  =   Bit_RESET;
                }
                BSP_MOTO_ClkSet(id, st);
                                                            /// 处理等待时间 和 更新变量
                if ((time == 0) && ((jog == 0) || (JogEn == FALSE))) {
                    MOTO_HghHeight[id]  =   hgh;
                    wait    =   20;
                } else {
                    wait    =   1;
                }
            }
        }
    }
}

//void  MOTO_TaskOdd (void *data)
//{
//    if (*(MOTO_USE_TypeDef *)data == MOTO_USE_HGH) {
//        MOTO_Hgh(BSP_ID_ODD);
//    } else {
//        MOTO_Cut(BSP_ID_ODD);
//    }
////    MOTO_Hgh(BSP_ID_ODD);
//}
//
//void  MOTO_TaskEven (void *data)
//{
//    if (*(MOTO_USE_TypeDef *)data == MOTO_USE_HGH) {
//        MOTO_Hgh(BSP_ID_EVEN);
//    } else {
//        MOTO_Cut(BSP_ID_EVEN);
//    }
////    MOTO_Hgh(BSP_ID_EVEN);
//}


static  void  MOTO_TaskHghOdd (void *data)
{
    MOTO_Hgh(BSP_ID_ODD);
}

static  void  MOTO_TaskHghEven (void *data)
{
    MOTO_Hgh(BSP_ID_EVEN);
}


void  MOTO_IRQandClr (void)
{
    BitAction       st;
    u8              i;
    
    
    for (i = 0; i < BSP_DEVICE_ID_N; i++) {
        if (TIM_GetITStatus(BSP_TMR_8M, MOTO_FlagCC[i]) != RESET) { /// 如果是电机比较中断
            TIM_ClearITPendingBit(BSP_TMR_8M, MOTO_FlagCC[i]);      /// 清中断
            
            *MOTO_CCR[i] +=  MOTO_Timing[i];                        /// 设置下个中断数据
            
            if (MOTO_StepState[i] == Bit_RESET) {
                st  =   Bit_SET;
            } else {
                st  =   Bit_RESET;
                OSSemPost(MOTO_SemIrqPtr[i]);                       /// 脉冲下降沿时, 通知控制任务
            }
            BSP_MOTO_ClkSet((BSP_ID_TypeDef)i, st);                 /// 设置改变后的clk
            MOTO_StepState[i] = st;                                 /// 记录clk电平状态
        }
    }
}




/// End of file -----------------------------------------------------------------------------------------
