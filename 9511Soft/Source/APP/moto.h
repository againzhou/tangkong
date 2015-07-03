
/// ----------------------------------------------------------------------------
/// File Name          : moto.c
/// Description        : �����������
///     ���߳��ֹ�14��, m = 1, d = mz = 14mm, �ܳ� = 14pi =  Լ44mm
///     ������ߵ����ԭ���෴, �����궨�� MOTO_CUT_IN �� MOTO_CUT_OUT
///     ��ߵ��ͬ��
/// ----------------------------------------------------------------------------
/// History:

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

/// y10/m11/d30     v0.02   chenyong    modify
/// 1   �޸� MOTO_DU_TO_POT �ĺ궨��, ����ǿ��ת��Ϊ u32
/// 2   ԭ��1�㷶Χ��Ϊ��λ����ͣ��, ���ڸ�Ϊ1.5��
/// 3   ���� MOTO_HghTestIsUp() ����, ���ٱ�����
/// 4   ����#define MOTO_HGH_POS_UP 10

/// y10/m08/d03     v0.01   chenyong    setup





#ifndef  MOTO_H
#define  MOTO_H

/// EXTERNS --------------------------------------------------------------------
#ifdef   MOTO_MODULE
    #define  MOTO_EXT               /// ������Ϊ "�ⲿ", ��ģ����
#else
    #define  MOTO_EXT  extern       /// ����Ϊ "�ⲿ"
#endif


#include  <ucos_ii.h>               /// OS�˵�ͷ�ļ�
#include  <bsp.h>

//  #ifndef  CFG_H      /// û��ͳһ����

/// USER DEFINED CONSTANTS ------------------------------------------------------------------------------

#define     MOTO_HGH_POS_NUM        11                      /// ��Ϊ11�����: 0 ~ 9�� �� ���뵵
#define     MOTO_HGH_POS_ERR        MOTO_HGH_POS_NUM        /// ���λ���쳣 ����0~10����
#define     MOTO_HGH_POS_UP         10                      /// ���10��ʾ��ͷ����


/// Exported types -------------------------------------------------------------

/// ���������״̬, Ŀǰû�а�λʹ��, ֻ�ǰ�ö��ʹ��
///     7       6       5       4       3       2       1       0
///   sensor (blank)   pro  overtime (blank) reverse (blank) running
///
///     bit7        ������ߵ����Ч
///     bit2        ���ڼ��ߵ����Ч
///     bit6,3,1    ����reserve

/// ���������״̬, ���ȼ��ɸߵ���
typedef enum
{
    MOTO_ST_ACTING  =   0,          /// ����������
                                    /// ����Ϊ��ֹ״̬
    MOTO_ST_PRO,                    /// ������ϵ��¶���ֹͣ
    MOTO_ST_OVTIME,                 /// ������ʱ���¶���ֹͣ(һ��Ϊ��������)
    MOTO_ST_CUTLOOPER,              /// ������������, ����û������
    MOTO_ST_REVERSE,                /// �����ת���¶���ֹͣ
//    MOTO_ST_POTERR,                 /// ���ת�����λ��û�б仯
    MOTO_ST_OK,                     /// ��ֹ�޹��� �� �������
} MOTO_ST_TypeDef;

/// ��������ʹ�õ������־����
typedef enum
{	MOTO_CUT_POS0   =   0,          /// ���0
    MOTO_CUT_POS1   ,          /// ���1
    MOTO_CUT_POS2   ,
    MOTO_CUT_POS3   ,
    MOTO_CUT_POS4   ,
    MOTO_CUT_POS5   ,
    MOTO_CUT_POS6   ,
    MOTO_CUT_POS7   ,
    MOTO_CUT_POS8   ,
    MOTO_CUT_POS9   ,
	MOTO_CUT_D		,
    MOTO_CUT_H      ,          /// ���10, ������λ��
    MOTO_CUT_BACK   ,          /// �ջ�
    MOTO_CUT_HALF   ,          /// ���
    MOTO_CUT_FULL   ,          /// ��������
	MOTO_CUT_O,
	MOTO_CUT_NONE   ,          /// ��������
	

} MOTO_CUT_CMD_TypeDef;

