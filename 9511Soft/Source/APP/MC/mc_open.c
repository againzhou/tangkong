
/// ----------------------------------------------------------------------------
/// File Name          : mc_open.c
/// Description        : �ջ�����Ŀ������Ʋ���
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

#define  MCO_MODULE        /// ģ�鶨��, ���������ⲿ����


/// Includes --------------------------------------------------------------------------------------------
#include  <ucos_ii.h>               /// OS�˵�ͷ�ļ�
#include  "mc_base.h"


/// !!!
extern        s32     MCL_MNowR;      /// ���ص����ǰȦ��
extern        u16     MCL_MNowE;      /// ���ص����ǰ���� (������)

/// Private define --------------------------------------------------------------------------------------

#define     MCO_TQ_RUN          BSP_MOT_TQ_50
#define     MCO_TQ_STOP         BSP_MOT_TQ_20

#define     MCO_TAB_N           32

#define     MCO_E_HALF_ELE      (BSP_ENC_N_T_PR / MCB_POLE_PAIRS / 2)       /// ��Ƕȵ�һ��, ��������λ

/// Private typedef -------------------------------------------------------------------------------------
/// Private macro ---------------------------------------------------------------------------------------
/// Private function prototypes -------------------------------------------------------------------------
/// Private variables -----------------------------------------------------------------------------------

static  OS_EVENT*   MCO_StopSemPtr;     /// �������ƽ����ź���ָ��

static  bool    MCO_IsToOrg;            /// ����ģʽ��, ��ԭ�㷽ʽ, ����Ϊλ�÷�ʽ

static  BitAction   MCO_ClkSta;         /// ��¼�����ƽ�ĸߵ�״̬

static  u16     MCO_ClkNum;             /// ��ԭ�㳬ʱ��, ����λ�ò���

/// �����ٶȶ�ʱ����������
static  u8      MCO_SpdIMax =   0;      /// ��Ӧ��󿪻��ٶ�
static  u8      MCO_SpdIMin =   0;      /// ��Ӧ��С�����ٶ�
static  u16     MCO_SpdINow =   0;      /// ��Ӧ��ǰ�����ٶ�

static  u8      MCO_AccUp   =   1;      /// �������ټ��ٶ�
static  u8      MCO_AccDn   =   1;      /// �������ټ��ٶ�


/// Private consts --------------------------------------------------------------------------------------

static  const   u16     MCO_ClkTimD2[MCO_TAB_N]    =   {
    32000,  28800,  25920,  23328,  20995,  18896,  17006,  15306,
    13775,  12397,  11158,  10042,   9038,   8134,   7321,   6589,
     5930,   5337,   4803,   4323,   3890,   3501,   3151,   2836,
     2553,   2297,   2068,   1861,   1675,   1507,   1357,   1221
};


/// Private functions ---------------------------------------------------------





