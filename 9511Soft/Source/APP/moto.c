
/// ----------------------------------------------------------------------------
/// File Name          : moto.c
/// Description        : �����������
///     ���߳��ֹ�14��, m = 1, d = mz = 14mm, �ܳ� = 14pi =  Լ44mm
///     ������ߵ����ԭ���෴, �����궨�� MOTO_CUT_IN �� MOTO_CUT_OUT
///     ��ߵ��ͬ��
/// ----------------------------------------------------------------------------
/// History:

/// y11/m03/d06     v0.06   chenyong    modify
/// �������ر�ԭ9511��С, ������Ҫ�����˶��ٶ�.
/// �� MOTO_HghSpdEngine ����Ϊ 2, ԭ��Ϊ 3

/// y11/m01/d18     v0.05   chenyong    modify
/// �����жϺ��� MOTO_IRQandClr(), ������Ӧ�Ķ�

/// y10/m12/d22     v0.04   chenyong    modify
/// 1   ���� com.h, ��Ϊ�����亯���ж��ܷ�΢��
/// 2   �Լ��ߺ���ߵ������, ���λ��ֵ, ����PC924��������
///     ȥ��ȫ�ֱ��� MOTO_HghRelativity[] �� MOTO_HghPotVal[]
///     ���Ӻ��� MOTO_HghPotGetAve(), ��ΪPC924�ĵ�λ����PC9511�෴
/// 3   ���Ӻ��� MOTO_CutGetIsActing() �� MOTO_HghGetIsActing()
///     ��Ϊʹ�� MOTO_CutGetState() �� MOTO_HghGetState() ��Ѵ���״̬���, ���²�����
/// 4   �޸ĺ��� MOTO_Start(), ʹ����˶�ǰ�����ƽΪ��
/// 5   �޸ĺ��� MOTO_HghStepUntil(), ʹ����ǰ����, ���ٵ�λ
/// 6   �޸ĺ��� MOTO_HghChange(), ���ڶ���16��
/// 7   �޸ĺ��� MOTO_Hgh(), ����΢������

/// y10/m11/d30     v0.03   chenyong    modify
/// �޸�10��߷�Χ, ԭ��Ϊ+-6, ��Ϊ+-12

/// y10/m11/d30     v0.02   chenyong    modify
/// 1   �޸� MOTO_DU_TO_POT �ĺ궨��, ����ǿ��ת��Ϊ u32
/// 2   ԭ��1�㷶Χ��Ϊ��λ����ͣ��, ���ڸ�Ϊ1.5��
/// 3   ���� MOTO_HghTestIsUp() ����, ���ٱ�����
/// 4   ����#define MOTO_HGH_POS_UP 10

/// y10/m08/d03     v0.01   chenyong    setup





#define  MOTO_MODULE     /// ģ�鶨��, ���������ⲿ����


/// Includes --------------------------------------------------------------------------------------------
#include  <bsp.h>
#include  "com.h"
#include  "moto.h"

extern u8 Speed_pare[];
extern u8 HeadUpDown[BSP_DEVICE_ID_N];
extern u8 UpHeadSpeed;
/// Private define --------------------------------------------------------------------------------------

/// 1   ͨ�ö���    ------------------------------------------------------------

/// �������ת��һȦ��Ҫ��������
#define     MOTO_N_P_PR                 1600

/// ����ת��������
#define     MOTO_DU_TO_PULSE(n)         ((n) * MOTO_N_P_PR / 360)

/// 2   ������ض���    --------------------------------------------------------

/// ���ߵ������
#ifndef  PC924                                              /// for PC9511
    #define     MOTO_CUT_IN             BSP_MOT_DIR_CCW
    #define     MOTO_CUT_OUT            BSP_MOT_DIR_CW
#else                                                       /// for PC924
    #define     MOTO_CUT_IN             BSP_MOT_DIR_CCW
    #define     MOTO_CUT_OUT            BSP_MOT_DIR_CW
#endif

/// ���ߵ�λ��������Ч��ƽ
#define     MOTO_CUT_SWI_ON             BSP_SWITCH_LOW
#define     MOTO_CUT_SWI_OFF            BSP_SWITCH_HIGH
//#define     MOTO_CUT_SWI_ON             BSP_SWITCH_HIGH
//#define     MOTO_CUT_SWI_OFF            BSP_SWITCH_LOW


/// û�м�������, �����������״̬ (���������������ظ�)
#define     MOTO_CUT_CMD_NONE           ((MOTO_CUT_CMD_TypeDef)0)

/// ���������ٶ�
#define     MOTO_CUT_SPD_MAX_ENGINE     4
#define     MOTO_CUT_SPD_MAX_GEAR       38


//#define     MOTO_CUT_FULL_STEP_N        1333            /// Ĭ�ϳ�������, ���뿪�ӽ����ص���� 300��
//#define     MOTO_CUT_HALF_STEP_N        356             /// Ĭ�ϰ������, ���뿪�ӽ����ص������  80��

#define     MOTO_CUT_LOOPER_N           400             /// ������ص����������ֵʱ, ����Ϊ����û����
u32     	MOTO_CUT_IN_OVER_N   =      750;            /// ���ڼ��ص���ʱ
#define     MOTO_CUT_OUT_OVER_N         750             /// ���ڼ�������ʱ
//#define     MOTO_CUT_IN_GO_ON_N         100             /// ��λ���ּ����ջصĲ���
#define     MOTO_CUT_IN_GO_ON_N         25              /// ��λ���ּ����ջصĲ���


/// 3   �����ض���    --------------------------------------------------------

/// ��ߵ������
#ifndef  PC924                                              /// for PC9511
    #define     MOTO_HGH_DN             BSP_MOT_DIR_CW
    #define     MOTO_HGH_UP             BSP_MOT_DIR_CCW
#else                                                       /// for PC924
    #define     MOTO_HGH_DN             BSP_MOT_DIR_CW
    #define     MOTO_HGH_UP             BSP_MOT_DIR_CCW
#endif


