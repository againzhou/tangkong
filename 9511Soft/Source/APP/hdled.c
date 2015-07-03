
/// ----------------------------------------------------------------------------
/// File Name          : hdled.c
/// Description        : 机头指示灯控制模块
/// ----------------------------------------------------------------------------
/// History:

/// y10/m08/d25     v0.01   chenyong    setup




#define  HDLED_MODULE       /// 模块定义, 用区分内外部定义


/// Includes --------------------------------------------------------------------------------------------
#include  <ucos_ii.h>               /// OS核的头文件
#include  "hdled.h"



/// Private define --------------------------------------------------------------------------------------
#define     HDLED_SPARKLE_MS            100

/// Private typedef -------------------------------------------------------------------------------------
typedef  void  HDLED_FUNC_TypeDef (void *);

/// Private macro ---------------------------------------------------------------------------------------
/// Private function prototypes -------------------------------------------------------------------------
/// Private variables -----------------------------------------------------------------------------------
static  HDLED_FUNC_TypeDef *HDLED_TaskFunc[BSP_DEVICE_ID_N];                            /// 任务函数
static  OS_STK              HDLED_TaskStk[BSP_DEVICE_ID_N][APP_TASK_HDLED_STK_SIZE];    /// 任务栈空间
static  BSP_LED_TypeDef     HDLED_StateRed[BSP_DEVICE_ID_N];                            /// 红灯状态
static  BSP_LED_TypeDef     HDLED_StateGre[BSP_DEVICE_ID_N];                            /// 绿灯状态

static  s32                 HDLED_SparklingTimes[BSP_DEVICE_ID_N];

/// Private consts --------------------------------------------------------------------------------------
const   BSP_ID_TypeDef      HDLED_TaskData[BSP_DEVICE_ID_N]                             /// 任务参数
                                =   {BSP_ID_ODD, BSP_ID_EVEN};
const   u8                  HDLED_TaskPrio[BSP_DEVICE_ID_N]                             /// 任务优先级
                                =   {APP_TASK_HDLED_ODD_PRIO, APP_TASK_HDLED_EVEN_PRIO};

/// Private functions ---------------------------------------------------------
static  void    HDLED_TaskSparkling (void *data);





void  HDLED_Init (void)
{
    u32         i;
    
    
    for (i = 0; i < BSP_DEVICE_ID_N; i++) {
        HDLED_StateRed[i]   =   BSP_LED_OFF;                            /// 红灯状态: 关闭
        HDLED_StateGre[i]   =   BSP_LED_ON;                             /// 绿灯状态: 打开
        
        HDLED_TaskFunc[i]   =   HDLED_TaskSparkling;
        
        /// 建立奇数头开环电机任务
        OSTaskCreateExt((void (*)(void *)) HDLED_TaskFunc[i],           /// 任务函数入口(指针)
                        (void          * )&HDLED_TaskData[i],           /// 函数参数指针
                                                                        /// 栈顶指针
                        (OS_STK        * )&HDLED_TaskStk[i][APP_TASK_HDLED_STK_SIZE - 1],
                        (u8              ) HDLED_TaskPrio[i],           /// 优先级
                        (u16             ) HDLED_TaskPrio[i],           /// ID标识, 暂与优先级相同
                        (OS_STK        * )&HDLED_TaskStk[i][0],         /// 栈底指针
                        (u32             ) APP_TASK_HDLED_STK_SIZE,     /// 栈空间大小, 单位为OS_STK
                        (void          * ) 0,                           /// TCB扩展, 附属信息
                                                                        /// 操作信息
                        (u16             )(OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));
    }
}

HDLED_ST_TypeDef  HDLED_Get (BSP_ID_TypeDef id)
{
    if (id < BSP_DEVICE_ID_N) {
        return (HDLED_ST_TypeDef)((HDLED_StateRed[id] << HDLED_ST_BIT_RED)
                                | (HDLED_StateGre[id] << HDLED_ST_BIT_GRE));
    } else {
        return (HDLED_ST_OFF);
    }
}
extern   bool                COM_9511IsTset;
void  HDLED_Set (BSP_ID_TypeDef id, HDLED_ST_TypeDef st)
{
	if(COM_9511IsTset  ==   FALSE	)
    if (id < BSP_DEVICE_ID_N) {
        HDLED_StateRed[id]  =   (BSP_LED_TypeDef)((st >> HDLED_ST_BIT_RED) & 1);
        HDLED_StateGre[id]  =   (BSP_LED_TypeDef)((st >> HDLED_ST_BIT_GRE) & 1);
        
        BSP_HD_LedSet(id, HDLED_StateRed[id], HDLED_StateGre[id]);
    }
}

void  HDLED_Sparkle (BSP_ID_TypeDef id, s32 times)
{
    if (id < BSP_DEVICE_ID_N) {
        HDLED_SparklingTimes[id]    =   times;
        if (times > 0) {
        OSTaskResume(HDLED_TaskPrio[id]);

        }
    }
}


static  void  HDLED_TaskSparkling (void *data)
{
#if OS_CRITICAL_METHOD == 3
    OS_CPU_SR       cpu_sr = 0;
#endif
    BSP_ID_TypeDef  id;
    
    
    id  =   *((BSP_ID_TypeDef*)data);
    
    for ( ; ; ) {
        
        OS_ENTER_CRITICAL();
        if (HDLED_SparklingTimes[id] > 0) {
            HDLED_SparklingTimes[id]--;
            OS_EXIT_CRITICAL();
        
            BSP_HD_LedSet(id, BSP_LED_ON , BSP_LED_OFF);
            OSTimeDlyHMSM(0, 0, 0, HDLED_SPARKLE_MS);
            BSP_HD_LedSet(id, BSP_LED_OFF, BSP_LED_OFF);
            OSTimeDlyHMSM(0, 0, 0, HDLED_SPARKLE_MS);
            
        } else {
            OS_EXIT_CRITICAL();
            
        BSP_HD_LedSet(id, HDLED_StateRed[id], HDLED_StateGre[id]);
            OSTaskSuspend(HDLED_TaskPrio[id]);

    }
    }
}








/// End of file -----------------------------------------------------------------------------------------
