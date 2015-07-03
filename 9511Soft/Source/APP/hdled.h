
/// ----------------------------------------------------------------------------
/// File Name          : hdled.h
/// Description        : 机头指示灯控制模块
/// ----------------------------------------------------------------------------
/// History:

/// y10/m08/d21     v0.01   chenyong    setup



#ifndef  HDLED_H
#define  HDLED_H

/// EXTERNS --------------------------------------------------------------------
#ifdef   HDLED_MODULE
    #define  HDLED_EXT                          /// 不定义为 "外部", 本模块内
#else
    #define  HDLED_EXT  extern                  /// 定义为 "外部"
#endif


#include  <ucos_ii.h>                           /// OS核的头文件
#include  <bsp.h>


/// USER DEFINED CONSTANTS ------------------------------------------------------------------------------

#define     HDLED_ST_BIT_GRE        0           /// led green 状态用 bit0 表示
#define     HDLED_ST_BIT_RED        1           /// led red   状态用 bit1 表示



/// Exported types -------------------------------------------------------------

/// 机头灯的状态, bit0 表示 red led, bit1 表示 green led
typedef enum
{
    HDLED_ST_OFF    =   (BSP_LED_OFF << HDLED_ST_BIT_RED) | (BSP_LED_OFF << HDLED_ST_BIT_GRE),
    HDLED_ST_RED    =   (BSP_LED_ON  << HDLED_ST_BIT_RED) | (BSP_LED_OFF << HDLED_ST_BIT_GRE),
    HDLED_ST_GRE    =   (BSP_LED_OFF << HDLED_ST_BIT_RED) | (BSP_LED_ON  << HDLED_ST_BIT_GRE),
    HDLED_ST_ORA    =   (BSP_LED_ON  << HDLED_ST_BIT_RED) | (BSP_LED_ON  << HDLED_ST_BIT_GRE),
} HDLED_ST_TypeDef;


/// FUNCTION PROTOTYPES ---------------------------------------------------------------------------------

void                HDLED_Init      (void);
HDLED_ST_TypeDef    HDLED_Get       (BSP_ID_TypeDef id);
void                HDLED_Set       (BSP_ID_TypeDef id, HDLED_ST_TypeDef st);

//void                HDLED_Sparkle   (BSP_ID_TypeDef id);
void                HDLED_Sparkle   (BSP_ID_TypeDef id, s32 times);




#endif
