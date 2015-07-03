
/// ----------------------------------------------------------------------------
/// File Name          : dbg.c
/// Description        : 调试和试验相关, 用于BSP调试, 硬件测试和老化
/// ----------------------------------------------------------------------------
/// History:

/// y10/m07/d23     v0.01   chenyong    setup



/// Includes --------------------------------------------------------------------------------------------
#include  <includes.h>
#include  <bsp.h>
//#include  <ucos_ii.h>               /// OS核的头文件
//#include  <stm32f10x.h>             /// cy  091120  added
//#include  <stdio.h>                 /// cy  091211  added
#include  "moto.h"
#include  "hdled.h"
#include  "io.h"

/// Private typedef -------------------------------------------------------------------------------------

/// Private define --------------------------------------------------------------------------------------
//#ifndef NULL
//#define NULL    0
//#endif
//#define  DBG_VERSION    ("V  1.01")

#define  DBG_LED_DIP    1           /// LED1 用于显示 DIP 状态
#define  DBG_LED_491    0           /// LED2 用于显示 491 状态

/// Private macro ---------------------------------------------------------------------------------------
/// Private function prototypes -------------------------------------------------------------------------
/// Private variables -----------------------------------------------------------------------------------
/// Private consts --------------------------------------------------------------------------------------




/// Private functions ---------------------------------------------------------
static  void    DBG_StartAlarmTestLed   (void);
static  void    DBG_EndAlarm            (void);
static  void    DBG_HD                  (void);
static  bool    DBG_SqnSwiIsOK          (u8  phase);
static  bool    DBG_C491IsOK            (void);

///// for close loop motor test
//static  void    DBG_MOTCAngSpd          (u8 LeadAng);
//static  void    DBG_MOTCGoBack          (u8 LeadAng);
///// for close loop motor test



