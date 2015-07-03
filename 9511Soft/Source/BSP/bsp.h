
/// ----------------------------------------------------------------------------
/// File Name          : bsp.h
/// Description        : �弶֧�ְ���ͷ�ļ�
/// ----------------------------------------------------------------------------
/// History:

/// y11/m01/d18     v0.07   chenyong    modify
/// ����� PC924 ���ж� �� ��������
/// ��BSP_NVIC_Cfg()��, �����ϸ�, �����ж�����
/// ����һ�㺯��, ���ƿ���, ֻ�� MOTC(�ջ����) ��������, ������������

/// y10/m12/d24     v0.07   chenyong    modify
/// ���Ӻ��� BSP_MOTO_ClkGet() �� BSP_MOTC_ClkGet()

/// y10/m12/d23     v0.06   chenyong    modify
/// ����� PC924 ���ж� �� ��������

/// y10/m12/d03     v0.05   chenyong    modify
/// ���ı������������

/// y10/m11/d30     v0.04   chenyong    modify
/// ���ĺ��� BSP_POT_GetOne(), ԭ���̶�ȡ��Ԫ0��ֵ, ��Ϊ����ȡֵ

/// y10/m09/d10     v0.03   chenyong    modify
/// ����ΪPC924��������ֻ��BSP_Judge924()��������Ч
/// �������ͨ��ȫ�ֱ����жϰ�����
/// ����

/// y10/m07/d12     v0.02   chenyong    modify
/// ʹ��ioģ��. io �� MPU �������; bsp �빦���������

/// y10/m07/d05     v0.01   chenyong    setup





/// Define to prevent recursive inclusion --------------------------------------
#ifndef BSP_H
#define BSP_H


/// EXTERNS --------------------------------------------------------------------
#ifdef   BSP_MODULE
    #define  BSP_EXT                /// ������Ϊ "�ⲿ", ��ģ����
#else
    #define  BSP_EXT  extern        /// ����Ϊ "�ⲿ"
#endif


/// Includes -------------------------------------------------------------------
#include  <stm32f10x.h>



/// Exported macro -------------------------------------------------------------

#define     BSP_LED_ID_N                2
#define     BSP_DEVICE_ID_N             2

#define     BSP_DIP_N_BITS              10

#define     BSP_ENC_N_A_PR              400                     /// ������A��B������ÿȦ����
#define     BSP_ENC_N_T_PR              (BSP_ENC_N_A_PR * 4)    /// ������ÿȦ���붨ʱ���������
#define     BSP_ENC_N_T_PHR             (BSP_ENC_N_T_PR / 2)    /// ������ÿ��Ȧ���붨ʱ���������

/// ----     491    ----
#define     BSP_USART_491               USART1
#define     BSP_USART_491_IRQHandler    USART1_IRQHandler

/// ----   8MTimer  ----
    #define     BSP_TMR_8M              TIM1                        /// ��ʱ��, ��Ҫ����125nSΪ��λ�Ķ�ʱ
    #define     BSP_TMR_8M_CLK          8000000                     /// ��ʱ��ʱ��Ϊ8MHz
    #define     BSP_TMR_8M_IRQHandler   TIM1_CC_IRQHandler
    
    #define     BSP_CC_MC_CLOSE         1                           /// �ջ�����ıջ����ƶ�ʱ
    #define     BSP_CC_MC_OPEN          2                           /// �ջ�����Ŀ������ƶ�ʱ
    #define     BSP_CC_CLK_ODD          3                           /// ʹ��cc3, ����������ʱ����
    #define     BSP_CC_CLK_EVEN         4                           /// ʹ��cc4, ����ż����ʱ����
    
//#if     (BSP_CC_CLK_ODD == BSP_CC_CLK_EVEN)
//    #error  "BSP_CC_CLK_ODD and BSP_CC_CLK_EVEN use the same cc !"
//#endif