/// û���������, �����������״̬ (���������������ظ�)
#define     MOTO_HGH_CMD_NONE           ((MOTO_HGH_CMD_TypeDef)(-1))

/// ��������ٶ�
#define     MOTO_HGH_SPD_MAX_ENGINE     4
#define     MOTO_HGH_SPD_MAX_GEAR       10

/// ��߽Ƕ����
#define     MOTO_HGH_DU_POT_MAX         320                     /// ��ߵ�λ�������Ч����

#ifndef  PC924                                                  /// for PC9511
    #define     MOTO_HGH_DU_MD0         12.5                    /// ���0�м�λ�õĶ���
    #define     MOTO_HGH_DU_MD1         22.5                    /// ���1�м�λ�õĶ���
    #define     MOTO_HGH_DU_MD2         32.5                    /// ...
    #define     MOTO_HGH_DU_MD3         42.5
    #define     MOTO_HGH_DU_MD4         52.5
    #define     MOTO_HGH_DU_MD5         62.5
    #define     MOTO_HGH_DU_MD6         72.5
    #define     MOTO_HGH_DU_MD7         82.5
    #define     MOTO_HGH_DU_MD8         92.5
    #define     MOTO_HGH_DU_MD9        102.5                    /// ���9�м�λ�õĶ���
    #define     MOTO_HGH_DU_MD10       262.5                    /// ���10(����)�м�λ�õĶ���
#else                                                           /// for PC924
    #define     MOTO_HGH_DU_MD0         56.25                   /// ���0�м�λ�õĶ���
    #define     MOTO_HGH_DU_MD1         66.25                   /// ���1�м�λ�õĶ���
    #define     MOTO_HGH_DU_MD2         76.25                   /// ...
    #define     MOTO_HGH_DU_MD3         86.25
    #define     MOTO_HGH_DU_MD4         96.25
    #define     MOTO_HGH_DU_MD5        106.25
    #define     MOTO_HGH_DU_MD6        116.25
    #define     MOTO_HGH_DU_MD7        126.25
    #define     MOTO_HGH_DU_MD8        136.25
    #define     MOTO_HGH_DU_MD9        146.25                   /// ���9�м�λ�õĶ���
    #define     MOTO_HGH_DU_MD10       295                      /// ���10(����)�м�λ�õĶ���
#endif

#define     MOTO_HGH_DU_DN0             5                       /// ���0����λ�õĶ���
#define     MOTO_HGH_DU_UP9             (MOTO_HGH_DU_MD9 + 5)   /// ���9����λ�õĶ���
#define     MOTO_HGH_DU_DN10            (MOTO_HGH_DU_MD10 - 12) /// ���10����λ�õĶ���
#define     MOTO_HGH_DU_UP10            (MOTO_HGH_DU_MD10 + 12) /// ���10����λ�õĶ���

/// ����(<MOTO_HGH_DU_POT_MAX)ת��Ϊ��λ��ADֵ(16λ��)
#define     MOTO_DU_TO_POT(n)           ((u32)(((n) * (1 << 16)) / MOTO_HGH_DU_POT_MAX))

/// ����Ϊ��Ӧ�ĵ�λ��ֵ
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

/// ���0~9λ���޷�����
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
/// ������
#define     MOTO_HGH_POS_ERR_DN         (MOTO_HGH_POS_NUM + 0)          /// ��С�����0�Ĵ�����
#define     MOTO_HGH_POS_ERR_MD         (MOTO_HGH_POS_NUM + 1)          /// �����9��10֮��Ĵ�����
#define     MOTO_HGH_POS_ERR_UP         (MOTO_HGH_POS_NUM + 2)          /// �ڴ������10�Ĵ�����
#define     MOTO_HGH_POS_FLOAT          (MOTO_HGH_POS_NUM + 3)          /// ��λ������״̬


/// ÿ�������Ӧ��AD�仯ֵ. ʵ�ʻ���������Ϊ1:4.
#define     MOTO_HGH_POT_STEP           ((360 << 16) / MOTO_N_P_PR / MOTO_HGH_DU_POT_MAX / 4)
/// ÿ���˲�ֵ. ��16������. ��װ������Ϊ1:1, ���Թ�װ�˲�Ϊ4��
#define     MOTO_HGH_POT_STEP_FILTER    (MOTO_HGH_POT_STEP * 16)
/// ÿ100mS�˲�ֵ, ��10KHz����
#define     MOTO_HGH_POT_100MS_FILTER   (MOTO_HGH_POT_STEP * 10000 / 10 / 4)

/// Private typedef -------------------------------------------------------------------------------------

typedef  void MOTO_FUNC_TypeDef (void *);






/// Private macro ---------------------------------------------------------------------------------------
/// Private function prototypes -------------------------------------------------------------------------
/// Private variables -----------------------------------------------------------------------------------

/// 1   ͨ�ñ���    ------------------------------------------------------------
static  MOTO_FUNC_TypeDef  *MOTO_TaskFunc[BSP_DEVICE_ID_N];                 /// ������
                                                                            /// ��������״̬
static  MOTO_ST_TypeDef     MOTO_State[BSP_DEVICE_ID_N]     =   {MOTO_ST_OK, MOTO_ST_OK};
static  u32                 MOTO_ClkNum[BSP_DEVICE_ID_N]    =   {0, 0};     /// �������������
/// cy 100909
//static  u8                  MOTO_SpdGear[BSP_DEVICE_ID_N]   =   {0, 0};   /// �ٶȵ� ������ʱ���
/// cy 100909

static  OS_EVENT*           MOTO_SemIrqPtr[BSP_DEVICE_ID_N];                /// ��ʱ�ж�֪ͨ�����õ��ź���
                                                                            /// �����ƽ״̬
static  BitAction           MOTO_StepState[BSP_DEVICE_ID_N] =   {Bit_RESET, Bit_RESET};
static  u16                 MOTO_Timing[BSP_DEVICE_ID_N];                   /// ��ʱ���

