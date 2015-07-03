
/// ----------------------------------------------------------------------------
/// File Name          : com.c
/// Description        : 通讯应用模块(全双工通讯)
///
///     因为使用 OS 资源, 所以放在APP层.
/// ----------------------------------------------------------------------------
/// History:

/// y11/m03/d06     v0.06   chenyong    modify
/// 1   去掉以前多加的一条语句，
/// 函数 COM_9511GetHghEven() 中去掉语句 "rt  =   MOTO_HghGetHeight(BSP_ID_EVEN);"
/// 2   原9511板在进出补绣时，针高不变化. 为修改此问题, 状态切换时，不改变针高. 
/// 更改如下：
///     函数 COM_9511UpStChange()   去掉函数COM_HghUpdate()的调用
///     函数 COM_HghUpdate()        增加返回值, 判断是否完成动作
///     增加变量 COM_HghIsAim[]     记录针高是否与机头开关状态同步
///     函数 COM_9511FaultOff()     增加调用 COM_HghUpdate() 的条件, 针高与机头开关状态不同步则调用
///     函数 COM_9511MAOrgIn()      增加调用 COM_HghUpdate() 的条件, 针高与机头开关状态不同步则调用
///     函数 COM_UpStByKeyUp()      增加调用函数 COM_HghUpdate(), 并记录返回值
///     函数 COM_UpStByKeyMd()      增加调用函数 COM_HghUpdate(), 并记录返回值
///     函数 COM_UpStByKeyDn()      增加调用函数 COM_HghUpdate(), 并记录返回值
/// 3   刺绣时，主控报断线停车, 而没有机头亮红灯.
///     分析为9511上报断线后，主控仍然发现线夹信号.
///     修改函数 COM_9511BrkDetect(), 去掉函数 COM_9511UpStByMARun()的调用, 避免线夹信号清灯
/// 4   断检过于灵敏, 增加函数 COM_XJ_3(), 3此线夹信号调用1次函数 COM_XJ_3.
/// 5   修改针高回复函数. 针高没有运动前, 回复实际针高

/// y10/m12/d22     v0.05   chenyong    modify
/// 加入函数 bool COM_JogIsEn(BSP_ID_TypeDef id), 用于针高电机模块判断能否微动

/// y10/m12/d17     v0.04   chenyong    modify
/// 因为主控有发针高前没有发主轴到位的情况, 修改了COM_HighCmd()函数
/// "跳跃剪线"参数设为否时, 越框前, 主控发不到位, 越框程序中升针高没有被9511执行, 针在下位就越框了
/// 另外, COM_HghUpdate()函数中应加入限制条件 COM_9511MAIsRunning == TRUE,  !!! 目前没有加入 !!!
    
/// y10/m11/d30     v0.03   chenyong    modify
/// 1   改变定义 COM_HGH_NOT_DROP, 原为(10), 改为(MOTO_HGH_POS_UP)
/// 2   修改函数 COM_9511GetHghOdd() 和 COM_9511GetHghEven(), 减少误报"针高不一致"
/// 3   修改函数 COM_9511GetDnErr(), 主轴不转不报断线

/// y10/m08/d27     v0.02   chenyong    modify

/// y10/m08/d06     v0.01   shengjiaq   setup





#define  COM_MODULE     /// 模块定义, 用区分内外部定义


/// Includes --------------------------------------------------------------------------------------------
#include  <bsp.h>
#include  <ucos_ii.h>

#include  "com.h"

#include  "c491.h"
#include  "moto.h"
#include  "in.h"
#include  "hdled.h"
#include "../3ds/crypto_des3.h"
#include "../3ds/bitstream.h"



/// Private define --------------------------------------------------------------------------------------

#define  COM_BIT_ODD(data)              (data & (1 << 1))
#define  COM_BIT_EVE(data)              (data & (1 << 0))

#define  VERSION9511                    3                       /// 1 - 无CRC,  2 - 有CRC

#define  COM_9511_ADDR_ALL              0

#define  COM_MAX_FRAME_LEN              10

#define  COM_BIT_ADDR_9511              7
#define  COM_MSK_9511_WHOLE             ((1 << (COM_BIT_ADDR_9511 + 1)) - 1)//1 0000 0000 -1=0xFF
#define  COM_MSK_9511_DATA              ((1 <<  COM_BIT_ADDR_9511     ) - 1)//1000 0000 -1 =0x3F
#define  COM_BIT_ADDR_924               8
#define  COM_MSK_924_WHOLE              ((1 << (COM_BIT_ADDR_924  + 1)) - 1)
#define  COM_MSK_924_DATA               ((1 <<  COM_BIT_ADDR_924      ) - 1)

/// ------- cy 101007   a   -------
#define  COM_LEN_TAB9511                (sizeof(COM_9511Tbl) / sizeof(COM_9511Tbl[0]))
#define  COM_LEN_TAB924                 (sizeof(COM_924Tbl)  / sizeof(COM_924Tbl[0]))
/// ------- cy 101007   a   -------

#define  COM_ERR_ONE_TIME               (BSP_LED_Toggle(1))


#define  COM_HGH_NOT_DROP               (MOTO_HGH_POS_UP)

#define  Tangkong_High0                  175
#define  Tangkong_High1                  160
#define  Tangkong_High2                  145
#define  Tangkong_High3                  130
#define  Tangkong_High4                  115
#define  Tangkong_High5                  100
#define  Tangkong_High6                  85
#define  Tangkong_High7                  70
#define  Tangkong_High8                  55
#define  Tangkong_High9                  40
//u8 	Tangkong_High[10]={175,160,145,130,115,100,85,70,55,40};
//u8 	Tangkong_High[10]={45,50,55,60,65,70,75,80,85,90};
u8 	Tangkong_High[10]={15,20,25,30,35,40,45,45,45,45};
/// Exported types -------------------------------------------------------------

typedef  void  COM_EXE_F_TypeDef (u16 buf[], u8 len);

typedef struct {
    u16                 cmd;
    u8                  len;
	u8                  run_sta;
    COM_EXE_F_TypeDef  *DnExe;
    COM_EXE_F_TypeDef  *UpExe;
	
} COM_ITEM_TypeDef;

typedef enum
{
    COM_UP_ST_LOCK      =   0,
    COM_UP_ST_NORMAL,
    COM_UP_ST_BROKEN,
    COM_UP_ST_PATCH_LOCK,
    COM_UP_ST_PATCH_IDLE,
    COM_UP_ST_PATCH_NORMAL,
    COM_UP_ST_PATCH_BROKEN,
//    COM_UP_ST_FAULT,
} COM_UP_ST_TypeDef;


/// for 9511板

/// 9511板地址从1按序排列

/// 回复, 主控接收, 9511发出
typedef enum
{
  	COM_8_RES_BUSY         =   0x12,       /// 忙
    COM_8_RES_SQN_POS      =   0x14,       /// 金片装置导通到位
    COM_8_RES_SQN_NOTPOS,                  /// 金片装置不到位
    COM_8_RES_DAT_ERR      =   0x16,       /// 数据校验错误
    COM_8_RES_EVE_KEYERR   =   0x20,       /// 偶数头机头开关异常(偶下机头撞环)
    COM_8_RES_ODD_KEYERR   =   0x21,       /// 奇数头机头开关异常(奇下机头撞环)
    COM_8_RES_EVE_OVERTIM  =   0x30,       /// 偶数头超时(针高剪线)
    COM_8_RES_ODD_OVERTIM  =   0x31,       /// 奇数头超时(针高剪线)
    COM_8_RES_EVE_REVERSE  =   0x40,       /// 偶数头反转(针高)            
    COM_8_RES_ODD_REVERSE  =   0x41,       /// 奇数头反转(针高)
    COM_8_RES_EVE_BREAK    =   0x50,       /// 偶数头断线
    COM_8_RES_ODD_BREAK    =   0x51,       /// 奇数头断线    
    COM_8_RES_ALL_BREAK    =   0x13,       /// 都断线
    COM_8_RES_EVE_NOTPOS   =   0x60,       /// 偶数头不到位,针高异常(不在档),剪线异常(不在原点)
    COM_8_RES_ODD_NOTPOS   =   0x61,       /// 奇数头不到位,针高异常(不在档),剪线异常(不在原点)
    
} COM_8_RES_TypeDef;



/// for 924板

/// 机头板地址
typedef enum
{
    COM_9_ADDR_DN_NO16     =   0xde,
    COM_9_ADDR_UP_NO16     =   0xdf,
    COM_9_ADDR_DN_COM      =   0xe0,
    COM_9_ADDR_DN_NO1,
    COM_9_ADDR_DN_NO2,
    COM_9_ADDR_DN_NO3,
    COM_9_ADDR_DN_NO4,
    COM_9_ADDR_DN_NO5,
    COM_9_ADDR_DN_NO6,
    COM_9_ADDR_DN_NO7,
    COM_9_ADDR_DN_NO8,
    COM_9_ADDR_DN_NO9,
    COM_9_ADDR_DN_NO10,
    COM_9_ADDR_DN_NO11,
    COM_9_ADDR_DN_NO12,
    COM_9_ADDR_DN_NO13,
    COM_9_ADDR_DN_NO14,
    COM_9_ADDR_DN_NO15,
    COM_9_ADDR_UP_COM      =   0xf0,
    COM_9_ADDR_UP_NO1,
    COM_9_ADDR_UP_NO2,
    COM_9_ADDR_UP_NO3,
    COM_9_ADDR_UP_NO4,
    COM_9_ADDR_UP_NO5,
    COM_9_ADDR_UP_NO6,
    COM_9_ADDR_UP_NO7,
    COM_9_ADDR_UP_NO8,
    COM_9_ADDR_UP_NO9,
    COM_9_ADDR_UP_NO10,
    COM_9_ADDR_UP_NO11,
    COM_9_ADDR_UP_NO12,
    COM_9_ADDR_UP_NO13,
    COM_9_ADDR_UP_NO14,
    COM_9_ADDR_UP_NO15,
    
} COM_9_Addr_TypeDef;

/// 回复, 主控接收, 9511 或 924 发出
typedef enum
{
    COM_9_RES_ODD_NOTPOS   =   0x01,       /// 奇数头不到位
    COM_9_RES_EVE_NOTPOS,                  /// 偶数头不到位
    COM_9_RES_ALL_NOTPOS,                  /// 都不到位
    COM_9_RES_ODD_HGHERR   =   0x04,       /// 奇数头针高异常
    COM_9_RES_EVE_HGHERR,                  /// 偶数头针高异常
    COM_9_RES_ALL_HGHERR,                  /// 都针高异常
    COM_9_RES_ODD_BREAK    =   0x11,       /// 奇数头断线
    COM_9_RES_EVE_BREAK,                   /// 偶数头断线
    COM_9_RES_ALL_BREAK,                   /// 都断线
    COM_9_RES_ODD_KEYERR   =   0x21,       /// 奇数头机头开关异常
    COM_9_RES_EVE_KEYERR,                  /// 偶数头机头开关异常
    COM_9_RES_ALL_KEYERR,                  /// 都机头开关异常
    COM_9_RES_ODD_REVERSE  =   0x31,       /// 奇数头反转(针高)
    COM_9_RES_EVE_REVERSE,                 /// 偶数头反转(针高)
    COM_9_RES_ALL_REVERSE,                 /// 都反转(针高)
    COM_9_RES_ODD_OVERTIM  =   0x41,       /// 奇数头超时(针高剪线)
    COM_9_RES_EVE_OVERTIM,                 /// 偶数头超时(针高剪线)
    COM_9_RES_ALL_OVERTIM,                 /// 都超时(针高剪线)
    COM_9_RES_ODD_DHOVERT  =   0x51,       /// 奇数头超时(DH轴)
    COM_9_RES_EVE_DHOVERT,                 /// 偶数头超时(DH轴)
    COM_9_RES_ALL_DHOVERT,                 /// 都超时(DH轴)
    COM_9_RES_CUTTAP_BUSY  =   0x62,       /// 忙1(剪线Z轴在转动)
    COM_9_RES_DH_BUSY,                     /// 忙2(DH轴在转动)
    COM_9_RES_CRC_ERR,                     /// CRC数据错误
    COM_9_RES_BUSY,                        /// 忙
    COM_9_RES_SQN_POS      =   0x70,       /// 金片装置到位
    COM_9_RES_SQN_NOTPOS,                  /// 金片装置不到位
    COM_9_RES_DN_DATLOSS   =   0x90,       /// 下机头数据丢失
    COM_9_RES_UP_DATLOSS,                  /// 上机头数据丢失

} COM_9_RES_TypeDef;


/// Private variables ----------------------------------------------------------

static  bool                COM_IsFault[BSP_DEVICE_ID_N]    =   {FALSE, FALSE};     /// 故障机头 (上 和 下)
//static  bool              COM_TKFault[BSP_DEVICE_ID_N]    =   {TRUE, TRUE};     /// 故障机头 (上 和 下)
static  bool                COM_DnIsEnable[BSP_DEVICE_ID_N] =   {TRUE,  TRUE};      /// 剪线使能状态 (下)
static  bool                COM_TKIsEnable[BSP_DEVICE_ID_N] =   {FALSE,  FALSE};      /// 剪线使能状态 (下)

static  bool                COM_SwiDirIsEnable[BSP_DEVICE_ID_N] =   {FALSE,  FALSE};      /// 剪线使能状态 (下)
static  COM_UP_ST_TypeDef   COM_UpStNow[BSP_DEVICE_ID_N];
static  COM_UP_ST_TypeDef   COM_UpStLast[BSP_DEVICE_ID_N];                            /// 上机头状态 (上)
static  bool                COM_HghIsAim[BSP_DEVICE_ID_N]   =   {FALSE, FALSE};     /// 针高是否在目标位置 (上)
static  bool                COM_IsHghRest                   =   TRUE;               /// 上电针高静止(上)
  bool                COM_9511IsTset                  =   FALSE;              /// 是否测试状态 (上)(机头灯让给主控)

/// cy  101115  added
/// 正常刺绣时, COM_9511MAIsInOrg = FALSE, COM_9511MAIsRunning = TRUE
/// 越框时,     COM_9511MAIsInOrg = FALSE, COM_9511MAIsRunning = FALSE
/// 上述由主控通讯命令改变, COM_9511MAIsInOrg 与实际意义不符合
static  bool                COM_9511MAIsInOrg               =   FALSE;              /// 主轴是否在零位
static  bool                COM_9511MAIsRunning             =   FALSE;              /// 主轴是否在运转
/// cy  101115  added
static  void  COM_9511SetGreALL (u16 buf[], u8 len);//全变绿灯，主控发指令
static  u8                  COM_WorkHigh                    =   0;                  /// 设定针高值 (上)


