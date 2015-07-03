
/// ----------------------------------------------------------------------------
/// File Name          : hdled.h
/// Description        : ��ͷָʾ�ƿ���ģ��
/// ----------------------------------------------------------------------------
/// History:

/// y10/m08/d21     v0.01   chenyong    setup



#ifndef  HDLED_H
#define  HDLED_H

/// EXTERNS --------------------------------------------------------------------
#ifdef   HDLED_MODULE
    #define  HDLED_EXT                          /// ������Ϊ "�ⲿ", ��ģ����
#else
    #define  HDLED_EXT  extern                  /// ����Ϊ "�ⲿ"
#endif


#include  <ucos_ii.h>                           /// OS�˵�ͷ�ļ�
#include  <bsp.h>


/// USER DEFINED CONSTANTS ------------------------------------------------------------------------------

#define     HDLED_ST_BIT_GRE        0           /// led green ״̬�� bit0 ��ʾ
#define     HDLED_ST_BIT_RED        1           /// led red   ״̬�� bit1 ��ʾ



/// Exported types -------------------------------------------------------------

/// ��ͷ�Ƶ�״̬, bit0 ��ʾ red led, bit1 ��ʾ green led
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