//static  OS_STK              MOTO_TaskMotoOddStk[APP_TASK_MOTO_STK_SIZE];  /// ��ͷ�����������ջ�ռ�
//static  OS_STK              MOTO_TaskMotoEvenStk[APP_TASK_MOTO_STK_SIZE]; /// żͷ�����������ջ�ռ�
                                                                            /// �����������ջ�ռ�
static  OS_STK              MOTO_TaskMotoStk[BSP_DEVICE_ID_N][APP_TASK_MOTO_STK_SIZE];

/// 2   ������ر���    --------------------------------------------------------

/// Ĭ�ϳ�������, ���뿪�ӽ����ص���� 300��
static  u16                 MOTO_CutFullSteps               =   MOTO_DU_TO_PULSE(300);
/// Ĭ�ϰ������, ���뿪�ӽ����ص������  80��
static  u16                 MOTO_CutHalfSteps               =   40;//MOTO_DU_TO_PULSE(80);

static  u16                 MOTO_CutSpdEngine               =   1;              /// 1.0 time speed
static  u8                  MOTO_CutDownAcc                 =   1;              /// ���ټ��ٶ�

/// �������������
static  MOTO_CUT_CMD_TypeDef    MOTO_CutCmd[BSP_DEVICE_ID_N]
                                    =   {MOTO_CUT_CMD_NONE, MOTO_CUT_CMD_NONE};

/// 3   �����ض���    --------------------------------------------------------

//static  u16                 MOTO_HghSpdEngine               =   3;
static  u16                 MOTO_HghSpdEngine               =   0;
static  u8                  MOTO_HghDownAcc                 =   1;              /// ���ټ��ٶ�

///// ��ߵ�����λ����������ԣ�
///// 1 - ����� ���UP��Ӧ��λ��ֵ����; 0 - �����޵�λ�� �� ��϶��; -1 - �����
//static  s8                  MOTO_HghRelativity[BSP_DEVICE_ID_N];
//
//static  u16                 MOTO_HghPotVal[BSP_DEVICE_ID_N];                    /// ��ߵ�λ��ֵ

static  u8                  MOTO_HghHeight[BSP_DEVICE_ID_N];                    /// ���ֵ 0~10,E1~3


/// ������������
static  MOTO_HGH_CMD_TypeDef    MOTO_HghCmd[BSP_DEVICE_ID_N]
                                    =   {MOTO_HGH_CMD_NONE, MOTO_HGH_CMD_NONE};

static  OS_EVENT*               MOTO_SemCmdPtr[BSP_DEVICE_ID_N];                /// cy 100909

/// Private consts --------------------------------------------------------------------------------------
/// ���������ʱ�ж��õı�־
const       uint16_t        MOTO_FlagCC[BSP_DEVICE_ID_N]    =   {BSP_FLAG_CC_CLK_ODD, BSP_FLAG_CC_CLK_EVEN};
/// ���������ʱ�õıȽϼĴ��� (��ַ�ǳ���)
volatile    uint16_t* const MOTO_CCR[BSP_DEVICE_ID_N]       =   {&BSP_CCR_CLK_ODD , &BSP_CCR_CLK_EVEN};
/// �������ȼ�
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
        MOTO_SemIrqPtr[i]   =   OSSemCreate(0);                         /// ��ʼ���ж�ʹ�õ��ź���
        MOTO_SemCmdPtr[i]   =   OSSemCreate(0);                         /// ��ʼ����������ʹ�õ��ź���
    
        /// ��������ͷ�����������
        OSTaskCreateExt((void (*)(void *)) MOTO_TaskFunc[i],            /// ���������(ָ��)
                        (void          * ) 0,                           /// ��������ָ��
                                                                        /// ջ��ָ��
                        (OS_STK        * )&MOTO_TaskMotoStk[i][APP_TASK_MOTO_STK_SIZE - 1],
                        (u8              ) MOTO_TaskPrio[i],            /// ���ȼ�
                        (u16             ) MOTO_TaskPrio[i],            /// ID��ʶ, �������ȼ���ͬ
                        (OS_STK        * )&MOTO_TaskMotoStk[i][0],      /// ջ��ָ��
                        (u32             ) APP_TASK_MOTO_STK_SIZE,      /// ջ�ռ��С, ��λΪOS_STK
                        (void          * ) 0,                           /// TCB��չ, ������Ϣ
                                                                        /// ������Ϣ
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
    /// ����Ϊu8, ���ü�������Χ
    MOTO_CutFullSteps   =   n10du * 10 * MOTO_N_P_PR / 360;
}

void  MOTO_CutSetHalfAngle (u8 n10du)
{
    /// ����Ϊu8, ���ü�������Χ
    MOTO_CutHalfSteps   =   n10du * 10 * MOTO_N_P_PR / 360;
}
void  MOTO_TKSetFullAngle (u8 n10du)
{
    /// ����Ϊu8, ���ü�������Χ
    MOTO_CutFullSteps   =   n10du  * MOTO_N_P_PR / 360;
}

