
/// ----------------------------------------------------------------------------
/// File Name          : com.h
/// Description        : ͨѶӦ��ģ��(ȫ˫��ͨѶ)
///
///     ��Ϊʹ�� OS ��Դ, ���Է���APP��.
/// ----------------------------------------------------------------------------
/// History:

/// y10/m12/d22     v0.05   chenyong    modify
/// ���뺯�� bool COM_JogIsEn(BSP_ID_TypeDef id), ������ߵ��ģ���ж��ܷ�΢��

/// y10/m08/d27     v0.02   chenyong    modify

/// y10/m08/d06     v0.01   shengjiaq   setup





/// Define to prevent recursive inclusion --------------------------------------
#ifndef  com_H
#define  com_H


/// EXTERNS --------------------------------------------------------------------
#ifdef   COM_MODULE
#define  COM_EXT                /// ������Ϊ "�ⲿ", ��ģ����
#else
#define  COM_EXT  extern        /// ����Ϊ "�ⲿ"
#endif


/// Includes -------------------------------------------------------------------
/// Exported macro -------------------------------------------------------------
/// Exported constants ---------------------------------------------------------
/// Global variables -----------------------------------------------------------
	
/// Exported functions ---------------------------------------------------------

void  COM_Init          (void);
void  COM_RxHandle      (u16 rx);
void  COM_KeyHandle     (void);
bool  COM_JogIsEn       (BSP_ID_TypeDef id);
void  COM_DXSparkel     (void);

#endif
/// End of file -----------------------------------------------------------------------------------------
