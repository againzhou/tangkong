
/// ----------------------------------------------------------------------------
/// File Name          : in.h
/// Description        : 输入任务模块
///     此模块主要用于断线检测 和 机头开关去抖
/// ----------------------------------------------------------------------------
/// History:

/// y10/m08/d18     v0.01   chenyong    setup



#ifndef  IN_H
#define  IN_H

/// EXTERNS --------------------------------------------------------------------
#ifdef   IN_MODULE
    #define  IN_EXT                 /// 不定义为 "外部", 本模块内
#else
    #define  IN_EXT  extern         /// 定义为 "外部"
#endif



#include  <bsp.h>


/// USER DEFINED CONSTANTS -----------------------------------------------------

#define     HDKEY_ST_BIT_UP        0            /// key up  状态用 bit0 表示
#define     HDKEY_ST_BIT_DN        1            /// key dn  状态用 bit1 表示


/// Exported types -------------------------------------------------------------

/// 机头开关的状态, bit0 表示 key up, bit1 表示 key dn
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
