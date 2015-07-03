
/// ----------------------------------------------------------------------------
/// File Name          : in.c
/// Description        : 输入任务模块
///     此模块主要用于断线检测 和 机头开关去抖
/// ----------------------------------------------------------------------------
/// History:

/// y11/m02/d26     v0.02   chenyong    modify
/// 断线检测标准原来为20%, 为避免误报, 改为5%

/// y10/m08/d18     v0.01   chenyong    setup





#define  IN_MODULE     /// 模块定义, 用区分内外部定义


/// Includes --------------------------------------------------------------------------------------------
#include  <ucos_ii.h>               /// OS核的头文件
#include  "in.h"


/// Private define --------------------------------------------------------------------------------------
#define     IN_TEN_0                    BSP_SWITCH_LOW      /// 小张力状态时接近开关电平为低
//#define     IN_TEN_0                    BSP_SWITCH_HIGH     /// 小张力状态时接近开关电平为高

/// 张力采样采用固定频率 最大频率为 OS_TICKS_PER_SEC
#define     IN_SAMPLE_PERIOD_TEN        1
#define     IN_SAMPLE_PERIOD_HDKEY      10

//#define     IN_TEN_MULTIPLE             100                 /// 张力放大倍数
//#define     IN_TEN_VAL_MAX              1                   /// 张力单次采样最大值
u8 IN_TEN_MULTIPLE=20; 
//#define     IN_BRK_LEVEL                5                   /// 断线检测标准(原板为33%)
u8 IN_BRK_LEVEL=1;

/// Private typedef -------------------------------------------------------------------------------------
/// Private macro ---------------------------------------------------------------------------------------
/// Private function prototypes -------------------------------------------------------------------------
/// Private variables -----------------------------------------------------------------------------------

///// 下面两个变量与机头板功能共同决定断线检测是否工作
//static  bool    IN_BrkIsEnable[BSP_DEVICE_ID_N] =   {TRUE , TRUE };          /// 断线检测是否使能
/// 此变量与机头板功能共同决定断线检测是否工作

/// !!! cy 100908
//static  bool    IN_BrkTypeIsDn[BSP_DEVICE_ID_N] =   {FALSE, FALSE};          /// 断线检测是否使用下断线检测方式
static  bool    IN_BrkTypeIsDn                  =   FALSE;                  /// 断线检测是否使用下断线检测方式
/// !!! cy 100908

/// 主轴是否转动决定张力采样是否使能, 线夹信号也可使能张力采样
static  bool    IN_BrkIsSample[BSP_DEVICE_ID_N] =   {FALSE, FALSE};         /// 张力采样是否使能

//static  u32     IN_TenSumFi[BSP_DEVICE_ID_N]    =   {0, 0};                  /// 张力采样和值, 上断线检测方式
//static  u32     IN_TenNumFi[BSP_DEVICE_ID_N]    =   {0, 0};                  /// 张力采样数量, 上断线检测方式
//static  bool    IN_BrkIsFi[BSP_DEVICE_ID_N]     =   {FALSE, FALSE};          /// 是否断线    , 上断线检测方式
//
//static  u32     IN_TenSumSe[BSP_DEVICE_ID_N]    =   {0, 0};                  /// 张力采样和值, 下断线检测方式
//static  u32     IN_TenNumSe[BSP_DEVICE_ID_N]    =   {0, 0};                  /// 张力采样数量, 下断线检测方式
//static  bool    IN_BrkIsSe[BSP_DEVICE_ID_N]     =   {FALSE, FALSE};          /// 是否断线    , 下断线检测方式

static  u32     IN_TenSum[BSP_DEVICE_ID_N]      =   {0, 0};                 /// 张力采样和值
static  u32     IN_TenNum[BSP_DEVICE_ID_N]      =   {0, 0};                 /// 张力采样数量
static  bool    IN_BrkIsBroken[BSP_DEVICE_ID_N] =   {FALSE, FALSE};         /// 是否断线


static  BSP_HD_KEY_TypeDef  IN_HDKeyUpLast[BSP_DEVICE_ID_N] =   {BSP_HD_KEY_OFF, BSP_HD_KEY_OFF};
static  BSP_HD_KEY_TypeDef  IN_HDKeyUpNow[BSP_DEVICE_ID_N]  =   {BSP_HD_KEY_OFF, BSP_HD_KEY_OFF};
static  BSP_HD_KEY_TypeDef  IN_HDKeyDnLast[BSP_DEVICE_ID_N] =   {BSP_HD_KEY_OFF, BSP_HD_KEY_OFF};
static  BSP_HD_KEY_TypeDef  IN_HDKeyDnNow[BSP_DEVICE_ID_N]  =   {BSP_HD_KEY_OFF, BSP_HD_KEY_OFF};