static  u16                 COM_9511CheckSum                =   0;                  /// 9511校验和
static  u16                 COM_9511CheckNum                =   0;                  /// 9511校验数据个数
static  bool                COM_9511CheckErr                =   FALSE;              /// 9511校验数据是否错误
bool                COM_9511TangkongDir             =   TRUE;
bool                COM_9511TangkongEnable[2]             =  { TRUE,TRUE};
bool                COM_9511TangkongPatch[2]             =  { 0,0};
static  u8                  COM_Tangkong_WorkHigh           =   0;
static  u8                  COM_Tangkong_WorkHigh_Last      =0;
MOTO_HGH_CMD_TypeDef		COM_Tangkong_WorkCmd;
/// Private function prototypes ------------------------------------------------

COM_EXE_F_TypeDef       COM_CmdDoNothing;

COM_EXE_F_TypeDef       COM_HighTo0;            /// 0x0080
COM_EXE_F_TypeDef       COM_HighTo1;            /// 0x0081
COM_EXE_F_TypeDef       COM_HighTo2;            /// ...
COM_EXE_F_TypeDef       COM_HighTo3;
COM_EXE_F_TypeDef       COM_HighTo4;
COM_EXE_F_TypeDef       COM_HighTo5;
COM_EXE_F_TypeDef       COM_HighTo6;
COM_EXE_F_TypeDef       COM_HighTo7;
COM_EXE_F_TypeDef       COM_HighTo8;
COM_EXE_F_TypeDef       COM_HighTo9;
COM_EXE_F_TypeDef       COM_HighTo10;           /// 0x008a
COM_EXE_F_TypeDef       COM_HeadUpDown;           /// 0x008b
COM_EXE_F_TypeDef       COM_HeadSpeed;           /// 0x008c
COM_EXE_F_TypeDef       COM_9511SetSol;         /// 0x008f
COM_EXE_F_TypeDef		COM_MainSetPssW;	
COM_EXE_F_TypeDef       COM_Tangkong_Action;
COM_EXE_F_TypeDef       COM_Tangkong_Position;
COM_EXE_F_TypeDef       COM_Tangkong_Sta;
COM_EXE_F_TypeDef       COM_Tangkong_SinglDir;
COM_EXE_F_TypeDef       COM_Tangkong_DoublDir;

COM_EXE_F_TypeDef		COM_Tangkong_HighTo0;
COM_EXE_F_TypeDef		COM_Tangkong_HighTo1;
COM_EXE_F_TypeDef		COM_Tangkong_HighTo2;
COM_EXE_F_TypeDef		COM_Tangkong_HighTo3;
COM_EXE_F_TypeDef		COM_Tangkong_HighTo4;
COM_EXE_F_TypeDef		COM_Tangkong_HighTo5;
COM_EXE_F_TypeDef		COM_Tangkong_HighTo6;
COM_EXE_F_TypeDef		COM_Tangkong_HighTo7;
COM_EXE_F_TypeDef		COM_Tangkong_HighTo8;
COM_EXE_F_TypeDef		COM_Tangkong_HighTo9;
COM_EXE_F_TypeDef		COM_Tangkong_HighTo10;


COM_EXE_F_TypeDef		COM_MainErasPssW;	
//COM_EXE_F_TypeDef       COM_9511FaultOff2;       /// 0x00d0
COM_EXE_F_TypeDef		COM_MainCh9511;	   ///0x00ce
COM_EXE_F_TypeDef		COM_MainConfirm;		///0x00cf
//COM_EXE_F_TypeDef       COM_9511DnC2ToUpBrk;      /// 0x00d2
COM_EXE_F_TypeDef       COM_9511FaultOff;       /// 0x00d0
COM_EXE_F_TypeDef       COM_9511DataCheck;      /// 0x00d1
COM_EXE_F_TypeDef       COM_9511DnToUpBrk;      /// 0x00d2
COM_EXE_F_TypeDef       COM_9511BrkDetMod;      /// 0x00d3
COM_EXE_F_TypeDef       COM_9511GetIsSqnUp;     /// 0x00d6
COM_EXE_F_TypeDef       COM_9511DnEnable;       /// 0x00d8
COM_EXE_F_TypeDef       COM_9511CutBack;        /// 0x00d9
COM_EXE_F_TypeDef       COM_9511CutHalf;        /// 0x00da
COM_EXE_F_TypeDef       COM_9511CutFull;        /// 0x00db
COM_EXE_F_TypeDef       COM_9511CutSetPara;     /// 0x00dc
COM_EXE_F_TypeDef       COM_9511GetVersion;     /// 0x00dd
COM_EXE_F_TypeDef       COM_9511SetRedOdd;      /// 0x00de
COM_EXE_F_TypeDef       COM_9511SetGreOdd;      /// 0x00df
COM_EXE_F_TypeDef       COM_9511SetOraOdd;      /// 0x00e0
COM_EXE_F_TypeDef       COM_9511TestEnter;      /// 0x00e1
COM_EXE_F_TypeDef       COM_9511TestExit;       /// 0x00e2
COM_EXE_F_TypeDef       COM_9511SetRedEven;     /// 0x00e3
COM_EXE_F_TypeDef       COM_9511SetGreEven;     /// 0x00e4
COM_EXE_F_TypeDef       COM_9511SetOraEven;     /// 0x00e5
COM_EXE_F_TypeDef       COM_9511MAOrgOut;       /// 0x00e6
COM_EXE_F_TypeDef       COM_9511GetHghOdd;      /// 0x00e7
COM_EXE_F_TypeDef       COM_9511GetHghEven;     /// 0x00e8
COM_EXE_F_TypeDef       COM_9511GetADOdd;       /// 0x00e9
COM_EXE_F_TypeDef       COM_9511GetADEven;      /// 0x00ea
COM_EXE_F_TypeDef       COM_9511MAOneLoop;      /// 0x00eb
COM_EXE_F_TypeDef       COM_9511SetGreALL;      /// 0x00ec
COM_EXE_F_TypeDef       COM_9511PatchExit;      /// 0x00ee
COM_EXE_F_TypeDef       COM_9511PatchEnter;     /// 0x00ef
COM_EXE_F_TypeDef       COM_9511GetHDLed;       /// 0x00f7
COM_EXE_F_TypeDef       COM_9511GetCutOrg;      /// 0x00f8
COM_EXE_F_TypeDef       COM_9511GetDnErr;       /// 0x00f9
COM_EXE_F_TypeDef       COM_9511MAOrgIn;        /// 0x00fa
COM_EXE_F_TypeDef       COM_9511GetUpErr;       /// 0x00fc
COM_EXE_F_TypeDef       COM_9511MAStop;         /// 0x00fd
COM_EXE_F_TypeDef       COM_9511MAStartUp;      /// 0x00fe
COM_EXE_F_TypeDef       COM_9511BrkDetect;      /// 0x00ff
#define HOLD     5
extern u8 UpHeadSpeed;
/// Private consts -------------------------------------------------------------

