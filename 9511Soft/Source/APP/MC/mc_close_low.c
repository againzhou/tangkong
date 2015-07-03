
/// ----------------------------------------------------------------------------
/// File Name          : mc_close_low.c
/// Description        : �ջ�����ıջ����ƵͲ㲿��
///
///     D   --  ��ʾ D��, ���뱳����, ��λΪ1/400Ȧ
///             (ë����ʱָ���߼��յ㷽��, ��ʽ��ʱָ���߼���㷽��)
///     DM  --  ��ʾ D������, ��λΪ����������, ��1/1600Ȧ
///     H   --  ��ʾ H��, ������׷���, ��λΪ1/400Ȧ
///             (��ԭ����ߵ�ʱ, H����D���غ�; ����ʱ, H�����D��ƫת-1/8r)
///     HM  --  ��ʾ H������, ��λΪ����������, ��1/1600Ȧ
///             (a��λHM��H��ԭ���غ�, b~f��λ���غ�)
///     M   --  ��ʾ���ص����, �������»�ͷ��ȷ�� D�� H, ����Ŀ�����.
///
///     R   --  ��ʾȦ��, ��λΪ"Ȧ"
///     V   --  ��ʾD ��H �᷽��, ��λΪ"1/400Ȧ", ͨѶ������˵�.
///     E   --  ��ʾDM��HM�᷽��, ��λΪ�������ֱ���, ��"1/1600Ȧ".
///
///     �ջ���������ֿ��Ʒ�ʽ: ������ʽ �� �ջ���ʽ
///     ����D��, �����¶���:  
///         1 ��ԭ��        (����)  ���һ�еԭ��, �ҵ���M��T���������      
///         2 ת��ĳ����    (�ջ�)  �ջ�����, һ������ʱʹ��
///         3 �Ҵ��ߵ�      (���)  ��1��2������, ����������������෴
///     ����H��, ��Ը���, ���л��������Ķ�����Ҫ�ջ�����, �����¶���:
///         1 ��ԭ��        (���)  �һ�еԭ���, Ȼ������λԭ��. ����Ϊ��ƫ��״̬
///         2 �Ҵ��ߵ�      (���)  �һ�еԭ���, Ȼ������λ���ߵ�. ����Ϊ��ƫ��״̬
///         3 ƫ�ƶ���      (�ջ�)  ������ת45��. ����Ϊƫ��״̬ (ƫ��״̬���ܴ���)
///         4 �򻷶���      (�ջ�)  H1 �� H2, ����T�����, ������T���������M�����
///         5 �λ�����      (����)  ������������һζ�
///         6 ��ɫ����      (����)  ֻ�Ǹ��µ�ǰ��λ �� M�����, T�����������
/// ----------------------------------------------------------------------------

/// History:

/// y10/m01/d17     v0.02   chenyong    modify
/// ��ģ��mcc�������ģ��

/// y10/m11/d08     v0.01   chenyong    setup



#ifdef  PC924

#define  MCL_MODULE        /// ģ�鶨��, ���������ⲿ����


/// Includes --------------------------------------------------------------------------------------------
#include  <ucos_ii.h>               /// OS�˵�ͷ�ļ�
#include  "mc_base.h"
//#include  "mc_open.h"


/// Private define --------------------------------------------------------------------------------------

#define     MCL_TQ_RUN          BSP_MOT_TQ_20


#define     MCL_E_MAX_TQ        (BSP_ENC_N_T_PR / MCB_POLE_PAIRS / 4)       /// �������λ��, ��������λ
#define     MCL_E_COMP_F        5                                           /// ��ת����, ��������λ
#define     MCL_E_COMP_B        (-4)                                        /// ��ת����, ��������λ
#define     MCL_E_MAX_TQ_F      (MCL_E_MAX_TQ + MCL_E_COMP_F)               /// ��ת�������λ��, ��������λ
#define     MCL_E_MAX_TQ_B      (MCL_E_MAX_TQ + MCL_E_COMP_B)               /// ��ת�������λ��, ��������λ

#define     MCL_ENDURE_DIF      (BSP_ENC_N_T_PR / 40)                       /// ��������������


/// for speed fast
#define     MCL_OddClkSet()     (GPIOD->BSRR    =   0x01 <<  9)
#define     MCL_OddClkReset()   (GPIOD->BRR     =   0x01 <<  9)

#define     MCL_EvenClkSet()    (GPIOD->BSRR    =   0x01 << 15)
#define     MCL_EvenClkReset()  (GPIOD->BRR     =   0x01 << 15)

#define     MCL_OddDirSetInc()  (GPIOB->BSRR    =   0x01 << 14)
#define     MCL_OddDirSetDec()  (GPIOB->BRR     =   0x01 << 14)

#define     MCL_EvenDirSetInc() (GPIOD->BSRR    =   0x01 << 12)
#define     MCL_EvenDirSetDec() (GPIOD->BRR     =   0x01 << 12)



/// Private typedef -------------------------------------------------------------------------------------
/// Private macro ---------------------------------------------------------------------------------------
/// Private function prototypes -------------------------------------------------------------------------
/// Private variables -----------------------------------------------------------------------------------

/// variables -------------------------------------------------------------------------------------------

s32     MCL_MNowR   =   0;      /// ���ص����ǰȦ��
u16     MCL_MNowE   =   0;      /// ���ص����ǰ���� (������)





/// Private consts --------------------------------------------------------------------------------------



/// Private functions ---------------------------------------------------------
            
            static  s8      MCL_GetDeltaP   (BSP_ID_TypeDef id);
            static  void    MCL_DecRE       (s32 *r, u16 *e);
            static  void    MCL_IncRE       (s32 *r, u16 *e);