static  OS_STK              IN_TaskScanStk[APP_TASK_IN_SCAN_STK_SIZE];

/// Private consts --------------------------------------------------------------------------------------
                                

/// Private functions ---------------------------------------------------------
static  u8      IN_TenGetOne    (BSP_ID_TypeDef id);
static  void    IN_TenSample    (BSP_ID_TypeDef id);
static  void    IN_TaskScan     (void          *data);
static  bool    IN_GetBrkIsWork (BSP_ID_TypeDef id);





void IN_Init(void)
{
    /// 建立输入扫描任务
    OSTaskCreateExt((void (*)(void *)) IN_TaskScan,                 /// 任务函数入口(指针)
                    (void          * ) 0,                           /// 函数参数指针
                                                                    /// 栈顶指针
                    (OS_STK        * )&IN_TaskScanStk[APP_TASK_IN_SCAN_STK_SIZE - 1],
                    (u8              ) APP_TASK_IN_SCAN_PRIO,       /// 优先级
                    (u16             ) APP_TASK_IN_SCAN_PRIO,       /// ID标识, 暂与优先级相同
                    (OS_STK        * )&IN_TaskScanStk[0],           /// 栈底指针
                    (u32             ) APP_TASK_IN_SCAN_STK_SIZE,   /// 栈空间大小, 单位为OS_STK
                    (void          * ) 0,                           /// TCB扩展, 附属信息
                                                                    /// 操作信息
                    (u16             )(OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));
}


/// cy 100909
///// ! 下机头板不使用此口, 所以不需要此函数
///// ! 以后下机头板如果使用此口, 可另外编写函数, 而只上机头调用此函数
//void  IN_HDKey_Get (BSP_ID_TypeDef id, BSP_HD_KEY_TypeDef *up, BSP_HD_KEY_TypeDef *dn)
//{
//    if (id < BSP_DEVICE_ID_N) {
//        if ((IN_HDKeyUpLast[id] == BSP_HD_KEY_ON) && (IN_HDKeyUpNow[id] == BSP_HD_KEY_ON)) {
//            *up =   BSP_HD_KEY_ON;
//        } else {
//            *up =   BSP_HD_KEY_OFF;
//        }
//        if ((IN_HDKeyDnLast[id] == BSP_HD_KEY_ON) && (IN_HDKeyDnNow[id] == BSP_HD_KEY_ON)) {
//            *dn =   BSP_HD_KEY_ON;
//        } else {
//            *dn =   BSP_HD_KEY_OFF;
//        }
//    } else {
//        *up =   BSP_HD_KEY_OFF;
//        *dn =   BSP_HD_KEY_OFF;
//    }
//}
/// cy 100909

/// ! 下机头板不使用此口, 所以不需要此函数
/// ! 以后下机头板如果使用此口, 可另外编写函数, 而只上机头调用此函数
HDKEY_ST_TypeDef  IN_HDKey_GetSt (BSP_ID_TypeDef id)
{
    BSP_HD_KEY_TypeDef  up;
    BSP_HD_KEY_TypeDef  dn;
    
    
    if (id < BSP_DEVICE_ID_N) {//连续两次进入到采集拨码开关的状态后，两次的值一样才能够认为拨码开关的状态发生改变
        if ((IN_HDKeyUpLast[id] == BSP_HD_KEY_ON) && (IN_HDKeyUpNow[id] == BSP_HD_KEY_ON)) {
            up  =   BSP_HD_KEY_ON;
        } else {
            up  =   BSP_HD_KEY_OFF;
        }
        if ((IN_HDKeyDnLast[id] == BSP_HD_KEY_ON) && (IN_HDKeyDnNow[id] == BSP_HD_KEY_ON)) {
            dn  =   BSP_HD_KEY_ON;
        } else {
            dn  =   BSP_HD_KEY_OFF;
        }
    } else {
        up  =   BSP_HD_KEY_OFF;
        dn  =   BSP_HD_KEY_OFF;
    }
    return  (HDKEY_ST_TypeDef)((up << HDKEY_ST_BIT_UP) | (dn << HDKEY_ST_BIT_DN));
}