#if     BSP_CC_MC_CLOSE == 1
    #define     BSP_FLAG_CC_MC_CLOSE    TIM_FLAG_CC1
    #define     BSP_CCR_MC_CLOSE        (BSP_TMR_8M->CCR1)
#elif   BSP_CC_MC_CLOSE == 2
    #define     BSP_FLAG_CC_MC_CLOSE    TIM_FLAG_CC2
    #define     BSP_CCR_MC_CLOSE        (BSP_TMR_8M->CCR2)
#elif   BSP_CC_MC_CLOSE == 3
    #define     BSP_FLAG_CC_MC_CLOSE    TIM_FLAG_CC3
    #define     BSP_CCR_MC_CLOSE        (BSP_TMR_8M->CCR3)
#elif   BSP_CC_MC_CLOSE == 4
    #define     BSP_FLAG_CC_MC_CLOSE    TIM_FLAG_CC4
    #define     BSP_CCR_MC_CLOSE        (BSP_TMR_8M->CCR4)
#endif

#if     BSP_CC_MC_OPEN == 1
    #define     BSP_FLAG_CC_MC_OPEN     TIM_FLAG_CC1
    #define     BSP_CCR_MC_OPEN         (BSP_TMR_8M->CCR1)
#elif   BSP_CC_MC_OPEN == 2
    #define     BSP_FLAG_CC_MC_OPEN     TIM_FLAG_CC2
    #define     BSP_CCR_MC_OPEN         (BSP_TMR_8M->CCR2)
#elif   BSP_CC_MC_OPEN == 3
    #define     BSP_FLAG_CC_MC_OPEN     TIM_FLAG_CC3
    #define     BSP_CCR_MC_OPEN         (BSP_TMR_8M->CCR3)
#elif   BSP_CC_MC_OPEN == 4
    #define     BSP_FLAG_CC_MC_OPEN     TIM_FLAG_CC4
    #define     BSP_CCR_MC_OPEN         (BSP_TMR_8M->CCR4)
#endif

#if     BSP_CC_CLK_ODD == 1
    #define     BSP_FLAG_CC_CLK_ODD     TIM_FLAG_CC1
    #define     BSP_CCR_CLK_ODD         (BSP_TMR_8M->CCR1)
#elif   BSP_CC_CLK_ODD == 2
    #define     BSP_FLAG_CC_CLK_ODD     TIM_FLAG_CC2
    #define     BSP_CCR_CLK_ODD         (BSP_TMR_8M->CCR2)
#elif   BSP_CC_CLK_ODD == 3
    #define     BSP_FLAG_CC_CLK_ODD     TIM_FLAG_CC3
    #define     BSP_CCR_CLK_ODD         (BSP_TMR_8M->CCR3)
#elif   BSP_CC_CLK_ODD == 4
    #define     BSP_FLAG_CC_CLK_ODD     TIM_FLAG_CC4
    #define     BSP_CCR_CLK_ODD         (BSP_TMR_8M->CCR4)
#endif

#if     BSP_CC_CLK_EVEN == 1
    #define     BSP_FLAG_CC_CLK_EVEN    TIM_FLAG_CC1
    #define     BSP_CCR_CLK_EVEN        (BSP_TMR_8M->CCR1)
#elif   BSP_CC_CLK_EVEN == 2
    #define     BSP_FLAG_CC_CLK_EVEN    TIM_FLAG_CC2
    #define     BSP_CCR_CLK_EVEN        (BSP_TMR_8M->CCR2)
#elif   BSP_CC_CLK_EVEN == 3
    #define     BSP_FLAG_CC_CLK_EVEN    TIM_FLAG_CC3
    #define     BSP_CCR_CLK_EVEN        (BSP_TMR_8M->CCR3)
#elif   BSP_CC_CLK_EVEN == 4
    #define     BSP_FLAG_CC_CLK_EVEN    TIM_FLAG_CC4
    #define     BSP_CCR_CLK_EVEN        (BSP_TMR_8M->CCR4)
#endif

