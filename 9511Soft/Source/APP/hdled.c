
/// ----------------------------------------------------------------------------
/// File Name          : hdled.c
/// Description        : ��ͷָʾ�ƿ���ģ��
/// ----------------------------------------------------------------------------
/// History:

/// y10/m08/d25     v0.01   chenyong    setup




#define  HDLED_MODULE       /// ģ�鶨��, ���������ⲿ����


/// Includes --------------------------------------------------------------------------------------------
#include  <ucos_ii.h>               /// OS�˵�ͷ�ļ�
#include  "hdled.h"



/// Private define --------------------------------------------------------------------------------------
#define     HDLED_SPARKLE_MS            100

/// Private typedef -------------------------------------------------------------------------------------
typedef  void  HDLED_FUNC_TypeDef (void *);

/// Private macro ---------------------------------------------------------------------------------------
/// Private function prototypes -------------------------------------------------------------------------
/// Private variables -----------------------------------------------------------------------------------
static  HDLED_FUNC_TypeDef *HDLED_TaskFunc[BSP_DEVICE_ID_N];                            /// ������
static  OS_STK              HDLED_TaskStk[BSP_DEVICE_ID_N][APP_TASK_HDLED_STK_SIZE];    /// ����ջ�ռ�
static  BSP_LED_TypeDef     HDLED_StateRed[BSP_DEVICE_ID_N];                            /// ���״̬
static  BSP_LED_TypeDef     HDLED_StateGre[BSP_DEVICE_ID_N];                            /// �̵�״̬

static  s32                 HDLED_SparklingTimes[BSP_DEVICE_ID_N];

/// Private consts --------------------------------------------------------------------------------------
const   BSP_ID_TypeDef      HDLED_TaskData[BSP_DEVICE_ID_N]                             /// �������
                                =   {BSP_ID_ODD, BSP_ID_EVEN};
const   u8                  HDLED_TaskPrio[BSP_DEVICE_ID_N]                             /// �������ȼ�
                                =   {APP_TASK_HDLED_ODD_PRIO, APP_TASK_HDLED_EVEN_PRIO};

/// Private functions ---------------------------------------------------------
static  void    HDLED_TaskSparkling (void *data);





void  HDLED_Init (void)
{
    u32         i;
    
    
    for (i = 0; i < BSP_DEVICE_ID_N; i++) {
        HDLED_StateRed[i]   =   BSP_LED_OFF;                            /// ���״̬: �ر�
        HDLED_StateGre[i]   =   BSP_LED_ON;                             /// �̵�״̬: ��
        
        HDLED_TaskFunc[i]   =   HDLED_TaskSparkling;
        
        /// ��������ͷ�����������
        OSTaskCreateExt((void (*)(void *)) HDLED_TaskFunc[i],           /// ���������(ָ��)
                        (void          * )&HDLED_TaskData[i],           /// ��������ָ��
                                                                        /// ջ��ָ��
                        (OS_STK        * )&HDLED_TaskStk[i][APP_TASK_HDLED_STK_SIZE - 1],
                        (u8              ) HDLED_TaskPrio[i],           /// ���ȼ�
                        (u16             ) HDLED_TaskPrio[i],           /// ID��ʶ, �������ȼ���ͬ
                        (OS_STK        * )&HDLED_TaskStk[i][0],         /// ջ��ָ��
                        (u32             ) APP_TASK_HDLED_STK_SIZE,     /// ջ�ռ��С, ��λΪOS_STK
                        (void          * ) 0,                           /// TCB��չ, ������Ϣ
                                                                        /// ������Ϣ
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
