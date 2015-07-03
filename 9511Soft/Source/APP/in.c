
/// ----------------------------------------------------------------------------
/// File Name          : in.c
/// Description        : ��������ģ��
///     ��ģ����Ҫ���ڶ��߼�� �� ��ͷ����ȥ��
/// ----------------------------------------------------------------------------
/// History:

/// y11/m02/d26     v0.02   chenyong    modify
/// ���߼���׼ԭ��Ϊ20%, Ϊ������, ��Ϊ5%

/// y10/m08/d18     v0.01   chenyong    setup





#define  IN_MODULE     /// ģ�鶨��, ���������ⲿ����


/// Includes --------------------------------------------------------------------------------------------
#include  <ucos_ii.h>               /// OS�˵�ͷ�ļ�
#include  "in.h"


/// Private define --------------------------------------------------------------------------------------
#define     IN_TEN_0                    BSP_SWITCH_LOW      /// С����״̬ʱ�ӽ����ص�ƽΪ��
//#define     IN_TEN_0                    BSP_SWITCH_HIGH     /// С����״̬ʱ�ӽ����ص�ƽΪ��

/// �����������ù̶�Ƶ�� ���Ƶ��Ϊ OS_TICKS_PER_SEC
#define     IN_SAMPLE_PERIOD_TEN        1
#define     IN_SAMPLE_PERIOD_HDKEY      10

//#define     IN_TEN_MULTIPLE             100                 /// �����Ŵ���
//#define     IN_TEN_VAL_MAX              1                   /// �������β������ֵ
u8 IN_TEN_MULTIPLE=20; 
//#define     IN_BRK_LEVEL                5                   /// ���߼���׼(ԭ��Ϊ33%)
u8 IN_BRK_LEVEL=1;

/// Private typedef -------------------------------------------------------------------------------------
/// Private macro ---------------------------------------------------------------------------------------
/// Private function prototypes -------------------------------------------------------------------------
/// Private variables -----------------------------------------------------------------------------------

///// ���������������ͷ�幦�ܹ�ͬ�������߼���Ƿ���
//static  bool    IN_BrkIsEnable[BSP_DEVICE_ID_N] =   {TRUE , TRUE };          /// ���߼���Ƿ�ʹ��
/// �˱������ͷ�幦�ܹ�ͬ�������߼���Ƿ���

/// !!! cy 100908
//static  bool    IN_BrkTypeIsDn[BSP_DEVICE_ID_N] =   {FALSE, FALSE};          /// ���߼���Ƿ�ʹ���¶��߼�ⷽʽ
static  bool    IN_BrkTypeIsDn                  =   FALSE;                  /// ���߼���Ƿ�ʹ���¶��߼�ⷽʽ
/// !!! cy 100908

/// �����Ƿ�ת���������������Ƿ�ʹ��, �߼��ź�Ҳ��ʹ����������
static  bool    IN_BrkIsSample[BSP_DEVICE_ID_N] =   {FALSE, FALSE};         /// ���������Ƿ�ʹ��

//static  u32     IN_TenSumFi[BSP_DEVICE_ID_N]    =   {0, 0};                  /// ����������ֵ, �϶��߼�ⷽʽ
//static  u32     IN_TenNumFi[BSP_DEVICE_ID_N]    =   {0, 0};                  /// ������������, �϶��߼�ⷽʽ
//static  bool    IN_BrkIsFi[BSP_DEVICE_ID_N]     =   {FALSE, FALSE};          /// �Ƿ����    , �϶��߼�ⷽʽ
//
//static  u32     IN_TenSumSe[BSP_DEVICE_ID_N]    =   {0, 0};                  /// ����������ֵ, �¶��߼�ⷽʽ
//static  u32     IN_TenNumSe[BSP_DEVICE_ID_N]    =   {0, 0};                  /// ������������, �¶��߼�ⷽʽ
//static  bool    IN_BrkIsSe[BSP_DEVICE_ID_N]     =   {FALSE, FALSE};          /// �Ƿ����    , �¶��߼�ⷽʽ

static  u32     IN_TenSum[BSP_DEVICE_ID_N]      =   {0, 0};                 /// ����������ֵ
static  u32     IN_TenNum[BSP_DEVICE_ID_N]      =   {0, 0};                 /// ������������
static  bool    IN_BrkIsBroken[BSP_DEVICE_ID_N] =   {FALSE, FALSE};         /// �Ƿ����


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
    /// ��������ɨ������
    OSTaskCreateExt((void (*)(void *)) IN_TaskScan,                 /// ���������(ָ��)
                    (void          * ) 0,                           /// ��������ָ��
                                                                    /// ջ��ָ��
                    (OS_STK        * )&IN_TaskScanStk[APP_TASK_IN_SCAN_STK_SIZE - 1],
                    (u8              ) APP_TASK_IN_SCAN_PRIO,       /// ���ȼ�
                    (u16             ) APP_TASK_IN_SCAN_PRIO,       /// ID��ʶ, �������ȼ���ͬ
                    (OS_STK        * )&IN_TaskScanStk[0],           /// ջ��ָ��
                    (u32             ) APP_TASK_IN_SCAN_STK_SIZE,   /// ջ�ռ��С, ��λΪOS_STK
                    (void          * ) 0,                           /// TCB��չ, ������Ϣ
                                                                    /// ������Ϣ
                    (u16             )(OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));
}


/// cy 100909
///// ! �»�ͷ�岻ʹ�ô˿�, ���Բ���Ҫ�˺���
///// ! �Ժ��»�ͷ�����ʹ�ô˿�, �������д����, ��ֻ�ϻ�ͷ���ô˺���
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