///// �ջ�������Ʒ����ݶ�Ϊ, ֻʹ��һ���ж�
///// ʹ��Timer3������ж�, ��ʱ��ʹ�ñȽ��ж�, �����ж��ڵĲ�����
///// ----   MC_Timer  ----
//    #define     BSP_TMR_MC              TIM3                        /// ��ʱ��, ��Ҫ���ڱջ��������
//    #define     BSP_TMR_MC_T            10                          /// �ջ���������(uS), ��Ҫȡ��������
//    #define     BSP_TMR_MC_F            (1000000 / BSP_TMR_MC_T)    /// �ջ�����Ƶ��
////    #define     BSP_TMR_MC_CLK          1000000                     /// ��ʱ��ʱ��Ϊ1MHz
//    #define     BSP_TMR_MC_IRQHandler   TIM3_IRQHandler
//    
////    #define     BSP_FLAG_CC_MOTC        TIM_FLAG_CC1
////    #define     BSP_CCR_MOTC            (BSP_TMR_MC->CCR1)
//    #define     BSP_FLAG_CC_MOTC        TIM_FLAG_Update

///// �ջ�������������ʹ�õ��ж�, ʹ��Timer3�ıȽ��ж�1
///// ----   MCC_Timer  ----
//    #define     BSP_TMR_MCC             TIM3                        /// ��ʱ��3, ��Ҫ���ڱջ�������������
//    #define     BSP_TMR_MC_IRQHandler   TIM3_IRQHandler
//    
//    #define     BSP_FLAG_CC_MCC         TIM_FLAG_CC1
//    #define     BSP_CCR_MCC             (BSP_TMR_MCC->CCR1)

/// ----   ENC_Timer  ----
    #define     BSP_TMR_E_ODD           TIM2                        /// Encoder_ODD
    #define     BSP_TMR_E_EVEN          TIM4                        /// Encoder_EVEN
    
    #define     BSP_TMR_E_ODD_IRQHandler    TIM2_IRQHandler         /// Encoder_ODD  irq
    #define     BSP_TMR_E_EVEN_IRQHandler   TIM4_IRQHandler         /// Encoder_EVEN irq
    
/// ----   Encoder Index  ----
    #define     BSP_EXIT_LINE_I_E_ODD   EXTI_Line2                  /// Index_Encoder_ODD
    #define     BSP_EXIT_LINE_I_E_EVEN  EXTI_Line8                  /// Index_Encoder_EVEN
    
    #define     BSP_I_E_ODD_IRQHandler  EXTI2_IRQHandler            /// Index_Encoder_ODD  irq
    #define     BSP_I_E_EVEN_IRQHandler EXTI9_5_IRQHandler          /// Index_Encoder_EVEN irq

/// ģ������Ķ�ʱ��, ʹ��Timer3
/// ----   CNT_Timer  ----
    #define     BSP_TMR_CNT             TIM3
    #define     BSP_TMR_CNT_IRQHandler  TIM3_IRQHandler
    
    #define     BSP_CC_MCC_DEC          1                           /// cc1���ڱջ������λ���������
    #define     BSP_CC_MCC_INC          2                           /// cc2���ڱջ������λ����������
    
#if     (BSP_CC_MCC_DEC == BSP_CC_MCC_INC)
    #error  "BSP_CC_MCC_DEC and BSP_CC_MCC_INC use the same cc !"
#endif

#if     BSP_CC_MCC_DEC == 1
    #define     BSP_FLAG_CC_MCC_DEC     TIM_FLAG_CC1
    #define     BSP_CCR_MCC_DEC         (BSP_TMR_CNT->CCR1)
#elif   BSP_CC_MCC_DEC == 2
    #define     BSP_FLAG_CC_MCC_DEC     TIM_FLAG_CC2
    #define     BSP_CCR_MCC_DEC         (BSP_TMR_CNT->CCR2)