/// 断线检测方式是否下断检
/// !!! cy 100908
//void  IN_BRK_SetIsDn (BSP_ID_TypeDef id, bool IsDn)
//{
//    if (id < BSP_DEVICE_ID_N) {
//        IN_BrkTypeIsDn[id]  =   IsDn;
//    }
//}
void  IN_BRK_SetIsDn (bool IsDn)
{
    IN_BrkTypeIsDn  =   IsDn;
}
/// !!! cy 100908

///// 断线检测是否使能
//void  IN_BRK_SetIsEn (BSP_ID_TypeDef id, bool IsEn)
//{
//    if (id < BSP_DEVICE_ID_N) {
//        IN_BrkIsEnable[id]  =   IsEn;
//    }
//}

/// 主轴启动时调用, 张力采样线使能
void  IN_BRK_SampleBegin (void)
{
#if OS_CRITICAL_METHOD == 3             /// Allocate storage for CPU status register
    OS_CPU_SR  cpu_sr = 0;
#endif
    u32     i;
    
    
    for (i = 0; i < BSP_DEVICE_ID_N; i++) {
        OS_ENTER_CRITICAL();
        IN_TenSum[i]        =   0;
        IN_TenNum[i]        =   0;
        IN_BrkIsBroken[i]   =   FALSE;
        IN_BrkIsSample[i]   =   TRUE;
        OS_EXIT_CRITICAL();
    }
}

/// 主控发线夹信号时调用
/// 主要更新 IN_BrkIsBroken, IN_TenSum, IN_TenNum
/// 如果采样没有使能, 可以使能采样
void  IN_BRK_SentXJ (void)//三次线夹进一次这个函数
{
#if OS_CRITICAL_METHOD == 3             /// Allocate storage for CPU status register
    OS_CPU_SR  cpu_sr = 0;
#endif
    u32     i;
    
    
    for (i = 0; i < BSP_DEVICE_ID_N; i++) {
        OS_ENTER_CRITICAL();
        if (IN_GetBrkIsWork((BSP_ID_TypeDef)i)) {               /// 断线检测工作
            if (IN_BrkIsSample[i]) {                            /// 张力采样使能
//在任务中一直是：      断线是加0，不断线每次加1     100            每次都会加1       5     /// 张力值小于断线检测标准
                if (IN_TenSum[i] * IN_TEN_MULTIPLE < IN_TenNum[i] * IN_BRK_LEVEL) {
                    IN_BrkIsBroken[i]   =   TRUE;               /// 认为断线是
                } else {                                        /// 否则(包括采样数为0)
                    IN_BrkIsBroken[i]   =   FALSE;              /// 认为断线否
                }
            } else {                                            /// 张力采样禁止
                IN_BrkIsBroken[i]   =   FALSE;                  /// 认为断线否
                IN_BrkIsSample[i]   =   TRUE;                   /// 张力采样使能
            }
        } else {                                                /// 断线检测没有工作
            IN_BrkIsBroken[i]   =   FALSE;                      /// 认为断线否
        }
        IN_TenSum[i]    =   0;
        IN_TenNum[i]    =   0;
        OS_EXIT_CRITICAL();
    }
}

/// 主轴停止时调用
void  IN_BRK_SampleStop (void)
{
#if OS_CRITICAL_METHOD == 3             /// Allocate storage for CPU status register
    OS_CPU_SR  cpu_sr = 0;
#endif
    u32     i;
    
    
    for (i = 0; i < BSP_DEVICE_ID_N; i++) {
        OS_ENTER_CRITICAL();
        IN_TenSum[i]        =   0;
        IN_TenNum[i]        =   0;
        IN_BrkIsBroken[i]   =   FALSE;//是否断线，对于下断检，如果停车就FALSE，那么停车后再查就已经清0了。
        IN_BrkIsSample[i]   =   FALSE;//不断线采样
        OS_EXIT_CRITICAL();
    }
}
//
bool  IN_BRK_GetIsBrk (BSP_ID_TypeDef id)
{
#if OS_CRITICAL_METHOD == 3             /// Allocate storage for CPU status register
    OS_CPU_SR  cpu_sr = 0;
#endif
    bool    rt;
    
    if (id < BSP_DEVICE_ID_N) {
        /// 报告错误后, 如果无动作则报告正确. !原来协议要求
        OS_ENTER_CRITICAL();
        rt                  =   IN_BrkIsBroken[id];
        IN_BrkIsBroken[id]  =   FALSE;
        OS_EXIT_CRITICAL();
    } else {
        rt                  =   FALSE;
    }
    
    return  rt;
}



