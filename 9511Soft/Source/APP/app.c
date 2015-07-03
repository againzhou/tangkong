
/// ----------------------------------------------------------------------------
/// File Name          : app.c
/// Description        : Ӧ�ó����ļ�
/// ----------------------------------------------------------------------------
/// History:

/// y10/m07         v0.01   chenyong    setup



/// INCLUDE FILES ---------------------------------------------------------------------------------------
#include <includes.h>
#include "../3ds/crypto_des3.h"
#include "../Key/des3key.h"

/// LOCAL DEFINES ---------------------------------------------------------------------------------------

/// LOCAL GLOBAL VARIABLES ------------------------------------------------------------------------------
static  OS_STK         App_TaskStartStk[APP_TASK_START_STK_SIZE];       /// ��ʼ����         ջ�ռ� 128
static  OS_STK         App_TaskUserIFStk[APP_TASK_USER_IF_STK_SIZE];    /// �û���Ϣ��ʾ���� ջ�ռ� 256




/// LOCAL FUNCTION PROTOTYPES ---------------------------------------------------------------------------

static  void  App_TaskCreate       (void);

static  void  App_TaskStart        (void        *p_arg);
static  void  App_TaskUserIF       (void        *p_arg);
crypto_des3_ctx_t ds3_crypto_des3_ctx_t;
unsigned char Key[24];//={0xff,0x55,0x44,0x99,0x33,0x25,0x47,0x98};
u8 ds3_key_len=8;
u8 ID_ShunXu[]={1,8,4,3,0,5,11,2,6,7,10,9};
u8 Get_D[12];
u8 Get_Result[4];
u8 TreahHold_9511=0;
u8 Secrit_For_LED=0;
extern void UID_Change2_D(u8 * dst,u8 * shunxu);
extern void Caculate_120(u8 *D,u8 *Result);
extern void Get_9511_ID(void);
extern void Get_Main_ID(void);
extern void Des3PeriKeyInit(crypto_des3_ctx_t* pPeriKey,unsigned char *Key,unsigned char KeySize);
bool ID_Sta_9511=TRUE;
u8 Stream_9511_To_Main[10];
u8 ds3_src_tmp[8]={0x00,0x00,0xff,0xff,0xFF,0xff,0xff,0xff};
u8 Confirm_status=0;
u8 Main_Securit[4];//��������
u16 Sec_Number=0;
u8 thir_status=0;//��ǰ��Ч������״̬
//u8 Speed_pare[]={100,88,76,63,55},HeadUpDown_Status=0,HeadUpDown[BSP_DEVICE_ID_N]={0,0},UpHeadSpeed=100;
//u8 Speed_pare[]={130,120,110,90,80},HeadUpDown_Status=0,HeadUpDown[BSP_DEVICE_ID_N]={0,0},UpHeadSpeed=100;
u8 Speed_pare[]={130,120,110,100,90},HeadUpDown_Status[]={0,0},HeadUpDown[BSP_DEVICE_ID_N]={0,0},UpHeadSpeed=100;
extern void COM_MainSetPssW(u16 buf[], u8 len);
extern void COM_MainErasPssW(u16 buf[], u8 len);
/// -----------------------------------------------------------------------------------------------------
/// @brief This is the standard entry point for C code.
///   It is assumed that your code will call main() once you have performed all necessary initialization.
/// @param None
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
int  main (void)
{
    INT8U       os_err;
//    u32 i;
    
//    BSP_IntDisAll();                        /// Disable all ints until we are ready to accept them
    SystemInit();
    /// if need remap vector,  NVIC_SetVectorTable(,)
  	
    OSInit();                               /// Initialize "uC/OS-II, The Real-Time Kernel"
    
    os_err = OSTaskCreateExt((void (*)(void *)) App_TaskStart,              /// ��ʼ�������ָ��
                             (void          * ) 0,                          /// ����ָ��
                                                                            /// ջ��ָ��
                             (OS_STK        * )&App_TaskStartStk[APP_TASK_START_STK_SIZE - 1],
                             (INT8U           ) APP_TASK_START_PRIO,        /// ���ȼ�
                             (INT16U          ) APP_TASK_START_PRIO,        /// ��ʶ, �������ȼ���ͬ
                             (OS_STK        * )&App_TaskStartStk[0],        /// ջ��ָ��
                             (INT32U          ) APP_TASK_START_STK_SIZE,    /// ջ�ռ��С, ��λΪOS_STK
                             (void          * ) 0,                          /// TCB��չ, ������Ϣ
                                                                            /// ������Ϣ
                             (INT16U          )(OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));

#if (OS_TASK_NAME_SIZE >= 11)                                               /// ��������
    OSTaskNameSet(APP_TASK_START_PRIO, (INT8U      *)"Start Task", &os_err);
#endif

    OSStart();                              /// Start multitasking (i.e. give control to uC/OS-II).

    return (0);
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  The startup task.  The uC/OS-II ticker should only be initialize once multitasking starts.
/// @param p_arg : Argument passed to 'App_TaskStart()' by 'OSTaskCreate()'.
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
static  void  App_TaskStart (void *p_arg)
{
    u16     rx;//,buf[10]={0x00,0x11,0xaa45,0x7788,0x9923,0xee34,0x99,0xdd,0x55,0xeeff};
    u32     i;
    
    (void)p_arg;                                            /// ʵ���޲���, ������뱨��
    
    BSP_Init();                                             /// Initialize BSP functions.
    
                                                            /// �˺�������OS����, ��ֲ���ṩ��os_cpu_c.c
    OS_CPU_SysTickInit();                                   /// Initialize the SysTick.

#if (OS_TASK_STAT_EN > 0)
    OSStatInit();                                           /// Determine CPU capacity. ͳ��
#endif
    
    ////////////////////////////////////////////////////////////
    //// �Ƿ����"OSTimeDlyHMSM(0, 0, 1, 0);" ,ʹͳ��������ʱ���ʼ��
    ////////////////////////////////////////////////////////////
    /// ���õȴ���, �乤����OSStatInit()���.
    /// ֻ����ͳ������û������ǰ, OSIdleCtrMax Ϊ0.1S�ڵ��ۼӺ�; ���к�Ϊ1mS�ڵ��ۼӺ�, ����100.
    ///  !! ����, OSIdleCtrMax û�а���ϵͳ�����жϵ�ʱ��, ʵ����Ӧ�ø���. 
    ///     ���ϵͳ���Ĺ��Ӻ���ռ�ô���ʱ��, ��CPUʹ����ͳ�Ʋ�׼ȷ.
    ///  !! ����, OSIdleCtrMax ���ܵ���ʱ���Ż�Ӱ��(1.4��). ����, Ҳ��ʹ���ʲ����Ż�Ӱ��.
    ///  ͳ������ִ������Ϊ0.1S
    
    
    C491_Init();                    /// ������, ʹ���ж�
    IN_Init();                      /// ������ɨ������
    HDLED_Init();                   /// ����������
    SECURITY_9511_STAT=FALSE;
	MAIN_SECURITY_STAT=FALSE;
//                                    /// ���������ʼ�� �� �ϻ�
//    if (DBG_IsDebug() == FALSE) {   /// ���ϻ�, ��������
//        if (BSP_DIP_BDFunc == BSP_DIP_BD_FUNC_UP) {
//            MOTO_Init(MOTO_USE_HGH, MOTO_USE_HGH);
//        } else {
//            MOTO_Init(MOTO_USE_CUT, MOTO_USE_CUT);
//        }
//    } else {
////        MOTO_Init(MOTO_USE_HGH, MOTO_USE_HGH);
//        if (BSP_DIP_BDFunc == BSP_DIP_BD_FUNC_UP) {
//            MOTO_Init(MOTO_USE_HGH, MOTO_USE_HGH);
//        } else {
//            MOTO_Init(MOTO_USE_CUT, MOTO_USE_CUT);
//        }
//        DBG_ALL(8);                 /// �ϻ�8Сʱ
//    }
    
//    DBG_IsDebug();              /// !!!!!!!! TEST
//    
//    if (BSP_DIP_BDFunc == BSP_DIP_BD_FUNC_UP) {
//        MOTO_Init(MOTO_USE_HGH, MOTO_USE_HGH);
//        if (DBG_IsDebug() != FALSE) {
//            DBG_ALL(8);                 /// �ϻ�8Сʱ
//        }
//    } else {
//        MOTO_Init(MOTO_USE_CUT, MOTO_USE_CUT);
//    }

    if (DBG_IsDebug() == FALSE) {       /// ���ϻ�, ��������
        if (BSP_DIP_BDFunc == BSP_DIP_BD_FUNC_UP) {
            MOTO_Init(MOTO_USE_HGH, MOTO_USE_HGH);
        } else {
            MOTO_Init(MOTO_USE_CUT, MOTO_USE_CUT);
        }
    } else {                            /// �ϻ�
        BSP_DIP_BDFunc  =   BSP_DIP_BD_FUNC_UP;
        MOTO_Init(MOTO_USE_HGH, MOTO_USE_HGH);
        DBG_ALL(8);                     /// 8Сʱ
    }
	//memcpy(Key, "012345678901234567890123", sizeof(Key));
	Des3PeriKeyInit(&ds3_crypto_des3_ctx_t,Key,sizeof(Key));
  	//crypto_des3_set_key(&ds3_crypto_des3_ctx_t, Key, sizeof(Key));
   	UID_Change2_D(Get_D,ID_ShunXu);	
	Caculate_120(Get_D,Get_Result);
	Get_9511_ID();
	Get_Main_ID();
//	COM_MainSetPssW(buf, 10);
//	COM_MainErasPssW(buf, 10);
	if(SECURITY_9511_STAT==TRUE)//���9511�����ˣ�����Ȩ�޽��ͣ��ȵ����ط���֤����������Ȩ��
	{
		TreahHold_9511=5;		
	}
	//���9511�����ܣ�����������0������Ҫ���������޳�ʼ��Ϊ0������ָ�������
 /*	if((Get_Result[0]!=PRO_9511_ID[0])||(Get_Result[1]!=PRO_9511_ID[1])||(Get_Result[3]!=PRO_9511_ID[3])||(Get_Result[2]!=PRO_9511_ID[2]))
		{
			ID_Sta_9511=FALSE;
			while(1)
			{
				for(i=0;i<0xA0000;i++)
					i++;
				BSP_LED_Toggle(1);
			}
		} */ 
 if(SECURITY_9511_STAT==TRUE)
 {
 	TreahHold_9511=5;
	Secrit_For_LED=1;
 }
    App_TaskCreate();                                       /// Create application tasks.
    
    
    COM_Init();
    
    
    
    for (i = 0; i < BSP_LED_ID_N; i++) {
       // IO_Init(io_id_led[i], GPIO_Mode_Out_OD);
        BSP_LED_Set(i, BSP_LED_OFF);
    }
    while (TRUE) {                                      /// Task body, infinite loop
//        BSP_LED_Toggle(1);
////        OSTimeDlyHMSM(0, 0, 0, 500);
//        OSTimeDlyHMSM(0, 0, 0, (BSP_POT_GetAve(BSP_ID_ODD) + BSP_POT_GetAve(BSP_ID_EVEN)) >> 6);
        
//        COM_Init();
//        COM_9511();

        rx  =   C491_ReceiveOne(100);
        if (rx != C491_R_OVRETIME_NODATA) {
            COM_RxHandle(rx);
        }
        COM_KeyHandle();
    if (BSP_DIP_RedIsSpkl)
		COM_DXSparkel();
    }
}


/// -----------------------------------------------------------------------------------------------------
/// @brief  Create the application tasks.
/// @param None
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
static  void  App_TaskCreate (void)
{
    INT8U       os_err;
    
    
    /// �����û���Ϣ��ʾ����, ������
    os_err = OSTaskCreateExt((void (*)(void *)) App_TaskUserIF,
                             (void          * ) 0,
                             (OS_STK        * )&App_TaskUserIFStk[APP_TASK_USER_IF_STK_SIZE - 1],
                             (INT8U           ) APP_TASK_USER_IF_PRIO,
                             (INT16U          ) APP_TASK_USER_IF_PRIO,
                             (OS_STK        * )&App_TaskUserIFStk[0],
                             (INT32U          ) APP_TASK_USER_IF_STK_SIZE,
                             (void          * ) 0,
                             (INT16U          )(OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));
    
#if (OS_TASK_NAME_SIZE > 12)
    OSTaskNameSet(APP_TASK_USER_IF_PRIO, "User IF Task", &os_err);
#endif
    
}




static  u32  App_GetDelay (void)
{
    static  const   u8      TaskPrio[]  =   {
        APP_TASK_START_PRIO,
        APP_TASK_MOTO_ODD_PRIO,
        APP_TASK_MOTO_EVEN_PRIO,
        APP_TASK_IN_SCAN_PRIO,
        APP_TASK_HDLED_ODD_PRIO,
        APP_TASK_HDLED_EVEN_PRIO,
        APP_TASK_USER_IF_PRIO,
    };
    
    u32             i;
    OS_STK_DATA     StkData;
    
    
    for (i = 0; i < (sizeof(TaskPrio) / sizeof(TaskPrio[0])); i++) {
        if ((OSTaskStkChk(TaskPrio[i], &StkData) != OS_ERR_NONE) || (StkData.OSFree < StkData.OSUsed)) {
            return  250;
        }
    }
    
    if (BSP_Is924) {
        return  500;
    } else {
        return  750;
    }
}

/// -----------------------------------------------------------------------------------------------------
/// @brief Updates LCD.
/// @param p_arg : Argument passed to 'App_TaskUserIF()' by 'OSTaskCreate()'.
/// @retval : None. This is a task, never returns.
/// -----------------------------------------------------------------------------------------------------
static  void  App_TaskUserIF (void *p_arg)
{
	 u32 id;
    (void)p_arg;                        /// ʵ���޲���, ������뱨��
   
    for ( ; ; ) {
        OSTimeDlyHMSM(0, 0, 0, App_GetDelay());
        BSP_LED_Toggle(0);
		if(Secrit_For_LED!=0) //���벻һ��
		{
			BSP_LED_Toggle(1);
			for(id=0;id<BSP_DEVICE_ID_N;id++)
				HDLED_Sparkle((BSP_ID_TypeDef)id,1 << 30);
		}

    }
}










/*
*********************************************************************************************************
*********************************************************************************************************
*                                          uC/OS-II APP HOOKS
*********************************************************************************************************
*********************************************************************************************************
*/
/// ���º���ΪӦ�ó���ʹ�õĹ��Ӻ���
/// ��Ҫ����̽����

#if (OS_APP_HOOKS_EN > 0)
/*
*********************************************************************************************************
*                                      TASK CREATION HOOK (APPLICATION)
*
* Description : This function is called when a task is created.
*
* Argument(s) : ptcb   is a pointer to the task control block of the task being created.
*
* Note(s)     : (1) Interrupts are disabled during this call.
*********************************************************************************************************
*/

void  App_TaskCreateHook (OS_TCB *ptcb)
{
    
}

/*
*********************************************************************************************************
*                                    TASK DELETION HOOK (APPLICATION)
*
* Description : This function is called when a task is deleted.
*
* Argument(s) : ptcb   is a pointer to the task control block of the task being deleted.
*
* Note(s)     : (1) Interrupts are disabled during this call.
*********************************************************************************************************
*/

void  App_TaskDelHook (OS_TCB *ptcb)
{
    (void)ptcb;
}

/*
*********************************************************************************************************
*                                      IDLE TASK HOOK (APPLICATION)
*
* Description : This function is called by OSTaskIdleHook(), which is called by the idle task.  This hook
*               has been added to allow you to do such things as STOP the CPU to conserve power.
*
* Argument(s) : none.
*
* Note(s)     : (1) Interrupts are enabled during this call.
*********************************************************************************************************
*/

#if OS_VERSION >= 251
void  App_TaskIdleHook (void)
{
    
}
#endif

/*
*********************************************************************************************************
*                                        STATISTIC TASK HOOK (APPLICATION)
*
* Description : This function is called by OSTaskStatHook(), which is called every second by uC/OS-II's
*               statistics task.  This allows your application to add functionality to the statistics task.
*
* Argument(s) : none.
*********************************************************************************************************
*/

void  App_TaskStatHook (void)
{
    
}

/*
*********************************************************************************************************
*                                        TASK SWITCH HOOK (APPLICATION)
*
* Description : This function is called when a task switch is performed.  This allows you to perform other
*               operations during a context switch.
*
* Argument(s) : none.
*
* Note(s)     : (1) Interrupts are disabled during this call.
*
*               (2) It is assumed that the global pointer 'OSTCBHighRdy' points to the TCB of the task that
*                   will be 'switched in' (i.e. the highest priority task) and, 'OSTCBCur' points to the
*                  task being switched out (i.e. the preempted task).
*********************************************************************************************************
*/

#if OS_TASK_SW_HOOK_EN > 0
void  App_TaskSwHook (void)
{
    
}
#endif

/*
*********************************************************************************************************
*                                     OS_TCBInit() HOOK (APPLICATION)
*
* Description : This function is called by OSTCBInitHook(), which is called by OS_TCBInit() after setting
*               up most of the TCB.
*
* Argument(s) : ptcb    is a pointer to the TCB of the task being created.
*
* Note(s)     : (1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/

#if OS_VERSION >= 204
void  App_TCBInitHook (OS_TCB *ptcb)
{
    (void)ptcb;
}
#endif

/*
*********************************************************************************************************
*                                        TICK HOOK (APPLICATION)
*
* Description : This function is called every tick.
*
* Argument(s) : none.
*
* Note(s)     : (1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/

#if OS_TIME_TICK_HOOK_EN > 0
u32  Tick_cnt =   OS_TICKS_PER_SEC;
void  App_TimeTickHook (void)
{
   #if OS_CRITICAL_METHOD == 3             /// Allocate storage for CPU status register
    OS_CPU_SR  cpu_sr = 0;
#endif
    
    
    Tick_cnt--;
    if (Tick_cnt == 0) {
        Tick_cnt =   OS_TICKS_PER_SEC/2;
        OS_ENTER_CRITICAL();
        BSP_SOL_TmrProc();
        OS_EXIT_CRITICAL();
    } 
}
#endif
#endif


/// End of file -----------------------------------------------------------------------------------------