#elif   BSP_CC_MCC_DEC == 3
    #define     BSP_FLAG_CC_MCC_DEC     TIM_FLAG_CC3
    #define     BSP_CCR_MCC_DEC         (BSP_TMR_CNT->CCR3)
#elif   BSP_CC_MCC_DEC == 4
    #define     BSP_FLAG_CC_MCC_DEC     TIM_FLAG_CC4
    #define     BSP_CCR_MCC_DEC         (BSP_TMR_CNT->CCR4)
#endif

#if     BSP_CC_MCC_INC == 1
    #define     BSP_FLAG_CC_MCC_INC     TIM_FLAG_CC1
    #define     BSP_CCR_MCC_INC         (BSP_TMR_CNT->CCR1)
#elif   BSP_CC_MCC_INC == 2
    #define     BSP_FLAG_CC_MCC_INC     TIM_FLAG_CC2
    #define     BSP_CCR_MCC_INC         (BSP_TMR_CNT->CCR2)
#elif   BSP_CC_MCC_INC == 3
    #define     BSP_FLAG_CC_MCC_INC     TIM_FLAG_CC3
    #define     BSP_CCR_MCC_INC         (BSP_TMR_CNT->CCR3)
#elif   BSP_CC_MCC_INC == 4
    #define     BSP_FLAG_CC_MCC_INC     TIM_FLAG_CC4
    #define     BSP_CCR_MCC_INC         (BSP_TMR_CNT->CCR4)
#endif

    #define     BSP_COUNTER_PR          8000                        /// ����һȦģ�����Ϊ8000
    
    #define     BSP_SPD_D10_MAX         100                         /// ����ٶ� (��λ10rpm)
    #define     BSP_SPD_D10_MIN         8                           /// ��С�ٶ�, ��֤��Ƶ��Ϊu16,(��λ10rpm)
                                                                    /// �ٶ�����
    #define     BSP_SPD_NUM             (BSP_SPD_D10_MAX - BSP_SPD_D10_MIN + 1)

   	#define CONFIRM_MAINCONTRAL_ADDR1   0x0801f800
	#define CONFIRM_MAINCONTRAL_ADDR2   0x0801f400
	#define CONFIRM_9511_ID_ADDR       0x0801fc00
	#define CONFIRM_STM32_ID_ADDR	   0x1FFFF7E8
	#define CONFIRM_FASH_ID_ADDR	   0x1FFFF7E0//

/// Exported types -------------------------------------------------------------

/// ��ż�豸id
typedef enum
{
    BSP_ID_ODD          =   0,
    BSP_ID_EVEN         =   1,
} BSP_ID_TypeDef;

/// LED״̬(�������Ϻͻ�ͷ���ذ��ϵĵ�, ������ߵ�ƽ��ͬ��Ҫ�ֿ�����)
typedef enum
{
    BSP_LED_OFF         =   1,
    BSP_LED_ON          =   0,
} BSP_LED_TypeDef;

/// ��ͷ����״̬
typedef enum
{
    BSP_HD_KEY_OFF      =   0,
    BSP_HD_KEY_ON       =   1,
} BSP_HD_KEY_TypeDef;

/// DIP�����İ�Ĺ�������   0 - �ϻ�ͷ��, 1 - �»�ͷ��
typedef enum {
    BSP_DIP_BD_FUNC_UP  =   0,
    BSP_DIP_BD_FUNC_DN  =   1,
} BSP_DIP_BD_FUNC_TypeDef;

/// ���״̬    ���� - 0, ���� - 1
typedef enum
{
    BSP_MOT_ST_ERROR    =   0,
    BSP_MOT_ST_NORMAL   =   1,
} BSP_MOT_ST_TypeDef;

/// �������    cw - 0, ccw - 1
typedef enum
{
    BSP_MOT_DIR_CW      =   0,
    BSP_MOT_DIR_CCW     =   1,
} BSP_MOT_DIR_TypeDef;

