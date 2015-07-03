
/// ----------------------------------------------------------------------------
/// File Name          : com.h
/// Description        : 通讯应用模块(全双工通讯)
///
///     因为使用 OS 资源, 所以放在APP层.
/// ----------------------------------------------------------------------------
/// History:

/// y10/m12/d22     v0.05   chenyong    modify
/// 加入函数 bool COM_JogIsEn(BSP_ID_TypeDef id), 用于针高电机模块判断能否微动

/// y10/m08/d27     v0.02   chenyong    modify

/// y10/m08/d06     v0.01   shengjiaq   setup





/// Define to prevent recursive inclusion --------------------------------------
#ifndef  com_H
#define  com_H


/// EXTERNS --------------------------------------------------------------------
#ifdef   COM_MODULE
#define  COM_EXT                /// 不定义为 "外部", 本模块内
#else
#define  COM_EXT  extern        /// 定义为 "外部"
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