/// ! �»�ͷ�岻ʹ�ô˿�, ���Բ���Ҫ�˺���
/// ! �Ժ��»�ͷ�����ʹ�ô˿�, �������д����, ��ֻ�ϻ�ͷ���ô˺���
HDKEY_ST_TypeDef  IN_HDKey_GetSt (BSP_ID_TypeDef id)
{
    BSP_HD_KEY_TypeDef  up;
    BSP_HD_KEY_TypeDef  dn;
    
    
    if (id < BSP_DEVICE_ID_N) {//�������ν��뵽�ɼ����뿪�ص�״̬�����ε�ֵһ�����ܹ���Ϊ���뿪�ص�״̬�����ı�
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

/// ���߼�ⷽʽ�Ƿ��¶ϼ�
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

///// ���߼���Ƿ�ʹ��
//void  IN_BRK_SetIsEn (BSP_ID_TypeDef id, bool IsEn)
//{
//    if (id < BSP_DEVICE_ID_N) {
//        IN_BrkIsEnable[id]  =   IsEn;
//    }
//}

/// ��������ʱ����, ����������ʹ��
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

/// ���ط��߼��ź�ʱ����
/// ��Ҫ���� IN_BrkIsBroken, IN_TenSum, IN_TenNum
/// �������û��ʹ��, ����ʹ�ܲ���
void  IN_BRK_SentXJ (void)//�����߼н�һ���������
{
#if OS_CRITICAL_METHOD == 3             /// Allocate storage for CPU status register
    OS_CPU_SR  cpu_sr = 0;
#endif
    u32     i;
    
    
    for (i = 0; i < BSP_DEVICE_ID_N; i++) {
        OS_ENTER_CRITICAL();
        if (IN_GetBrkIsWork((BSP_ID_TypeDef)i)) {               /// ���߼�⹤��
            if (IN_BrkIsSample[i]) {                            /// ��������ʹ��
//��������һֱ�ǣ�      �����Ǽ�0��������ÿ�μ�1     100            ÿ�ζ����1       5     /// ����ֵС�ڶ��߼���׼
                if (IN_TenSum[i] * IN_TEN_MULTIPLE < IN_TenNum[i] * IN_BRK_LEVEL) {
                    IN_BrkIsBroken[i]   =   TRUE;               /// ��Ϊ������
                } else {                                        /// ����(����������Ϊ0)
                    IN_BrkIsBroken[i]   =   FALSE;              /// ��Ϊ���߷�
                }
            } else {                                            /// ����������ֹ
                IN_BrkIsBroken[i]   =   FALSE;                  /// ��Ϊ���߷�
                IN_BrkIsSample[i]   =   TRUE;                   /// ��������ʹ��
            }
        } else {                                                /// ���߼��û�й���
            IN_BrkIsBroken[i]   =   FALSE;                      /// ��Ϊ���߷�
        }
        IN_TenSum[i]    =   0;
        IN_TenNum[i]    =   0;
        OS_EXIT_CRITICAL();
    }
}

/// ����ֹͣʱ����
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
        IN_BrkIsBroken[i]   =   FALSE;//�Ƿ���ߣ������¶ϼ죬���ͣ����FALSE����ôͣ�����ٲ���Ѿ���0�ˡ�
        IN_BrkIsSample[i]   =   FALSE;//�����߲���
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
        /// ��������, ����޶����򱨸���ȷ. !ԭ��Э��Ҫ��
        OS_ENTER_CRITICAL();
        rt                  =   IN_BrkIsBroken[id];
        IN_BrkIsBroken[id]  =   FALSE;
        OS_EXIT_CRITICAL();
    } else {
        rt                  =   FALSE;
    }
    
    return  rt;
}



/// ���߼���Ƿ���Ҫ����
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
//    if (IN_BrkTypeIsDn[id] == FALSE) {      /// �϶��߼�ⷽʽ
    if (IN_BrkTypeIsDn == FALSE) {          /// �϶��߼�ⷽʽ
/// !!! cy 100908

        swi =   BSP_SWI_FiGet(id);
    } else {                                /// �¶��߼�ⷽʽ
        swi =   BSP_SWI_SeGet(id);
    }
    
    if (swi == IN_TEN_0) {//����1�����ݽ�CPU�Ǹߵ�ƽ����ʾ����
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
    
    
    ten =   IN_TenGetOne(id);//�������ֻ�Ƿ���0��1�����߷���0��û�ж��߷���1
    
    OS_ENTER_CRITICAL();
    if ((IN_TenSum[id] <= ((u32)-1) - ten) && (IN_TenNum[id] <= ((u32)-1) - 1)) {
        IN_TenSum[id]  +=   ten; //	�����Ǽ�0
        IN_TenNum[id]  +=   1;//������1
    }
    OS_EXIT_CRITICAL();
}

static  void  IN_TaskScan (void *data)
{
    u32                 nTen    =   0;
    u32                 nHDKey  =   0;
    u32                 i;
    
    
    data = data;                                            /// ������뾯��: ����û��ʹ��
    
    for (;;) {                                              /// ��������ѭ��, ������
        OSTimeDlyHMSM(0, 0, 0, 1);                          /// ִ������Ϊ1ms
        
        nTen++;
        if (nTen >= IN_SAMPLE_PERIOD_TEN) {
            nTen    =   0;
            for (i = 0; i < BSP_DEVICE_ID_N; i++) {//�����س�ʼ����ʱ�򴫵������ضϼ�ģʽ������IN_GetBrkIsWork�ܹ�ȷ��״̬
                if (IN_GetBrkIsWork((BSP_ID_TypeDef)i) && (IN_BrkIsSample[i])) {
                    IN_TenSample((BSP_ID_TypeDef)i);	  //���������������߼о��ܹ�IN_BrkIsSample��TRUE
                }//����ֻ�м���10�β���ֵ����û��IN_BrkIsBroken[i]��������
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