/// �������    
typedef enum
{
    BSP_MOT_TQ_100      =   0,
    BSP_MOT_TQ_75       =   1,
    BSP_MOT_TQ_50       =   2,
    BSP_MOT_TQ_20       =   3,
} BSP_MOT_TQ_TypeDef;
typedef enum
{
    BSP_MOT_M_0      =   0,
    BSP_MOT_M_1       =   1,
    BSP_MOT_M_2       =   2,
    BSP_MOT_M_3       =   3,
	BSP_MOT_M_4      =    4,
    BSP_MOT_M_5       =   5,
    BSP_MOT_M_6       =   6,
    BSP_MOT_M_7       =   7,
} BSP_MOT_M123_TypeDef;
/// �ջ��������    0 - 0.45��, 1 - 0.225��
typedef enum
{
    BSP_MOT_M_P45       =   0,
    BSP_MOT_M_P225      =   1,
} BSP_MOT_M_TypeDef;

/// �ջ������ʼ��״̬      0 - ��ʼ��״̬, 1 - ͨ��״̬
typedef enum
{
    BSP_MOT_IST_INITIAL =   0,
    BSP_MOT_IST_NORMAL  =   1,
} BSP_MOT_IST_TypeDef;

/// ����������״̬(û�ж���On��Off, ��Ϊ����pnp��npn�ǲ�ͬ��)
typedef enum
{
    BSP_SWITCH_HIGH     =   0,
    BSP_SWITCH_LOW      =   1,
} BSP_SWITCH_TypeDef;

/// 491_RxBuf״̬
typedef enum
{
    BSP_491_RXBUF_NORMAL    =   0,
    BSP_491_RXBUF_FULL,
    BSP_491_RXBUF_EMPTY,
} BSP_491_RxBuf_TypeDef;

/// 491_Tx״̬
typedef enum
{
    BSP_491_TX_NORMAL   =   0,
    BSP_491_TX_FULL,
} BSP_491_Tx_TypeDef;

/// ����Ƭ��Ĵ���״̬    
typedef enum
{
    BSP_EMB_ST_OFF      =   0,
    BSP_EMB_ST_NORMAL   =   1,
    BSP_EMB_ST_PATCH    =   2,
} BSP_EMB_ST_TypeDef;

/// �ջ��������    0 - 0.45��, 1 - 0.225��

/// SOL״̬
typedef enum
{
    BSP_SOL_OFF         =   0,
    BSP_SOL_ON          =   1,
} BSP_SOL_TypeDef;

/// Global variables -----------------------------------------------------------

BSP_EXT     bool                        BSP_Is924;          /// �Ƿ�924

BSP_EXT     BSP_MOT_TQ_TypeDef          BSP_DIP_TQ;         /// �趨�ĵ���
BSP_EXT     BSP_DIP_BD_FUNC_TypeDef     BSP_DIP_BDFunc;     /// �幦��
BSP_EXT     u8                          BSP_DIP_9511Addr;   /// 9511���ַ
BSP_EXT     u8                          BSP_DIP_924Addr;    ///  924���ַ
BSP_EXT		bool    SECURITY_9511_STAT ; //������	  TRUE����
BSP_EXT		bool	MAIN_SECURITY_STAT ; //������	  TRUE����
BSP_EXT     u8 STM32_ID[12];
BSP_EXT     u8 PRO_9511_ID[4];
BSP_EXT     u8 MAIN_Sec_ID[4];

/// Exported functions ---------------------------------------------------------

void                    BSP_Init            (void);

u16                     BSP_TM8M_Get        (void);
void                    BSP_TM8M_Delay      (u16 time);

void                    BSP_TM_CNT_SpdSet   (u16 spd_d10);
u16                     BSP_TM_CNT_Get      (void);

/// -------- HMI --------
/// -------- led --------
void                    BSP_LED_Set         (u8 led, BSP_LED_TypeDef st);
void                    BSP_LED_Toggle      (u8 led);
/// -------- Head Board --------
void                    BSP_HD_KeyGet       (BSP_ID_TypeDef id, BSP_HD_KEY_TypeDef *up,
                                                                BSP_HD_KEY_TypeDef *down);
