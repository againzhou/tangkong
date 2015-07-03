
/// ----------------------------------------------------------------------------
/// File Name          : mc_cmd.c
/// Description        : �ջ������������Ʋ���
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

#define  MCC_MODULE        /// ģ�鶨��, ���������ⲿ����


/// Includes --------------------------------------------------------------------------------------------
#include  <ucos_ii.h>               /// OS�˵�ͷ�ļ�
#include  "mc_base.h"
#include  "mc_open.h"
#include  "mc_close_low.h"
#include  "mc_close_high.h"


/// Private define --------------------------------------------------------------------------------------

#define     MCC_V_NUM_PR        400                                         /// һ��Ȧ��Ӧ������ֵ
#define     MCC_V_HLF_PR        (MCC_V_NUM_PR >> 1)                         /// ��Ȧ��Ӧ������ֵ

#define     MCC_V_HOFFSET       (-(MCC_V_NUM_PR / 8))                       /// ����ƫ��ֵΪ -r/8

#define     MCC_N_HM_GEAR       64                                          /// H�������ֳ���
#define     MCC_N_H_GEAR        16                                          /// H��(Looper)���ֳ���
#define     MCC_N_COLOR_GEAR    6                                           /// ��ɫʱH��������ת���ĳ���
#define     MCC_N_LOOPER        6                                           /// Looper ����, ����ɫ��

#define     MCC_D_M_RATIO       1                                           /// D���е������
#define     MCC_H_M_RATIO       (MCC_N_HM_GEAR / MCC_N_H_GEAR)              /// H���е������
#define     MCC_V_TO_E(v)       ((v) * BSP_ENC_N_T_PR / MCC_V_NUM_PR)       /// ��������ת�ɱ�����λ��


/// ������H����λԭ�㷽ʽ����������ͽ���   !!! ע�͵���ʾ��������λԭ��
#define     MCC_H_ORG_IS_PN     1

/// ��H�����λԭ��ʹ��ߵ��Ӧ��ת������
#if MCC_H_ORG_IS_PN
    /// n % m, ��ʹ�����[-m/2, m/2)��Χ��      !!! Ҫ��n��m��������
    #define GET_PN_MOD(n, m)    ((((n) + (m) / 2) % (m)) - (m) / 2)

    #define MCC_GET_N_G_H_O(c)  (GET_PN_MOD((c) * MCC_N_COLOR_GEAR, MCC_N_H_GEAR))
    #define MCC_GET_N_G_H_T(c)  (GET_PN_MOD((c) * MCC_N_COLOR_GEAR + MCC_N_H_GEAR / 2, MCC_N_H_GEAR))
#else
    #define MCC_GET_N_G_H_O(c)  ((c) * MCC_N_COLOR_GEAR % MCC_N_H_GEAR)
    #define MCC_GET_N_G_H_T(c)  (((c) * MCC_N_COLOR_GEAR + MCC_N_H_GEAR / 2) % MCC_N_H_GEAR)
#endif

/// ����λʱ, ���HMԭ��ת��H��ԭ����Ե�����ֵ
#define     MCC_GET_V_H_O(c)    (MCC_GET_N_G_H_O(c) * MCC_V_NUM_PR / MCC_N_H_GEAR)
/// ����λʱ, ���HMԭ��ת��H�ᴩ�ߵ���Ե�����ֵ
#define     MCC_GET_V_H_T(c)    (MCC_GET_N_G_H_T(c) * MCC_V_NUM_PR / MCC_N_H_GEAR)
///// ����λʱ, ���HMԭ��ת��H��ԭ����Ե�HM���ֵ
//#define     MCC_GET_E_H_O(c)    (MCC_GET_N_G_H_O(c) * BSP_ENC_N_T_PR / MCC_N_HM_GEAR)