void  MCO_Init (void)
{
    MCO_StopSemPtr  =   OSSemCreate(0);
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  �������Ʒ�ʽ�Ķ�ʱ�жϷ�����
///         ���������: һ���˶���λ���˳�, �����ҵ�ԭ����˳�
/// @param  None
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
void  MCO_IRQ (void)
{
    BitAction       st_odd;
    BitAction       st_even;
    u32             ceiling;
    
    
    BSP_CCR_MC_OPEN +=  MCO_ClkTimD2[MCO_SpdINow];                  /// �����¸��ж�ʱ��
    
    if (MCO_ClkSta == Bit_RESET) {                                  /// ����ǵ͵�ƽ������ʱ��
        MCO_ClkSta  =   Bit_SET;                                    /// ����Ϊ�ߵ�ƽ״̬
        
        if (MCO_ClkNum != 0) {                                      /// ���û�г�ʱ��λ
            MCO_ClkNum--;
            if (MCO_IsToOrg != FALSE) {                             /// ��ԭ��ģʽ
                if (MCB_OrgIs0[BSP_ID_ODD] == FALSE) {              /// �������ͷû���ҵ�ԭ��
                    st_odd  =   Bit_SET;                            /// ��Ϊ�ߵ�ƽ
                } else {                                            /// �������ͷ�Ѿ��ҵ�ԭ��
                    st_odd  =   Bit_RESET;                          /// ��Ϊ�͵�ƽ
                }
                if (MCB_OrgIs0[BSP_ID_EVEN] == FALSE) {             /// ���ż��ͷû���ҵ�ԭ��
                    st_even =   Bit_SET;                            /// ��Ϊ�ߵ�ƽ
                } else {                                            /// ���ż��ͷ�Ѿ��ҵ�ԭ��
                    st_even =   Bit_RESET;                          /// ��Ϊ�͵�ƽ
                }
            } else {                                                /// ����λ�ÿ���ģʽ
                st_odd  =   Bit_SET;
                st_even =   Bit_SET;
            }
        } else {                                                    /// ����Ѿ�����Ŀ��λ�û�ʱ
            st_odd  =   Bit_RESET;
            st_even =   Bit_RESET;
        }
        
        BSP_MOTC_ClkSet(BSP_ID_ODD , st_odd);                       /// ���������ƽ
        BSP_MOTC_ClkSet(BSP_ID_EVEN, st_even);
        
        if ((st_odd == Bit_RESET) && (st_even == Bit_RESET)) {      /// ���跢����
            TIM_ITConfig(BSP_TMR_8M, BSP_FLAG_CC_MC_OPEN, DISABLE); /// �ر��ж�
            OSSemPost(MCO_StopSemPtr);                              /// �����ź���
        } else {
            ;
        }
        
    } else {                                                        /// ����Ǹߵ�ƽ�½���ʱ��
        MCO_ClkSta  =   Bit_RESET;                                  /// ����Ϊ�͵�ƽ�׶�
        
        BSP_MOTC_ClkSet(BSP_ID_ODD , Bit_RESET);                    /// ���������ƽΪ��
        BSP_MOTC_ClkSet(BSP_ID_EVEN, Bit_RESET);
        
        ceiling     =   MCO_ClkNum * MCO_AccDn + MCO_SpdIMin;       /// ���㵽λǰ����ʱ������
        if (ceiling > MCO_SpdIMax) {                                /// �����������
            ceiling =   MCO_SpdIMax;
        }
        
        MCO_SpdINow   +=   MCO_AccUp;                               /// ��������ʱ�ٶ�
        if (MCO_SpdINow > ceiling) {                                /// ����
            MCO_SpdINow    =   ceiling;
        }
    }
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  ��������˶�, ֻ������˶�, ���Һ���
///         һ�����ڻ�ɫ, �λ�, ��ԭ��ʱ��ƫ���˶�
/// @param  dir     :   �˶��ķ���
///         enc     :   �˶���·��, ��λ�Ǳ���������
///         spd_min :   ��С�˶��ٶ�, 0 ~ MCO_TAB_N - 1, ��֤ spd_min <= spd_max
///         spd_max :   ����˶��ٶ�, 0 ~ MCO_TAB_N - 1, ��֤ spd_min <= spd_max
/// @retval :   None
/// -----------------------------------------------------------------------------------------------------
void  MCO_Mov (BSP_MOT_DIR_TypeDef dir, u16 enc, u8 spd_min, u8 spd_max)
{
    u8      err;
    
    /// !!!         ������Χ
    
    
    MCO_IsToOrg     =   FALSE;                                  /// ����Ϊ����ģʽ, ����ԭ��ģʽ
    
    MCO_SpdIMin     =   spd_min;                                /// ������С�ٶ�
    MCO_SpdIMax     =   spd_max;                                /// ��������ٶ�
    MCO_SpdINow     =   spd_min;                                /// ���õ�ǰ�ٶ�
    
    BSP_MOTC_TQSet(BSP_ID_ODD , MCO_TQ_RUN);                    /// ���ÿ�������
    BSP_MOTC_TQSet(BSP_ID_EVEN, MCO_TQ_RUN);
    
    BSP_MOTC_ClkSet(BSP_ID_ODD , Bit_RESET);                    /// ���������źŵ�ƽΪ��
    BSP_MOTC_ClkSet(BSP_ID_EVEN, Bit_RESET);
    MCO_ClkSta      =   Bit_RESET;                              /// ��¼�����ƽΪ��
    
    BSP_MOTC_DirSet(BSP_ID_ODD , dir);                          /// ���÷���
    BSP_MOTC_DirSet(BSP_ID_EVEN, dir);
    
    MCO_ClkNum      =   enc * MCB_P_DIV_E;                      /// �����˶���������
    
    BSP_CCR_MC_OPEN =   BSP_TM8M_Get() + spd_min;               /// ��������ʱ��
    
    TIM_ClearITPendingBit(BSP_TMR_8M, BSP_FLAG_CC_MC_OPEN);     /// ���ж�
    TIM_ITConfig(BSP_TMR_8M, BSP_FLAG_CC_MC_OPEN, ENABLE);      /// ���ж�
    
    OSSemPend(MCO_StopSemPtr, 0, &err);                         /// �ȴ��˶�����
    
    BSP_MOTC_TQSet(BSP_ID_ODD , BSP_MOT_TQ_20);                 /// ����ΪС����
    BSP_MOTC_TQSet(BSP_ID_EVEN, BSP_MOT_TQ_20);
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  �����ԭ��, ��������ԭ��
///         0λ, ������0, �������(��Ƕ�)���߶���
/// @param  :   None
/// @retval :   TRUE    -   �ɹ�
///             FALSE   -   ʧ��, ��ʱ
/// -----------------------------------------------------------------------------------------------------
bool  MCO_Org (void)
{
    BSP_MOT_DIR_TypeDef  dir;
    u8      i;
    u8      j;
    u16     enc;
    u8      err;
    
    
//    MCH_CloseExit();
    
    /// ����һ������ҹ�ԭ��, λ�ÿ�֪, �������״���ԭ��
    if ((MCB_OrgIs0[BSP_ID_ODD] != FALSE) || (MCB_OrgIs0[BSP_ID_EVEN] != FALSE)) {
                                                                /// ��ǰλ����ԭ�㸽��
        if ((MCL_MNowE <= MCO_E_HALF_ELE) || (MCL_MNowE >= BSP_ENC_N_T_PR - MCO_E_HALF_ELE)) {
            MCO_Mov(MCB_DIR_INC, MCO_E_HALF_ELE * 2, 13, 29);
            OSTimeDlyHMSM(0, 0, 0, 100);
            dir =   MCB_DIR_DEC;
        } else if (MCL_MNowE < BSP_ENC_N_T_PHR) {
            dir =   MCB_DIR_DEC;
        } else if (MCL_MNowE > BSP_ENC_N_T_PHR) {
            dir =   MCB_DIR_INC;
        } else {
            if (MCL_MNowR < 0) {
                dir =   MCB_DIR_INC;
            } else {
                dir =   MCB_DIR_DEC;
            }
        }
    } else {                                                    /// �״���ԭ�㰴�������˶�
        dir =   MCB_DIR_DEC;
    }
    
    MCO_IsToOrg             =   TRUE;                           /// ������Ҫ��ԭ��
    MCB_OrgIs0[BSP_ID_ODD]  =   FALSE;
    MCB_OrgIs0[BSP_ID_EVEN] =   FALSE;
    
    BSP_MOTC_TQSet(BSP_ID_ODD , MCO_TQ_RUN);                    /// ���õ���
    BSP_MOTC_TQSet(BSP_ID_EVEN, MCO_TQ_RUN);
    
    BSP_MOTC_DirSet(BSP_ID_ODD , dir);                          /// ���÷���
    BSP_MOTC_DirSet(BSP_ID_EVEN, dir);
    
    BSP_MOTC_ClkSet(BSP_ID_ODD , Bit_RESET);                    /// ���������źŵ�ƽΪ��
    BSP_MOTC_ClkSet(BSP_ID_EVEN, Bit_RESET);
    MCO_ClkSta      =   Bit_RESET;                              /// ��¼�����ƽΪ��
    
    MCO_ClkNum      =   MCB_N_P_PR * 55 / 50;                   /// ������ԭ�㳬ʱ������
    
    MCO_SpdIMin     =   13;                                     /// ������С�ٶ�
    MCO_SpdIMax     =   29;                                     /// ��������ٶ�
    MCO_SpdINow     =   MCO_SpdIMin;                            /// ���õ�ǰ�ٶ�
    
    BSP_CCR_MC_OPEN =   BSP_TM8M_Get() + MCO_SpdINow;           /// ��������ʱ��
    
    TIM_ClearITPendingBit(BSP_TMR_8M, BSP_FLAG_CC_MC_OPEN);     /// ���ж�
    TIM_ITConfig(BSP_TMR_8M, BSP_FLAG_CC_MC_OPEN, ENABLE);      /// ���ж�
    
    OSSemPend(MCO_StopSemPtr, 0, &err);                         /// �ȴ��˶�����
    
    BSP_MOTC_TQSet(BSP_ID_ODD , BSP_MOT_TQ_20);                 /// ����С����
    BSP_MOTC_TQSet(BSP_ID_EVEN, BSP_MOT_TQ_20);
    
    if (MCO_ClkNum != 0) {                                      /// û�г�ʱ
        /// ����Ϊ��ȷ��ԭ��
        /// !!! Ӧʹ��С������ԭ��, С������λ׼, ����ջ�����״̬һ��.
        /// !!! �л���������ʹλ�øı� !!!
        OSTimeDlyHMSM(0, 0, 0, 200);                            /// �����������, һ��Խ��Խ��, >=100
        
        for (i = 0; i < BSP_DEVICE_ID_N; i++) {
            enc =   BSP_ENC_CntGet((BSP_ID_TypeDef)i);
            if (enc < BSP_ENC_N_T_PHR) {
                dir =   MCB_DIR_DEC;
            } else {
                dir =   MCB_DIR_INC;
                enc =   BSP_ENC_N_T_PR - enc;
            }
            
            BSP_MOTC_DirSet((BSP_ID_TypeDef)i, dir);
            for (j = enc * MCB_P_DIV_E; j != 0; j--) {
                OSTimeDlyHMSM(0, 0, 0, 1);
                BSP_MOTC_ClkSet((BSP_ID_TypeDef)i, Bit_SET);
                OSTimeDlyHMSM(0, 0, 0, 1);
                BSP_MOTC_ClkSet((BSP_ID_TypeDef)i, Bit_RESET);
            }
            
            MCB_PulseR[i]    =   0;                             /// �������(��Ƕ�)��������
            MCB_PulseE[i]    =   0;
        }
        
//        MCH_SetMOrg();
        
        return  TRUE;                                           /// ������ԭ����ȷ
        
    } else {
        return  FALSE;                                          /// ������ԭ�����, ����ʱ
    }
}




#endif

/// End of file -----------------------------------------------------------------------------------------
