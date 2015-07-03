
/// ----------------------------------------------------------------------------
/// File Name          : mc_close_high.c
/// Description        : �ջ�����ıջ����Ƹ߲㲿��
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

#define  MCH_MODULE        /// ģ�鶨��, ���������ⲿ����


/// Includes --------------------------------------------------------------------------------------------
#include  <ucos_ii.h>               /// OS�˵�ͷ�ļ�
#include  "mc_base.h"
#include  "mc_close_low.h"


/// Private define --------------------------------------------------------------------------------------
/// Private typedef -------------------------------------------------------------------------------------
/// Private macro ---------------------------------------------------------------------------------------
/// Private function prototypes -------------------------------------------------------------------------
/// Private variables -----------------------------------------------------------------------------------

/// E=0~1599,R=0; E=1600~3199,R=1; E=-1600~-1,R=-1

static  s32     MCH_MDecR   =   0;      /// ���ص����������Ȧ��
static  u16     MCH_MDecE   =   0;      /// ���ص���������޷��� (������)
static  s32     MCH_MIncR   =   0;      /// ���ص����������Ȧ��
static  u16     MCH_MIncE   =   0;      /// ���ص���������޷��� (������)


/// variables -------------------------------------------------------------------------------------------

s32     MCH_MStarR  =   0;      ///  D�� H�ῪʼȦ��
u16     MCH_MStarE  =   0;      ///  D�� H�Ὺʼ���� (������)
s32     MCH_MStopR  =   0;      ///  D�� H�����Ȧ��
u16     MCH_MStopE  =   0;      ///  D�� H��������� (������)
s32     MCH_MBackR  =   0;      ///  H���۷���Ȧ��
u16     MCH_MBackE  =   0;      ///  H���۷��㷽�� (������)





/// Private consts --------------------------------------------------------------------------------------

/// Private functions ---------------------------------------------------------
            
            static  void    MCH_DecRE       (s32 *r, u16 *e);
            static  void    MCH_IncRE       (s32 *r, u16 *e);




            
/// -----------------------------------------------------------------------------------------------------
/// @brief  D �� H �����Ŀ��λ����������, ��ԭ��
/// @param  :   None
/// @retval :   None
/// -----------------------------------------------------------------------------------------------------
void  MCH_SetMOrg (void)
{
    MCH_MStarR  =   0;      ///  D�� H������ʼȦ��
    MCH_MStarE  =   0;      ///  D�� H������ʼ���� (������)
    MCH_MBackR  =   0;      ///  H�������ص�Ȧ��
    MCH_MBackE  =   0;      ///  H�������ص㿪ʼ���� (������)
    MCH_MStopR  =   0;      ///  D�� H�����Ȧ��
    MCH_MStopE  =   0;      ///  D�� H��������� (������)   
    
    MCH_MDecR   =   0;      /// ���ص����������Ȧ��
    MCH_MDecE   =   0;      /// ���ص���������޷��� (������)
    MCH_MIncR   =   0;      /// ���ص����������Ȧ��
    MCH_MIncE   =   0;      /// ���ص���������޷��� (������)
    
    MCL_SetMOrg();
}

/// --------    �ջ����� -- Ŀ��λ�ÿ������    --------

void  MCH_CloseEnter (BSP_MOT_TQ_TypeDef tq, u16 StartTiming)
{
    BSP_MOTC_TQSet(BSP_ID_ODD , tq);                            /// ���õ���
    BSP_MOTC_TQSet(BSP_ID_EVEN, tq);
    
    BSP_MOTC_ClkSet(BSP_ID_ODD , Bit_RESET);                    /// ���������ƽΪ��
    BSP_MOTC_ClkSet(BSP_ID_EVEN, Bit_RESET);
    
    BSP_CCR_MC_CLOSE    =   BSP_TM8M_Get() + StartTiming;       /// ��������ʱ��
    TIM_ClearITPendingBit(BSP_TMR_8M, BSP_FLAG_CC_MC_CLOSE);    /// ���ж�
    TIM_ITConfig(BSP_TMR_8M, BSP_FLAG_CC_MC_CLOSE, ENABLE);     /// ���ж�
}