//#define     MCC_GET_N_G_H_O(c)  ((c) * MCC_N_COLOR_GEAR % MCC_N_H_GEAR)
//#define     MCC_GET_NP_G_H_O(c) (MCC_GET_N_G_H_O(c))
//#define     MCC_GET_NN_G_H_O(c) (MCC_GET_N_G_H_O(c) - MCC_N_H_GEAR)
//#define     MCC_GET_VP_H_O(c)   (MCC_GET_NP_G_H_O(c) * MCC_V_NUM_PR / MCC_N_H_GEAR)
//#define     MCC_GET_VN_H_O(c)   (MCC_GET_NN_G_H_O(c) * MCC_V_NUM_PR / MCC_N_H_GEAR)
//#define     MCC_GET_EP_HM_O(c)  (MCC_GET_NP_G_H_O(c) * BSP_ENC_N_T_PR / MCC_N_HM_GEAR)
//#define     MCC_GET_EN_HM_O(c)  (MCC_GET_NN_G_H_O(c) * BSP_ENC_N_T_PR / MCC_N_HM_GEAR)
//
//#define     MCC_GET_V_H_T(c)    ((MCC_GET_VP_H_O(c) + MCC_GET_VN_H_O(c)) / 2)
//#define     MCC_GET_E_HM_T(c)   ((MCC_GET_EP_HM_O(c) + MCC_GET_EN_HM_O(c)) / 2)
//
//#define     MCC_GET_N_G_H_T(c)  (((c) * MCC_N_COLOR_GEAR + MCC_N_H_GEAR / 2) % MCC_N_H_GEAR)
//#define     MCC_GET_NP_G_H_T(c) (MCC_GET_N_G_H_T(c))
//#define     MCC_GET_NN_G_H_T(c) (MCC_GET_N_G_H_T(c) - MCC_N_H_GEAR)
//#define     MCC_GET_VP_H_T(c)   (MCC_GET_NP_G_H_T(c) * MCC_V_NUM_PR / MCC_N_H_GEAR)
//#define     MCC_GET_VN_H_T(c)   (MCC_GET_NN_G_H_T(c) * MCC_V_NUM_PR / MCC_N_H_GEAR)
//#define     MCC_GET_EP_HM_T(c)  (MCC_GET_NP_G_H_T(c) * BSP_ENC_N_T_PR / MCC_N_HM_GEAR)
//#define     MCC_GET_EN_HM_T(c)  (MCC_GET_NN_G_H_T(c) * BSP_ENC_N_T_PR / MCC_N_HM_GEAR)








/// Private typedef -------------------------------------------------------------------------------------
/// Private macro ---------------------------------------------------------------------------------------
/// Private function prototypes -------------------------------------------------------------------------
/// Private variables -----------------------------------------------------------------------------------

/// --------    cmd    --------
static  s32     MCC_DAimR   =   0;      ///  D��Ŀ��Ȧ��
static  u16     MCC_DAimV   =   0;      ///  D��Ŀ�귽��
static  s16     MCC_DDifV   =   0;      ///  D�᷽���ֵ


static  s8      MCC_THCompV =   0;      /// T��򻷲�������ֵ
static  u8      MCC_ColNow  =   0;      /// ��ǰ��λ��
                                        /// V�����ڻ���ƫת����Ӧ������ֵ
static  s16     MCC_OffSetV =   MCC_V_HOFFSET;

static  bool    MCC_HIsN45  =   FALSE;  /// H��ƫ��-45��,������״̬,H���D��T����ƫ�ƽ�

                                        /// ��������ֻ����һ��ΪTRUE
//static  bool    MCC_IsEmb   =   FALSE;  /// ����״̬    H���D��T����ƫ�ƽ�
//static  bool    MCC_IsOrg   =   FALSE;  /// ԭ��״̬    VDH�������
//static  bool    MCC_IsThd   =   FALSE;  /// ���ߵ�״̬  VDH��������λ���Ȧ






/// Private consts --------------------------------------------------------------------------------------

static  const  s16      MCC_THOrgV[MCC_N_LOOPER]    =   {
    MCC_GET_V_H_O(0),
    MCC_GET_V_H_O(1),
    MCC_GET_V_H_O(2),
    MCC_GET_V_H_O(3),
    MCC_GET_V_H_O(4),
    MCC_GET_V_H_O(5)
};


/// Private functions ---------------------------------------------------------

            static  void    MCC_DeltaRE     (s32 *r, u16 *e, s16 e_delta);

            
            static  void    MCC_UpdateD     (u8 IsNeg, u8 direction);
            static  void    MCC_UpdateDM    (void);
            static  void    MCC_UpdateHM    (void);

            static  void    MCC_DtoDM       (s32 *mr, u16 *me, s32 r, u16 v);
            static  void    MCC_HtoHM       (s32 *mr, u16 *me, s32 r, u16 v);


            
            


            
            



/// --------    ͨѶ�������    --------