static  const  COM_ITEM_TypeDef  COM_9511Tbl[]  =   {
    {0x0080,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},           /// 针高0   UP  无数据
    {0x0081,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},           /// 针高1   UP  无数据
    {0x0082,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},           /// ...
    {0x0083,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},
    {0x0084,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},
    {0x0085,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},
    {0x0086,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},
    {0x0087,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},
    {0x0088,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},
    {0x0089,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},           /// 针高9   UP  无数据
    {0x008a,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},          /// 针高10  UP  无数据
    {0x008b,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x008c,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x008d,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x008e,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x008f,    1,  5,COM_CmdDoNothing,   COM_9511SetSol},      /// 空命令
    
    {0x0090,    0,  5,COM_CmdDoNothing,   COM_Tangkong_HighTo0},      /// 空命令
    {0x0091,    0,  5,COM_CmdDoNothing,   COM_Tangkong_HighTo1},      /// 空命令
    {0x0092,    0,  5,COM_CmdDoNothing,   COM_Tangkong_HighTo2},      /// 空命令
    {0x0093,    0,  5,COM_CmdDoNothing,   COM_Tangkong_HighTo3},      /// 空命令
    {0x0094,    0,  5,COM_CmdDoNothing,   COM_Tangkong_HighTo4},      /// 空命令
    {0x0095,    0,  5,COM_CmdDoNothing,   COM_Tangkong_HighTo5},      /// 空命令
    {0x0096,    0,  5,COM_CmdDoNothing,   COM_Tangkong_HighTo6},      /// 空命令
    {0x0097,    0,  5,COM_CmdDoNothing,   COM_Tangkong_HighTo7},      /// 空命令
    {0x0098,    0,  5,COM_CmdDoNothing,   COM_Tangkong_HighTo8},      /// 空命令
    {0x0099,    0,  5,COM_CmdDoNothing,   COM_Tangkong_HighTo9},      /// 空命令
    {0x009a,    0,  5,COM_CmdDoNothing,   COM_Tangkong_HighTo10},      /// 空命令
    {0x009b,    0,  5,COM_CmdDoNothing,   COM_Tangkong_SinglDir},      /// 空命令
    {0x009c,    0,  5,COM_CmdDoNothing,   COM_Tangkong_DoublDir},      /// 空命令
    {0x009d,    0,  5,COM_CmdDoNothing,   COM_Tangkong_Position},      /// 空命令
    {0x009e,    0,  5,COM_CmdDoNothing,   COM_Tangkong_Sta},      /// 空命令
    {0x009f,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    
    {0x00a0,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x00a1,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x00a2,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x00a3,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x00a4,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x00a5,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x00a6,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x00a7,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x00a8,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x00a9,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x00aa,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x00ab,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x00ac,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x00ad,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x00ae,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x00af,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    
    {0x00b0,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x00b1,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x00b2,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x00b3,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x00b4,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x00b5,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x00b6,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x00b7,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x00b8,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x00b9,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x00ba,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x00bb,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x00bc,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x00bd,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x00be,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x00bf,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    
    {0x00c0,    5,  HOLD,COM_CmdDoNothing,   COM_9511FaultOff},      ///新关闭故障机头
    {0x00c1,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x00c2,    5,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      ///新上传断检
    {0x00c3,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x00c4,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x00c5,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x00c6,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x00c7,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x00c8,    5,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      ///新剪刀使能
    {0x00c9,    10, 60,COM_CmdDoNothing,    COM_CmdDoNothing},      /// 设置下机头密码
    {0x00ca,    0,  60,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 擦除上机机头密码
    {0x00cb,    0,  60,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 擦除下机头密码
    {0x00cc,    10, 60,COM_CmdDoNothing,   COM_CmdDoNothing},      	/// 设置上机头密码
    {0x00cd,    0,  60,COM_CmdDoNothing,   COM_CmdDoNothing},   	/// 查下机头认证
    {0x00ce,    0,  60,COM_CmdDoNothing,   COM_CmdDoNothing},   		/// 查上机头认证
    {0x00cf,    10, 60,COM_CmdDoNothing,   COM_CmdDoNothing},    	///认证上下机头都执行
    
    {0x00d0,    5,  HOLD,COM_CmdDoNothing,  COM_9511FaultOff},      /// 旧关闭故障机头            上和下  数据5
    {0x00d1,    2,  HOLD,COM_CmdDoNothing,  COM_CmdDoNothing},     /// 数据校验                上和下  数据2
    {0x00d2,    5,  HOLD,COM_CmdDoNothing,  COM_CmdDoNothing},     /// 通知上机头下断检的情况  上      数据5
    {0x00d3,    5,  HOLD,COM_CmdDoNothing,  COM_CmdDoNothing},     /// 断线检测方式            上和下  数据5
    {0x00d4,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x00d5,    1,  HOLD,COM_CmdDoNothing,   COM_HeadSpeed},      /// 空命令
    {0x00d6,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},    /// 查询金片是否抬起        上      数据0
    {0x00d7,    2,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 蔡专用 H10的AD值    视为空命令
    {0x00d8,    5,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 下机头板使能            下      数据5
    {0x00d9,    0,  HOLD,COM_CmdDoNothing,    COM_CmdDoNothing},      /// 剪刀收回                下      数据0
    {0x00da,    0,  HOLD,COM_CmdDoNothing,    COM_CmdDoNothing},      /// 剪刀半出                下      数据0
    {0x00db,    0,  HOLD,COM_CmdDoNothing,    COM_CmdDoNothing},      /// 剪线                    下      数据0
    {0x00dc,    3,  HOLD,COM_CmdDoNothing,    COM_CmdDoNothing},      /// 设置剪线参数            下      数据3
    {0x00dd,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},    /// 查询上机头板版本        上      数据0
    {0x00de,    0,  HOLD,COM_CmdDoNothing,   COM_9511SetRedOdd},     /// 奇头亮红灯, AD > 设定值 上      数据0
    {0x00df,    0,  HOLD,COM_CmdDoNothing,   COM_9511SetGreOdd},     /// 奇头亮绿灯, AD== 设定值 上      数据0
    
    {0x00e0,    0,  HOLD,COM_CmdDoNothing,   COM_9511SetOraOdd},     /// 奇头亮黄灯, AD < 设定值 上      数据0
    {0x00e1,    0,  HOLD,COM_CmdDoNothing,   COM_9511TestEnter},     /// 进入测试状态            上      数据0
    {0x00e2,    0,  HOLD,COM_CmdDoNothing,   COM_9511TestExit},      /// 退出测试状态            上      数据0
    {0x00e3,    0,  HOLD,COM_CmdDoNothing,   COM_9511SetRedEven},    /// 偶头亮红灯, AD > 设定值 上      数据0
    {0x00e4,    0,  HOLD,COM_CmdDoNothing,   COM_9511SetGreEven},    /// 偶头亮绿灯, AD== 设定值 上      数据0
    {0x00e5,    0,  HOLD,COM_CmdDoNothing,   COM_9511SetOraEven},    /// 偶头亮黄灯, AD < 设定值 上      数据0
    {0x00e6,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 停车时主轴不在零位      上      数据0
    {0x00e7,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},     /// 奇头针高查询            上      数据0
    {0x00e8,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},    /// 偶头针高查询            上      数据0
    {0x00e9,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 奇头电位器查询          上      数据0
    {0x00ea,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},     /// 偶头电位器查询          上      数据0
    {0x00eb,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},     /// 点动                    上      数据0
    {0x00ec,    0,  HOLD,COM_CmdDoNothing,   COM_9511SetGreALL},     /// 清灯即全绿              上      数据0
    {0x00ed,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令(?有改动)
    {0x00ee,    0,  HOLD,COM_CmdDoNothing,   COM_9511PatchExit},     /// 退出补绣                上      数据0
    {0x00ef,    0,  HOLD,COM_CmdDoNothing,   COM_9511PatchEnter},    /// 进入补绣                上      数据0
    
    {0x00f0,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x00f1,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x00f2,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x00f3,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x00f4,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x00f5,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x00f6,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 空命令
    {0x00f7,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// 查灯状态(PatchIdle例外) 上      数据0
    {0x00f8,    0,  HOLD,COM_CmdDoNothing,  COM_CmdDoNothing},      /// 差剪刀是否回原点        下      数据0
    {0x00f9,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},
    {0x00fa,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},
    
    {0x00fb,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},
    
    {0x00fc,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},
    {0x00fd,    0,  HOLD,COM_CmdDoNothing,  COM_CmdDoNothing},
    {0x00fe,    0,  HOLD,COM_CmdDoNothing,  COM_CmdDoNothing},
    {0x00ff,    0,  HOLD,COM_CmdDoNothing,  COM_CmdDoNothing},
};

extern u8 Stream_9511_To_Main[10];
extern u8 Secrit_For_LED;
extern u8 Speed_pare[];
extern u8 HeadUpDown[BSP_DEVICE_ID_N];
extern u8 UpHeadSpeed;
extern u8 HeadUpDown_Status[];

/// 动作刷新, 灯 和 针高
static  void    COM_LedUpdate           (BSP_ID_TypeDef id);
/// cy  110304
//static  void    COM_HghUpdate           (BSP_ID_TypeDef id);
static  bool    COM_HghUpdate           (BSP_ID_TypeDef id);
/// cy  110304
static  void    COM_SqnUpdate           (BSP_ID_TypeDef id);
//static  void    COM_AllUpdate           (void);


static  void    COM_9511UpStChange      (BSP_ID_TypeDef id, COM_UP_ST_TypeDef NewSt);
static  void    COM_9511UpStByExitP     (void);
static  void    COM_9511UpStByEnterP    (void);
static  void    COM_9511UpStByMARun     (void);
static  void    COM_9511UpStByBroken    (BSP_ID_TypeDef id);
static  void    COM_UpStByKeyUp         (BSP_ID_TypeDef id);
static  void    COM_UpStByKeyMd         (BSP_ID_TypeDef id);
static  void    COM_UpStByKeyDn         (BSP_ID_TypeDef id);


static  void    COM_HighCmd             (u8 high);

static  u16     COM_9511Decode          (u16 buf[], u8 len);

static  BSP_EMB_ST_TypeDef  COM_GetEmbSt        (BSP_ID_TypeDef id);
static  u16                 COM_9511EmbStToBit  (BSP_EMB_ST_TypeDef st);
static  BSP_EMB_ST_TypeDef  COM_GetSqnEmbSt        (BSP_ID_TypeDef id);


static  bool                COM_9511IsAddr      (u16 data);
//static  bool                COM_924IsAddr       (u16 data);
static  bool                COM_9511IsValidAddr (u16 data,u16 data2);
//static  bool                COM_924IsValidAddr  (u16 data);

/// cy  110227  modify
static  void                COM_XJ_3            (void);
static  void  COM_ID_9511CutBack (BSP_ID_TypeDef id,u16 buf[], u8 len);//剪刀收回
static  void  COM_ID_9511CutHalf (BSP_ID_TypeDef id,u16 buf[], u8 len);//剪刀半出
static  void  COM_ID_9511CutFull (BSP_ID_TypeDef id,u16 buf[], u8 len);//剪线满入
static  void  COM_ID_9511CutH (BSP_ID_TypeDef id,u16 buf[], u8 len);//剪刀半出
static  void  COM_ID_9511CutD (BSP_ID_TypeDef id,u16 buf[], u8 len);//剪线满入
/// cy  110227  modify

unsigned char ds3_src[8];
unsigned char ds3_dst[8];
extern u8 TreahHold_9511;
u8 TK_Org[2]={0,0};
static  bool  COM_GetIsBroken (COM_UP_ST_TypeDef st)
{
    switch (st) {
        
    case COM_UP_ST_BROKEN:
    case COM_UP_ST_PATCH_BROKEN:
        return  TRUE;
        
    case COM_UP_ST_NORMAL:
    case COM_UP_ST_LOCK:
    case COM_UP_ST_PATCH_NORMAL:
    case COM_UP_ST_PATCH_LOCK:
    case COM_UP_ST_PATCH_IDLE:
    default:
        return  FALSE;
        
    }
}

static  bool  COM_GetIsDrop (COM_UP_ST_TypeDef st)
{
    switch (st) {
        
    case COM_UP_ST_BROKEN:
    case COM_UP_ST_NORMAL:
    case COM_UP_ST_PATCH_BROKEN:
    case COM_UP_ST_PATCH_NORMAL:
        return  TRUE;
        
    case COM_UP_ST_LOCK:
    case COM_UP_ST_PATCH_LOCK:
    case COM_UP_ST_PATCH_IDLE:
    default:
        return  FALSE;
        
    }
}
//这是根据机头的状态，更新机头开关板LED用的，注意不是发给主控的绣作状态
static  HDLED_ST_TypeDef  COM_GetHDLedSt (COM_UP_ST_TypeDef st,BSP_ID_TypeDef id)
{
    switch (st) {
        
    case COM_UP_ST_BROKEN:
    case COM_UP_ST_PATCH_BROKEN:
        return  HDLED_ST_RED; //断线就应该是红灯
    case COM_UP_ST_PATCH_NORMAL:
		if((COM_UpStLast[id]==COM_UP_ST_BROKEN)||(COM_UpStLast[id]==COM_UP_ST_PATCH_BROKEN))
			return  HDLED_ST_RED;
		else
        	return  HDLED_ST_GRE;   
    case COM_UP_ST_NORMAL:
    case COM_UP_ST_PATCH_IDLE:
   // case COM_UP_ST_PATCH_NORMAL:
        return  HDLED_ST_GRE;
        
    case COM_UP_ST_LOCK:
    case COM_UP_ST_PATCH_LOCK:
    default:
        return  HDLED_ST_OFF;
        
    }
}

static  BSP_EMB_ST_TypeDef  COM_GetEmbSt (BSP_ID_TypeDef id)
{//注意这里是获取的刺绣的状态：补绣、正常绣、停绣，是告诉主控用的。响应的是主控0xF7命令
    if (COM_IsFault[id] == FALSE) {
        switch (COM_UpStNow[id]) {
        case COM_UP_ST_PATCH_BROKEN:
        case COM_UP_ST_PATCH_NORMAL:
		
            return  BSP_EMB_ST_PATCH;//2
        case COM_UP_ST_PATCH_IDLE:
        case COM_UP_ST_NORMAL:
		case COM_UP_ST_BROKEN://这里断线，但是对于主控来讲是正常绣的刺绣状态。
			return  BSP_EMB_ST_NORMAL;//1因为如果是下断检，主控中会将剪线关闭，所以这里要返回正常，对应集中剪线返回正常与断线都一样	
        case COM_UP_ST_PATCH_LOCK:
        case COM_UP_ST_LOCK:
        default:
            return  BSP_EMB_ST_OFF;//0
        }
    } else {
        return  BSP_EMB_ST_OFF;
    }
}

static  BSP_EMB_ST_TypeDef  COM_GetSqnEmbSt (BSP_ID_TypeDef id)
{
    if (COM_IsFault[id] == FALSE) {
        switch (COM_UpStNow[id]) {
        case COM_UP_ST_PATCH_BROKEN:
        case COM_UP_ST_PATCH_NORMAL:
		case COM_UP_ST_BROKEN://这里是断线，对与金片绣来讲就已经是补绣了，因为这样它才能亮红灯
            return  BSP_EMB_ST_PATCH;
        case COM_UP_ST_PATCH_IDLE:
        case COM_UP_ST_NORMAL:
			return  BSP_EMB_ST_NORMAL;	
        case COM_UP_ST_PATCH_LOCK:
        case COM_UP_ST_LOCK:
        default:
            return  BSP_EMB_ST_OFF;
        }
    } else {
        return  BSP_EMB_ST_OFF;
    }
}
static  void  COM_LedUpdate (BSP_ID_TypeDef id)
{
    HDLED_ST_TypeDef    led;
    
    
    if (COM_9511IsTset == FALSE) {                  /// 非测试状态才更新显示
        if (COM_IsFault[id] == FALSE) {             /// 未关闭故障机头, 按状态更新显示
/// !!! cy 100905
//            led =   (HDLED_ST_TypeDef)((COM_UpStBits[COM_UpStNow[id]] >> COM_UP_BIT_HD_LED) & COM_UP_MSK_HD_LED);
            led =   COM_GetHDLedSt(COM_UpStNow[id],id);
/// !!! cy 100905
        } else {                                    /// 关闭故障机头, 灯为灭
            led =   (HDLED_ST_TypeDef)HDLED_ST_OFF;
        }
        HDLED_Set(id, led);                         /// 更新显示
        

        if (BSP_DIP_RedIsSpkl) {
            if (led == HDLED_ST_RED) {
                HDLED_Sparkle(id, 1 << 30);
            } else {
                HDLED_Sparkle(id, 0);
            }
        }

    } else {                                        /// 测试状态不更新显示
        ;
    }
}

static void MOT_CUT_Get_SwiDirIsStatus()
{
		u32 id;
		for(id=0;id<BSP_DEVICE_ID_N;id++)
		{
	    if ((BSP_SWI_FiGet(id) == BSP_SWITCH_LOW)||(COM_9511TangkongDir==FALSE)) /// 如没有回位, 需回位
		     COM_SwiDirIsEnable[id]=TRUE;
		else
			COM_SwiDirIsEnable[id]=FALSE;
		}			
}
/// cy  110304
//static  void  COM_HghUpdate (BSP_ID_TypeDef id)
static  bool  COM_HghUpdate (BSP_ID_TypeDef id)
/// cy  110304
{
    MOTO_HGH_CMD_TypeDef    cmd;
	MOT_CUT_Get_SwiDirIsStatus();
        if ((((COM_GetIsDrop(COM_UpStNow[id]) != FALSE)&&(COM_TKIsEnable[id]==TRUE)&&(COM_SwiDirIsEnable[id]==TRUE))||
			((COM_GetIsDrop(COM_UpStNow[id]) != FALSE)&&(COM_9511TangkongDir==FALSE)&&((MOTO_CUT_CMD_TypeDef)COM_Tangkong_WorkCmd==MOTO_CUT_D))||
			((COM_GetIsDrop(COM_UpStNow[id]) != FALSE)&&(COM_9511TangkongEnable[id]==FALSE)&&((MOTO_CUT_CMD_TypeDef)COM_Tangkong_WorkCmd==MOTO_CUT_D)))&&
			(COM_IsFault[id]==FALSE)&&
			(IN_HDKey_GetSt((BSP_ID_TypeDef)id)!=HDKEY_ST_DN)){          /// 落针, 参与刺绣, 按命令运动
/// !!! cy 100905
            cmd =   (MOTO_CUT_CMD_TypeDef)COM_Tangkong_WorkCmd;
        } else {                                                /// 不参与刺绣, 提升
            cmd =   (MOTO_HGH_CMD_TypeDef)MOTO_CUT_NONE;

        }
	/*	if((COM_Tangkong_WorkCmd==MOTO_CUT_D)||(COM_Tangkong_WorkCmd==MOTO_CUT_FULL))
			HeadUpDown_Status[id]=0;
		else if(COM_Tangkong_WorkCmd==MOTO_CUT_H)
			HeadUpDown_Status[id]=0;
		else if(COM_Tangkong_WorkCmd==MOTO_CUT_HALF)
			HeadUpDown_Status[id]=0; */ 
        MOTO_CutSetCmd(id, cmd);
        COM_IsHghRest = FALSE;                                  /// 上电后有针高操作则针高以后可以动作

/// cy  110304
//    } else {                                                    /// 否则, 不动作
//        ;
//    }
        return  TRUE;
   /* } else {                                                    /// 否则, 不动作
        return  FALSE;
    }*/
/// cy  110304
}
static  bool  COM_HghUpdate_key (BSP_ID_TypeDef id)
/// cy  110304
{
    MOTO_HGH_CMD_TypeDef    cmd;
		MOT_CUT_Get_SwiDirIsStatus();
        if (((((COM_GetIsDrop(COM_UpStNow[id]) != FALSE)||(COM_GetIsDrop(COM_UpStLast[id]) != FALSE))&&(COM_TKIsEnable[id]==TRUE)
		&&(COM_SwiDirIsEnable[id]==TRUE))||(COM_9511IsTset==TRUE))
		&&(COM_IsFault[id]==FALSE))
		 {          /// 落针, 参与刺绣, 按命令运动
/// !!! cy 100905
            cmd =   (MOTO_CUT_CMD_TypeDef)COM_Tangkong_WorkCmd;
        } else {                                                /// 不参与刺绣, 提升
            cmd =   (MOTO_HGH_CMD_TypeDef)MOTO_CUT_NONE;

        }
	/*	if((COM_Tangkong_WorkCmd==MOTO_CUT_D)||(COM_Tangkong_WorkCmd==MOTO_CUT_FULL))
			HeadUpDown_Status[id]=0;
		else if(COM_Tangkong_WorkCmd==MOTO_CUT_H)
			HeadUpDown_Status[id]=0;
		else if(COM_Tangkong_WorkCmd==MOTO_CUT_HALF)
			HeadUpDown_Status[id]=0; */
        MOTO_CutSetCmd(id, cmd);
        COM_IsHghRest = FALSE;                                  /// 上电后有针高操作则针高以后可以动作

/// cy  110304
//    } else {                                                    /// 否则, 不动作
//        ;
//    }
        return  TRUE;
   /* } else {                                                    /// 否则, 不动作
        return  FALSE;
    }*/
/// cy  110304
}
static  void  COM_SqnUpdate (BSP_ID_TypeDef id)
{
    BSP_SQN_EmbStSend(id, COM_GetSqnEmbSt(id));
}



static  u16  COM_9511EmbStToBit (BSP_EMB_ST_TypeDef st)
{
    switch (st) {
    case BSP_EMB_ST_PATCH: //2
        return  1 << 3;
    case BSP_EMB_ST_NORMAL:	//1
        return  1 << 2;
    case BSP_EMB_ST_OFF: //0
    default:
        return  1 << 1;
    }
}


/// -----------------------------------------------------------------------------------------------------
/// @brief 处理上机头板收到针高位置的命令函数。
///        处理结果  根据机头状态调电机动作函数
/// @param high  针高位置值
/// @return None
/// -----------------------------------------------------------------------------------------------------
////////
static  void  COM_HighCmd (u8 high)//针高函数
{
    u32     i;
          COM_Tangkong_WorkHigh=high;
		 if((high>0)&&(high<10))
		 {
		  MOTO_TKSetHalfAngle(Tangkong_High[COM_Tangkong_WorkHigh]);
		  MOTO_TKSetFullAngle(Tangkong_High[COM_Tangkong_WorkHigh]);
		   if(COM_9511TangkongDir==FALSE)
		   	//	COM_ID_9511CutHalf(id,0,0);
				COM_Tangkong_WorkCmd=MOTO_CUT_HALF;
		   else
		      //  COM_ID_9511CutFull(id,0,0);
			  COM_Tangkong_WorkCmd=MOTO_CUT_FULL;
		}
		else if(high==0)
			COM_Tangkong_WorkCmd=MOTO_CUT_D;
		else if(high==10)
			COM_Tangkong_WorkCmd=MOTO_CUT_H;
		else if(high==11)
			COM_Tangkong_WorkCmd=MOTO_CUT_O;
		else
			COM_Tangkong_WorkCmd=MOTO_CUT_NONE;
    // COM_WorkHigh        =   high;
    /// cy 101217 added
    /// 因为有发针高前没有发主轴到位的情况
    /// "跳跃剪线"参数设为否时, 越框前, 主控发不到位, 越框程序升起针高没有被执行, 针在下位就越框了
    COM_9511MAIsInOrg   =   TRUE;
    /// cy 101217 added
    for (i = 0; i < BSP_DEVICE_ID_N; i++) {
		COM_TKIsEnable[i]=TRUE;
		if(high==0)
			COM_9511TangkongEnable[i]=FALSE;
        COM_HghUpdate((BSP_ID_TypeDef)i);
    }
    
//    /// cy 100906
//    if (COM_9511IsTset != FALSE) {
//        COM_9511TestExit(buf, len);
//    }
//    /// cy 100906
}

/// -----------------------------------------------------------------------------------------------------
/// @brief     处理数据组中数据的函数。
/// 处理结果   找出对应字节中的对应位。
///            对数据求和保存于 check_data_add ，字节数存为：check_data_num。
/// @return    segment_select：对应位的值。
/// -----------------------------------------------------------------------------------------------------
////////
static  u16  COM_9511Decode (u16 buf[], u8 len)
{
    const   u8      nbit    =   2;
            u8      q;
            u8      r;
    
	if((BSP_DIP_9511Addr>0)&&(BSP_DIP_9511Addr<0x10))
	{    
    	q   =   (BSP_DIP_9511Addr - 1) / 3;
    	r   =   (BSP_DIP_9511Addr - 1) % 3;
	}
	if((BSP_DIP_9511Addr>0x70)&&(BSP_DIP_9511Addr<0x80))
	{    
    	q   =   (BSP_DIP_9511Addr-0x70 - 1) / 3;
    	r   =   (BSP_DIP_9511Addr-0x70 - 1) % 3;
	}
     
    if (q < len) {
        ;
    } else {
        q   =   0;
        r   =   0;
    }
    
    return  (buf[q] >> (nbit * (3 - 1 - r))) & ((1 << nbit) - 1);
/*	}
	else
		return 0;  */
	 
}
static  void  COM_CmdDoNothing (u16 buf[], u8 len)
{
    ;
}

/// -----------------------------------------------------------------------------------------------------
/// @brief 处理上机头板收到针高位置10的命令函数。
///        参与：上机头板
/// 处理结果  根据机头状态调电机动作函数
/// @return 无。
/// -----------------------------------------------------------------------------------------------------
////////
extern u16                 MOTO_CutHalfSteps;

static void COM_Tangkong_HighTo10(u16 buf[], u8 len)
{	  
			 
		     COM_HighCmd(10);

}
/*static void COM_ID_Tangkong_HighToD(BSP_ID_TypeDef id,u16 buf[], u8 len)
{
	  
		  COM_ID_9511CutD(id,0,0);

}*/	
void  COM_Tangkong_HighTo0 (u16 buf[], u8 len)
{		 
//    COM_9511TangkongEnable             =   FALSE;
    COM_HighCmd(0);
}
static  void  COM_Tangkong_HighTo1 (u16 buf[], u8 len)
{
//	HeadUpDown_Status=0;
    COM_HighCmd(1);
}
static  void  COM_Tangkong_HighTo2 (u16 buf[], u8 len)
{
//	HeadUpDown_Status=0;
    COM_HighCmd(2);
}
static  void  COM_Tangkong_HighTo3 (u16 buf[], u8 len)
{
//	HeadUpDown_Status=0;
    COM_HighCmd(3);
}
static  void  COM_Tangkong_HighTo4 (u16 buf[], u8 len)
{
//	HeadUpDown_Status=0;
    COM_HighCmd(4);
}
static  void  COM_Tangkong_HighTo5 (u16 buf[], u8 len)
{
//	HeadUpDown_Status=0;
    COM_HighCmd(5);
}
static  void COM_Tangkong_HighTo6 (u16 buf[], u8 len)
{
//	HeadUpDown_Status=0;
    COM_HighCmd(6);
}
static  void  COM_Tangkong_HighTo7 (u16 buf[], u8 len)
{
//	HeadUpDown_Status=0;
    COM_HighCmd(7);
}
static  void  COM_Tangkong_HighTo8 (u16 buf[], u8 len)
{
//	HeadUpDown_Status=0;
    COM_HighCmd(8);
}
static  void  COM_Tangkong_HighTo9 (u16 buf[], u8 len)
{
//	HeadUpDown_Status=0;
    COM_HighCmd(9);
} 
/*static  void  COM_Tangkong_HighTo10 (u16 buf[], u8 len)
{
	COM_ID_9511CutH(id,0,0);
}*/
static  void  COM_HeadSpeed (u16 buf[], u8 len)
{
	u16 buf_tmp,len_tmp=0;
	buf_tmp=buf[0];
	len_tmp=(buf_tmp-0x80);
	if((len_tmp<5)&&(len_tmp>=0))
		UpHeadSpeed=Speed_pare[len_tmp];
	else
		UpHeadSpeed=100;
}
static  void  COM_HeadUpDown(u16 buf[], u8 len)
{
   
 //  HeadUpDown=1;
// 	HeadUpDown_Status=1;
//   COM_HighCmd(COM_WorkHigh);
}
static  u16  COM_Get9511TKStatus (void)
{
    MOTO_ST_TypeDef     MotoSt[BSP_DEVICE_ID_N];
    bool                broken[BSP_DEVICE_ID_N];
    u32                 i;
    u16                 rt;
    
    
    for (i = 0; i < BSP_DEVICE_ID_N; i++) {
        if ((COM_IsFault[i] == FALSE) && (COM_DnIsEnable[i] == TRUE)) {
            MotoSt[i]   =   MOTO_CutGetState((BSP_ID_TypeDef)i);
        } else {
            MotoSt[i]   =   MOTO_ST_OK;
        }
    }
    
    if ((MotoSt[BSP_ID_ODD] == MOTO_ST_ACTING) || (MotoSt[BSP_ID_EVEN] == MOTO_ST_ACTING)) {
        rt  =   0x22;//COM_8_RES_BUSY;                 /// ?
    } else if ((MotoSt[BSP_ID_ODD]  == MOTO_ST_PRO) || (MotoSt[BSP_ID_ODD]  == MOTO_ST_OVTIME)) {
        rt  =  2;// COM_8_RES_ODD_OVERTIM;          /// ???
    } else if ((MotoSt[BSP_ID_EVEN] == MOTO_ST_PRO) || (MotoSt[BSP_ID_EVEN] == MOTO_ST_OVTIME)) {
        rt  =  0x20;// COM_8_RES_EVE_OVERTIM;          /// ???
    } /*else if  (MotoSt[BSP_ID_ODD]  == MOTO_ST_CUTLOOPER) {
        rt  =   COM_8_RES_ODD_KEYERR;           /// ???
    } else if  (MotoSt[BSP_ID_EVEN] == MOTO_ST_CUTLOOPER) {
        rt  =   COM_8_RES_EVE_KEYERR;           /// ???
    } else if ((broken[BSP_ID_ODD] == TRUE) && (broken[BSP_ID_EVEN] == TRUE) && (COM_9511MAIsRunning != FALSE)) {
        rt  =   COM_8_RES_ALL_BREAK;            /// ???
    } else if ((broken[BSP_ID_ODD] == TRUE) && (COM_9511MAIsRunning != FALSE)) {
        rt  =   COM_8_RES_ODD_BREAK;            /// ???
    } else if ((broken[BSP_ID_EVEN] == TRUE) && (COM_9511MAIsRunning != FALSE)) {
        rt  =   COM_8_RES_EVE_BREAK;            /// ???
    } else if  (COM_9511CheckErr == TRUE) {
        rt  =   COM_8_RES_DAT_ERR;              /// ??????
    } else {
        rt  =   BSP_DIP_9511Addr;               /// ??
    }
     */
    return  rt;
}
u16 COM_Tangkong_Pos_Last=0;
static  void  COM_Tangkong_Position(u16 buf[], u8 len)
{
     u16     rt=0; 
//最低1，0位是4，脫離位2，運動過程中報上一狀態
//起來自己找原點，初始化接近開關不在原點報3	 

  MOTO_ST_TypeDef     MotoSt[BSP_DEVICE_ID_N];
  u32                 i;
   for (i = 0; i < BSP_DEVICE_ID_N; i++) {
   if((IN_HDKey_GetSt((BSP_ID_TypeDef)i)==HDKEY_ST_DN)||(COM_9511TangkongPatch[i]==2))
   {
   		if(i==0)
			rt=5;
		else
			{
				rt &=0x0F;
				rt|=(5<<4);
			}
   }
   else
   {
   if(TK_Org[i]==0)//没找过
   {
   		if(i==0)
			rt=3;
		else
		{
			rt &=0x0F;
			rt|=(3<<4);
		}
   }
   else//找过了
   {
        if ((COM_IsFault[i] == FALSE) && (COM_DnIsEnable[i] == TRUE)) 
		{
            MotoSt[i]   =   MOTO_CutGetState((BSP_ID_TypeDef)i);
        } 
		else 
		{
            MotoSt[i]   =   MOTO_ST_OK;
        }
    //成功了
		if(MotoSt[i]  ==   MOTO_ST_OK)
		{
			if(i==0)
			{
			 	if(BSP_SWI_FiGet(BSP_ID_ODD)  != BSP_SWITCH_LOW)//不亮
				{
					rt=0x01;//
				}
				else
				{
					if(HeadUpDown_Status[0]==1)
						rt=0x4;	//工作位
					if(HeadUpDown_Status[0]==2)
						rt=0x02;//在脱离位
		
				}
			}
			if(i==1)
			{
				if(BSP_SWI_FiGet(BSP_ID_EVEN) != BSP_SWITCH_LOW)
				{
						rt &=0x0F;
				    	rt=rt|(1<<4); //停下位 
				}
				
				else
				{
					if(HeadUpDown_Status[1]==1)
					{
						rt &=0x0F;
						rt=rt|(1<<6);	//工作位
					}
					if(HeadUpDown_Status[1]==2)
					{
						rt &=0x0F;
						rt=rt|(1<<5);//在脱离位
					}
				}
			}
		 COM_Tangkong_Pos_Last=rt;
		}
		else//没有成功
		{
			 if(TK_Org[i]==1) //已经找过了
			   {
			   		if(i==0)
						rt=3;
					else
					{
						rt &=0x0F;
						rt|=(3<<4);
					}
			   }
			else
				rt=COM_Tangkong_Pos_Last;
		//	TK_Org[i]=1;
		}
	}//结束找过了
	}
  }//结束for
    C491_SendOne(rt, 0);
}
static  void  COM_Tangkong_Sta(u16 buf[], u8 len)
{
	u16 rt;
 	rt  =   BSP_DIP_9511Addr;               /// 正确
    C491_SendOne(rt, 0);  

}
static  void    COM_Tangkong_SinglDir(u16 buf[], u8 len)
{
	COM_9511TangkongDir=FALSE;
}
static  void    COM_Tangkong_DoublDir(u16 buf[], u8 len)
{
	COM_9511TangkongDir=TRUE;
//	COM_HighCmd(0);
}
/// -----------------------------------------------------------------------------------------------------
/// @brief 处理机头板关闭机头数据命令函数。
///        参与：上、下机头板
///        处理结果  全局变量 COM_HD_valid_even，COM_HD_valid_odd
/// @param u8* p_buffer, u8 len
/// @return 无。
/// -----------------------------------------------------------------------------------------------------
////////
static  void  COM_9511FaultOff (u16 buf[], u8 len) //主控0xd0命令，COM_IsFault==TRUE表示主控关闭了这个机头
{
    u16     decode;
    u32     i;
    
    
    decode  =   COM_9511Decode(buf, len);
    
//    if (decode & (1 << 1)) {
    if (COM_BIT_ODD(decode)) {
        COM_IsFault[BSP_ID_ODD]     =   FALSE;
    } else {
        COM_IsFault[BSP_ID_ODD]     =   TRUE;
    }
    
//    if (decode & (1 << 0)) {
    if (COM_BIT_EVE(decode)) {
        COM_IsFault[BSP_ID_EVEN]    =   FALSE;
    } else {
        COM_IsFault[BSP_ID_EVEN]    =   TRUE;
    }
    
//    COM_AllUpdate();
    for (i = 0; i < BSP_DEVICE_ID_N; i++) {
        COM_LedUpdate((BSP_ID_TypeDef)i);
        
//        BSP_SQN_EmbStSend((BSP_ID_TypeDef)i, COM_GetEmbSt((BSP_ID_TypeDef)i));
        COM_SqnUpdate((BSP_ID_TypeDef)i);
        
        /// 可加等待0.3S        /// !!! cy  100908
        
//        COM_HghUpdate((BSP_ID_TypeDef)i);
        if (COM_IsHghRest == FALSE) {           /// 为安全上电后针高不动, 直到有主控命令或开关变化
            
/// cy  110304
//            COM_HghUpdate((BSP_ID_TypeDef)i);
            if (COM_HghIsAim[i] == FALSE) {
                COM_HghIsAim[i] =   COM_HghUpdate((BSP_ID_TypeDef)i);
            }
/// cy  110304
//		COM_TKFault[i]=FALSE;
        }
    }
}
static  void  COM_9511DataCheck (u16 buf[], u8 len)
{
    /// 之前的命令不含有数据, 则认为此校验命令无效
    if (   (COM_9511CheckNum == 0)                                          
        || (   ((buf[0] & COM_MSK_9511_DATA) == COM_9511CheckNum)           /// 或者校验和与个数都正确
            && ((buf[1] & COM_MSK_9511_DATA) == COM_9511CheckSum)   )   ) {              
        COM_9511CheckErr    =   FALSE;
    } else {
        COM_9511CheckErr    =   TRUE;
    }
}

static  void  COM_9511DnToUpBrk (u16 buf[], u8 len)
{
    u16     decode;
    
    
    decode  =   COM_9511Decode(buf, len);
    
    if (COM_BIT_ODD(decode)) {                  /// 奇数断线
        COM_9511UpStByBroken(BSP_ID_ODD);
    } else {                                    /// 不断线不处理
        ;
    }
    
    if (COM_BIT_EVE(decode)) {                  /// 奇数断线
        COM_9511UpStByBroken(BSP_ID_EVEN);
    } else {                                    /// 不断线不处理
        ;
    }
}

/// Byte0   为断线检测板类型:   0 - 上9511, 1 - 下9511, 2 - ef192, 3 - ef196; 
/// Byte1   为断线检测类型：    0 - 无, 1 - 开关类型(挑线弹簧 或 接近开关), 2 - 脉冲类型(斩光轮)
/// Byte2~4 保留
extern u8 IN_TEN_MULTIPLE;
static  void  COM_9511BrkDetMod (u16 buf[], u8 len)//0xd3命令的执行函数，
{//在主控的init3中有发这个命令
    bool    IsDn;
//    u32     i;        /// !!! cy 100908
     u8 q=2;
    
    if ((buf[0] & COM_MSK_9511_DATA) == 0) {
        IsDn    =   FALSE; //上机头断检是0
    } else {
        IsDn    =   TRUE;//下机头断检是1
    }
    
/// !!! cy 100908
//    for (i = 0; i < BSP_DEVICE_ID_N; i++) {
//        IN_BRK_SetIsDn((BSP_ID_TypeDef)i, IsDn);
//    }
    IN_BRK_SetIsDn(IsDn);
/// !!! cy 100908

	if(q<len)
		IN_TEN_MULTIPLE=buf[q]&0x7F;
	if((IN_TEN_MULTIPLE<1)||(IN_TEN_MULTIPLE>99))
		IN_TEN_MULTIPLE=20;
}

static  void  COM_9511GetIsSqnUp (u16 buf[], u8 len)
{
    u16     res;
    
    
    if (BSP_SWI_SeGet(BSP_ID_ODD) == BSP_SWITCH_LOW) {
        res =   COM_8_RES_SQN_POS;
    } else {
        res =   COM_8_RES_SQN_NOTPOS;
    }
    C491_SendOne(res, 0);
}

/// -----------------------------------------------------------------------------------------------------
/// @brief 处理下机头板剪刀使能数据命令函数。
///        参与：下机头板
///        处理结果  全局变量 COM_cut_eab_odd，COM_cut_eab_even
/// @param u8* p_buffer, u8 len
/// @return 无。
/// -----------------------------------------------------------------------------------------------------
////////
static  void  COM_9511DnEnable (u16 buf[], u8 len)
{
    u16     decode;
    
    
    decode              =   COM_9511Decode(buf, len);
    
    if (COM_BIT_ODD(decode)) {
        COM_DnIsEnable[BSP_ID_ODD]  =   TRUE;
    } else {
        COM_DnIsEnable[BSP_ID_ODD]  =   FALSE;
    }
    
    if (COM_BIT_EVE(decode)) {
        COM_DnIsEnable[BSP_ID_EVEN] =   TRUE;
    } else {
        COM_DnIsEnable[BSP_ID_EVEN] =   FALSE;
    }
}

static  void  COM_9511CutBack (u16 buf[], u8 len)//剪刀收回
{
    u32     i;
    
    
    for (i = 0; i < BSP_DEVICE_ID_N; i++) {
        if ((COM_IsFault[i] == FALSE)&&(COM_GetIsDrop(COM_UpStNow[i]) != FALSE)) {
            MOTO_CutSetCmd((BSP_ID_TypeDef)i, MOTO_CUT_BACK);
        }
    }
}

static  void  COM_9511CutHalf (u16 buf[], u8 len)//剪刀半出
{
    u32     i;
    
    
    for (i = 0; i < BSP_DEVICE_ID_N; i++) {
        if ((COM_IsFault[i] == FALSE) && (COM_DnIsEnable[i] == TRUE)&&(COM_GetIsDrop(COM_UpStNow[i]) != FALSE)) {
            MOTO_CutSetCmd((BSP_ID_TypeDef)i, MOTO_CUT_HALF);
        }
    }
}

static  void  COM_9511CutFull (u16 buf[], u8 len)//剪线满入
{
    u32     i;
    
    
    for (i = 0; i < BSP_DEVICE_ID_N; i++) {
        if ((COM_IsFault[i] == FALSE) && (COM_DnIsEnable[i] == TRUE)&&(COM_GetIsDrop(COM_UpStNow[i]) != FALSE)) {
            MOTO_CutSetCmd((BSP_ID_TypeDef)i, MOTO_CUT_FULL);
        }
    }
}
static  void  COM_ID_9511CutBack (BSP_ID_TypeDef id,u16 buf[], u8 len)//剪刀收回
{
    //u32     i;
    
    
   // for (i = 0; i < BSP_DEVICE_ID_N; i++) {
        if ((COM_IsFault[id] == FALSE)&&(COM_GetIsDrop(COM_UpStNow[id]) != FALSE)) {
            MOTO_CutSetCmd(id, MOTO_CUT_BACK);
       // }
    }
}
static  void  COM_ID_9511CutH (BSP_ID_TypeDef id,u16 buf[], u8 len)//剪刀收回
{
    //u32     i;
    
    
   // for (i = 0; i < BSP_DEVICE_ID_N; i++) {
        if ((COM_IsFault[id] == FALSE)&&(COM_GetIsDrop(COM_UpStNow[id]) != FALSE)) {
            MOTO_CutSetCmd(id, MOTO_CUT_H);
       // }
    }
}static  void  COM_ID_9511CutD (BSP_ID_TypeDef id,u16 buf[], u8 len)//剪刀收回
{
    //u32     i;
    
    
   // for (i = 0; i < BSP_DEVICE_ID_N; i++) {
        if ((COM_IsFault[id] == FALSE)&&(COM_GetIsDrop(COM_UpStNow[id]) != FALSE)) {
            MOTO_CutSetCmd(id, MOTO_CUT_D);
       // }
    }
}
static  void  COM_ID_9511CutHalf (BSP_ID_TypeDef id,u16 buf[], u8 len)//剪刀半出
{
   // u32     i;
    
    
   // for (i = 0; i < BSP_DEVICE_ID_N; i++) {
        if ((COM_IsFault[id] == FALSE) && (COM_DnIsEnable[id] == TRUE)&&(COM_GetIsDrop(COM_UpStNow[id]) != FALSE)) {
            MOTO_CutSetCmd(id, MOTO_CUT_HALF);
     //   }
    }
}

static  void  COM_ID_9511CutFull (BSP_ID_TypeDef id,u16 buf[], u8 len)//剪线满入
{
   // u32     i;
    
    
    //for (i = 0; i < BSP_DEVICE_ID_N; i++) {
        if ((COM_IsFault[id] == FALSE) && (COM_DnIsEnable[id] == TRUE)&&(COM_GetIsDrop(COM_UpStNow[id]) != FALSE)) {
            MOTO_CutSetCmd(id, MOTO_CUT_FULL);
        //}
    }
}
static  void  COM_9511CutSetPara (u16 buf[], u8 len)
{
    u32     i;
    
    
    for (i = 0; i < BSP_DEVICE_ID_N; i++) {
        /// 以下3个参数, 不在此判断范围
        MOTO_CutSetFullAngle((u8)(buf[0] & COM_MSK_9511_DATA));
        MOTO_CutSetHalfAngle((u8)(buf[1] & COM_MSK_9511_DATA));
        MOTO_SetSpdEngine(MOTO_USE_CUT, buf[2] & COM_MSK_9511_DATA);
    }
}

static  void  COM_9511GetVersion (u16 buf[], u8 len)
{
    C491_SendOne(VERSION9511, 0);
}

static  void  COM_9511SetRedOdd (u16 buf[], u8 len)
{
    HDLED_Set(BSP_ID_ODD, HDLED_ST_RED);
}

static  void  COM_9511SetGreOdd (u16 buf[], u8 len)
{
    HDLED_Set(BSP_ID_ODD, HDLED_ST_GRE);
}

static  void  COM_9511SetOraOdd (u16 buf[], u8 len)
{
    HDLED_Set(BSP_ID_ODD, HDLED_ST_ORA);
}

static  void  COM_9511SetRedEven (u16 buf[], u8 len)
{
    HDLED_Set(BSP_ID_EVEN, HDLED_ST_RED);
}

static  void  COM_9511SetGreEven (u16 buf[], u8 len)
{
    HDLED_Set(BSP_ID_EVEN, HDLED_ST_GRE);
}

static  void  COM_9511SetOraEven (u16 buf[], u8 len)
{
    HDLED_Set(BSP_ID_EVEN, HDLED_ST_ORA);
}
static  void  COM_9511TestEnter (u16 buf[], u8 len)
{
	if((IN_HDKey_GetSt(0))!=HDKEY_ST_DN)
    	COM_9511SetRedOdd(0,0);
	if((IN_HDKey_GetSt(1))!=HDKEY_ST_DN)
		COM_9511SetRedEven(0,0);
	COM_9511IsTset  =   TRUE;
	COM_Tangkong_WorkHigh_Last=COM_Tangkong_WorkHigh;
    if(COM_Tangkong_WorkHigh==0)
		  COM_Tangkong_WorkHigh=6;
			
}

static  void  COM_9511TestExit (u16 buf[], u8 len)
{
    u32     i;
    COM_9511IsTset  =   FALSE;
    if((IN_HDKey_GetSt(0))!=HDKEY_ST_DN)
    	COM_9511SetGreOdd(0,0);
	if((IN_HDKey_GetSt(1))!=HDKEY_ST_DN)
		COM_9511SetGreEven(0,0);
    
    COM_Tangkong_WorkHigh=COM_Tangkong_WorkHigh_Last;
//    COM_AllUpdate();
//    for (i = 0; i < BSP_DEVICE_ID_N; i++) {
//        COM_LedUpdate((BSP_ID_TypeDef)i);
/// !!! cy 100908   测试状态可改变针高, 则退出不用刷新
////        COM_HghUpdate((BSP_ID_TypeDef)i);
//        if (COM_IsHghRest == FALSE) {           /// 为安全上电后针高不动, 直到有主控命令或开关变化
//            COM_HghUpdate((BSP_ID_TypeDef)i);
//        }
/// !!! cy 100908   测试状态可改变针高, 则退出不用刷新
//    }
}
static  void  COM_9511MAOrgOut (u16 buf[], u8 len)
{
    COM_9511MAIsInOrg   =   FALSE;
}

#define COM_TIMES_TEST_AD   8
static  void  COM_9511GetHghOdd (u16 buf[], u8 len)
{
    u16     rt;
    
    
    if (MOTO_HghGetIsActing(BSP_ID_ODD) != FALSE) {
        rt  =   COM_8_RES_BUSY;
    } else {
/// cy  101130  modify
//        rt  =   MOTO_HghGetHeight(BSP_ID_ODD);
        
        if (COM_9511MAIsRunning) {
            rt  =   COM_WorkHigh;
        } else {
            rt  =   MOTO_HghGetHeight(BSP_ID_ODD);
/// cy  110228
//            if (rt != MOTO_HGH_POS_UP) {
            if ((rt != MOTO_HGH_POS_UP) && (COM_IsHghRest == FALSE)) {
/// cy  110228
                if (COM_WorkHigh != MOTO_HGH_POS_UP) {
                    rt  =   COM_WorkHigh;
                } else {
                    if (MOTO_HghTestIsUp(BSP_ID_ODD, COM_TIMES_TEST_AD) != FALSE) {
                        rt  =   MOTO_HGH_POS_UP;
                    } else {
                        ;
                    }
                }
            } else {
                ;
            }
        }
/// cy  101130  modify
    }
    C491_SendOne(rt, 0);
}

static  void  COM_9511GetHghEven (u16 buf[], u8 len)
{
    u16     rt;
    
    
    if (MOTO_HghGetIsActing(BSP_ID_EVEN) != FALSE) {
        rt  =   COM_8_RES_BUSY;
    } else {
/// cy  101130  modify
//        rt  =   MOTO_HghGetHeight(BSP_ID_EVEN);
        
        if (COM_9511MAIsRunning) {
            rt  =   COM_WorkHigh;
        } else {
            rt  =   MOTO_HghGetHeight(BSP_ID_EVEN);
/// cy  110228
//            if (rt != MOTO_HGH_POS_UP) {
            if ((rt != MOTO_HGH_POS_UP) && (COM_IsHghRest == FALSE)) {
/// cy  110228
                if (COM_WorkHigh != MOTO_HGH_POS_UP) {
                    rt  =   COM_WorkHigh;
                } else {
                    if (MOTO_HghTestIsUp(BSP_ID_EVEN, COM_TIMES_TEST_AD) != FALSE) {
                        rt  =   MOTO_HGH_POS_UP;
                    } else {
                        ;
                    }
                }
            } else {
                ;
            }
        }
/// cy  101130  modify
    }
    C491_SendOne(rt, 0);
}

static  void  COM_9511GetADOdd (u16 buf[], u8 len)
{
    u16     rt;
    
    
    if (MOTO_HghGetIsActing(BSP_ID_ODD) != FALSE) {
        rt  =   COM_8_RES_BUSY;
    } else {
        rt  =   BSP_POT_GetAve(BSP_ID_ODD) >> 8;               /// 回高8位
    }
    C491_SendOne(rt, 0);
}

static  void  COM_9511GetADEven (u16 buf[], u8 len)
{
    u16     rt;
    
    
    if (MOTO_HghGetIsActing(BSP_ID_EVEN) != FALSE) {
        rt  =   COM_8_RES_BUSY;
    } else {
        rt  =   BSP_POT_GetAve(BSP_ID_EVEN) >> 8;               /// 回高8位
    }
    C491_SendOne(rt, 0);
}

/// 处理主轴信息, 不处理断线检测信息
static  void  COM_9511MAOneLoop (u16 buf[], u8 len)
{
    u32     i;
    
    
    COM_9511MAIsInOrg   =   FALSE;
    COM_9511MAIsRunning =   TRUE;               /// cy  101115  added
    
    for (i = 0; i < BSP_DEVICE_ID_N; i++) {
        if (COM_IsFault[i] == FALSE) {

            HDLED_Sparkle((BSP_ID_TypeDef)i, 3);

        }
    }
}

/// ! 可能不用了
static  void  COM_9511SetGreALL (u16 buf[], u8 len)//全变绿灯，主控发指令
{

    u32     i;
    
    
    for (i = 0; i < BSP_DEVICE_ID_N; i++) {
//        if ((COM_IsFault[i] == FALSE) && (COM_UpStBits[COM_UpStNow[i]] & (1 << COM_UP_BIT_IS_DROP))) {
        if ((COM_IsFault[i] == FALSE) && (COM_GetIsBroken(COM_UpStNow[i]) != FALSE)&&(COM_UpStLast[i]!=COM_UP_ST_BROKEN)) {
            HDLED_Set((BSP_ID_TypeDef)i, HDLED_ST_GRE);
        }
    }
}

static  void  COM_9511PatchExit (u16 buf[], u8 len)
{
    COM_9511UpStByExitP();
	COM_9511SetGreALL(0,0);
}

static  void  COM_9511PatchEnter (u16 buf[], u8 len) //进入补绣
{
    COM_9511UpStByEnterP();	//这个函数只是改变了上机头的状态，没有改变机头电机的高度，为什么？
}

/// ?查询状态 和 查询灯状态 皆可? PatchIdle 例外
/// ?红-断线，绿-参与刺绣
static  void  COM_9511GetHDLed (u16 buf[], u8 len)
{


    u16     rt;
    
    
    rt  =   COM_9511EmbStToBit(COM_GetEmbSt(BSP_ID_ODD))  << 4;
    rt |=   COM_9511EmbStToBit(COM_GetEmbSt(BSP_ID_EVEN)) << 0;
    
    C491_SendOne(rt, 0);
}



static  void  COM_9511MAOrgIn (u16 buf[], u8 len)
{
    u32     i;
    
    
    COM_9511MAIsInOrg   =   TRUE;
    
//    COM_AllUpdate();
    for (i = 0; i < BSP_DEVICE_ID_N; i++) {
//        COM_HghUpdate((BSP_ID_TypeDef)i);
         if (COM_IsHghRest == FALSE) {           /// 为安全上电后针高不动, 直到有主控命令或开关变化
            
/// cy  110304
//            COM_HghUpdate((BSP_ID_TypeDef)i);
            if (COM_HghIsAim[i] == FALSE) {
                COM_HghIsAim[i] =   COM_HghUpdate((BSP_ID_TypeDef)i);
            }
/// cy  110304

        }
    }
}



/// 处理主轴信息, 也处理断线检测信息
/// 因为处理断线检测信息, 所以对下机头有效
static  void  COM_9511MAStartUp (u16 buf[], u8 len)
{
    u32     i;
    
    COM_9511MAIsInOrg   =   FALSE;
    COM_9511MAIsRunning =   TRUE; 
    if (BSP_DIP_BDFunc == BSP_DIP_BD_FUNC_UP) {//上机头
       // COM_9511MAIsInOrg   =   FALSE;
       // COM_9511MAIsRunning =   TRUE;               /// cy  101115  added
        
        COM_9511UpStByMARun();
        
        for (i = 0; i < BSP_DEVICE_ID_N; i++) {
            if (COM_IsFault[i] == FALSE) {

                HDLED_Sparkle((BSP_ID_TypeDef)i, 3);

            }
        }
    }
    
    /// 断线检测信息处理
    IN_BRK_SampleBegin();
}

/// 不处理主轴信息, 处理断线检测信息
static  void  COM_9511MAStop (u16 buf[], u8 len)//0xfd主轴停车的命令
{
    COM_9511MAIsRunning =   FALSE;          /// cy  101115  added
    
    /// 断线检测信息处理
    IN_BRK_SampleStop();
}

static  void  COM_XJ_3 (void)
{
  /*  static  u32  i = 0;
    
    
    i++;
    if (i == 3) {
        i = 0; */
        IN_BRK_SentXJ();
   // }
}
//线夹命令0xFF。因为只有主控发了线夹信号才会改变IN_BrkIsBroken[i]的值，主控程序运行中没有看到这个发线夹命令？   
static  void  COM_9511BrkDetect (u16 buf[], u8 len)//在emb函数中有SendDJtoHead的宏定义，所以是有发断检信号的。在断线测试中是自己连发了三次0xFF
{

    u32                 i;
    COM_9511MAIsInOrg   =   FALSE;
    COM_9511MAIsRunning =   TRUE; 
    
    if (BSP_DIP_BDFunc == BSP_DIP_BD_FUNC_UP) {//上机头
       // COM_9511MAIsInOrg   =   FALSE;
       // COM_9511MAIsRunning =   TRUE;               /// cy  101115  added
        
/// cy  110226  modify
//        COM_9511UpStByMARun();                      /// 清断线状态
/// cy  110226  modify

/// cy  110227  modify        
//        IN_BRK_SentXJ();
        COM_XJ_3();
/// cy  110227  modify
    
        for (i = 0; i < BSP_DEVICE_ID_N; i++) {
/// !!! cy 100905
//            if (   (COM_IsFault[i] == FALSE)        /// ! 可有可无
//                && (COM_GetIsDrop(COM_UpStNow[i]) == TRUE)
//                && (IN_BRK_GetIsBrk((BSP_ID_TypeDef)i) != FALSE)   ) {
            if (   (COM_GetIsDrop(COM_UpStNow[i]) == TRUE)
                	&& (IN_BRK_GetIsBrk((BSP_ID_TypeDef)i) != FALSE)   ) { //读取IN_BrkIsBroken[i]上机头检查到了断线
/// !!! cy 100905
                		COM_9511UpStByBroken((BSP_ID_TypeDef)i);//更新上机头灯
            }
        }
    } else {//下机头
/// cy  110227  modify
//        IN_BRK_SentXJ();
        COM_XJ_3();//这个函数里面调用IN_BRK_SendXJ函数，给IN_BrkIsBroken[i]变量赋值
/// cy  110227  modify
    }
    

}


static  u16  COM_Get9511DnStatus (void)
{
    MOTO_ST_TypeDef     MotoSt[BSP_DEVICE_ID_N];
    bool                broken[BSP_DEVICE_ID_N];
    u32                 i;
    u16                 rt;
    
    
    for (i = 0; i < BSP_DEVICE_ID_N; i++) {
        if ((COM_IsFault[i] == FALSE) && (COM_DnIsEnable[i] == TRUE)) {
            MotoSt[i]   =   MOTO_CutGetState((BSP_ID_TypeDef)i);
            broken[i]   =   IN_BRK_GetIsBrk((BSP_ID_TypeDef)i);
        } else {
            MotoSt[i]   =   MOTO_ST_OK;
            broken[i]   =   FALSE;
        }
    }
    
    if ((MotoSt[BSP_ID_ODD] == MOTO_ST_ACTING) || (MotoSt[BSP_ID_EVEN] == MOTO_ST_ACTING)) {
        rt  =   COM_8_RES_BUSY;                 /// ?
    } else if ((MotoSt[BSP_ID_ODD]  == MOTO_ST_PRO) || (MotoSt[BSP_ID_ODD]  == MOTO_ST_OVTIME)) {
        rt  =   COM_8_RES_ODD_OVERTIM;          /// ???
    } else if ((MotoSt[BSP_ID_EVEN] == MOTO_ST_PRO) || (MotoSt[BSP_ID_EVEN] == MOTO_ST_OVTIME)) {
        rt  =   COM_8_RES_EVE_OVERTIM;          /// ???
    } else if  (MotoSt[BSP_ID_ODD]  == MOTO_ST_CUTLOOPER) {
        rt  =   COM_8_RES_ODD_KEYERR;           /// ???
    } else if  (MotoSt[BSP_ID_EVEN] == MOTO_ST_CUTLOOPER) {
        rt  =   COM_8_RES_EVE_KEYERR;           /// ???
    } else if ((broken[BSP_ID_ODD] == TRUE) && (broken[BSP_ID_EVEN] == TRUE) && (COM_9511MAIsRunning != FALSE)) {
        rt  =   COM_8_RES_ALL_BREAK;            /// ???
    } else if ((broken[BSP_ID_ODD] == TRUE) && (COM_9511MAIsRunning != FALSE)) {
        rt  =   COM_8_RES_ODD_BREAK;            /// ???
    } else if ((broken[BSP_ID_EVEN] == TRUE) && (COM_9511MAIsRunning != FALSE)) {
        rt  =   COM_8_RES_EVE_BREAK;            /// ???
    } else if  (COM_9511CheckErr == TRUE) {
        rt  =   COM_8_RES_DAT_ERR;              /// ??????
    } else {
        rt  =   BSP_DIP_9511Addr;               /// ??
    }
    
    return  rt;
}

static  void  COM_9511GetCutOrg (u16 buf[], u8 len)
{
   u16     rt;
    
/// 110905
//    if (   ((COM_IsFault[BSP_ID_ODD]  == FALSE) && (MOTO_CutGetIsActing(BSP_ID_ODD)  != FALSE))
//        || ((COM_IsFault[BSP_ID_EVEN] == FALSE) && (MOTO_CutGetIsActing(BSP_ID_EVEN) != FALSE))   ) {
//        rt  =   COM_8_RES_BUSY;
//    } else if ((COM_IsFault[BSP_ID_ODD]  == FALSE) && (BSP_SWI_FiGet(BSP_ID_ODD)  != BSP_SWITCH_LOW)) {
//        rt  =   COM_8_RES_ODD_NOTPOS;
//    } else if ((COM_IsFault[BSP_ID_EVEN] == FALSE) && (BSP_SWI_FiGet(BSP_ID_EVEN) != BSP_SWITCH_LOW)) {
//        rt  =   COM_8_RES_EVE_NOTPOS;
//    } else {
//        rt  =   BSP_DIP_9511Addr;
//    }
    rt  =   COM_Get9511DnStatus();          /// ????
    if (rt == BSP_DIP_9511Addr) {           /// ?????, ??????
        if ((COM_IsFault[BSP_ID_ODD]  == FALSE) && (BSP_SWI_FiGet(BSP_ID_ODD)  != BSP_SWITCH_LOW)) {
            rt  =   COM_8_RES_ODD_NOTPOS;
        } else if ((COM_IsFault[BSP_ID_EVEN] == FALSE) && (BSP_SWI_FiGet(BSP_ID_EVEN) != BSP_SWITCH_LOW)) {
            rt  =   COM_8_RES_EVE_NOTPOS;
        }
    }
/// 110905
    
    C491_SendOne(rt, 0);
}

static  void  COM_9511GetDnErr (u16 buf[], u8 len)
{

    
    u16     rt;
    
    
    rt  =   COM_Get9511DnStatus();
/// cy  110905
    
    C491_SendOne(rt, 0);

}

static  void  COM_9511GetUpErr (u16 buf[], u8 len)
{
    MOTO_ST_TypeDef     MotoSt[BSP_DEVICE_ID_N];
//    bool                broken[BSP_DEVICE_ID_N];        /// !!! cy 100904
    bool                broken[BSP_DEVICE_ID_N];        /// !!! cy 100905
    bool                KeyErr[BSP_DEVICE_ID_N];
    bool                HghErr[BSP_DEVICE_ID_N];
    u32                 i;
    u16                 rt;
    
    
    for (i = 0; i < BSP_DEVICE_ID_N; i++) {
        if (COM_IsFault[i] == FALSE) {
            MotoSt[i]   =   MOTO_HghGetState((BSP_ID_TypeDef)i);
/// !!! cy 100905
/// !!! cy 100904
//            if (COM_UpStBits[COM_UpStNow[i]] & (1 << COM_UP_BIT_IS_DROP)) {
//                broken[i]   =   IN_BRK_GetIsBrk((BSP_ID_TypeDef)i);
//            } else {
//                broken[i]   =   FALSE;
//            }
/// !!! cy 100904
            broken[i]   =   COM_GetIsBroken(COM_UpStNow[i]);//通过判断上机头的状态来的
/// !!! cy 100905和IN_BRK_GetIsBrk不同，它是根据采样值来的
            if (IN_HDKey_GetSt((BSP_ID_TypeDef)i) == HDKEY_ST_ERR) {
                KeyErr[i]   =   TRUE;
            } else {
                KeyErr[i]   =   FALSE;
            }
            if (MOTO_HghGetHeight((BSP_ID_TypeDef)i) == MOTO_HGH_POS_ERR) {
                HghErr[i]   =   TRUE;
            } else {
                HghErr[i]   =   FALSE;
            }
        } else {
            MotoSt[i]   =   MOTO_ST_OK;
//            broken[i]   =   FALSE;                /// !!! cy 100904
            broken[i]   =   FALSE;                /// !!! cy 100905
            KeyErr[i]   =   FALSE;
            HghErr[i]   =   FALSE;
        }
        
/// !!! cy 100904
//        if (broken[i]) {
//            COM_9511UpStByBroken((BSP_ID_TypeDef)i);
//        }
/// !!! cy 100904
    }
    
    if ((MotoSt[BSP_ID_ODD] == MOTO_ST_ACTING) || (MotoSt[BSP_ID_EVEN] == MOTO_ST_ACTING)) {
        rt  =   COM_8_RES_BUSY;                /// 忙
    } else if ((MotoSt[BSP_ID_ODD]  == MOTO_ST_PRO) || (MotoSt[BSP_ID_ODD]  == MOTO_ST_OVTIME)) {
        rt  =   COM_8_RES_ODD_OVERTIM;         /// 奇超时
    } else if ((MotoSt[BSP_ID_EVEN] == MOTO_ST_PRO) || (MotoSt[BSP_ID_EVEN] == MOTO_ST_OVTIME)) {
        rt  =   COM_8_RES_EVE_OVERTIM;         /// 偶超时
    } else if  (MotoSt[BSP_ID_ODD]  == MOTO_ST_REVERSE) {
        rt  =   COM_8_RES_ODD_REVERSE;         /// 奇反转
    } else if  (MotoSt[BSP_ID_EVEN] == MOTO_ST_REVERSE) {
        rt  =   COM_8_RES_EVE_REVERSE;         /// 偶反转
    } else if  (KeyErr[BSP_ID_ODD] == TRUE) {
        rt  =   COM_8_RES_ODD_KEYERR;          /// 奇开关坏
    } else if  (KeyErr[BSP_ID_EVEN] == TRUE) {
        rt  =   COM_8_RES_EVE_KEYERR;          /// 偶开关坏
    } else if ((broken[BSP_ID_ODD] == TRUE) && (COM_9511MAIsRunning != FALSE)) {
        rt  =   COM_8_RES_ODD_BREAK;           /// 奇断线
    } else if ((broken[BSP_ID_EVEN] == TRUE) && (COM_9511MAIsRunning != FALSE)) {
        rt  =   COM_8_RES_EVE_BREAK;           /// 偶断线
/// !!! cy 101115

//    } else if  (HghErr[BSP_ID_ODD] == TRUE) {         /// cy  101015  m
    } else if ((HghErr[BSP_ID_ODD]  == TRUE) && (COM_9511MAIsInOrg == TRUE)) {
        rt  =   COM_8_RES_ODD_NOTPOS;          /// 奇针高异常
//    } else if  (HghErr[BSP_ID_EVEN] == TRUE) {        /// cy  101015  m
    } else if ((HghErr[BSP_ID_EVEN] == TRUE) && (COM_9511MAIsInOrg == TRUE)) {
        rt  =   COM_8_RES_EVE_NOTPOS;          /// 偶针高异常
    } else if  (COM_9511CheckErr == TRUE) {
        rt  =   COM_8_RES_DAT_ERR;             /// 数据校验错误
    } else {
        rt  =   BSP_DIP_9511Addr;               /// 正确
    }
    C491_SendOne(rt, 0);
}




static  void  COM_9511UpStChange   (BSP_ID_TypeDef      id,
                                    COM_UP_ST_TypeDef   NewSt)
{
	if(COM_UpStLast[id] != COM_UpStNow[id])
		COM_TKIsEnable[id]=TRUE;	
    COM_UpStLast[id]     =   COM_UpStNow[id];
	COM_UpStNow[id]     =   NewSt;
    
    COM_LedUpdate(id);
//    BSP_SQN_EmbStSend(id, COM_GetEmbSt(id));
    COM_SqnUpdate(id);
    
    /// 可加等待0.3S        /// !!! cy  100908
    
//    COM_HghUpdate(id);    /// cy  110304
}

/// 因退出补绣而改变状态
static  void  COM_9511UpStByExitP (void)
{
    u32     i;
 COM_9511SetGreALL(0,0);   
//因为只有补绣状态，所以只对补绣的状态进行处理    
    for (i = 0; i < BSP_DEVICE_ID_N; i++) {
        switch (COM_UpStNow[i]) {
        case COM_UP_ST_PATCH_LOCK:
            COM_9511UpStChange((BSP_ID_TypeDef)i, COM_UP_ST_LOCK);
            break;
        case COM_UP_ST_PATCH_IDLE:
		 		COM_9511UpStChange((BSP_ID_TypeDef)i, COM_UP_ST_NORMAL);
            break;
        case COM_UP_ST_PATCH_NORMAL: 
			if((COM_UpStLast[i]==COM_UP_ST_BROKEN)||(COM_UpStLast[i]==COM_UP_ST_PATCH_BROKEN))
				COM_9511UpStChange((BSP_ID_TypeDef)i, COM_UP_ST_BROKEN);
			else        
            	COM_9511UpStChange((BSP_ID_TypeDef)i, COM_UP_ST_NORMAL);
            break;
        case COM_UP_ST_PATCH_BROKEN:
            	COM_9511UpStChange((BSP_ID_TypeDef)i, COM_UP_ST_BROKEN);
            break;
        default:                                /// 其他状态不变
			COM_UpStNow[i]=COM_UP_ST_NORMAL;	
            break;
        }
		COM_9511TangkongPatch[i]=0;
    }
}

/// 因进入补绣而改变状态，是被命令函数调用的，
static  void  COM_9511UpStByEnterP (void)
{
    u32     i;
    
    
    for (i = 0; i < BSP_DEVICE_ID_N; i++) {
        switch (COM_UpStNow[i]) {
        case COM_UP_ST_LOCK:
            COM_9511UpStChange((BSP_ID_TypeDef)i, COM_UP_ST_PATCH_LOCK);//这个函数只改变了LED和金片板的LED状态，没有改变针高。
            break;
        case COM_UP_ST_NORMAL:
            COM_9511UpStChange((BSP_ID_TypeDef)i, COM_UP_ST_PATCH_IDLE);
            break;
        case COM_UP_ST_BROKEN:
//            COM_9511UpStChange((BSP_ID_TypeDef)i, COM_UP_ST_PATCH_NORMAL);    !!! cy 100904
            COM_9511UpStChange((BSP_ID_TypeDef)i, COM_UP_ST_PATCH_BROKEN);
            break;
        default:                                /// 其他状态不变
            break;
        }
		COM_9511TangkongPatch[i]=2;
    }
}

/// 因主轴转动而改变状态
static  void  COM_9511UpStByMARun (void)
{
    u32     i;
    
    
    for (i = 0; i < BSP_DEVICE_ID_N; i++) {
        switch (COM_UpStNow[i]) {
        case COM_UP_ST_BROKEN:
            COM_9511UpStChange((BSP_ID_TypeDef)i, COM_UP_ST_NORMAL);
            break;
        case COM_UP_ST_PATCH_BROKEN:
            COM_9511UpStChange((BSP_ID_TypeDef)i, COM_UP_ST_PATCH_NORMAL);
            break;
        default:                                /// 其他状态不变
            break;
        }
    }
}

/// 因断线而改变状态
static  void  COM_9511UpStByBroken (BSP_ID_TypeDef id)
{
    switch (COM_UpStNow[id]) {
    case COM_UP_ST_NORMAL:
        COM_9511UpStChange(id, COM_UP_ST_BROKEN);
        break;
    case COM_UP_ST_PATCH_NORMAL:
        COM_9511UpStChange(id, COM_UP_ST_PATCH_BROKEN);
        break;
    default:                                /// 其他状态不变
        break;
    }
}
u8 Test_Status[2]={1,1};
/// 因机头开关上位而改变状态
static  void  COM_UpStByKeyUp (BSP_ID_TypeDef id)
{
    switch (COM_UpStNow[id]) {
    case COM_UP_ST_LOCK:
    case COM_UP_ST_NORMAL:
        COM_9511UpStChange(id, COM_UP_ST_BROKEN);
/// cy  110304
		COM_Tangkong_WorkCmd=MOTO_CUT_NONE;
		COM_9511TangkongPatch[id]=1;
        COM_HghIsAim[id]    =   COM_HghUpdate_key(id);
/// cy  110304
        break;
    case COM_UP_ST_PATCH_LOCK:
    case COM_UP_ST_PATCH_IDLE:
    case COM_UP_ST_PATCH_NORMAL:
        COM_9511UpStChange(id, COM_UP_ST_PATCH_BROKEN);
/// cy  110304
		COM_Tangkong_WorkCmd=MOTO_CUT_NONE;
		COM_9511TangkongPatch[id]=1;
        COM_HghIsAim[id]    =   COM_HghUpdate_key(id);
/// cy  110304
        break;
    default:                                /// 其他状态不变
        break;
    }
	if((COM_9511IsTset==TRUE)&&(BSP_SWI_FiGet(id)  == BSP_SWITCH_LOW))
	{	
		MOTO_TKSetHalfAngle(Tangkong_High[COM_Tangkong_WorkHigh]);
		MOTO_TKSetFullAngle(Tangkong_High[COM_Tangkong_WorkHigh]);
		COM_TKIsEnable[id]=TRUE;
		COM_Tangkong_WorkCmd=MOTO_CUT_HALF;
		Test_Status[id]=0;
		COM_Tangkong_WorkHigh=6;
        COM_HghIsAim[id]    =   COM_HghUpdate_key(id);
	//	TK_Org[id]=1;	
	}
		
}

/// 因机头开关中位而改变状态
static  void  COM_UpStByKeyMd (BSP_ID_TypeDef id)
{
    switch (COM_UpStNow[id]) {
    case COM_UP_ST_LOCK:
        COM_9511UpStChange(id, COM_UP_ST_NORMAL);
/// cy  110304
		COM_Tangkong_WorkCmd=MOTO_CUT_D;
        COM_HghIsAim[id]    =   COM_HghUpdate_key(id);
		
/// cy  110304
        break;
    case COM_UP_ST_PATCH_LOCK:
        COM_9511UpStChange(id, COM_UP_ST_PATCH_IDLE);
/// cy  110304
		COM_Tangkong_WorkCmd=MOTO_CUT_D;
        COM_HghIsAim[id]    =   COM_HghUpdate_key(id);
	
/// cy  110304
        break;
    default:                                /// 其他状态不变
	if((TK_Org[id]==0)&&(BSP_SWI_FiGet(id)  == BSP_SWITCH_LOW)&&(IN_HDKey_GetSt((BSP_ID_TypeDef)id)!=HDKEY_ST_DN))
	{	COM_TKIsEnable[id]=TRUE;
		COM_Tangkong_WorkCmd=MOTO_CUT_D;
        COM_HghIsAim[id]    =   COM_HghUpdate_key(id);
		TK_Org[id]=1;	
	}
		
        break;
    }
}

/// 因机头开关下位而改变状态
static  void  COM_UpStByKeyDn (BSP_ID_TypeDef id)
{
    switch (COM_UpStNow[id]) {
    case COM_UP_ST_BROKEN:
    case COM_UP_ST_NORMAL:
	if(COM_9511IsTset==FALSE)
	{
        COM_9511UpStChange(id, COM_UP_ST_LOCK);
/// cy  110304
		COM_Tangkong_WorkCmd=MOTO_CUT_H;
        COM_HghIsAim[id]    =   COM_HghUpdate_key(id);
		TK_Org[id]=0;
	}
	else
	{	
		if(Test_Status[id]==0)
		{
		COM_Tangkong_WorkCmd=MOTO_CUT_D;
        COM_HghIsAim[id]    =   COM_HghUpdate_key(id);
		Test_Status[id]=1;
		}	
	}
/// cy  110304
        break;
    case COM_UP_ST_PATCH_BROKEN:
    case COM_UP_ST_PATCH_NORMAL:
    case COM_UP_ST_PATCH_IDLE:
	if(COM_9511IsTset==FALSE)
	{
        COM_9511UpStChange(id, COM_UP_ST_PATCH_LOCK);
		COM_Tangkong_WorkCmd=MOTO_CUT_H;
        COM_HghIsAim[id]    =   COM_HghUpdate_key(id);
		TK_Org[id]=0;
	}
	else
	{	
		if(Test_Status[id]==0)
		{
		COM_Tangkong_WorkCmd=MOTO_CUT_D;
        COM_HghIsAim[id]    =   COM_HghUpdate_key(id);
		Test_Status[id]=1;
		}	
	}
/// cy  110304
        break;
    default:                                /// 其他状态不变
        break;
    }
   //if(COM_9511IsTset==TRUE)
   
}



/// -----------------------------------------------------------------------------------------------------
/// @brief  判断收到的内容是否为地址, 用于9511
/// @param  data : 需要判断的内容
/// @retval : 是否
/// -----------------------------------------------------------------------------------------------------
static  bool  COM_9511IsAddr (u16 data)
{
    data   &=   COM_MSK_9511_WHOLE;
    if ((data >> COM_BIT_ADDR_9511) == 0) {	//9511的板地址是0x01~0x0f，所以左移7为是0，924的地址是从0xda~0xff
        return  TRUE;
    } else {
        return  FALSE;
    }
}

///// -----------------------------------------------------------------------------------------------------
///// @brief  判断收到的内容是否为地址, 用于924
///// @param  data : 需要判断的内容
///// @retval : 是否
///// -----------------------------------------------------------------------------------------------------
//static  bool  COM_924IsAddr (u16 data)
//{
//    data   &=   COM_MSK_924_WHOLE;
//    if ((data >> COM_BIT_ADDR_924) != 0) {
//        return  TRUE;
//    } else {
//        return  FALSE;
//    }
//}

/// -----------------------------------------------------------------------------------------------------
/// @brief  判断收到的内容是否为有效地址, 用于9511
/// @param  data : 需要判断的内容
/// @retval : 是否
/// -----------------------------------------------------------------------------------------------------
static  bool  COM_9511IsValidAddr (u16 data,u16 data2)
{
    data   &=   COM_MSK_9511_WHOLE;	//这个数据是0xFF,从0x71~0x7F相与能够保留住这个值
    if ((data == COM_9511_ADDR_ALL) || (data == BSP_DIP_9511Addr)) {//COM_9511_ADDR_ALL是0,9511公共地址
         if((data2==0xc0)||(data2==0xc2)||(data2==0xc8))
		 {
		 	if((BSP_DIP_9511Addr>0x70)&&(BSP_DIP_9511Addr<0x80))
		 		return  TRUE;
			else
				return FALSE;
		 }
		  else if((data2==0xd0)||(data2==0xd2)||(data2==0xd8))
		 {
		 	if((BSP_DIP_9511Addr>0x0)&&(BSP_DIP_9511Addr<0x10))
		 		return  TRUE;
			else
				return FALSE;
		 }
		 else
		  return TRUE;
		
    } else {
        return  FALSE;
    }
}

/// 求校验和(累计和的低7位)
static  u16  COM_9511GetCheckSum (u16 buf[], u8 len)
{
    u16     sum;
    u8      i;
    
    
    sum =   0;
    for (i = 0; i < len; i++) {
        sum    +=   buf[i];
    }
    return  (sum & COM_MSK_9511_DATA);
}

static  void  COM_9511RxHandle (u16 rx)
{
    /// iFrame 表示收到的帧数据长度：
    ///     0                       -   帧开始, 此时应该收地址, 保存有效数据后 +1
    ///     1 ~ (数据长度 + 1)      -   此时应该收命令或数据, 保存后 +1
    ///     (数据长度 + 2)          -   帧数据收全, 调用执行函数后清零
    static  u32     iFrame  =   0;
            u16     cmd;
            u32     iTable;
            u8      len;
            u16    *pData;
            bool    IsMyCmd;
    static  u16     buf[COM_MAX_FRAME_LEN];
    
    
    if (COM_9511IsAddr(rx)) {                                   /// 收到地址
        if (iFrame == 0) {                                      /// 正常情况, 帧同步
            ;
        } else {                                                /// 异常情况, 帧未同步
            iFrame  =   0;                                      /// 强制同步, 设帧开始, 即清零
            COM_ERR_ONE_TIME;                                   /// 指示异常一次
        }
        buf[iFrame++]   =   rx;                                 /// 保存地址, 帧长度+1
        
    } else {                                                    /// 收到非地址, 是命令或数据
        if (iFrame != 0) {                                      /// 如果不是帧开始, 则正常情况, 帧同步
            buf[iFrame++]   =   rx;                             /// 保存数据或命令
            cmd             =   buf[1];                         /// 命令为收到的第2个数
            
/// ------- cy 101007   a   -------                             /// 因为协议表从0x80到0xFF, 所以不加此处也行
//            if ((cmd >= COM_9511Tbl[0].cmd) && (cmd <= COM_9511Tbl[COM_LEN_TAB9511 - 1].cmd)) {
/// ------- cy 101007   a   -------
            
            iTable          =   cmd - COM_9511Tbl[0].cmd;       /// 通过命令, 得到查表位置
            len             =   COM_9511Tbl[iTable].len;        /// 数据长度
            if (iFrame == len + 2) {                            /// 帧数据收全
                               
                /// !!! 因为可能有金片命令, 所以不必判断此命令至少对上或下一种机头板有效
                /// 也可先判断地址, 金片板和机头板地址不同
                if (COM_9511IsValidAddr(buf[0],buf[1])) {              /// 有效地址则执行
                    pData   =   &buf[2];
/// 按上下板功能调用执行函数，这里只能是二选一，不能说一个板子去同时执行上下机头的函数
                    if (BSP_DIP_BDFunc == BSP_DIP_BD_FUNC_UP) {
					   if(COM_9511Tbl[iTable].run_sta>TreahHold_9511)
                        	COM_9511Tbl[iTable].UpExe(pData, len);
                        if (COM_9511Tbl[iTable].UpExe != COM_CmdDoNothing) {
                            IsMyCmd =   TRUE;
                        } else {
                            IsMyCmd =   FALSE;
                        }
                    } else {
                        COM_9511Tbl[iTable].DnExe(pData, len);
                        if (COM_9511Tbl[iTable].DnExe != COM_CmdDoNothing) {
                            IsMyCmd =   TRUE;
                        } else {
                            IsMyCmd =   FALSE;
                        }
                    }
                    /// 更新校验数值, 在数据个数不为零, 不是校验命令, 并且是本板命令
                    if ((len != 0) && (cmd != 0x00d1) && (IsMyCmd != FALSE)) {
                        COM_9511CheckSum    =   COM_9511GetCheckSum(pData, len);
                        COM_9511CheckNum    =   len & COM_MSK_9511_DATA;
                    } else {
                        COM_9511CheckSum    =   0;
                        COM_9511CheckNum    =   0;
                    }
                } else {                                        /// 无效地址不执行
                        ;
                }
                
/// !!! cy 100906

                iFrame  =   0;                                  /// 设为帧开始
            } else {                                            /// 帧数据未收全, 则不处理
                ;
            }
            
/// ------- cy 101007   a   -------                             /// 因为协议表从0x80到0xFF, 所以不加此处也行
//            } else {
//                COM_ERR_ONE_TIME;                               /// 因为协议表到0xFF, 所以不会是新协议, 即错误
//            }
/// ------- cy 101007   a   -------
            
        } else {                                                /// 在帧开始, 则异常情况, 帧未同步
            COM_ERR_ONE_TIME;                                   /// 指示异常一次
        }
    }
}



static  void  COM_924RxHandle (u16 rx)
{
    ;
}

void  COM_RxHandle (u16 rx)
{
    if (BSP_Is924) {
        COM_924RxHandle(rx);
    } else {
        COM_9511RxHandle(rx);
    }
}


void  COM_KeyHandle (void)
{
    u32     i;
    
    
    if (BSP_DIP_BDFunc == BSP_DIP_BD_FUNC_UP) {     /// 上机头板才处理按键
        for (i = 0; i < BSP_DEVICE_ID_N; i++) {
            if (MOTO_HghGetIsActing((BSP_ID_TypeDef)i) == FALSE) {
                switch (IN_HDKey_GetSt((BSP_ID_TypeDef)i)) {
                case HDKEY_ST_MD:
                    COM_UpStByKeyMd((BSP_ID_TypeDef)i);
                    break;
                case HDKEY_ST_UP:
                    COM_UpStByKeyUp((BSP_ID_TypeDef)i);
                    break;
                case HDKEY_ST_DN:
                    COM_UpStByKeyDn((BSP_ID_TypeDef)i);
                    break;
                case HDKEY_ST_ERR:
                default:                                /// 其他状态不变
                    break;
                }
            }
        }
    }
}

static  void  COM_9511Init (void)
{
    u32     i;
    
    
    if (BSP_DIP_BDFunc == BSP_DIP_BD_FUNC_UP) {     /// 上机头板才需要初始化状态
    	for (i = 0; i < BSP_DEVICE_ID_N; i++) {
            switch (IN_HDKey_GetSt((BSP_ID_TypeDef)i)) {
            case HDKEY_ST_MD:
                COM_UpStNow[i]  =   COM_UP_ST_NORMAL;
                break;
            case HDKEY_ST_UP:
                COM_UpStNow[i]  =   COM_UP_ST_BROKEN;
                break;
            case HDKEY_ST_DN:
                COM_UpStNow[i]  =   COM_UP_ST_LOCK;
                break;
            case HDKEY_ST_ERR:
            default:
                COM_UpStNow[i]  =   COM_UP_ST_LOCK;
                break;
            }
            COM_LedUpdate((BSP_ID_TypeDef)i);
            COM_SqnUpdate((BSP_ID_TypeDef)i);
            /// 不刷新针高, 针高不能动, 直到开关改变或命令
			COM_UpStLast[i]     =   COM_UpStNow[i];
    	}
    }
}

static  void  COM_924Init (void)
{
    ;
}

void  COM_Init (void)
{
    if (BSP_Is924) {
        COM_924Init();
    } else {
        COM_9511Init();
    }
}

bool  COM_JogIsEn (BSP_ID_TypeDef id)
{
    if (id < BSP_DEVICE_ID_N) {
        if (COM_IsFault[id] || COM_9511IsTset || COM_9511MAIsRunning) {
            return  FALSE;
        } else {
            return  FALSE;
        }
    } else {
        return  FALSE;
    }
}
void  COM_DXSparkel (void)
{
	u32 id;
	if (BSP_DIP_BDFunc == BSP_DIP_BD_FUNC_UP) 
	{ 
	  	if(COM_9511MAIsRunning ==FALSE)
			{
				for(id=0;id<BSP_DEVICE_ID_N;id++)
					if (id < BSP_DEVICE_ID_N) 
			  		{
						if (COM_IsFault[id] ==FALSE)//机头没有关闭
						{
							if(COM_UpStNow[id]==COM_UP_ST_BROKEN)
							{
								HDLED_Sparkle((BSP_ID_TypeDef)id,1<<30);
							}
						}
					}
		   }
	}							
}

extern crypto_des3_ctx_t ds3_crypto_des3_ctx_t;
extern u8 ds3_src_tmp[8];
extern u8 Confirm_status;
extern u8 Main_Securit[4];
static void COM_MainConfirm(u16 buf[], u8 len)
{
	u8 i;
	u8 buf_tmp[10];
	for(i=0;i<10;i++)
		buf_tmp[i]=*(u8*)(buf+i);
	BitStreamDecode(ds3_src,8,buf_tmp); //解流
	crypto_des3_decrypt(&ds3_crypto_des3_ctx_t,
                    ds3_dst,
                    ds3_src);//解密
	//判断是否加密
	 if((ds3_dst[4]!=0xff)||(ds3_dst[5]!=0xff)||(ds3_dst[6]!=0xff)||(ds3_dst[7]!=0xff))//是加密主板
	 {
	 	//从倒数第二个页取数据，如果9511已经加密了，判断密码是否一致,不一致设置禁止变量
		//9511没有加密，直接退出
		MAIN_SECURITY_STAT=TRUE;
	 }
	 for(i=0;i<4;i++)
	 {
	 	Main_Securit[i]=ds3_dst[4+i];
	 }
	 if(SECURITY_9511_STAT==TRUE)//9511加密
	 {
	 	//修改运行权限
		//密码一致,修改运行门限为0,9511运行
		//密码不一致，不修改运行权限，9511不运行
			if((MAIN_Sec_ID[0] !=Main_Securit[0])||(MAIN_Sec_ID[1]!=Main_Securit[1])||
			   (MAIN_Sec_ID[2]!=Main_Securit[2])||(MAIN_Sec_ID[3]!=Main_Securit[3]))
				{
					TreahHold_9511=5;
					Secrit_For_LED=1;
				}	
	//	 }
		 else //密码一致运行
		 {
		 	TreahHold_9511=0;
		 	Secrit_For_LED=0;
		 }
	// }
	 }
	 //9511加密，主控不加密，不修改运行权限，9511不能运行
	for(i=0;i<4;i++)
	{
		ds3_src_tmp[i]=~ds3_dst[i];
	}
	Confirm_status=1;
	
}
extern void  	Get_Main_ID(void);//获得主控加密时候写入的4字节数据
extern u16 Sec_Number;
extern u8 thir_status;
void Get_Stream()
{
	u8 i;
	Get_Main_ID();
  for(i=0;i<4;i++)
  	ds3_src_tmp[i+4]=MAIN_Sec_ID[i];
	crypto_des3_encrypt(&ds3_crypto_des3_ctx_t,
                    ds3_dst,
                    ds3_src_tmp);
	BitStreamEncode(ds3_dst,8,Stream_9511_To_Main);
}
static void COM_MainCh9511(u16 buf[], u8 len)
{
	u8 i;

	u16 buf_tmp[10];
	Get_Stream();
	for(i=0;i<10;i++)
		buf_tmp[i]=Stream_9511_To_Main[i];
	for(i=0;i<10;i++)
	{
		C491_SendOne(buf_tmp[i], 0);
	}
}

void COM_MainErasPssW(u16 buf[], u8 len)
{
	u8 i,j=0;
	u8 status= 0,tmp_read;
	u32 Addr;
	FLASH_Unlock();
	Addr=CONFIRM_MAINCONTRAL_ADDR1;
	for(i=0;i<5;i++)
	{
		FLASH_ErasePage(Addr);
		for(j=0;j<20;j++)
		{
			tmp_read=(*(u8*)(Addr+i));
			if(tmp_read!=0xFF)
			{
				status=1;
				break;
			}	
		}
		if(status==0)
			break;
	}
	Addr=CONFIRM_MAINCONTRAL_ADDR2;
	for(i=0;i<5;i++)
	{
		FLASH_ErasePage(Addr);
		for(j=0;j<20;j++)
		{
			tmp_read=(*(u8*)(Addr+i));
			if(tmp_read!=0xFF)
			{
				status=1;
				break;
			}	
		}
		if(status==0)
			break;
	}
	FLASH_Lock();
	Get_Stream();
}

void PRO_MainSetPssW(u16 buf[], u8 len,u32 Addr)
{
	u8 i;//,tmp_buf[20];//,tmp_in_buf[10];//,j,ID_status=0;
	u16 tmp_out_buf[10];
	FLASH_Status status= FLASH_COMPLETE;
	for(i=0;i<10;i++)
	{
		tmp_out_buf[i]=buf[i];	
	}
	FLASH_Unlock();
	for(i=0;i<5;i++)
	{
		status=FLASH_ErasePage(Addr);
		if(status == FLASH_COMPLETE)
			break;
	}
	if(status == FLASH_COMPLETE)
		{		
			for(i=0;i<len;i++)
			{
				status =FLASH_ProgramHalfWord((Addr+i*2),tmp_out_buf[i]);
			}
		}
	FLASH_Lock();	
}


void COM_MainSetPssW(u16 buf[], u8 len)
{
	//u8 thir_status=0;

	u32 Addr1,Addr2;
	u8 i,tmp_in_buf[10],j,ID_status=0,ds3_tmp[8],tmp1_u8,tmp2_u8;
	u16 tmp_out_buf[9],tmp_read_buf[10];
	Addr1=CONFIRM_MAINCONTRAL_ADDR1;
	Addr2=CONFIRM_MAINCONTRAL_ADDR2;
	for(i=0;i<10;i++)//获取数据
	 {
		tmp_in_buf[i]=*(u8*)(buf+i);
	 }
	BitStreamDecode(ds3_src,8,tmp_in_buf); //解流
	crypto_des3_decrypt(&ds3_crypto_des3_ctx_t,
                    ds3_dst,
                    ds3_src);//解密
	for(i=4;i<8;i++)
	{
		ds3_tmp[i]=ds3_dst[i];
	}
	for(i=0;i<4;i++)
	{
		ds3_tmp[i]=ds3_src_tmp[i];
	}
	crypto_des3_encrypt(&ds3_crypto_des3_ctx_t,
                    ds3_dst,
                    ds3_tmp);
	for(i=0;i<8;i++)
	{
		if(i<4)
		{
			tmp_out_buf[i]=ds3_dst[i*2]+(((u16)ds3_dst[i*2+1])<<8);
		}
		else
		{
			tmp1_u8=~ds3_dst[(i-4)*2];
			tmp2_u8=~ds3_dst[(i-4)*2+1];
			tmp_out_buf[i]=tmp1_u8+(tmp2_u8<<8);	
		}
	}
	tmp_out_buf[8]=Sec_Number+1;		
	switch(thir_status)
	{	
		case 2:
		case 0:
		case 4:
		case 5://
			for(j=0;j<5;j++)
			{
				PRO_MainSetPssW(tmp_out_buf,9,Addr1);
				for(i=0;i<8;i++)
				{
				   tmp_read_buf[i]=*(u16*)(Addr1+i*2);
				}
				for(i=0;i<8;i++)
				{
					if(tmp_read_buf[i]!=tmp_out_buf[i])
						ID_status=1;
				}
				
				if(ID_status==0)
				{
					break;	
				}
			}
		 break;
		 case 1:
		 case 3://第一块加密无效，第二块不加密
		 	for(j=0;j<5;j++)
			{
				PRO_MainSetPssW(tmp_out_buf,9,Addr2);
				for(i=0;i<10;i++)
				{
				   tmp_read_buf[i]=*(u16*)(Addr2+i*2);
				}
				for(i=0;i<10;i++)
				{
					if(tmp_read_buf[i]!=tmp_out_buf[i])
						ID_status=1;
				}
				
				if(ID_status==0)
				{
					break;	
				}
			}
		 break;
		 default :
		 break;
	}
	Get_Stream();
}
static  void  COM_9511SetSol (u16 buf[], u8 len)
{
    BSP_ID_TypeDef      id;
    BSP_SOL_TypeDef     st;
    u16                 tm;
    
    
    buf[0] &=   COM_MSK_9511_DATA;
    
    id      =   (BSP_ID_TypeDef)((buf[0] >> 0) & 1);
    st      =   (BSP_SOL_TypeDef)((buf[0] >> 1) & 1);
    tm      =   buf[0] >> 2;
    
    BSP_SOL_Set(id, st, tm);
}
/// End of file -----------------------------------------------------------------------------------------