void  MCH_CloseExit (void)
{
    TIM_ITConfig(BSP_TMR_8M, BSP_FLAG_CC_MC_CLOSE, DISABLE);    /// �ر��ж�
    
    BSP_MOTC_ClkSet(BSP_ID_ODD , Bit_RESET);                    /// ���������ƽΪ��
    BSP_MOTC_ClkSet(BSP_ID_EVEN, Bit_RESET);
    
    BSP_MOTC_TQSet(BSP_ID_ODD , BSP_MOT_TQ_20);                 /// ����ΪС����
    BSP_MOTC_TQSet(BSP_ID_EVEN, BSP_MOT_TQ_20);
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  �ջ������˶�, �����򷢳�΢��ָ��, ����D���H��ջ��˶�����
///         �����ڲ�����, ���Բ���������Χ
/// @param  spd_d10 :   �����ٶ�ֵ��1/10, ��λ�� 10rpm
///         cnt     :   ��ʼ�˶���ʱ�䣬  ��λ������ģ�����������
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
void  MCH_CloseMov1 (u8 spd_d10, u16 cnt)
{
#if OS_CRITICAL_METHOD == 3
    OS_CPU_SR  cpu_sr = 0;
#endif
    s32     dif;
    
    
    BSP_TM_CNT_SpdSet(spd_d10);
    
    TIM_ITConfig(BSP_TMR_CNT, BSP_FLAG_CC_MCC_DEC, DISABLE);        /// �ر�ģ�������λ���ж�
    TIM_ITConfig(BSP_TMR_CNT, BSP_FLAG_CC_MCC_INC, DISABLE);        /// �ر�ģ�������λ���ж�
        
    dif =   (MCH_MStopR - MCH_MStarR) * BSP_ENC_N_T_PR + MCH_MStopE - MCH_MStarE;
        
    OS_ENTER_CRITICAL();
    MCL_MNowR   =   MCH_MStarR;
    MCL_MNowE   =   MCH_MStarE;
    
    if (dif < 0) {
        MCH_MDecR   =   MCH_MStopR;
        MCH_MDecE   =   MCH_MStopE;
        MCH_MIncR   =   MCH_MStarR;
        MCH_MIncE   =   MCH_MStarE;
        OS_EXIT_CRITICAL();
        
        BSP_CCR_MCC_DEC =   BSP_TM_CNT_Get() + cnt;                 /// ��������ʱ��
        TIM_ClearITPendingBit(BSP_TMR_CNT, BSP_FLAG_CC_MCC_DEC);    /// ���ж�
        TIM_ITConfig(BSP_TMR_CNT, BSP_FLAG_CC_MCC_DEC, ENABLE);     /// ���ж�
        
    } else if (dif == 0) {
        OS_EXIT_CRITICAL();
        
    } else {
        MCH_MDecR   =   MCH_MStarR;
        MCH_MDecE   =   MCH_MStarE;
        MCH_MIncR   =   MCH_MStopR;
        MCH_MIncE   =   MCH_MStopE;
        OS_EXIT_CRITICAL();
        
        BSP_CCR_MCC_INC =   BSP_TM_CNT_Get() + cnt;                 /// ��������ʱ��
        TIM_ClearITPendingBit(BSP_TMR_CNT, BSP_FLAG_CC_MCC_INC);    /// ���ж�
        TIM_ITConfig(BSP_TMR_CNT, BSP_FLAG_CC_MCC_INC, ENABLE);     /// ���ж�
    }
    
    MCH_CloseEnter(BSP_MOT_TQ_20, 100);
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  �ջ�˫���˶�, ������һ����΢��ָ����ٷ�����һ�����΢��ָ��, ����H�����ջ��˶�����
///         �����ڲ�����, ���Բ���������Χ
/// @param  spd_d10 :   �����ٶ�ֵ��1/10, ��λ�� 10rpm
///         cnt1    :   ��һ����ʼ�˶���ʱ�䣬  ��λ������ģ�����������
///         cnt2    :   ��һ����ʼ�˶���ʱ�䣬  ��λ������ģ�����������
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
void  MCH_CloseMov2 (u8 spd_d10, u16 cnt1, u16 cnt2)
{
#if OS_CRITICAL_METHOD == 3
    OS_CPU_SR  cpu_sr = 0;
#endif
    u16     cnt;
    
    
    BSP_TM_CNT_SpdSet(spd_d10);
    
    TIM_ITConfig(BSP_TMR_CNT, BSP_FLAG_CC_MCC_DEC, DISABLE);        /// �ر�ģ�������λ���ж�
    TIM_ITConfig(BSP_TMR_CNT, BSP_FLAG_CC_MCC_INC, DISABLE);        /// �ر�ģ�������λ���ж�
    
    OS_ENTER_CRITICAL();
    
    MCL_MNowR   =   MCH_MStarR;
    MCL_MNowE   =   MCH_MStarE;
    
    MCH_MIncR   =   MCH_MBackR;
    MCH_MIncE   =   MCH_MBackE;
    MCH_MDecR   =   MCH_MStopR;
    MCH_MDecE   =   MCH_MStopE;
    
    OS_EXIT_CRITICAL();
        
    cnt         =   BSP_TM_CNT_Get();
    
    BSP_CCR_MCC_INC =   cnt + cnt1;                                 /// ��������ʱ��
    TIM_ClearITPendingBit(BSP_TMR_CNT, BSP_FLAG_CC_MCC_INC);        /// ���ж�
    TIM_ITConfig(BSP_TMR_CNT, BSP_FLAG_CC_MCC_INC, ENABLE);         /// ���ж�
    
    BSP_CCR_MCC_DEC =   cnt + cnt2;                                 /// ��������ʱ��
    TIM_ClearITPendingBit(BSP_TMR_CNT, BSP_FLAG_CC_MCC_DEC);        /// ���ж�
    TIM_ITConfig(BSP_TMR_CNT, BSP_FLAG_CC_MCC_DEC, ENABLE);         /// ���ж�
    
    MCH_CloseEnter(BSP_MOT_TQ_20, 100);
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  ������ֵ��1, ���������Ȧ����1
///         �����ڲ�����, ���Բ���������Χ
/// @param  r   :   ���Ȧ���ĵ�ַ, *e���жϷ�Χ, Ӧ��[0, BSP_ENC_N_T_PR)��,
///         e   :   ��ű�����ֵ�ĵ�ַ, *rҲ���жϷ�Χ
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
static  void  MCH_DecRE (s32 *r, u16 *e)
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
static  void  MCH_IncRE (s32 *r, u16 *e)
{
    if ((*e) < BSP_ENC_N_T_PR - 1) {
        (*e)++;
    } else {
        (*e)    =   0;
        (*r)++;
    }
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  ����Ŀ��λ�ü���, ��ģ������ж��е���. for D-, H-(H2)
///         ����Ŀ��λ�ú������������Ŀ��λ��, ��û�����Ϲرմ��ж�, �����¸��ж��йر�
/// @param  None
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
void  MCH_AimDecIRQ (void)
{
#if OS_CRITICAL_METHOD == 3
    OS_CPU_SR  cpu_sr = 0;
#endif
    
    
    OS_ENTER_CRITICAL();
    
    if ((MCL_MNowR - MCH_MDecR) * BSP_ENC_N_T_PR + MCL_MNowE - MCH_MDecE > 0) {     /// �����������λ��
        MCH_DecRE(&MCL_MNowR, &MCL_MNowE);                                          /// λ�ü���
        BSP_CCR_MCC_DEC +=  7;                                                      /// �����¸�����ʱ��
    } else {
        TIM_ITConfig(BSP_TMR_CNT, BSP_FLAG_CC_MCC_DEC, DISABLE);                    /// �رն�Ӧ�ж�
    }
    
    OS_EXIT_CRITICAL();
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  ����Ŀ��λ������, ��ģ������ж��е���. for D+, H+(H1)
///         ����Ŀ��λ�ú������������Ŀ��λ��, ��û�����Ϲرմ��ж�, �����¸��ж��йر�
/// @param  None
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
void  MCH_AimIncIRQ (void)
{
#if OS_CRITICAL_METHOD == 3
    OS_CPU_SR  cpu_sr = 0;
#endif
    
    
    OS_ENTER_CRITICAL();
    
    if ((MCL_MNowR - MCH_MIncR) * BSP_ENC_N_T_PR + MCL_MNowE - MCH_MIncE < 0) {     /// ���С������λ��
        MCH_IncRE(&MCL_MNowR, &MCL_MNowE);                                          /// λ������
        BSP_CCR_MCC_INC +=  7;                                                      /// �����¸�����ʱ��
    } else {
        TIM_ITConfig(BSP_TMR_CNT, BSP_FLAG_CC_MCC_INC, DISABLE);                    /// �رն�Ӧ�ж�
    }
    
    OS_EXIT_CRITICAL();
}




#endif

/// End of file -----------------------------------------------------------------------------------------