/// -----------------------------------------------------------------------------------------------------
/// @brief  ���� D����ر���. ���Է�������ת��ΪD ���������
/// @param  IsNeg       :   ���Է��������Ƿ�Ϊ��ֵ
///         direction   :   ���Է�������ֵ
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
static  void  MCC_UpdateD (u8 IsNeg, u8 direction)
{
    u16     abs;
    s16     dif;
    
    
    /// ����������ֵ(����)ת����ֵ, ��ΧΪ[0, MCC_V_NUM_PR)
    if (IsNeg != 0) {
        if (direction != 0) {
            abs =   MCC_V_NUM_PR - direction;
        } else {
            abs =   0;
        }
    } else {
        abs =   direction;
    }
    
    /// �����ֵ, ʹ����(-MCC_V_NUM_PR, MCC_V_NUM_PR)������. ǰ����MCC_DAimV��[0, MCC_V_NUM_PR)��
    dif =   abs - MCC_DAimV;
    
    /// ����"�����ֵ", ʹ����[-MCC_V_HLF_PR, MCC_V_HLF_PR)������
    /// ���Ҹ��� "Ȧ��" �� "������ֵ" 
    if (dif < -MCC_V_HLF_PR) {
        MCC_DDifV    =   dif + MCC_V_NUM_PR;
        MCC_DAimR++;
    } else if (dif == -MCC_V_HLF_PR) {
        if (MCC_DDifV < 0) {
            MCC_DDifV   =   MCC_V_HLF_PR;
            MCC_DAimR++;
        } else {
            ;
        }
    } else if (dif < MCC_V_HLF_PR) {
        MCC_DDifV    =   dif;
    } else if (dif == MCC_V_HLF_PR) {
        if (MCC_DDifV > 0) {
            MCC_DDifV   =  -MCC_V_HLF_PR;
            MCC_DAimR--;
        } else {
            ;
        }
    } else {
        MCC_DDifV    =   dif - MCC_V_NUM_PR;
        MCC_DAimR--;
    }
    MCC_DAimV   =   abs;
}


/// --------    �������ݺ���    --------


static  void  MCC_DeltaRE (s32 *r, u16 *e, s16 e_delta)
{
    s32     sum;        /// ��
    s16     quo;        /// ��
    s16     rem;        /// ����
    
    
    sum =   *e + e_delta;
    quo =   sum / BSP_ENC_N_T_PR;
    rem =   sum % BSP_ENC_N_T_PR;
    if (rem < 0) {
        rem +=   BSP_ENC_N_T_PR;
        quo--;
    } else {
        ;
    }
    (*e)    =   rem;
    (*r)   +=   quo;
}


/// --------    �ջ����� -- Ŀ��λ���������    --------