void                    BSP_HD_LedSet       (BSP_ID_TypeDef id, BSP_LED_TypeDef red,
                                                                BSP_LED_TypeDef green);
/// -------- dip --------
u32                     BSP_DIP_Get         (void);

/// -------- EXECUTOR --------
/// -------- solenoid --------
/// ����
/// -------- motor (controled by open loop)  --------
BSP_MOT_ST_TypeDef      BSP_MOTO_StatusGet  (BSP_ID_TypeDef id);
BitAction               BSP_MOTO_ClkGet     (BSP_ID_TypeDef id);
void                    BSP_MOTO_ClkSet     (BSP_ID_TypeDef id, BitAction bit);
void                    BSP_MOTO_DirSet     (BSP_ID_TypeDef id, BSP_MOT_DIR_TypeDef dir);
void                    BSP_MOTO_TQSet      (BSP_ID_TypeDef id, BSP_MOT_TQ_TypeDef tq);
void  					BSP_MOTO_MSet (BSP_ID_TypeDef id, BSP_MOT_M123_TypeDef tq);
#ifdef  PC924
/// -------- motor (controled by closed loop) --------
BSP_MOT_ST_TypeDef      BSP_MOTC_StatusGet  (BSP_ID_TypeDef id);
BitAction               BSP_MOTC_ClkGet     (BSP_ID_TypeDef id);
void                    BSP_MOTC_ClkSet     (BSP_ID_TypeDef id, BitAction bit);
BSP_MOT_DIR_TypeDef     BSP_MOTC_DirGet     (BSP_ID_TypeDef id);
void                    BSP_MOTC_DirSet     (BSP_ID_TypeDef id, BSP_MOT_DIR_TypeDef dir);
void                    BSP_MOTC_TQSet      (BSP_ID_TypeDef id, BSP_MOT_TQ_TypeDef tq);
void                    BSP_MOTC_MSet       (BSP_ID_TypeDef id, BSP_MOT_M_TypeDef m);
void                    BSP_MOTC_STSet      (BSP_ID_TypeDef id, BSP_MOT_IST_TypeDef ist);
#endif

/// -------- SENSOR --------
/// -------- encoder --------
void                    BSP_ENC_CntSet      (BSP_ID_TypeDef id, u16 cnt);
u16                     BSP_ENC_CntGet      (BSP_ID_TypeDef id);
/// -------- 24V detect --------
/// ����
/// -------- approach switch --------
BSP_SWITCH_TypeDef      BSP_SWI_FiGet       (BSP_ID_TypeDef id);
BSP_SWITCH_TypeDef      BSP_SWI_SeGet       (BSP_ID_TypeDef id);
/// -------- potentiometer --------
u16                     BSP_POT_GetOne      (BSP_ID_TypeDef id);
u16                     BSP_POT_GetAve      (BSP_ID_TypeDef id);

/// -------- COMMUNICATION --------
/// -------- 491 --------
void                    BSP_491_Init        (void);
void                    BSP_491_DeSet       (FunctionalState NewState);
/// -------- sequin --------
void                    BSP_SQN_EmbStSend   (BSP_ID_TypeDef id, BSP_EMB_ST_TypeDef st);
void  					Get_Stm32_ID(void);
void  					Get_9511_ID(void);
void                    BSP_SOL_Set         (BSP_ID_TypeDef id, BSP_SOL_TypeDef st, u16 tmr);
void                    BSP_SOL_TmrProc     (void);
BSP_EXT     bool                        BSP_DIP_SpdIsSlow;  /// ��ͷ����ٶ��Ƿ����� (���ش�)
BSP_EXT     bool                        BSP_DIP_RedIsSpkl;  /// ���ߺ���Ƿ���˸

#endif
/// End of file -----------------------------------------------------------------------------------------
