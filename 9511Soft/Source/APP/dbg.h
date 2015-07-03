
/// ----------------------------------------------------------------------------
/// File Name          : dbg.h
/// Description        : ���Ժ��������, ����BSP����, Ӳ�����Ժ��ϻ�
/// ----------------------------------------------------------------------------
/// History:

/// y10/m07/d23     v0.01   chenyong    setup




/// Define to prevent recursive inclusion --------------------------------------
#ifndef DBG_H
#define DBG_H


/// Includes -------------------------------------------------------------------
#include "stm32f10x.h"

/// Exported types -------------------------------------------------------------
/// Exported constants ---------------------------------------------------------
/// Exported macro -------------------------------------------------------------
/// Exported functions ---------------------------------------------------------
bool    DBG_IsDebug     (void);
void    DBG_ALL         (u16 hour);


#endif
/// End of file -----------------------------------------------------------------------------------------
