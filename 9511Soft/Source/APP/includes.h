
/// ----------------------------------------------------------------------------
/// File Name          : includes.h
/// Description        : 头文件
/// ----------------------------------------------------------------------------
/// History:

/// y10/m07         v0.01   chenyong    setup


#ifndef  INCLUDES_H
#define  INCLUDES_H


#include  <ucos_ii.h>               /// OS核的头文件

#include  <stm32f10x.h>

/// 应用配置文件和应用模块的头文件
#include  <app_cfg.h>               /// 任务应用相关
#include  <bsp.h>                   /// 板支持包

#include  "c491.h"
#include  "in.h"
#include  "dbg.h"
#include  "hdled.h"
#include  "moto.h"
#include  "com.h"


#endif
/// End of file -----------------------------------------------------------------------------------------
