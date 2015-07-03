
/// ----------------------------------------------------------------------------
/// File Name          : app_cfg.h
/// Description        : 应用程序配置文件
/// ----------------------------------------------------------------------------
/// History:

/// y10/m07         v0.01   chenyong    setup



#ifndef  APP_CFG_H
#define  APP_CFG_H


/// MODULE ENABLE / DISABLE -----------------------------------------------------------------------------

//  #define  APP_OS_PROBE_EN                         DEF_ENABLED
//  #define  APP_PROBE_COM_EN                        DEF_ENABLED


/// TASKS NAMES -----------------------------------------------------------------------------------------


/// TASK PRIORITIES -------------------------------------------------------------------------------------

#define  APP_TASK_START_PRIO                            3

#define  APP_TASK_MOTO_ODD_PRIO                         11
#define  APP_TASK_MOTO_EVEN_PRIO                        12

#define  APP_TASK_IN_SCAN_PRIO                          15

#define  APP_TASK_HDLED_ODD_PRIO                        19
#define  APP_TASK_HDLED_EVEN_PRIO                       20

#define  APP_TASK_USER_IF_PRIO                          23

//#define  OS_PROBE_TASK_PRIO              (OS_LOWEST_PRIO - 3)
//#define  OS_TASK_TMR_PRIO                (OS_LOWEST_PRIO - 2)

/// TASK STACK SIZES  Size of the task stacks (# of OS_STK entries) -------------------------------------

//#define  APP_TASK_START_STK_SIZE                        128
////#define  APP_TASK_MOTO_STK_SIZE                          256
//#define  APP_TASK_MOTO_STK_SIZE                         384
//#define  APP_TASK_IN_SCAN_STK_SIZE                      128
//#define  APP_TASK_HDLED_STK_SIZE                         64
//#define  APP_TASK_USER_IF_STK_SIZE                       64

/// ------- cy101007    m   -------

////#define  APP_TASK_START_STK_SIZE                        84
//#define  APP_TASK_START_STK_SIZE                        96
//#define  APP_TASK_MOTO_STK_SIZE                         312     /// 可减少
//#define  APP_TASK_IN_SCAN_STK_SIZE                      70
//#define  APP_TASK_HDLED_STK_SIZE                        66
//#define  APP_TASK_USER_IF_STK_SIZE                      64
//#define  APP_TASK_START_STK_SIZE                        84

#define  APP_TASK_START_STK_SIZE                        128
#define  APP_TASK_MOTO_STK_SIZE                         256
#define  APP_TASK_IN_SCAN_STK_SIZE                      128
#define  APP_TASK_HDLED_STK_SIZE                        128
#define  APP_TASK_USER_IF_STK_SIZE                      128

/// ------- cy101007    m   -------

///// LIB -------------------------------------------------------------------------------------------------
//
//#define  uC_CFG_OPTIMIZE_ASM_EN                 DEF_ENABLED
//#define  LIB_STR_CFG_FP_EN                      DEF_DISABLED



#endif
/// End of file -----------------------------------------------------------------------------------------