/// 断线检测是否需要工作
static  bool  IN_GetBrkIsWork (BSP_ID_TypeDef id)
{
//    u8      dn;
//    u8      up;
//    
//    
//    dn  =   (IN_BrkTypeIsDn[id] == TRUE ) && (BSP_DIP_BDFunc == BSP_DIP_BD_FUNC_DN);
//    up  =   (IN_BrkTypeIsDn[id] == FALSE) && (BSP_DIP_BDFunc == BSP_DIP_BD_FUNC_UP);
//    
//    if (IN_BrkIsEnable[id] && (dn || up)) {

/// !!! cy 100908
//    if (   ((IN_BrkTypeIsDn[id] == TRUE ) && (BSP_DIP_BDFunc == BSP_DIP_BD_FUNC_DN))
//        || ((IN_BrkTypeIsDn[id] == FALSE) && (BSP_DIP_BDFunc == BSP_DIP_BD_FUNC_UP))   ) {
    if (   ((IN_BrkTypeIsDn == TRUE ) && (BSP_DIP_BDFunc == BSP_DIP_BD_FUNC_DN))
        || ((IN_BrkTypeIsDn == FALSE) && (BSP_DIP_BDFunc == BSP_DIP_BD_FUNC_UP))   ) {
/// !!! cy 100908

        return  TRUE;
    } else {
        return  FALSE;
    }
}

static  u8  IN_TenGetOne (BSP_ID_TypeDef id)
{
    BSP_SWITCH_TypeDef      swi;
    
    
/// !!! cy 100908
//    if (IN_BrkTypeIsDn[id] == FALSE) {      /// 上断线检测方式
    if (IN_BrkTypeIsDn == FALSE) {          /// 上断线检测方式
/// !!! cy 100908

        swi =   BSP_SWI_FiGet(id);
    } else {                                /// 下断线检测方式
        swi =   BSP_SWI_SeGet(id);
    }
    
    if (swi == IN_TEN_0) {//等于1，传递进CPU是高电平，表示断线
        return  0;
    } else {
        return  1;
    }
}

static  void  IN_TenSample (BSP_ID_TypeDef id)
{
#if OS_CRITICAL_METHOD == 3             /// Allocate storage for CPU status register
    OS_CPU_SR  cpu_sr = 0;
#endif
    u8      ten;
    
    
    ten =   IN_TenGetOne(id);//这个函数只是返回0、1，断线返回0，没有断线返回1
    
    OS_ENTER_CRITICAL();
    if ((IN_TenSum[id] <= ((u32)-1) - ten) && (IN_TenNum[id] <= ((u32)-1) - 1)) {
        IN_TenSum[id]  +=   ten; //	断线是加0
        IN_TenNum[id]  +=   1;//次数加1
    }
    OS_EXIT_CRITICAL();
}

static  void  IN_TaskScan (void *data)
{
    u32                 nTen    =   0;
    u32                 nHDKey  =   0;
    u32                 i;
    
    
    data = data;                                            /// 避免编译警告: 参数没有使用
    
    for (;;) {                                              /// 任务无限循环, 不返回
        OSTimeDlyHMSM(0, 0, 0, 1);                          /// 执行周期为1ms
        
        nTen++;
        if (nTen >= IN_SAMPLE_PERIOD_TEN) {
            nTen    =   0;
            for (i = 0; i < BSP_DEVICE_ID_N; i++) {//在主控初始化的时候传递了主控断检模式，所以IN_GetBrkIsWork能够确定状态
                if (IN_GetBrkIsWork((BSP_ID_TypeDef)i) && (IN_BrkIsSample[i])) {
                    IN_TenSample((BSP_ID_TypeDef)i);	  //主轴启动、三次线夹就能够IN_BrkIsSample变TRUE
                }//这里只有计算10次采样值，并没有IN_BrkIsBroken[i]变量操作
            }
        }
        
        nHDKey++;
        if (nHDKey >= IN_SAMPLE_PERIOD_HDKEY) {
            nHDKey  =   0;
            for (i = 0; i < BSP_DEVICE_ID_N; i++) {
                IN_HDKeyUpLast[i]   =   IN_HDKeyUpNow[i];
                IN_HDKeyDnLast[i]   =   IN_HDKeyDnNow[i];
                BSP_HD_KeyGet((BSP_ID_TypeDef)i, &IN_HDKeyUpNow[i], &IN_HDKeyDnNow[i]);
            }
        }
    }
}





/// End of file -----------------------------------------------------------------------------------------