/// ��������յ�����������
typedef enum
{
    MOTO_HGH_POS0   =   0,          /// ���0
    MOTO_HGH_POS1   =   1,          /// ���1
    MOTO_HGH_POS2   =   2,
    MOTO_HGH_POS3   =   3,
    MOTO_HGH_POS4   =   4,
    MOTO_HGH_POS5   =   5,
    MOTO_HGH_POS6   =   6,
    MOTO_HGH_POS7   =   7,
    MOTO_HGH_POS8   =   8,
    MOTO_HGH_POS9   =   9,
    MOTO_HGH_POS10  =  10,          /// ���10, ������λ��
} MOTO_HGH_CMD_TypeDef;
typedef enum
{
    HOT_POS_DOWN_OVERR  =   0,          /// ����������
                                    /// ����Ϊ��ֹ״̬
    HOT_POS_DOWN,                    /// ������ϵ��¶���ֹͣ
    HOT_POS_MID,                 /// ������ʱ���¶���ֹͣ(һ��Ϊ��������)
    HOT_POS_UP,              /// ������������, ����û������
    HOT_POS_UP_OVERR,                /// �����ת���¶���ֹͣ

} HOT_POS_TypeDef;
///// �����͵Ļظ���Ϣ����
//typedef enum
//{
//    MOTO_RES_FINISH =   0,      /// ���
//    MOTO_RES_TIMEOUT,           /// ��ʱ
//} MOTO_RES_TypeDef;

/// ���������ʹ�����
typedef enum
{
    MOTO_USE_HGH   =   0,       /// �������
    MOTO_USE_CUT,               /// ���ڼ���
} MOTO_USE_TypeDef;

///// �������������ƽṹ
//typedef  struct
//{
//    u32         step;
//    u16         (*GetSpdFun)(u8*, s8);
//    u8          gear;
//    s8          acc;
//    BitAction   clk;
//} MOTO_STEP_CTRL_TypeDef;


/// Global variables -----------------------------------------------------------
//MOTO_EXT    u8          MOTO_StepDeltaOdd;
//MOTO_EXT    u8          MOTO_StepDeltaEven;
//MOTO_EXT    u32         MOTO_ClkNumOdd;
//MOTO_EXT    u32         MOTO_ClkNumEven;
////MOTO_EXT    u32         MOTO_StepSumOdd;
////MOTO_EXT    u32         MOTO_StepSumEven;
//MOTO_EXT    u16         MOTO_TimingOdd;
//MOTO_EXT    u16         MOTO_TimingEven;

/// for �ж�
//MOTO_EXT    BitAction                   MOTO_StepState[BSP_DEVICE_ID_N];
//MOTO_EXT    u16                         MOTO_Timing[BSP_DEVICE_ID_N];

/// for ��������
//MOTO_EXT    MOTO_CUT_CMD_TypeDef    MOTO_CutCmd[BSP_DEVICE_ID_N];
//MOTO_EXT    MOTO_HGH_CMD_TypeDef    MOTO_HghCmd[BSP_DEVICE_ID_N];


//MOTO_EXT    OS_EVENT*                   MOTO_SemIrqPtr[BSP_DEVICE_ID_N];
////MOTO_EXT    OS_EVENT*                   MOTO_SemCmdPtr[BSP_DEVICE_ID_N];      /// cy 100909

//MOTO_EXT    const       uint16_t        MOTO_FlagCC[BSP_DEVICE_ID_N];
//MOTO_EXT    volatile    uint16_t* const MOTO_CCR[BSP_DEVICE_ID_N];

/// FUNCTION PROTOTYPES ---------------------------------------------------------------------------------

void                MOTO_Init               (MOTO_USE_TypeDef odd, MOTO_USE_TypeDef even);
//void                MOTO_CutSetSpdEngine    (u8             engine);
void                MOTO_SetSpdEngine       (MOTO_USE_TypeDef use, u8 engine);

void                MOTO_CutSetFullAngle    (u8             n10du);
void                MOTO_CutSetHalfAngle    (u8             n10du);
MOTO_ST_TypeDef     MOTO_CutGetState        (BSP_ID_TypeDef id);
bool                MOTO_CutGetIsActing     (BSP_ID_TypeDef id);
void                MOTO_CutSetCmd          (BSP_ID_TypeDef id, MOTO_CUT_CMD_TypeDef cmd);

u8                  MOTO_HghGetHeight       (BSP_ID_TypeDef id);
bool                MOTO_HghTestIsUp        (BSP_ID_TypeDef id, u8 time);
MOTO_ST_TypeDef     MOTO_HghGetState        (BSP_ID_TypeDef id);
bool                MOTO_HghGetIsActing     (BSP_ID_TypeDef id);
void                MOTO_HghSetCmd          (BSP_ID_TypeDef id, MOTO_HGH_CMD_TypeDef cmd);

void                MOTO_IRQandClr          (void);
void                MOTO_TKSetFullAngle    (u8             n10du);
void                MOTO_TKSetHalfAngle    (u8             n10du);
HOT_POS_TypeDef     HOT_POS_Get();


#endif