/// -----------------------------------------------------------------------------------------------------
/// @brief  判断是否可以进入debug状态
/// @param  None
/// @retval : 是否
/// -----------------------------------------------------------------------------------------------------
bool  DBG_IsDebug (void)
{
    BSP_HD_KEY_TypeDef  up;
    BSP_HD_KEY_TypeDef  dn;
    
    
//    DBG_MOTCAngSpd(BSP_DIP_9511Addr + 2);
//    DBG_MOTCGoBack(BSP_DIP_9511Addr + 2);
    
    BSP_HD_KeyGet(BSP_ID_ODD, &up, &dn);
    if (up == BSP_HD_KEY_OFF) {
        return FALSE;
    }
    BSP_HD_KeyGet(BSP_ID_EVEN, &up, &dn);
    if (up == BSP_HD_KEY_OFF) {
        return FALSE;
    }
    return TRUE;
//    u32                 i;
//    
//    
//    for (i = 0; i < BSP_DEVICE_ID_N; i++) {
//        BSP_HD_KeyGet((BSP_ID_TypeDef)i, &up, &dn);
//        if (up == BSP_HD_KEY_ON) {
//            return TRUE;
//        }
//    }
//    return FALSE;
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  debug状态进入警示, 并测试两个led
/// @param  None
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
static  void  DBG_StartAlarmTestLed (void)
{
    u32     i;
    
    
    for (i = 0; i < 10; i++) {
        BSP_LED_Toggle(i & 1);
        OSTimeDlyHMSM(0, 0, 0, 200);
    }
    for (i = 0; i < BSP_LED_ID_N; i++) {
        BSP_LED_Set(i, BSP_LED_OFF);
    }
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  debug状态结束警示
/// @param  None
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
static  void  DBG_EndAlarm (void)
{
    u32     i;
    
    
    for ( ; ; ) {
        for (i = 0; i < BSP_DEVICE_ID_N; i++) {

            HDLED_Sparkle((BSP_ID_TypeDef)i, 3);

        }
        OSTimeDlyHMSM(0, 0, 1, 0);
    }
}

/// -----------------------------------------------------------------------------------------------------
/// @brief  测试机头板
/// @param  None
/// @retval : None
/// -----------------------------------------------------------------------------------------------------
static  void  DBG_HD (void)
{
    BSP_HD_KEY_TypeDef  up;
    BSP_HD_KEY_TypeDef  dn;
//    BSP_LED_TypeDef     r;
//    BSP_LED_TypeDef     g;
    HDLED_ST_TypeDef    st;
    u32                 i;
    
    
    for (i = 0; i < BSP_DEVICE_ID_N; i++) {
        BSP_HD_KeyGet((BSP_ID_TypeDef)i, &up, &dn);
//        if (up == BSP_HD_KEY_OFF) {
//            r   =   BSP_LED_OFF;
//            if (dn == BSP_HD_KEY_OFF) {
//                g   =   BSP_LED_ON;
//            } else {
//                g   =   BSP_LED_OFF;
//            }
//        } else {
//            r   =   BSP_LED_ON;
//            if (dn == BSP_HD_KEY_OFF) {
//                g   =   BSP_LED_OFF;
//            } else {
//                g   =   BSP_LED_ON;
//            }
//        }
//        BSP_HD_LedSet((BSP_ID_TypeDef)i, r, g);
        
        if (up == BSP_HD_KEY_OFF) {
            if (dn == BSP_HD_KEY_OFF) {
                st  =   HDLED_ST_OFF;
            } else {
                st  =   HDLED_ST_GRE;
            }
        } else {
            if (dn == BSP_HD_KEY_OFF) {
                st  =   HDLED_ST_RED;
            } else {
                st  =   HDLED_ST_ORA;
            }
        }
        HDLED_Set((BSP_ID_TypeDef)i, st);
    }
}

///// -----------------------------------------------------------------------------------------------------
///// @brief  测试DIP, 如果dip与测试值不同, 翻转相应LED
///// @param  测试值
///// @retval : 当前DIP值
///// -----------------------------------------------------------------------------------------------------
//u16  DBG_DIP (u16  value)
//{
////    switch (BSP_DIP_Get()) {
////    case 0:
////        BSP_LED_Set(0, BSP_LED_ON);
////        BSP_LED_Set(1, BSP_LED_OFF);
////        break;
////    case ((1 << 10) - 1):
////        BSP_LED_Set(0, BSP_LED_OFF);
////        BSP_LED_Set(1, BSP_LED_ON);
////        break;
////    default:
////        BSP_LED_Set(0, BSP_LED_ON);
////        BSP_LED_Set(1, BSP_LED_ON);
////        break;
////    }
//
//    u16     dip;
//    
//    
//    dip =   BSP_DIP_Get();
//    if ((value & ((1 << BSP_N_DIP) - 1)) != dip) {
//        BSP_LED_Toggle(DBG_LED_DIP);
//    }
//    return  dip;
//}


/*

void  DBG_MOTC (void)
{
    u32     i;
    u32     s;
    u16     encoder[8];
    
    
    s   =   0;
    BSP_ENC_CntSet(BSP_ID_ODD, 0);
    for (i = 0; i < 8; i++) {
        BSP_MOTC_ClkSet(BSP_ID_ODD, Bit_SET);
        OSTimeDlyHMSM(0, 0, 0, 10);
        s++;
        encoder[i]  =   BSP_ENC_CntGet(BSP_ID_ODD);
        BSP_MOTC_ClkSet(BSP_ID_ODD, Bit_RESET);
        OSTimeDlyHMSM(0, 0, 0, 10);
    }
    OSTimeDlyHMSM(0, 0, 0, 300);

}

static  u16 encoder[10];
static  u16 sssssss[10];
static  u16 en;
static  u16 ss;
void  DBG_MOTCMaxSpd (void)
{
    u32     i;
    u32     s;
    s32     tmp;
    u32     j;
    
    
    s   =   100;
//    BSP_MOTC_MSet(BSP_ID_ODD, BSP_MOT_M_P45);
    OSTimeDlyHMSM(0, 0, 0, 300);
    BSP_ENC_CntSet(BSP_ID_ODD, s);
    
//    BSP_MOTC_MSet(BSP_ID_ODD, BSP_MOT_M_P45);
//    OSTimeDlyHMSM(0, 0, 0, 3);
//    for (i = 0; i < 10; i++) {
//        for (j = 0; j < 32; j++) {
//            BSP_MOTC_ClkSet(BSP_ID_ODD, Bit_SET);
//            OSTimeDlyHMSM(0, 0, 0, 1);
//    //        s++;
//            s++;
//            BSP_MOTC_ClkSet(BSP_ID_ODD, Bit_RESET);
//            OSTimeDlyHMSM(0, 0, 0, 1);
//        }
//        sssssss[i]  =   s;
//        encoder[i]  =   BSP_ENC_CntGet(BSP_ID_ODD);
//    }
    
    
//    while (TRUE) {
    for (j = 0; j < 1000; j++) {
    
////    for (i = 0; i < 4; i++) {
//    for (i = 0; i < 8; i++) {
//        for (j = 0; j < 200; j++) {
//        BSP_MOTC_ClkSet(BSP_ID_ODD, Bit_SET);
//        OSTimeDlyHMSM(0, 0, 0, 1);
////        s++;
//        s++;
//        BSP_MOTC_ClkSet(BSP_ID_ODD, Bit_RESET);
//        OSTimeDlyHMSM(0, 0, 0, 1);
//        }
//        sssssss[i]  =   s;
//        encoder[i]  =   BSP_ENC_CntGet(BSP_ID_ODD);
//    }

    
        OSTimeDlyHMSM(0, 0, 0, 1);
        BSP_MOTC_DirSet(BSP_ID_ODD, BSP_MOT_DIR_CW);
        OSTimeDlyHMSM(0, 0, 0, 1);
        
        while (1) {
            ss  =   BSP_ENC_CntGet(BSP_ID_ODD);
            tmp =   s - ss + 4 * BSP_ENC_PULSE_N;
            tmp %=  4 * BSP_ENC_PULSE_N;
            if ((tmp < 8) || (tmp > 4 * BSP_ENC_PULSE_N - 1 - 8)) {
                BSP_MOTC_ClkSet(BSP_ID_ODD, Bit_SET);
//            DBG_Delay(100);
            BSP_US_Delay(20);
            BSP_MOTC_ClkSet(BSP_ID_ODD, Bit_RESET);
                //        s++;
            s++;
            if (s > 100 + 600) {
                break;
            }
            if (s <= 100 + 8) {
//                DBG_Delay(900);
                BSP_US_Delay(120);
            } else {
//                DBG_Delay(300);
                BSP_US_Delay(40);
            }
    //        OSTimeDlyHMSM(0, 0, 0, 1);
            }
        }
        
        OSTimeDlyHMSM(0, 0, 0, 1);
        BSP_MOTC_DirSet(BSP_ID_ODD, BSP_MOT_DIR_CCW);
        OSTimeDlyHMSM(0, 0, 0, 1);
        
        while (1) {
            ss  =   BSP_ENC_CntGet(BSP_ID_ODD);
            tmp =   s - ss + 4 * BSP_ENC_PULSE_N;
            tmp %=  4 * BSP_ENC_PULSE_N;
            if ((tmp <  8) || (tmp > 4 * BSP_ENC_PULSE_N - 1 - 8)) {
                BSP_MOTC_ClkSet(BSP_ID_ODD, Bit_SET);
//            DBG_Delay(100);
            BSP_US_Delay(20);
            BSP_MOTC_ClkSet(BSP_ID_ODD, Bit_RESET);
    //        s++;
            s--;
            if (s <= 100) {
                break;
            }
            if (s > 100 + 600 - 8) {
//                DBG_Delay(900);
            BSP_US_Delay(120);
            } else {
//                DBG_Delay(300);
            BSP_US_Delay(40);
            }
    //        OSTimeDlyHMSM(0, 0, 0, 1);
            }
        }


//        while (1) {
//            ss  =   BSP_ENC_CntGet(BSP_ID_ODD);
//            tmp =   s - ss + 4 * BSP_ENC_PULSE_N;
//            tmp %=  4 * BSP_ENC_PULSE_N;
//            if ((tmp < 12) || (tmp > 4 * BSP_ENC_PULSE_N - 1 - 12)) {
//                BSP_MOTC_ClkSet(BSP_ID_ODD, Bit_SET);
////            DBG_Delay(100);
//            BSP_US_Delay(10);
//            BSP_MOTC_ClkSet(BSP_ID_ODD, Bit_RESET);
//                //        s++;
//            s++;
////            if (s > 100 + 600) {
//            if (s > 1600 * 10000) {
//                break;
//            }
//            if (s <= 100 + 8) {
////                DBG_Delay(900);
//                BSP_US_Delay(120);
//            } else {
////                DBG_Delay(300);
//                BSP_US_Delay(10);
//            }
//    //        OSTimeDlyHMSM(0, 0, 0, 1);
//            }
//        }
        
    }
    
    
    BSP_MOTC_ClkSet(BSP_ID_ODD, Bit_RESET);
    OSTimeDlyHMSM(0, 0, 0, 300);

}

*/

///// for close loop motor test
//#define     P_W         (8 * 13)                /// delay 15uS
//
//static  void  DBG_MOTCAngSpd (u8 LeadAng)
//{
//    u16     enc;
//    u16     p;
//    s16     err;
//    u16     pw;
//    u32     i;
//    
//    
//    enc =   0;
//    p   =   0;
//    
//    
//
//    OSTimeDlyHMSM(0, 0, 2, 0);
//    pw  =   8 * (((BSP_DIP_Get() >> 2) & 0x07) + 8);
//    BSP_MOTO_TQSet(BSP_ID_ODD, BSP_DIP_TQ);
//    BSP_ENC_CntSet(BSP_ID_ODD, enc);
//    
//    BSP_MOTC_DirSet(BSP_ID_ODD, BSP_MOT_DIR_CW);
//    
//    for ( ; ; ) {
//    for (i = 0; i < 500; ) {
//        enc =   BSP_ENC_CntGet(BSP_ID_ODD);
//        err =   p - enc;
//        if (err < LeadAng - 2) {
//            BSP_LED_Set(0, BSP_LED_OFF);
//            BSP_LED_Set(1, BSP_LED_OFF);
//        } else if (err == LeadAng - 2) {
//            BSP_LED_Set(0, BSP_LED_ON);
//            BSP_LED_Set(1, BSP_LED_OFF);
//        } else if (err == LeadAng - 1) {
//            BSP_LED_Set(0, BSP_LED_OFF);
//            BSP_LED_Set(1, BSP_LED_ON);
//        } else {
//            BSP_LED_Set(0, BSP_LED_ON);
//            BSP_LED_Set(1, BSP_LED_ON);
//        }
//        if ((err < LeadAng) && (err >= -LeadAng)) {
//            BSP_MOTC_ClkSet(BSP_ID_ODD, Bit_SET);
//            BSP_TM8M_Delay(pw);
//            BSP_MOTC_ClkSet(BSP_ID_ODD, Bit_RESET);
//            BSP_TM8M_Delay(pw);
//            p++;
//            p  %=   BSP_ENC_PULSE_N * 4;
//            if (p == 0) {
//                i++;
//            }
////            if((p & 0x3F) == 0) {
////                BSP_LED_Toggle(0);
////                BSP_LED_Toggle(1);
////            }
//        }
//    }
//    OSTimeDlyHMSM(0, 0, 5, 0);
//    }
//}
//
//#define     STEP_GO     600
//#define     STEP_BACK   800
//
//static  void  DBG_MOTCGoBack (u8 LeadAng)
//{
//    u16     enc;
//    u16     p;
//    s16     err;
//    u32     i;
//    u16     pw;
//    u16     pb;
//    
//    
//    enc =   STEP_GO;
//    p   =   STEP_GO;
//    
//    OSTimeDlyHMSM(0, 0, 2, 0);
//    
//    pb  =   LeadAng - ((BSP_DIP_Get() >> 2) & 0x07) - 4;
//    pw  =   8 * 8;
//    BSP_MOTO_TQSet(BSP_ID_ODD, BSP_DIP_TQ);
//    BSP_ENC_CntSet(BSP_ID_ODD, enc);
//    
//    
//    for ( ; ; ) {
//    for (i = 0; i < 1000; i++) {
//    
//    BSP_MOTC_DirSet(BSP_ID_ODD, BSP_MOT_DIR_CW);
//    
//    for ( ; ; ) {
//        enc =   BSP_ENC_CntGet(BSP_ID_ODD);
//        err =   p - enc;
//        if ((err < LeadAng) && (err > -LeadAng)) {
//            BSP_MOTC_ClkSet(BSP_ID_ODD, Bit_SET);
//            BSP_TM8M_Delay(pw);
//            BSP_MOTC_ClkSet(BSP_ID_ODD, Bit_RESET);
//            BSP_TM8M_Delay(pw);
//            p++;
//            p  %=   BSP_ENC_PULSE_N * 4;
//        }
//        if (p > STEP_BACK) {
//            break;
//        }
//    }
//    
//    for ( ; ; ) {
//        if (BSP_ENC_CntGet(BSP_ID_ODD) < enc) {
//            break;
//        }
//        enc =   BSP_ENC_CntGet(BSP_ID_ODD);
//        err =   p - enc;
//        if (err <= -pb) {
//            BSP_MOTC_ClkSet(BSP_ID_ODD, Bit_SET);
//            BSP_TM8M_Delay(pw);
//            BSP_MOTC_ClkSet(BSP_ID_ODD, Bit_RESET);
//            BSP_TM8M_Delay(pw);
//            p++;
//            p  %=   BSP_ENC_PULSE_N * 4;
//        }
//    }
//    
//    BSP_MOTC_DirSet(BSP_ID_ODD, BSP_MOT_DIR_CCW);
//    
//    for ( ; ; ) {
//        enc =   BSP_ENC_CntGet(BSP_ID_ODD);
//        err =   p - enc;
//        if ((err < LeadAng) && (err > -LeadAng)) {
//            BSP_MOTC_ClkSet(BSP_ID_ODD, Bit_SET);
//            BSP_TM8M_Delay(pw);
//            BSP_MOTC_ClkSet(BSP_ID_ODD, Bit_RESET);
//            BSP_TM8M_Delay(pw);
//            p--;
//            p  %=   BSP_ENC_PULSE_N * 4;
//        }
//        if (p < STEP_GO) {
//            break;
//        }
//    }
//    
//    for ( ; ; ) {
//        if (BSP_ENC_CntGet(BSP_ID_ODD) > enc) {
//            break;
//        }
//        enc =   BSP_ENC_CntGet(BSP_ID_ODD);
//        err =   p - enc;
//        if (err >= pb) {
//            BSP_MOTC_ClkSet(BSP_ID_ODD, Bit_SET);
//            BSP_TM8M_Delay(pw);
//            BSP_MOTC_ClkSet(BSP_ID_ODD, Bit_RESET);
//            BSP_TM8M_Delay(pw);
//            p--;
//            p  %=   BSP_ENC_PULSE_N * 4;
//        }
//    }
//    
//    }
//    OSTimeDlyHMSM(0, 0, 5, 0);
//    }
//}
///// for close loop motor test

#define     DBG_MAX_US_SWI_TOGGLE       200     /// 由于阻容滤波, 开关输入信号最大翻转频率为100uS 
#define     DBG_PHASE_MASK              0x07    /// 只有8个状态循环
static  bool  DBG_SqnSwiIsOK (u8 phase)
{
    BSP_EMB_ST_TypeDef  stodd;
    BSP_EMB_ST_TypeDef  steven;
    u32                 outin;
    u32                 swiin;
    
    
    switch (phase & DBG_PHASE_MASK) {
    case 0:
        stodd   =   BSP_EMB_ST_NORMAL;
        steven  =   BSP_EMB_ST_OFF;
        outin   =   0x01;
        break;
    case 1:
        stodd   =   BSP_EMB_ST_NORMAL;
        steven  =   BSP_EMB_ST_PATCH;
        outin   =   0x03;
        break;
    case 2:
        stodd   =   (BSP_EMB_ST_TypeDef)(BSP_EMB_ST_NORMAL | BSP_EMB_ST_PATCH);
        steven  =   BSP_EMB_ST_PATCH;
        outin   =   0x07;
        break;
    case 3:
        stodd   =   (BSP_EMB_ST_TypeDef)(BSP_EMB_ST_NORMAL | BSP_EMB_ST_PATCH);
        steven  =   (BSP_EMB_ST_TypeDef)(BSP_EMB_ST_NORMAL | BSP_EMB_ST_PATCH);
        outin   =   0x0f;
        break;
    case 4:
        stodd   =   BSP_EMB_ST_PATCH;
        steven  =   (BSP_EMB_ST_TypeDef)(BSP_EMB_ST_NORMAL | BSP_EMB_ST_PATCH);
        outin   =   0x0e;
        break;
    case 5:
        stodd   =   BSP_EMB_ST_PATCH;
        steven  =   BSP_EMB_ST_NORMAL;
        outin   =   0x0c;
        break;
    case 6:
        stodd   =   BSP_EMB_ST_OFF;
        steven  =   BSP_EMB_ST_NORMAL;
        outin   =   0x08;
        break;
    case 7:
        stodd   =   BSP_EMB_ST_OFF;
        steven  =   BSP_EMB_ST_OFF;
        outin   =   0x00;
        break;
    default:
        break;
    }
    BSP_SQN_EmbStSend(BSP_ID_ODD , stodd);
    BSP_SQN_EmbStSend(BSP_ID_EVEN, steven);//steven=3，R36是3.3V
    
    BSP_TM8M_Delay(DBG_MAX_US_SWI_TOGGLE * 8);
    
    swiin   =   (BSP_SWI_FiGet(BSP_ID_ODD)  << 0)
              | (BSP_SWI_SeGet(BSP_ID_EVEN) << 1)
              | (BSP_SWI_SeGet(BSP_ID_ODD)  << 2)
              | (BSP_SWI_FiGet(BSP_ID_EVEN) << 3);
    if (swiin == outin) {
        return TRUE;
    } else {
        return FALSE;
    }
}

//#define     DBG_N_TxRx      10
//bool  DBG_C491IsOK (void)
//{
//    u16     buf[3]  =   {0x55, 0xaa, 0x5a};
//    u32     k;
//    u32     i;
//    
//    
//    for (k = 0; k < 1000; k++) {
//        C491_Send(DBG_N_TxRx, buf);
//        
//        for (i = 0; i < DBG_N_TxRx; i++) {
//            if (C491_ReceiveOne(0) != buf[i]) {
//                return FALSE;
//            }
//        }
//    }
//    return  TRUE;
//}
#define     DBG_C491_OVERTIME       3

static  bool  DBG_C491IsOK (void)
{
    u32     i;
    u16     mask;
    u16     random;
    u16     buf[C491_T_BUF_SIZE];
    
    
    if (BSP_Is924) {
        i   =   9;
    } else {
        i   =   8;
    }
    mask    =   (1 << i) - 1;
    random  =   BSP_TM8M_Get();
    
    for (i = 0; i < C491_T_BUF_SIZE; i++) {
        buf[i]  =   ((random >> i) + random) & mask;
    }
//    C491_Send(C491_T_BUF_SIZE, buf);
    if (C491_SendBuf(C491_T_BUF_SIZE, buf, DBG_C491_OVERTIME)) {
        return FALSE;
    }
    
    for (i = 0; i < C491_T_BUF_SIZE; i++) {
        if (C491_ReceiveOne(DBG_C491_OVERTIME) != buf[i]) {
            return FALSE;
        }
    }
    return  TRUE;
}
static  const   IO_ID_TypeDef   dbg_io_id_sol[BSP_DEVICE_ID_N]   =   {PC08_SOL_ODD, PC09_SOL_EVEN};
static  const   IO_ID_TypeDef   dbg_io_id_qf[1]  =   {PA08_SOL0};
void  DBG_SolideOK (u32 j)
 {
 	u32     i;
	if(j%2)
	{
		for(i=0;i<1;i++)
    		IO_OutSet(dbg_io_id_sol[(BSP_ID_TypeDef)i], BSP_SOL_OFF);
		//IO_OutSet(dbg_io_id_qf[(BSP_ID_TypeDef)0], BSP_SOL_OFF);
	}
	else
	{
	 	for(i=0;i<1;i++)
    		IO_OutSet(dbg_io_id_sol[(BSP_ID_TypeDef)i], BSP_SOL_ON);
	 	//IO_OutSet(dbg_io_id_qf[(BSP_ID_TypeDef)0], BSP_SOL_ON);
	}	
 }
#define     DBG_CYCLE_MS    500
void  DBG_ALL (u16 hour)
{
    u32     last;//,dbg_wait,dbg_maichong;
    u32     dip;
    u32     loop;
    u32     i;
  //  bool    isok;
    MOTO_HGH_CMD_TypeDef    hodd[4];
    MOTO_HGH_CMD_TypeDef    heve[4];
    u16 ODD_AD_Value_Now,ODD_AD_Value_Last,EVE_AD_Value_Now,EVE_AD_Value_Last;
    
    DBG_StartAlarmTestLed();
    
    last    =   BSP_DIP_Get();
  //  isok    =   TRUE;
    loop    =   hour * 3600 * 1000 / DBG_CYCLE_MS;
    hodd[0]    =   MOTO_HGH_POS0;
	hodd[1]    =   MOTO_HGH_POS3;
	hodd[2]    =   MOTO_HGH_POS7;
	hodd[3]    =   MOTO_HGH_POS10;
    heve[0]    =   MOTO_HGH_POS0;
    heve[1]    =   MOTO_HGH_POS3;
	heve[2]    =   MOTO_HGH_POS7;
	heve[3]    =   MOTO_HGH_POS10;
    for (i = 0; i < loop; i++) {
        /// 依次测试各个部分, 执行时间从短到长
        
        /// 1   检测DIP
        dip =   BSP_DIP_Get();
        if (dip != last) {
            BSP_LED_Toggle(DBG_LED_DIP);
            last    =   dip;
        }
        
        /// 2   检测机头板
        DBG_HD();
        
        /// 3   检测输入输出口, 约100uS
//        if (isok) {
           // isok    =   
			DBG_SqnSwiIsOK(i & DBG_PHASE_MASK);
       // }
        
        /// 4   检测491串口通讯, 1Byte 用100uS, C491_T_BUF_SIZE 个字节用1.6mS
        if (DBG_C491IsOK()) {
            BSP_LED_Toggle(DBG_LED_491);
        }

        /// 5   检测老化电机
        if (MOTO_HghGetState(BSP_ID_ODD) == MOTO_ST_OK) {
            MOTO_HghSetCmd(BSP_ID_ODD, hodd[0]);
            if (hodd[0] == MOTO_HGH_POS0) {
                hodd[0] = MOTO_HGH_POS10;
            } else {
                hodd[0] = MOTO_HGH_POS0;
            }
			
			

        }
        if (MOTO_HghGetState(BSP_ID_EVEN) == MOTO_ST_OK) {
            MOTO_HghSetCmd(BSP_ID_EVEN, heve[0]);
            if (heve[0] == MOTO_HGH_POS0) {
                heve[0] = MOTO_HGH_POS10;
            } else {
                heve[0] = MOTO_HGH_POS0;
            } 
			
        } /*
		for(dbg_maichong=0;dbg_maichong<100;dbg_maichong++)
		{
			BSP_MOTO_ClkSet((BSP_ID_TypeDef)0, (BitAction)0);
			for(dbg_wait=0;dbg_wait<200;dbg_wait++)
				dbg_wait++;
			BSP_MOTO_ClkSet((BSP_ID_TypeDef)0, (BitAction)1);
			for(dbg_wait=0;dbg_wait<200;dbg_wait++)
				dbg_wait++;
		}*/	
		//检查电位器
		ODD_AD_Value_Now=BSP_POT_GetAve(BSP_ID_ODD);
		EVE_AD_Value_Now=BSP_POT_GetAve(BSP_ID_EVEN);
		if(ODD_AD_Value_Last>ODD_AD_Value_Now)
		{
			if((ODD_AD_Value_Last-ODD_AD_Value_Now)>100)
			{
				IO_OutSet(dbg_io_id_qf[(BSP_ID_TypeDef)0], BSP_SOL_ON);	
			}
		}
		else
		{
			if((ODD_AD_Value_Now-ODD_AD_Value_Last)>100)
			{
				IO_OutSet(dbg_io_id_qf[(BSP_ID_TypeDef)0], BSP_SOL_ON);	
			}
		}
		if(EVE_AD_Value_Last>EVE_AD_Value_Now)
		{
			if((EVE_AD_Value_Last-EVE_AD_Value_Now)>100)
			{
				IO_OutSet(dbg_io_id_sol[(BSP_ID_TypeDef)1], BSP_SOL_ON);	
			}
		}
		else
		{
			if((EVE_AD_Value_Now-EVE_AD_Value_Last)>100)
			{
				IO_OutSet(dbg_io_id_sol[(BSP_ID_TypeDef)1], BSP_SOL_ON);	
			}
		}

		ODD_AD_Value_Last=ODD_AD_Value_Now;	
		EVE_AD_Value_Last=EVE_AD_Value_Now;	
		//6 电磁铁检查
	    DBG_SolideOK(i);
//        if (MOTO_CutGetState(BSP_ID_ODD) == MOTO_ST_OK) {
//            MOTO_CutSetCmd(BSP_ID_ODD, MOTO_CUT_FULL);
//        } 
//        if (MOTO_CutGetState(BSP_ID_EVEN) == MOTO_ST_OK) {
//            MOTO_CutSetCmd(BSP_ID_EVEN, MOTO_CUT_FULL);
//        }
        
        /// 使检测周期约为 DBG_CYCLE_MS
		
        OSTimeDlyHMSM(0, 0, 0, DBG_CYCLE_MS);
		IO_OutSet(dbg_io_id_qf[(BSP_ID_TypeDef)0], BSP_SOL_OFF);
		IO_OutSet(dbg_io_id_sol[(BSP_ID_TypeDef)1], BSP_SOL_OFF);
    }
    
    DBG_EndAlarm();
}




/// End of file -----------------------------------------------------------------------------------------
