
/// ----------------------------------------------------------------------------
/// File Name          : in.h
/// Description        : ��������ģ��
///     ��ģ����Ҫ���ڶ��߼�� �� ��ͷ����ȥ��
/// ----------------------------------------------------------------------------
/// History:

/// y10/m08/d18     v0.01   chenyong    setup



#ifndef  IN_H
#define  IN_H

/// EXTERNS --------------------------------------------------------------------
#ifdef   IN_MODULE
    #define  IN_EXT                 /// ������Ϊ "�ⲿ", ��ģ����
#else
    #define  IN_EXT  extern         /// ����Ϊ "�ⲿ"
#endif



#include  <bsp.h>


/// USER DEFINED CONSTANTS -----------------------------------------------------

#define     HDKEY_ST_BIT_UP        0            /// key up  ״̬�� bit0 ��ʾ
#define     HDKEY_ST_BIT_DN        1            /// key dn  ״̬�� bit1 ��ʾ


/// Exported types -------------------------------------------------------------

/// ��ͷ���ص�״̬, bit0 ��ʾ key up, bit1 ��ʾ key dn
typedef enum
{
    HDKEY_ST_MD     =   (BSP_HD_KEY_OFF << HDKEY_ST_BIT_UP) | (BSP_HD_KEY_OFF << HDKEY_ST_BIT_DN),
    HDKEY_ST_UP     =   (BSP_HD_KEY_ON  << HDKEY_ST_BIT_UP) | (BSP_HD_KEY_OFF << HDKEY_ST_BIT_DN),
    HDKEY_ST_DN     =   (BSP_HD_KEY_OFF << HDKEY_ST_BIT_UP) | (BSP_HD_KEY_ON  << HDKEY_ST_BIT_DN),
    HDKEY_ST_ERR    =   (BSP_HD_KEY_ON  << HDKEY_ST_BIT_UP) | (BSP_HD_KEY_ON  << HDKEY_ST_BIT_DN),
} HDKEY_ST_TypeDef;


/// Global variables -----------------------------------------------------------

/// FUNCTION PROTOTYPES --------------------------------------------------------

void                IN_Init             (void);

/// cy 100909
//void                IN_HDKey_Get        (BSP_ID_TypeDef id, BSP_HD_KEY_TypeDef *up, BSP_HD_KEY_TypeDef *dn);
/// cy 100909
HDKEY_ST_TypeDef    IN_HDKey_GetSt      (BSP_ID_TypeDef id);

//void                IN_BRK_SetIsDn      (BSP_ID_TypeDef id, bool IsDn);
void                IN_BRK_SetIsDn      (bool IsDn);
//void                IN_BRK_SetIsEn      (BSP_ID_TypeDef id, bool IsEn);
void                IN_BRK_SampleBegin  (void);
void                IN_BRK_SentXJ       (void);
void                IN_BRK_SampleStop   (void);
bool                IN_BRK_GetIsBrk     (BSP_ID_TypeDef id);





#endif