void  MOTO_TKSetHalfAngle (u8 n10du)
{
    /// ����Ϊu8, ���ü�������Χ
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
        /// ��������, ����޶����򱨸���ȷ. !ԭ��Э��Ҫ��
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
        /// ��������, ����޶����򱨸���ȷ. !ԭ��Э��Ҫ��
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



/// ���ڼ��ߵ��, ��춨ʱƵ��Ϊ 8M / 400        = 20 KHz,      �������Ƶ��Ϊ10K.
/// ������ߵ��, ��춨ʱƵ��Ϊ 8M / (1200 / 2) = 13.3 KHz,    �������Ƶ��Ϊ6.6K
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
    
    
    if (use == MOTO_USE_CUT) {                  /// ����ԭ���߶�ʱ��ʱ��Ϊ4MHz, ���Լ������߲��ó�2 
        if (SpdGear < MOTO_CUT_SPD_MAX_GEAR) {
            MOTO_Timing[id]     =   MOTO_CutSpdTab[MOTO_CutSpdEngine][SpdGear]*UpHeadSpeed/100;
        } else {
            MOTO_Timing[id]     =   MOTO_CutSpdTab[MOTO_CutSpdEngine][MOTO_CUT_SPD_MAX_GEAR - 1]*UpHeadSpeed/100;
        }
    } else if (use == MOTO_USE_HGH) {           /// ����ԭ��߶�ʱ��ʱ��Ϊ8MHz, ����������߳�2 
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
    MOTO_ClkNum[id]     =   0;                              /// �����ۼ�������
    /// ��ʼ����ʱ�ж�ʹ�õı��� MOTO_StepState �� MOTO_Timing
    MOTO_StepState[id]  =   Bit_RESET;                      /// ��¼�����ƽΪ��
    MOTO_SetSpd(id, use, 0);                                /// ���ó�ʼ�ٶ�Ϊ0��, ����ʼ�� MOTO_Timing
    
    BSP_MOTO_ClkSet(id, Bit_RESET);                         /// ���������ƽΪ��
    BSP_MOTO_DirSet(id, dir);                               /// ���÷���
	//    BSP_MOTO_TQSet(id, tq);                                 /// ���õ���
   //	BSP_MOTO_MSet((BSP_ID_TypeDef)id, BSP_MOT_M_5);
    BSP_MOTO_TQSet(id, BSP_MOT_TQ_75);
    *(MOTO_CCR[id]) =   BSP_TM8M_Get() + StartTiming;       /// ��������ʱ��
    TIM_ClearITPendingBit(BSP_TMR_8M, MOTO_FlagCC[id]);     /// ���ж�
    TIM_ITConfig(BSP_TMR_8M, MOTO_FlagCC[id], ENABLE);      /// ���ж�
}

static  MOTO_ST_TypeDef  MOTO_Step (BSP_ID_TypeDef id, MOTO_USE_TypeDef use, u32 nStep)
{
    u32     i;
    u8      err;
    u8      acc;
    u32     SpdGear;
    
    
    /// �ڲ�����, ����������Χ
    if (use == MOTO_USE_CUT) {                              /// ȡ�ý��ټ��ٶ�
        acc =   MOTO_CutDownAcc;
    } else if (use == MOTO_USE_HGH) {
        acc =   MOTO_HghDownAcc;
    } else {
        acc =   1;
    }
    
    for (i = nStep; i != 0; i--) {
        SpdGear =   ((i - 1) * acc)<<1;
        OSSemPend(MOTO_SemIrqPtr[id], 0, &err);                 /// �ȴ������½��ص�
        MOTO_ClkNum[id]++;                                      /// ��������1
//        MOTO_SetSpd(id, use, MOTO_ClkNum[id]);                  /// �ٶ���λ������, acc =  1
//        MOTO_SetSpd(id, use, (i - 1) * acc);                    /// ��λ����
        if (SpdGear > MOTO_ClkNum[id]) {
            SpdGear = MOTO_ClkNum[id]<<2;
        }
        MOTO_SetSpd(id, use, SpdGear);
        
        if (BSP_MOTO_StatusGet(id) == BSP_MOT_ST_ERROR) {
            return  MOTO_ST_PRO;                                /// ������� TB6560����
        }
    }
    
    return  MOTO_ST_OK;
}

static  void  MOTO_Stop  (BSP_ID_TypeDef    id,
                          u16               msTQDown)
{
    TIM_ITConfig(BSP_TMR_8M, MOTO_FlagCC[id], DISABLE);     /// �ر��ж�
    BSP_MOTO_ClkSet(id, Bit_RESET);                         /// ���������ƽΪ��
    OSTimeDlyHMSM(0, 0, 0, msTQDown);                       /// �ȴ�(ms)
   	BSP_MOTO_TQSet(id, BSP_MOT_TQ_50);
}



static  MOTO_ST_TypeDef  MOTO_CutStepUntil (BSP_ID_TypeDef id, u32 nStep, BSP_SWITCH_TypeDef swi)
{
    u32     i;
    u8      err;
    
    
    /// �ڲ�����, ����������Χ
    for (i = nStep; i != 0; i--) {
        OSSemPend(MOTO_SemIrqPtr[id], 0, &err);                 /// �ȴ������½��ص�
        MOTO_ClkNum[id]++;                                      /// ��������1
        MOTO_SetSpd(id, MOTO_USE_CUT, MOTO_ClkNum[id]);         /// �ٶ���λ������, acc =  1
//        MOTO_SetSpd(id, MOTO_USE_CUT, (i - 1) * 2);             /// ��λ���ٶ�      acc = -2
        
        if (BSP_MOTO_StatusGet(id) == BSP_MOT_ST_ERROR) {
            return  MOTO_ST_PRO;                                /// ������� TB6560����
        }
        if (BSP_SWI_FiGet(id) == swi) {                         /// Ŀ��״̬
            return  MOTO_ST_ACTING;
        }
    }
    
    return  MOTO_ST_OK;
}

static  MOTO_ST_TypeDef  MOTO_CutIn (BSP_ID_TypeDef id)
{
    MOTO_ST_TypeDef     st;
    
    
    if (BSP_SWI_FiGet(id) != MOTO_CUT_SWI_ON) {                                 /// û�е�λ
                                                                                /// 2/1000S������
        MOTO_Start(id, MOTO_USE_CUT, MOTO_CUT_IN, BSP_DIP_TQ, BSP_TMR_8M_CLK * 1 / 1000);
        
        st  =   MOTO_CutStepUntil(id, MOTO_CUT_IN_OVER_N, MOTO_CUT_SWI_ON);     /// �ص�, ֱ���ӽ�����
        switch (st) {
        case MOTO_ST_PRO:               /// TB6560����
            break;
        case MOTO_ST_ACTING:            /// �ﵽ�ӽ�����
            st  =   MOTO_ST_OK;
            break;
        case MOTO_ST_OK:                /// ����ָ������, ����ʱ
            st  =   MOTO_ST_OVTIME;
            break;
        default:                        /// ����
            break;
        }
        
        if (st == MOTO_ST_OK) {         /// ������쳣, ��Ҫ�ɿ��ص��ӽ����ش�
            st  =   MOTO_Step(id, MOTO_USE_CUT, MOTO_CUT_IN_GO_ON_N);
        }
        
        MOTO_Stop(id, 1);               /// ֹͣ����, 4ms������ΪС����
        
    } else {
        st  =   MOTO_ST_OK;
    }
    
    return  st;
}

static  MOTO_ST_TypeDef  MOTO_CutOut (BSP_ID_TypeDef id, u32 nStep)
{
    MOTO_ST_TypeDef     st;
    
    
    if (BSP_SWI_FiGet(id) != MOTO_CUT_SWI_ON) {                                 /// ��û�л�λ, ���λ
        st  =   MOTO_CutIn(id);
    } else {
        st  =   MOTO_ST_OK;
    }
    
    if (st == MOTO_ST_OK) {
                                                                                /// 2/1000S������
        MOTO_Start(id, MOTO_USE_CUT, MOTO_CUT_OUT, BSP_DIP_TQ, BSP_TMR_8M_CLK * 1 / 1000);
        
        st  =   MOTO_CutStepUntil(id, MOTO_CUT_OUT_OVER_N, MOTO_CUT_SWI_OFF);   /// ����, ֱ���뿪�ӽ�����
        switch (st) {
        case MOTO_ST_PRO:               /// TB6560����
            break;
        case MOTO_ST_ACTING:            /// �뿪�ӽ�����
            st  =   MOTO_ST_OK;
            break;
        case MOTO_ST_OK:                /// ����ָ������, ����ʱ
            st  =   MOTO_ST_OVTIME;
            break;
        default:                        /// ����
            break;
        }
        
        if (st == MOTO_ST_OK) {         /// ������쳣, ����Ŀ��λ��
            st  =   MOTO_Step(id, MOTO_USE_CUT, nStep);
        }
            
        MOTO_Stop(id, 1);               /// ֹͣ����, 4ms������ΪС����
    }
    
    return  st;    
}
static  MOTO_ST_TypeDef  MOTO_CutOutH (BSP_ID_TypeDef id, u32 nStep)
{
    MOTO_ST_TypeDef     st;  
	st=MOTO_CutOutD(id,nStep);  
    /*if (BSP_SWI_FiGet(id) != MOTO_CUT_SWI_ON) 
	{                                 /// ��û�л�λ, ���λ
		MOTO_Start(id, MOTO_USE_CUT, MOTO_CUT_OUT, BSP_DIP_TQ, BSP_TMR_8M_CLK * 1 / 1000);
        st  =   MOTO_CutStepUntil(id, MOTO_CUT_OUT_OVER_N, MOTO_CUT_SWI_ON);   /// ����, ֱ���뿪�ӽ�����
        switch (st) 
		{
        case MOTO_ST_PRO:               /// TB6560����
            break;
        case MOTO_ST_ACTING:            /// �뿪�ӽ�����
            st  =   MOTO_ST_OK;
            break;
        case MOTO_ST_OK:                /// ����ָ������, ����ʱ
            st  =   MOTO_ST_OVTIME;
            break;
        default:                        /// ����
            break;
        }
		 if (st == MOTO_ST_OK)          /// ������쳣, ����Ŀ��λ��
            st  =   MOTO_Step(id, MOTO_USE_CUT, 10);
            
        MOTO_Stop(id, 1);               /// ֹͣ����, 4ms������ΪС����
    }
     else 
        st  =   MOTO_ST_OK;
	if (st == MOTO_ST_OVTIME) 
	{
	    MOTO_Start(id, MOTO_USE_CUT, MOTO_CUT_IN, BSP_DIP_TQ, BSP_TMR_8M_CLK * 1 / 1000);        
        st  =   MOTO_CutStepUntil(id, MOTO_CUT_OUT_OVER_N, MOTO_CUT_SWI_ON);   /// ����, ֱ���뿪�ӽ�����
        switch (st) 
		{
        case MOTO_ST_PRO:               /// TB6560����
            break;
        case MOTO_ST_ACTING:            /// �뿪�ӽ�����
            st  =   MOTO_ST_OK;
            break;
        case MOTO_ST_OK:                /// ����ָ������, ����ʱ
            st  =   MOTO_ST_OVTIME;
            break;
        default:                        /// ����
            break;
        }
        if (st == MOTO_ST_OK)          /// ������쳣, ����Ŀ��λ��
            st  =   MOTO_Step(id, MOTO_USE_CUT, 200);
        MOTO_Stop(id, 1);               /// ֹͣ����, 4ms������ΪС����
	} 
	if (st == MOTO_ST_OK) 
	{                                                                              /// 2/1000S������
        MOTO_Start(id, MOTO_USE_CUT, MOTO_CUT_IN, BSP_DIP_TQ, BSP_TMR_8M_CLK * 1 / 1000);
        st  =   MOTO_CutStepUntil(id, MOTO_CUT_OUT_OVER_N, MOTO_CUT_SWI_OFF);   /// ����, ֱ���뿪�ӽ�����
        switch (st) 
		{
        case MOTO_ST_PRO:               /// TB6560����
            break;
        case MOTO_ST_ACTING:            /// �뿪�ӽ�����
            st  =   MOTO_ST_OK;
            break;
        case MOTO_ST_OK:                /// ����ָ������, ����ʱ
            st  =   MOTO_ST_OVTIME;
            break;
        default:                        /// ����
            break;
        }
        if (st == MOTO_ST_OK)          /// ������쳣, ����Ŀ��λ��
            st  =   MOTO_Step(id, MOTO_USE_CUT, 10);    
        MOTO_Stop(id, 1);               /// ֹͣ����, 4ms������ΪС����
   }*/
   if (st == MOTO_ST_OK) 
   { 
        MOTO_Start(id, MOTO_USE_CUT, MOTO_CUT_IN, BSP_DIP_TQ, BSP_TMR_8M_CLK * 1 / 1000);
        st  =   MOTO_CutStepUntil(id, 430, MOTO_CUT_SWI_OFF);   /// ����, ֱ���뿪�ӽ�����
        switch (st) {
        case MOTO_ST_PRO:               /// TB6560����
            break;
        case MOTO_ST_ACTING:            /// �뿪�ӽ�����
            st  =   MOTO_ST_OK;
            break;
        case MOTO_ST_OK:                /// ����ָ������, ����ʱ
            st  =   MOTO_ST_OVTIME;
            break;
        default:                        /// ����
            break;
        }
        
        if (st == MOTO_ST_OK)          /// ������쳣, ����Ŀ��λ��
            st  =   MOTO_Step(id, MOTO_USE_CUT, 10);     
        MOTO_Stop(id, 1);               /// ֹͣ����, 4ms������ΪС����
   }
  if (st == MOTO_ST_OK)
  {
		MOTO_Start(id, MOTO_USE_CUT, MOTO_CUT_OUT, BSP_DIP_TQ, BSP_TMR_8M_CLK * 1 / 1000);
        st  =   MOTO_CutStepUntil(id, MOTO_CUT_OUT_OVER_N, MOTO_CUT_SWI_ON);   /// ����, ֱ���뿪�ӽ�����
        switch (st) 
		{
        case MOTO_ST_PRO:               /// TB6560����
            break;
        case MOTO_ST_ACTING:            /// �뿪�ӽ�����
            st  =   MOTO_ST_OK;
            break;
        case MOTO_ST_OK:                /// ����ָ������, ����ʱ
            st  =   MOTO_ST_OVTIME;
            break;
        default:                        /// ����
            break;
        }
        if (st == MOTO_ST_OK)          /// ������쳣, ����Ŀ��λ��
            st  =   MOTO_Step(id, MOTO_USE_CUT, 10);            
        MOTO_Stop(id, 1);               /// ֹͣ����, 4ms������ΪС����
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
       if (BSP_SWI_FiGet(id) != MOTO_CUT_SWI_ON) {                                 /// û�е�λ
                                                                                /// 2/1000S������
        MOTO_Start(id, MOTO_USE_CUT, MOTO_CUT_IN, BSP_DIP_TQ, BSP_TMR_8M_CLK * 1 / 1000);
        
        st  =   MOTO_CutStepUntil(id, 400, MOTO_CUT_SWI_ON);     /// �ص�, ֱ���ӽ�����
        switch (st) {
        case MOTO_ST_PRO:               /// TB6560����
            break;
        case MOTO_ST_ACTING:            /// �ﵽ�ӽ�����
            st  =   MOTO_ST_OK;
            break;
        case MOTO_ST_OK:                /// ����ָ������, ����ʱ
            st  =   MOTO_ST_OVTIME;
            break;
        default:                        /// ����
            break;
        }
        
        if (st == MOTO_ST_OK) 
		{         /// ������쳣, ��Ҫ�ɿ��ص��ӽ����ش�
            st  =   MOTO_Step(id, MOTO_USE_CUT, MOTO_CUT_IN_GO_ON_N);
			if (BSP_SWI_FiGet(id) == MOTO_CUT_SWI_ON) 
			{ 
				st  =   MOTO_CutOutD(id, MOTO_CutFullSteps);
			}
		    
        }
        
        MOTO_Stop(id, 1);               /// ֹͣ����, 4ms������ΪС����
        
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
	{                                 /// ��û�л�λ, ���λ
        MOTO_Start(id, MOTO_USE_CUT, MOTO_CUT_IN, BSP_DIP_TQ, BSP_TMR_8M_CLK * 1 / 1000);        
        st  =   MOTO_CutStepUntil(id, MOTO_CUT_OUT_OVER_N, MOTO_CUT_SWI_ON);   /// ����, ֱ���뿪�ӽ�����
        switch (st) 
		{
	        case MOTO_ST_PRO:               /// TB6560����
	            break;
	        case MOTO_ST_ACTING:            /// �뿪�ӽ�����
	            st  =   MOTO_ST_OK;
	            break;
	        case MOTO_ST_OK:                /// ����ָ������, ����ʱ
	            st  =   MOTO_ST_OVTIME;
	            break;
	        default:                        /// ����
	            break;
        }
		if (st == MOTO_ST_OK)          /// ������쳣, ����Ŀ��λ��
            st  =   MOTO_Step(id, MOTO_USE_CUT, 10);    
        MOTO_Stop(id, 1);               /// ֹͣ����, 4ms������ΪС����
    }
    else
	{ 
	   st  =   MOTO_ST_OK;
	  // return;
	}
   }    
/*    if (BSP_SWI_FiGet(id) != MOTO_CUT_SWI_ON) 
	{                                 /// ��û�л�λ, ���λ
        MOTO_Start(id, MOTO_USE_CUT, MOTO_CUT_IN, BSP_DIP_TQ, BSP_TMR_8M_CLK * 1 / 1000);        
        st  =   MOTO_CutStepUntil(id, MOTO_CUT_OUT_OVER_N, MOTO_CUT_SWI_ON);   /// ����, ֱ���뿪�ӽ�����
        switch (st) 
		{
	        case MOTO_ST_PRO:               /// TB6560����
	            break;
	        case MOTO_ST_ACTING:            /// �뿪�ӽ�����
	            st  =   MOTO_ST_OK;
	            break;
	        case MOTO_ST_OK:                /// ����ָ������, ����ʱ
	            st  =   MOTO_ST_OVTIME;
	            break;
	        default:                        /// ����
	            break;
        }
		if (st == MOTO_ST_OK)          /// ������쳣, ����Ŀ��λ��
            st  =   MOTO_Step(id, MOTO_USE_CUT, 10);    
        MOTO_Stop(id, 1);               /// ֹͣ����, 4ms������ΪС����
    }
    else 
	{
	   st  =   MOTO_ST_OK;
	   return;
	} */
   /* if (st == MOTO_ST_OVTIME) 
	{
		MOTO_Start(id, MOTO_USE_CUT, MOTO_CUT_OUT, BSP_DIP_TQ, BSP_TMR_8M_CLK * 1 / 1000);
        st  =   MOTO_CutStepUntil(id, MOTO_CUT_OUT_OVER_N, MOTO_CUT_SWI_ON);   /// ����, ֱ���뿪�ӽ�����
        switch (st) 
		{
        case MOTO_ST_PRO:               /// TB6560����
            break;
        case MOTO_ST_ACTING:            /// �뿪�ӽ�����
            st  =   MOTO_ST_OK;
            break;
        case MOTO_ST_OK:                /// ����ָ������, ����ʱ
            st  =   MOTO_ST_OVTIME;
            break;
        default:                        /// ����
            break;
        }        
        if (st == MOTO_ST_OK)          /// ������쳣, ����Ŀ��λ��
           st  =   MOTO_Step(id, MOTO_USE_CUT, 10);            
        MOTO_Stop(id, 1);               /// ֹͣ����, 4ms������ΪС����
	} */
   // if (st == MOTO_ST_OK) 
   if (BSP_SWI_FiGet(id) == MOTO_CUT_SWI_ON)
	{                                                                                /// 2/1000S������
        MOTO_Start(id, MOTO_USE_CUT, MOTO_CUT_OUT, BSP_DIP_TQ, BSP_TMR_8M_CLK * 1 / 1000);
        st  =   MOTO_CutStepUntil(id, MOTO_CUT_OUT_OVER_N, MOTO_CUT_SWI_OFF);   /// ����, ֱ���뿪�ӽ�����
        switch (st) 
		{
        case MOTO_ST_PRO:               /// TB6560����
            break;
        case MOTO_ST_ACTING:            /// �뿪�ӽ�����
            st  =   MOTO_ST_OK;
            break;
        case MOTO_ST_OK:                /// ����ָ������, ����ʱ
            st  =   MOTO_ST_OVTIME;
            break;
        default:                        /// ����
            break;
        }
        if (st == MOTO_ST_OK)          /// ������쳣, ����Ŀ��λ��
            st  =   MOTO_Step(id, MOTO_USE_CUT, 10);    
        MOTO_Stop(id, 1);               /// ֹͣ����, 4ms������ΪС����
	}
   if (st == MOTO_ST_OVTIME) 
	{                                                                                /// 2/1000S������
        MOTO_Start(id, MOTO_USE_CUT, MOTO_CUT_OUT, BSP_DIP_TQ, BSP_TMR_8M_CLK * 1 / 1000);
        st  =   MOTO_CutStepUntil(id, MOTO_CUT_OUT_OVER_N, MOTO_CUT_SWI_OFF);   /// ����, ֱ���뿪�ӽ�����
        switch (st) 
		{
        case MOTO_ST_PRO:               /// TB6560����
            break;
        case MOTO_ST_ACTING:            /// �뿪�ӽ�����
            st  =   MOTO_ST_OK;
            break;
        case MOTO_ST_OK:                /// ����ָ������, ����ʱ
            st  =   MOTO_ST_OVTIME;
            break;
        default:                        /// ����
            break;
        }
        if (st == MOTO_ST_OK)          /// ������쳣, ����Ŀ��λ��
            st  =   MOTO_Step(id, MOTO_USE_CUT, 10);    
        MOTO_Stop(id, 1);               /// ֹͣ����, 4ms������ΪС����
	}
  if (st == MOTO_ST_OK)
   { 
		MOTO_Start(id, MOTO_USE_CUT, MOTO_CUT_IN, BSP_DIP_TQ, BSP_TMR_8M_CLK * 1 / 1000);        
        st  =   MOTO_CutStepUntil(id, MOTO_CUT_OUT_OVER_N, MOTO_CUT_SWI_ON);   /// ����, ֱ���뿪�ӽ�����
        switch (st) 
		{
        case MOTO_ST_PRO:               /// TB6560����
            break;
        case MOTO_ST_ACTING:            /// �뿪�ӽ�����
            st  =   MOTO_ST_OK;
            break;
        case MOTO_ST_OK:                /// ����ָ������, ����ʱ
            st  =   MOTO_ST_OVTIME;
            break;
        default:                        /// ����
            break;
        }
        if (st == MOTO_ST_OK)          /// ������쳣, ����Ŀ��λ��
            st  =   MOTO_Step(id, MOTO_USE_CUT, 10);
        MOTO_Stop(id, 1);               /// ֹͣ����, 4ms������ΪС����
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

/// ! �Ժ��������75% -> 100%
static  MOTO_ST_TypeDef  MOTO_CutFull (BSP_ID_TypeDef id)
{
    MOTO_ST_TypeDef     st;
    u32                 StepOut;
    u32                 StepIn;   
    st  =   MOTO_CutOut(id, MOTO_CutFullSteps);
    if (st == MOTO_ST_OK) {                         /// ������쳣, �ȴ����ջ�
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
static  void  MOTO_Cut (BSP_ID_TypeDef id) //�����ִ�к����е����������
{
    u8    err;
    for ( ; ; ) {
        OSSemPend(MOTO_SemCmdPtr[id], 0, &err);             /// һֱ�ȴ�����
        
        switch (MOTO_CutCmd[id]) {
        case MOTO_CUT_BACK:
            MOTO_State[id]  =   MOTO_ST_ACTING;             /// Ҳ�ɲ��ô˾�, �� MOTO_CUT_CMD_NONE ��֤
                                                            /// ʵ����, �����˾�֮ǰ����MOTO_ST_ACTING״̬
            MOTO_State[id]  =   MOTO_CutBack(id);
            break;
        case MOTO_CUT_HALF:
            MOTO_State[id]  =   MOTO_ST_ACTING;             /// Ҳ�ɲ��ô˾�, �� MOTO_CUT_CMD_NONE ��֤
            MOTO_State[id]  =   MOTO_CutHalf(id);
            break;
        case MOTO_CUT_FULL:
            MOTO_State[id]  =   MOTO_ST_ACTING;             /// Ҳ�ɲ��ô˾�, �� MOTO_CUT_CMD_NONE ��֤
            MOTO_State[id]  =   MOTO_CutFull(id);
            break;
		case MOTO_CUT_H:
			MOTO_State[id]  =   MOTO_ST_ACTING;             /// Ҳ�ɲ��ô˾�, �� MOTO_CUT_CMD_NONE ��֤
            MOTO_State[id]  =   MOTO_CutH(id);
			break;
		case MOTO_CUT_D:
			MOTO_State[id]  =   MOTO_ST_ACTING;             /// Ҳ�ɲ��ô˾�, �� MOTO_CUT_CMD_NONE ��֤
            MOTO_State[id]  =   MOTO_CutD(id);
			break;
		case MOTO_CUT_O:
			MOTO_State[id]  =   MOTO_ST_ACTING;             /// Ҳ�ɲ��ô˾�, �� MOTO_CUT_CMD_NONE ��֤
            MOTO_State[id]  =   MOTO_CutO(id);
			break;
        default:
            break;
        }
        MOTO_CutCmd[id]     =   (MOTO_CUT_CMD_TypeDef)MOTO_CUT_CMD_NONE;
    }
}

static  void  MOTO_TaskCutOdd (void *data)//�����ִ�к���
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
    
    
    /// �ڲ�����, ����������Χ
    
    now     =   MOTO_HghPotGetAve(id);
    rev     =   0;
    slow    =   MOTO_HGH_SPD_MAX_GEAR;
    
    for (i = nStep; i != 0; i--) {
        OSSemPend(MOTO_SemIrqPtr[id], 0, &err);                 /// �ȴ������½��ص�
        MOTO_ClkNum[id]++;                                      /// ��������1
        SpdGear =   (i - 1) * MOTO_HghDownAcc;
        if (SpdGear > slow) {
            SpdGear =   slow;
        }
        if (SpdGear > MOTO_ClkNum[id]) {
            SpdGear =   MOTO_ClkNum[id];
        }
        MOTO_SetSpd(id, MOTO_USE_HGH, SpdGear);                 /// �ٶ���λ������, acc =  1
        
        /// �����ж�
        if (BSP_MOTO_StatusGet(id) == BSP_MOT_ST_ERROR) {
            return  MOTO_ST_PRO;                                /// ������� TB6560����
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
        /// ��ת�ж�
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
        
        /// ����
        if (disnow < MOTO_DU_TO_POT(1.5)) {                     /// ͣ��λ��
            MOTO_HghHeight[id]  =   MOTO_HghGetFromPot(now);
            return  MOTO_ST_ACTING;
        } else if (disnow < MOTO_DU_TO_POT(3)) {                /// ����λ��
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
    
    
    /// ����������Χ
        
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
        
        st  =   MOTO_HghStepUntil(id, MOTO_N_P_PR * 4, position[hgh]);//��С���ֵĴ�������4��1��С������Ҫת4Ȧ
        switch (st) {
        case MOTO_ST_PRO:               /// TB6560����
            break;
        case MOTO_ST_ACTING:            /// �ﵽĿ��λ��
            st  =   MOTO_ST_OK;
            break;
        case MOTO_ST_OK:                /// ����ָ������, ����ʱ
            st  =   MOTO_ST_OVTIME;
            break;
        default:                        /// ����
            break;
        }
        
        MOTO_Stop(id, 10);              /// ֹͣ����, 10ms������ΪС����
        
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
        OSSemPend(MOTO_SemCmdPtr[id], wait, &err);          /// �ȴ�����
        
        if (MOTO_HghCmd[id] < MOTO_HGH_POS_NUM) {           /// ��Ч����
		if (HeadUpDown[id] ==1) 
			{ 
				HeadUpDown[id]=0;
				MOTO_HghCmd_Tmp=MOTO_HghCmd[id];

				MOTO_HghCmd[id]=(MOTO_HGH_CMD_TypeDef)10;
	            MOTO_State[id]  =   MOTO_ST_ACTING;             /// Ҳ�ɲ��ô˾�, �� MOTO_HGH_CMD_NONE ��֤
	            MOTO_State[id]  =   MOTO_HghChange(id, MOTO_HghCmd[id]);
	            MOTO_HghCmd[id] =   MOTO_HGH_CMD_NONE;
	
				MOTO_HghCmd[id]=MOTO_HghCmd_Tmp;
			}
        MOTO_State[id]  =   MOTO_ST_ACTING;             /// Ҳ�ɲ��ô˾�, �� MOTO_HGH_CMD_NONE ��֤
        MOTO_State[id]  =   MOTO_HghChange(id, MOTO_HghCmd[id]);
        MOTO_HghCmd[id] =   MOTO_HGH_CMD_NONE;
        } else {                                            /// ��Ч���� �� ��ʱ
            pot =   MOTO_HghPotGetAve(id);                  /// ��λ����ѹֵ
            hgh =   MOTO_HghGetFromPot(pot);                /// תΪ���ֵ
            
            if (hgh < MOTO_HGH_POS_NUM) {                   /// ����ֵ
                MOTO_HghHeight[id]  =   hgh;
                jog     =   8;
                time    =   8;
                wait    =   100;
                
            } else {                                        /// �쳣ֵ
                                                            /// ������ѯ
                if (time != 0) {
                    time--;
                } else {
                    ;
                }
                                                            /// ����΢��, ���򲻱�
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
                                                            /// �����ȴ�ʱ�� �� ���±���
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
        if (TIM_GetITStatus(BSP_TMR_8M, MOTO_FlagCC[i]) != RESET) { /// ����ǵ���Ƚ��ж�
            TIM_ClearITPendingBit(BSP_TMR_8M, MOTO_FlagCC[i]);      /// ���ж�
            
            *MOTO_CCR[i] +=  MOTO_Timing[i];                        /// �����¸��ж�����
            
            if (MOTO_StepState[i] == Bit_RESET) {
                st  =   Bit_SET;
            } else {
                st  =   Bit_RESET;
                OSSemPost(MOTO_SemIrqPtr[i]);                       /// �����½���ʱ, ֪ͨ��������
            }
            BSP_MOTO_ClkSet((BSP_ID_TypeDef)i, st);                 /// ���øı���clk
            MOTO_StepState[i] = st;                                 /// ��¼clk��ƽ״̬
        }
    }
}




/// End of file -----------------------------------------------------------------------------------------