/// -----------------------------------------------------------------------------------------------------
/// @brief  D ����������ת��ΪD ����λ������
/// @param  mr  :   ��ŵ��λ��Ȧ������    (���)
///         me  :   ��ŵ��λ�ñ���������  (���)
///         r   :   ����Ȧ������            (����)
///         v   :   ����λ������            (����)
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
static  void  MCC_DtoDM (s32 *mr, u16 *me, s32 r, u16 v)
{
#if MCC_D_M_RATIO > 1
    s32     quo;        /// ��
    s8      rem;        /// ����
#endif
    
    
#if MCC_D_M_RATIO > 1
    #error "D_M_RATIO != 1"
    quo =   r / MCC_H_M_RATIO;
    rem =   r % MCC_H_M_RATIO;
    if (rem < 0) {
        rem +=   MCC_H_M_RATIO;
        quo--;
    } else {
        ;
    }
    *mr =   quo;
    *me =   (MCC_V_TO_E(v_col) + rem * BSP_ENC_N_T_PR) / MCC_H_M_RATIO;
    
#elif MCC_D_M_RATIO == 1
    *mr =   r;
    *me =   MCC_V_TO_E(v);
    
#else
    #error "D_M_RATIO != 1"
#endif
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  H ����������ת��ΪH ����λ������
/// @param  mr  :   ��ŵ��λ��Ȧ������    (���)
///         me  :   ��ŵ��λ�ñ���������  (���)
///         r   :   ����Ȧ������            (����)
///         v   :   ����λ������            (����)
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
static  void  MCC_HtoHM (s32 *mr, u16 *me, s32 r, u16 v)
{
    s32     r_col;
    s16     v_col;
#if MCC_H_M_RATIO > 1
    s32     quo;        /// ��
    s8      rem;        /// ����
#endif
    
    
    v_col   =   v + MCC_THOrgV[MCC_ColNow];
//    v_col   =   v + MCC_HOrgV;
    if (v_col < 0) {
        v_col  +=   MCC_V_NUM_PR;
        r_col   =   r - 1;
    } else if (v_col < MCC_V_NUM_PR) {
        r_col   =   r;
    } else {
        v_col  -=   MCC_V_NUM_PR;
        r_col   =   r + 1;
    }
    
#if MCC_H_M_RATIO > 1
    quo =   r_col / MCC_H_M_RATIO;
    rem =   r_col % MCC_H_M_RATIO;
    if (rem < 0) {
        rem +=   MCC_H_M_RATIO;
        quo--;
    } else {
        ;
    }
    *mr =   quo;
    *me =   (MCC_V_TO_E(v_col) + rem * BSP_ENC_N_T_PR) / MCC_H_M_RATIO;
    
#elif MCC_H_M_RATIO == 1
    #error "H_M_RATIO should be larger than 1"
    *mr =   r_col;
    *me =   MCC_V_TO_E(v_col);
    
#else
    #error "H_M_RATIO should be larger than 1"
#endif
}


/// -----------------------------------------------------------------------------------------------------
/// @brief  ����D ������ر���. D������ת��ΪD ��������. �����ϻ�ͷ��
/// @param  None
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
static  void  MCC_UpdateDM (void)
{
    MCH_MStarR  =   MCH_MStopR;
    MCH_MStarE  =   MCH_MStopE;
//    MCH_MBackR  =   MCH_MStarR;
//    MCH_MBackE  =   MCH_MStarE;
    
    MCC_DtoDM(&MCH_MStopR, &MCH_MStopE, MCC_DAimR, MCC_DAimV);
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  ����H ������ر���. D������ת��ΪH ��������. �����»�ͷ��
/// @param  None
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
static  void  MCC_UpdateHM (void)
{
    s16     hv1;
    s32     hr1;
    s16     hv2;
    s32     hr2;
    
    
    hv1 =   MCC_DAimV + MCC_THCompV;
    hv2 =   MCC_DAimV;
    if (MCC_HIsN45 != FALSE) {
        hv1    +=   MCC_OffSetV;
        hv2    +=   MCC_OffSetV;
    } else {
        ;
    }
    
    /// MCC_THCompV��Χ��[-2r/40, 7r/40], MCC_OffSetV��Χ��[-r/4, 0), ����hv1��Χ��[-12r/40, 47r/40)
    if (hv1 < 0) {
        hr1     =   MCC_DAimR + 1 - 1;         /// +1 for h1
        hv1    +=   MCC_V_NUM_PR;
    } else if (hv1 < MCC_V_NUM_PR) {
        hr1     =   MCC_DAimR + 1;
    } else {
        hr1     =   MCC_DAimR + 1 + 1;
        hv1    -=   MCC_V_NUM_PR;
    }
    
    /// MCC_OffSetV��Χ��[-r/4, 0), ����hv1��Χ��[-r/4, r)
    if (hv2 < 0) {
        hr2     =   MCC_DAimR - 1;
        hv2    +=   MCC_V_NUM_PR;
    } else {
        hr2     =   MCC_DAimR;
    }
    
    MCH_MStarR  =   MCH_MStopR;
    MCH_MStarE  =   MCH_MStopE;
    
    MCC_HtoHM(&MCH_MBackR, &MCH_MBackE, hr1, hv1);
    MCC_HtoHM(&MCH_MStopR, &MCH_MStopE, hr2, hv2);
}





/// --------    ����  ���������ͱջ�����    --------
        
/// -----------------------------------------------------------------------------------------------------
/// @brief  D����ԭ��
/// @param  :   None
/// @retval :   TRUE    -   �ɹ�
///             FALSE   -   ʧ��, ��ʱ
/// -----------------------------------------------------------------------------------------------------
bool  MCC_DToOrg (void)
{
    MCH_CloseExit();
    
    if (MCO_Org() != FALSE) {               /// �ҵ��ԭ��
        
        MCH_SetMOrg();
                                            /// ����D ������
        MCC_DAimR   =   0;
        MCC_DAimV   =   0;
        
        MCC_UpdateDM();                     /// DM�����ݸ��� D�����ݸ���
        
        MCH_CloseMov1(8, 10);               /// �ջ��˶�
        
        return  TRUE;
        
    } else {
        return  FALSE;
    }
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  D���Ҵ��ߵ�
/// @param  :   None
/// @retval :   TRUE    -   �ɹ�
///             FALSE   -   ʧ��, ��ʱ
/// -----------------------------------------------------------------------------------------------------
bool  MCC_DToThd (void)
{
    MCH_CloseExit();
    
    if (MCO_Org() != FALSE) {               /// �ҵ��ԭ��
        
        MCH_SetMOrg();
                                            /// ����D ������
        if (MCL_MNowE < BSP_ENC_N_T_PHR) {
            MCC_DAimR   =   0;
        } else if (MCL_MNowE > BSP_ENC_N_T_PHR) {
            MCC_DAimR   =  -1;
        } else {
            if (MCL_MNowR < 0) {
                MCC_DAimR   =  -1;
            } else {
                MCC_DAimR   =   0;
            }
        }
        MCC_DAimV   =   MCC_V_HLF_PR;
        
        MCC_UpdateDM();                     /// DM�����ݸ��� D�����ݸ���
        
        MCH_CloseMov1(8, 10);               /// �ջ��˶�
        
        return  TRUE;
        
    } else {
        return  FALSE;
    }
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  H����ԭ��
/// @param  color   :   ָ������ɫ(��λ)��
/// @retval :   TRUE    -   �ɹ�
///             FALSE   -   ʧ��, ��ʱ
/// -----------------------------------------------------------------------------------------------------
bool  MCC_HToOrg (u8 color)
{
    if (color < MCC_N_LOOPER) {
        
        MCC_ColNow  =   color;              /// ��¼��ǰ��ɫ(����)��
        MCC_HIsN45  =   FALSE;              /// û��ƫ��, �Ǵ���״̬

        MCH_CloseExit();
        
        if (MCO_Org() != FALSE) {           /// �ҵ��ԭ��
            
            MCH_SetMOrg();
                                            /// ����D ������
            MCC_DAimR   =   0;
            MCC_DAimV   =   0;
            
            MCC_UpdateHM();                 /// HM�����ݸ��� D�����ݸ���
            
            MCH_CloseMov1(8, 10);           /// �ջ��˶�
            
            return  TRUE;
            
        } else {
            return  FALSE;
        }
        
    } else {
        return  FALSE;
    }
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  H���Ҵ��ߵ�
/// @param  color   :   ָ������ɫ(��λ)��
/// @retval :   TRUE    -   �ɹ�
///             FALSE   -   ʧ��, ��ʱ
/// -----------------------------------------------------------------------------------------------------
bool  MCC_HToThd (u8 color)
{
    if (color < MCC_N_LOOPER) {
        
        MCC_ColNow  =   color;              /// ��¼��ǰ��ɫ(����)��
        MCC_HIsN45  =   FALSE;              /// û��ƫ��, �Ǵ���״̬
        
        MCH_CloseExit();
        
        if (MCO_Org() != FALSE) {           /// �ҵ��ԭ��
            
            MCH_SetMOrg();
                                            /// ����D ������
            if (MCC_THOrgV[color] < 0) {
                MCC_DAimR   =   0;
            } else if (MCC_THOrgV[color] > 0) {
                MCC_DAimR   =  -1;
            } else {
                if (MCL_MNowE < BSP_ENC_N_T_PHR) {
                    MCC_DAimR   =   0;
                } else if (MCL_MNowE > BSP_ENC_N_T_PHR) {
                    MCC_DAimR   =  -1;
                } else {
                    if (MCL_MNowR < 0) {
                        MCC_DAimR   =  -1;
                    } else {
                        MCC_DAimR   =   0;
                    }
                }
            }
            MCC_DAimV   =   MCC_V_HLF_PR;
            
            MCC_UpdateHM();                 /// HM�����ݸ��� D�����ݸ���
            
            MCH_CloseMov1(8, 10);           /// �ջ��˶�
            
            return  TRUE;
            
        } else {
            return  FALSE;
        }
        
    } else {
        return  FALSE;
    }
}


/// -----------------------------------------------------------------------------------------------------
/// @brief  H��ζ�, ����
/// @param  dir :   ��ʼ�ζ��ķ���
///         n   :   �ζ��ĵ������
///         ts  :   �ζ�����
/// @retval :   None
/// -----------------------------------------------------------------------------------------------------
void  MCC_HShake (BSP_MOT_DIR_TypeDef dir, u16 n, u8 ts)
{
#define MCC_SHAKE_SPD_MIN   8
#define MCC_SHAKE_SPD_MAX   13

    u8      i;
    
    
    MCH_CloseExit();
    
//    MCO_Mov(dir, n * 2, 13, MCO_TAB_N - 1);
    MCO_Mov(dir, n, MCC_SHAKE_SPD_MIN, MCC_SHAKE_SPD_MAX);
    
    for (i = ts; i != 0; i--) {
        if (dir == MCB_DIR_DEC) {
            dir =   MCB_DIR_INC;
        } else {
            dir =   MCB_DIR_DEC;
        }
        
        MCO_Mov(dir, 2 * n, MCC_SHAKE_SPD_MIN, MCC_SHAKE_SPD_MAX);
        OSTimeDlyHMSM(0, 0, 0, 150);
    }
    
    if (dir == MCB_DIR_DEC) {
        dir =   MCB_DIR_INC;
    } else {
        dir =   MCB_DIR_DEC;
    }
    MCO_Mov(dir, n, MCC_SHAKE_SPD_MIN, MCC_SHAKE_SPD_MAX);
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  H��ζ�, ����
/// @param  to_col  :   ��ɫĿ�ĺ�
/// @retval :   None
/// -----------------------------------------------------------------------------------------------------
void  MCC_HToColor (u8 to_col)
{
    BSP_MOT_DIR_TypeDef     dir;
    u16                     dif;
    s8                      sgn;
    
    
    if (to_col < MCC_N_LOOPER) {
        MCH_CloseExit();
        
        if (to_col == MCC_ColNow) {
            return;
        } else if (to_col < MCC_ColNow) {
            sgn =  -1;
            dir =   MCB_DIR_DEC;
            dif =   MCC_ColNow - to_col;
        } else {
            sgn =   1;
            dir =   MCB_DIR_INC;
            dif =   to_col - MCC_ColNow;
        }
        dif *=  BSP_ENC_N_T_PR * MCC_N_COLOR_GEAR / MCC_N_HM_GEAR;
        MCO_Mov(dir, dif, 0, 0);
        
        MCC_DeltaRE (&MCL_MNowR, &MCL_MNowE, dif * sgn);
    }
}




void  MCC_Init (void)
{
//    MCC_TStarR  =   0;      ///  T�ῪʼȦ��
//    MCC_TStarV  =   0;      ///  T�Ὺʼ����

//    MCC_TStopR  =   0;      ///  T�����Ȧ��
//    MCC_TStopV  =   0;      ///  T���������
//    MCC_TDifV   =   0;      ///  T�᷽���ֵ
    MCC_DAimR   =   0;      ///  D��Ŀ��Ȧ��
    MCC_DAimV   =   0;      ///  D��Ŀ�귽��
    MCC_DDifV   =   0;      ///  D�᷽���ֵ
    
//    MCC_DStarR  =   0;      ///  D�ῪʼȦ��
//    MCC_DStarE  =   0;      ///  D�Ὺʼ���� (������)
//    MCC_DStopR  =   0;      ///  D�����Ȧ��
//    MCC_DStopE  =   0;      ///  D��������� (������)
//    
//    MCC_HStarR  =   0;      ///  H�ῪʼȦ��
//    MCC_HStarE  =   0;      ///  H�Ὺʼ���� (������)
//    MCC_HStopR  =   0;      ///  H�����Ȧ��
//    MCC_HStopE  =   0;      ///  H��������� (������)    
    
//    MCH_MStarR  =   0;      ///  D�� H�ῪʼȦ��
//    MCH_MStarE  =   0;      ///  D�� H�Ὺʼ���� (������)
//    MCH_MStopR  =   0;      ///  D�� H�����Ȧ��
//    MCH_MStopE  =   0;      ///  D�� H��������� (������)   
//    
//    MCL_MNowR   =   0;      /// ���ص����ǰȦ��
//    MCL_MNowE   =   0;      /// ���ص����ǰ���� (������)
//    
//    MCH_MDecR   =   0;      /// ���ص����������Ȧ��
//    MCH_MDecE   =   0;      /// ���ص���������޷��� (������)
//    MCH_MIncR   =   0;      /// ���ص����������Ȧ��
//    MCH_MIncE   =   0;      /// ���ص���������޷��� (������)
    MCH_SetMOrg();
    
    MCC_THCompV =   0;      /// T��򻷲�������ֵ
    MCC_ColNow  =   0;      /// ��ǰ��λ��
                            /// V�����ڻ���ƫת����Ӧ������ֵ
    MCC_OffSetV =   MCC_V_HOFFSET;
    
//    MCO_StopSemPtr  =   OSSemCreate(0);
    MCO_Init();
    
//    BSP_ENC_IndexEnable();
}



/// -----------------------------------------------------------------------------------------------------
/// @brief  �򻷽ǶȲ���
/// @param  Comp    :   ������λֵ, ��Χ��[0,9], 2Ϊ������, 0~1Ϊ����, 3~9Ϊ����.
///                     ÿ��Ϊ10/400r,��9��. ��Χ��[-2r/40,7r/40]
/// @retval :   None
/// -----------------------------------------------------------------------------------------------------
void  MCC_ComSetHAngComp (u8 Comp)
{
    if (Comp < 10) {
        MCC_THCompV =   (Comp - 2) * 10;
    } else {
        MCC_THCompV =   0;
    }
}


/// -----------------------------------------------------------------------------------------------------
/// @brief  ����ǰH���ƫ�ƶ���
/// @param  b1  :   ͨѶ��������1, ��ʾƫ�Ʒ�����źͶ�Ӧ�����ٶ�
///         b2  :   ͨѶ��������2, ��ʾƫ�ƾ���ֵ.
/// @retval :   None
/// -----------------------------------------------------------------------------------------------------
void  MCC_ComHOffset (u8 b1, u8 b2)
{
    u8      spd_d10;
    
    
                                                                /// ��¼ƫ��ֵ
    if ((b1 >> 7) && (b2 < MCC_V_NUM_PR / 4) && (b2 != 0)) {    /// ��Ч��ΧΪ(-r/4, 0)
        MCC_OffSetV =  -b2;
    } else {
        MCC_OffSetV =   MCC_V_HOFFSET;                          /// !��һ����Ĭ��ֵ, Ҳ��ʹ��ԭֵ
    }
    
    MCC_HIsN45   =   TRUE;                                      /// ��Ϊ����״̬, ƫ��״̬
    
    spd_d10 =   b1 & (~(1 << 7));                               /// ģ�������ٶ�
    
    MCC_UpdateHM();                                             /// HM�����ݸ��� D�����ݸ���
    
    MCH_CloseMov1(spd_d10, 10);                                 /// �ջ��˶�
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  ��������, ���� D��
/// @param  b1  :   ͨѶ��������1, ��ʾD�᷽����źͶ�Ӧ�����ٶ�
///         b2  :   ͨѶ��������2, ��ʾD�᷽�����ֵ.
/// @retval :   None
/// -----------------------------------------------------------------------------------------------------
void  MCC_ComD (u8 b1, u8 b2)
{
    u8      spd_d10;
    
    
    spd_d10 =   b1 & (~(1 << 7));           /// ����. �����������, �򲻸���, ����ԭ״̬.(BSP��������)
    
    MCC_UpdateD((b1 >> 7) & 0x01, b2);
    MCC_UpdateDM();
    
    MCH_CloseMov1(spd_d10, 10);             /// �ջ��˶�
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  ��������, ���� H��
/// @param  b1  :   ͨѶ��������1, ��ʾD�᷽����źͶ�Ӧ�����ٶ�
///         b2  :   ͨѶ��������2, ��ʾD�᷽�����ֵ.
/// @retval :   None
/// -----------------------------------------------------------------------------------------------------
void  MCC_ComH (u8 b1, u8 b2)
{
    u8      spd_d10;
    
    
    MCC_HIsN45   =   TRUE;                      /// ��Ϊ����״̬, ƫ��״̬
    
    spd_d10 =   b1 & (~(1 << 7));               /// ����. �����������, �򲻸���, ����ԭ״̬.(BSP��������)
    
    MCC_UpdateD((b1 >> 7) & 0x01, b2);
    MCC_UpdateHM();
    
    MCH_CloseMov2(spd_d10, 10, 4010);           /// �ջ��˶�
}











#endif

/// End of file -----------------------------------------------------------------------------------------