void  MCL_SetMOrg (void)
{
    MCL_MNowR   =   0;      /// ���ص����ǰȦ��
    MCL_MNowE   =   0;      /// ���ص����ǰ���� (������)
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  ���ݱ�������Ŀ��λ��, ��������λ������
/// @param  id    : The ID of close_loop motor (the odd or even)
/// @retval : ����λ������
///        -1   -   ����λ�ü���, ����������
///         0   -   ����λ�ò���, ��������
///         1   -   ����λ������, ����������
/// -----------------------------------------------------------------------------------------------------
static  s8  MCL_GetDeltaP (BSP_ID_TypeDef id)
{
    s32         d_pe;
    s32         d_cp;
    s8          delta;
#if OS_CRITICAL_METHOD == 3
    OS_CPU_SR   cpu_sr  =   0;
#endif
    
    
    OS_ENTER_CRITICAL();
    
    /// ����λ����ʵ��λ��(������ֵ)�Ĳ�ֵ
//    d_pe    =   (MCB_PulseR[id] - MCC_R[id]) * BSP_ENC_N_T_PR + MCB_PulseE[id] - BSP_ENC_CntGet(id);
    d_pe    =   MCB_PulseE[id] - BSP_ENC_CntGet(id);
    if (d_pe >= BSP_ENC_N_T_PHR) {
        d_pe   -=   BSP_ENC_N_T_PR;
    } else if (d_pe < -BSP_ENC_N_T_PHR) {
        d_pe   +=   BSP_ENC_N_T_PR;
    } else {
        ;
    }
    /// ����λ��������λ�õĲ�ֵ
    d_cp    =   (MCL_MNowR - MCB_PulseR[id]) * BSP_ENC_N_T_PR + MCL_MNowE - MCB_PulseE[id];
    
    OS_EXIT_CRITICAL();
    
    if (d_cp < 0) {
        if (d_pe > -MCL_E_MAX_TQ) {             /// û�е���������ؽǶ�
            delta   =  -1;                      /// ����λ��������λ�ÿ���
        } else if (d_pe == -MCL_E_MAX_TQ) {     /// ����������ؽǶ�
            delta   =   0;                      /// ����λ�ò���
        } else {                                /// ����������ؽǶ�
            delta   =   1;                      /// ����λ����ʵ��λ�ÿ���
        }
    } else if (d_cp > 0) {
        if (d_pe < MCL_E_MAX_TQ) {              /// û�е���������ؽǶ�
            delta   =   1;                      /// ����λ��������λ�ÿ���
        } else if (d_pe == MCL_E_MAX_TQ) {      /// ����������ؽǶ�
            delta   =   0;                      /// ����λ�ò���
        } else {                                /// ����������ؽǶ�
            delta   =  -1;                      /// ����λ����ʵ��λ�ÿ���
        }
    } else {
        if (d_pe < -MCL_E_MAX_TQ) {             /// ����������ؽǶ�
            delta   =   1;                      /// ����λ����ʵ��λ�ÿ���
        } else if (d_pe > MCL_E_MAX_TQ) {       /// ����������ؽǶ�
            delta   =  -1;                      /// ����λ����ʵ��λ�ÿ���
        } else {                                /// û�г���������ؽǶ�
            delta   =   0;                      /// ����λ�ò���
        }
    }
    
    return  delta;
}


/// -----------------------------------------------------------------------------------------------------
/// @brief  ������ֵ��1, ���������Ȧ����1
///         �����ڲ�����, ���Բ���������Χ
/// @param  r   :   ���Ȧ���ĵ�ַ, *e���жϷ�Χ, Ӧ��[0, BSP_ENC_N_T_PR)��,
///         e   :   ��ű�����ֵ�ĵ�ַ, *rҲ���жϷ�Χ
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
static  void  MCL_DecRE (s32 *r, u16 *e)
{
    if ((*e) != 0) {
        (*e)--;
    } else {
        (*e)    =   BSP_ENC_N_T_PR - 1;
        (*r)--;
    }
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  ������ֵ��1, ���������Ȧ����1
///         �����ڲ�����, ���Բ���������Χ
/// @param  r   :   ���Ȧ���ĵ�ַ, *e���жϷ�Χ, Ӧ��[0, BSP_ENC_N_T_PR)��,
///         e   :   ��ű�����ֵ�ĵ�ַ, *rҲ���жϷ�Χ
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
static  void  MCL_IncRE (s32 *r, u16 *e)
{
    if ((*e) < BSP_ENC_N_T_PR - 1) {
        (*e)++;
    } else {
        (*e)    =   0;
        (*r)++;
    }
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  �ջ��˶����ƵĶ�ʱ�жϷ�����
///         �����źŵĽ�����Ҫ���������ź�, ��������Ҫ����8uS, ��Ƶ��С��60K.
/// @param  None
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
void  MCL_IRQ (void)
{
    static  s8      DeltaOdd    =   0;
    static  s8      DeltaEven   =   0;
    
    
    if (DeltaOdd == 0) {
        MCL_OddClkReset();
    } else {
        MCL_OddClkSet();
    }
    
    if (DeltaEven == 0) {
        MCL_EvenClkReset();
    } else {
        MCL_EvenClkSet();
    }
    
    if (DeltaOdd == 0) {
        DeltaOdd    =   MCL_GetDeltaP(BSP_ID_ODD);
        if (DeltaOdd < 0) {
            MCL_OddDirSetDec();
        } else if (DeltaOdd > 0) {
            MCL_OddDirSetInc();
        }
    } else {
        if (DeltaOdd < 0) {
            MCL_DecRE(&MCB_PulseR[BSP_ID_ODD], &MCB_PulseE[BSP_ID_ODD]);
        } else {
            MCL_IncRE(&MCB_PulseR[BSP_ID_ODD], &MCB_PulseE[BSP_ID_ODD]);
        }
        DeltaOdd    =   0;
    }
    
    if (DeltaEven == 0) {
        DeltaEven   =   MCL_GetDeltaP(BSP_ID_EVEN);
        if (DeltaEven < 0) {
            MCL_EvenDirSetDec();
        } else if (DeltaEven > 0) {
            MCL_EvenDirSetInc();
        }
    } else {
        if (DeltaEven < 0) {
            MCL_DecRE(&MCB_PulseR[BSP_ID_EVEN], &MCB_PulseE[BSP_ID_EVEN]);
        } else {
            MCL_IncRE(&MCB_PulseR[BSP_ID_EVEN], &MCB_PulseE[BSP_ID_EVEN]);
        }
        DeltaEven   =   0;
    }
    
    BSP_CCR_MC_CLOSE    =   BSP_TM8M_Get() + BSP_TMR_8M_CLK * 3 / 1000000;
//    BSP_CCR_MC_CLOSE    =   BSP_TMR_8M_CLK * 20 / 1000000;
}




#endif

/// End of file -----------------------------------------------------------------------------------------
