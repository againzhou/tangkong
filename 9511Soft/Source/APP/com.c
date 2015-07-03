
/// ----------------------------------------------------------------------------
/// File Name          : com.c
/// Description        : ͨѶӦ��ģ��(ȫ˫��ͨѶ)
///
///     ��Ϊʹ�� OS ��Դ, ���Է���APP��.
/// ----------------------------------------------------------------------------
/// History:

/// y11/m03/d06     v0.06   chenyong    modify
/// 1   ȥ����ǰ��ӵ�һ����䣬
/// ���� COM_9511GetHghEven() ��ȥ����� "rt  =   MOTO_HghGetHeight(BSP_ID_EVEN);"
/// 2   ԭ9511���ڽ�������ʱ����߲��仯. Ϊ�޸Ĵ�����, ״̬�л�ʱ�����ı����. 
/// �������£�
///     ���� COM_9511UpStChange()   ȥ������COM_HghUpdate()�ĵ���
///     ���� COM_HghUpdate()        ���ӷ���ֵ, �ж��Ƿ���ɶ���
///     ���ӱ��� COM_HghIsAim[]     ��¼����Ƿ����ͷ����״̬ͬ��
///     ���� COM_9511FaultOff()     ���ӵ��� COM_HghUpdate() ������, ������ͷ����״̬��ͬ�������
///     ���� COM_9511MAOrgIn()      ���ӵ��� COM_HghUpdate() ������, ������ͷ����״̬��ͬ�������
///     ���� COM_UpStByKeyUp()      ���ӵ��ú��� COM_HghUpdate(), ����¼����ֵ
///     ���� COM_UpStByKeyMd()      ���ӵ��ú��� COM_HghUpdate(), ����¼����ֵ
///     ���� COM_UpStByKeyDn()      ���ӵ��ú��� COM_HghUpdate(), ����¼����ֵ
/// 3   ����ʱ�����ر�����ͣ��, ��û�л�ͷ�����.
///     ����Ϊ9511�ϱ����ߺ�������Ȼ�����߼��ź�.
///     �޸ĺ��� COM_9511BrkDetect(), ȥ������ COM_9511UpStByMARun()�ĵ���, �����߼��ź����
/// 4   �ϼ��������, ���Ӻ��� COM_XJ_3(), 3���߼��źŵ���1�κ��� COM_XJ_3.
/// 5   �޸���߻ظ�����. ���û���˶�ǰ, �ظ�ʵ�����

/// y10/m12/d22     v0.05   chenyong    modify
/// ���뺯�� bool COM_JogIsEn(BSP_ID_TypeDef id), ������ߵ��ģ���ж��ܷ�΢��

/// y10/m12/d17     v0.04   chenyong    modify
/// ��Ϊ�����з����ǰû�з����ᵽλ�����, �޸���COM_HighCmd()����
/// "��Ծ����"������Ϊ��ʱ, Խ��ǰ, ���ط�����λ, Խ������������û�б�9511ִ��, ������λ��Խ����
/// ����, COM_HghUpdate()������Ӧ������������ COM_9511MAIsRunning == TRUE,  !!! Ŀǰû�м��� !!!
    
/// y10/m11/d30     v0.03   chenyong    modify
/// 1   �ı䶨�� COM_HGH_NOT_DROP, ԭΪ(10), ��Ϊ(MOTO_HGH_POS_UP)
/// 2   �޸ĺ��� COM_9511GetHghOdd() �� COM_9511GetHghEven(), ������"��߲�һ��"
/// 3   �޸ĺ��� COM_9511GetDnErr(), ���᲻ת��������

/// y10/m08/d27     v0.02   chenyong    modify

/// y10/m08/d06     v0.01   shengjiaq   setup





#define  COM_MODULE     /// ģ�鶨��, ���������ⲿ����


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

#define  VERSION9511                    3                       /// 1 - ��CRC,  2 - ��CRC

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


/// for 9511��

/// 9511���ַ��1��������

/// �ظ�, ���ؽ���, 9511����
typedef enum
{
  	COM_8_RES_BUSY         =   0x12,       /// æ
    COM_8_RES_SQN_POS      =   0x14,       /// ��Ƭװ�õ�ͨ��λ
    COM_8_RES_SQN_NOTPOS,                  /// ��Ƭװ�ò���λ
    COM_8_RES_DAT_ERR      =   0x16,       /// ����У�����
    COM_8_RES_EVE_KEYERR   =   0x20,       /// ż��ͷ��ͷ�����쳣(ż�»�ͷײ��)
    COM_8_RES_ODD_KEYERR   =   0x21,       /// ����ͷ��ͷ�����쳣(���»�ͷײ��)
    COM_8_RES_EVE_OVERTIM  =   0x30,       /// ż��ͷ��ʱ(��߼���)
    COM_8_RES_ODD_OVERTIM  =   0x31,       /// ����ͷ��ʱ(��߼���)
    COM_8_RES_EVE_REVERSE  =   0x40,       /// ż��ͷ��ת(���)            
    COM_8_RES_ODD_REVERSE  =   0x41,       /// ����ͷ��ת(���)
    COM_8_RES_EVE_BREAK    =   0x50,       /// ż��ͷ����
    COM_8_RES_ODD_BREAK    =   0x51,       /// ����ͷ����    
    COM_8_RES_ALL_BREAK    =   0x13,       /// ������
    COM_8_RES_EVE_NOTPOS   =   0x60,       /// ż��ͷ����λ,����쳣(���ڵ�),�����쳣(����ԭ��)
    COM_8_RES_ODD_NOTPOS   =   0x61,       /// ����ͷ����λ,����쳣(���ڵ�),�����쳣(����ԭ��)
    
} COM_8_RES_TypeDef;



/// for 924��

/// ��ͷ���ַ
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

/// �ظ�, ���ؽ���, 9511 �� 924 ����
typedef enum
{
    COM_9_RES_ODD_NOTPOS   =   0x01,       /// ����ͷ����λ
    COM_9_RES_EVE_NOTPOS,                  /// ż��ͷ����λ
    COM_9_RES_ALL_NOTPOS,                  /// ������λ
    COM_9_RES_ODD_HGHERR   =   0x04,       /// ����ͷ����쳣
    COM_9_RES_EVE_HGHERR,                  /// ż��ͷ����쳣
    COM_9_RES_ALL_HGHERR,                  /// ������쳣
    COM_9_RES_ODD_BREAK    =   0x11,       /// ����ͷ����
    COM_9_RES_EVE_BREAK,                   /// ż��ͷ����
    COM_9_RES_ALL_BREAK,                   /// ������
    COM_9_RES_ODD_KEYERR   =   0x21,       /// ����ͷ��ͷ�����쳣
    COM_9_RES_EVE_KEYERR,                  /// ż��ͷ��ͷ�����쳣
    COM_9_RES_ALL_KEYERR,                  /// ����ͷ�����쳣
    COM_9_RES_ODD_REVERSE  =   0x31,       /// ����ͷ��ת(���)
    COM_9_RES_EVE_REVERSE,                 /// ż��ͷ��ת(���)
    COM_9_RES_ALL_REVERSE,                 /// ����ת(���)
    COM_9_RES_ODD_OVERTIM  =   0x41,       /// ����ͷ��ʱ(��߼���)
    COM_9_RES_EVE_OVERTIM,                 /// ż��ͷ��ʱ(��߼���)
    COM_9_RES_ALL_OVERTIM,                 /// ����ʱ(��߼���)
    COM_9_RES_ODD_DHOVERT  =   0x51,       /// ����ͷ��ʱ(DH��)
    COM_9_RES_EVE_DHOVERT,                 /// ż��ͷ��ʱ(DH��)
    COM_9_RES_ALL_DHOVERT,                 /// ����ʱ(DH��)
    COM_9_RES_CUTTAP_BUSY  =   0x62,       /// æ1(����Z����ת��)
    COM_9_RES_DH_BUSY,                     /// æ2(DH����ת��)
    COM_9_RES_CRC_ERR,                     /// CRC���ݴ���
    COM_9_RES_BUSY,                        /// æ
    COM_9_RES_SQN_POS      =   0x70,       /// ��Ƭװ�õ�λ
    COM_9_RES_SQN_NOTPOS,                  /// ��Ƭװ�ò���λ
    COM_9_RES_DN_DATLOSS   =   0x90,       /// �»�ͷ���ݶ�ʧ
    COM_9_RES_UP_DATLOSS,                  /// �ϻ�ͷ���ݶ�ʧ

} COM_9_RES_TypeDef;


/// Private variables ----------------------------------------------------------

static  bool                COM_IsFault[BSP_DEVICE_ID_N]    =   {FALSE, FALSE};     /// ���ϻ�ͷ (�� �� ��)
//static  bool              COM_TKFault[BSP_DEVICE_ID_N]    =   {TRUE, TRUE};     /// ���ϻ�ͷ (�� �� ��)
static  bool                COM_DnIsEnable[BSP_DEVICE_ID_N] =   {TRUE,  TRUE};      /// ����ʹ��״̬ (��)
static  bool                COM_TKIsEnable[BSP_DEVICE_ID_N] =   {FALSE,  FALSE};      /// ����ʹ��״̬ (��)

static  bool                COM_SwiDirIsEnable[BSP_DEVICE_ID_N] =   {FALSE,  FALSE};      /// ����ʹ��״̬ (��)
static  COM_UP_ST_TypeDef   COM_UpStNow[BSP_DEVICE_ID_N];
static  COM_UP_ST_TypeDef   COM_UpStLast[BSP_DEVICE_ID_N];                            /// �ϻ�ͷ״̬ (��)
static  bool                COM_HghIsAim[BSP_DEVICE_ID_N]   =   {FALSE, FALSE};     /// ����Ƿ���Ŀ��λ�� (��)
static  bool                COM_IsHghRest                   =   TRUE;               /// �ϵ���߾�ֹ(��)
  bool                COM_9511IsTset                  =   FALSE;              /// �Ƿ����״̬ (��)(��ͷ���ø�����)

/// cy  101115  added
/// ��������ʱ, COM_9511MAIsInOrg = FALSE, COM_9511MAIsRunning = TRUE
/// Խ��ʱ,     COM_9511MAIsInOrg = FALSE, COM_9511MAIsRunning = FALSE
/// ����������ͨѶ����ı�, COM_9511MAIsInOrg ��ʵ�����岻����
static  bool                COM_9511MAIsInOrg               =   FALSE;              /// �����Ƿ�����λ
static  bool                COM_9511MAIsRunning             =   FALSE;              /// �����Ƿ�����ת
/// cy  101115  added
static  void  COM_9511SetGreALL (u16 buf[], u8 len);//ȫ���̵ƣ����ط�ָ��
static  u8                  COM_WorkHigh                    =   0;                  /// �趨���ֵ (��)


static  u16                 COM_9511CheckSum                =   0;                  /// 9511У���
static  u16                 COM_9511CheckNum                =   0;                  /// 9511У�����ݸ���
static  bool                COM_9511CheckErr                =   FALSE;              /// 9511У�������Ƿ����
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
    {0x0080,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},           /// ���0   UP  ������
    {0x0081,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},           /// ���1   UP  ������
    {0x0082,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},           /// ...
    {0x0083,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},
    {0x0084,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},
    {0x0085,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},
    {0x0086,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},
    {0x0087,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},
    {0x0088,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},
    {0x0089,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},           /// ���9   UP  ������
    {0x008a,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},          /// ���10  UP  ������
    {0x008b,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x008c,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x008d,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x008e,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x008f,    1,  5,COM_CmdDoNothing,   COM_9511SetSol},      /// ������
    
    {0x0090,    0,  5,COM_CmdDoNothing,   COM_Tangkong_HighTo0},      /// ������
    {0x0091,    0,  5,COM_CmdDoNothing,   COM_Tangkong_HighTo1},      /// ������
    {0x0092,    0,  5,COM_CmdDoNothing,   COM_Tangkong_HighTo2},      /// ������
    {0x0093,    0,  5,COM_CmdDoNothing,   COM_Tangkong_HighTo3},      /// ������
    {0x0094,    0,  5,COM_CmdDoNothing,   COM_Tangkong_HighTo4},      /// ������
    {0x0095,    0,  5,COM_CmdDoNothing,   COM_Tangkong_HighTo5},      /// ������
    {0x0096,    0,  5,COM_CmdDoNothing,   COM_Tangkong_HighTo6},      /// ������
    {0x0097,    0,  5,COM_CmdDoNothing,   COM_Tangkong_HighTo7},      /// ������
    {0x0098,    0,  5,COM_CmdDoNothing,   COM_Tangkong_HighTo8},      /// ������
    {0x0099,    0,  5,COM_CmdDoNothing,   COM_Tangkong_HighTo9},      /// ������
    {0x009a,    0,  5,COM_CmdDoNothing,   COM_Tangkong_HighTo10},      /// ������
    {0x009b,    0,  5,COM_CmdDoNothing,   COM_Tangkong_SinglDir},      /// ������
    {0x009c,    0,  5,COM_CmdDoNothing,   COM_Tangkong_DoublDir},      /// ������
    {0x009d,    0,  5,COM_CmdDoNothing,   COM_Tangkong_Position},      /// ������
    {0x009e,    0,  5,COM_CmdDoNothing,   COM_Tangkong_Sta},      /// ������
    {0x009f,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    
    {0x00a0,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x00a1,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x00a2,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x00a3,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x00a4,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x00a5,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x00a6,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x00a7,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x00a8,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x00a9,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x00aa,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x00ab,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x00ac,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x00ad,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x00ae,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x00af,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    
    {0x00b0,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x00b1,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x00b2,    0,  5,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x00b3,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x00b4,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x00b5,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x00b6,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x00b7,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x00b8,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x00b9,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x00ba,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x00bb,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x00bc,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x00bd,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x00be,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x00bf,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    
    {0x00c0,    5,  HOLD,COM_CmdDoNothing,   COM_9511FaultOff},      ///�¹رչ��ϻ�ͷ
    {0x00c1,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x00c2,    5,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      ///���ϴ��ϼ�
    {0x00c3,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x00c4,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x00c5,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x00c6,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x00c7,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x00c8,    5,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      ///�¼���ʹ��
    {0x00c9,    10, 60,COM_CmdDoNothing,    COM_CmdDoNothing},      /// �����»�ͷ����
    {0x00ca,    0,  60,COM_CmdDoNothing,   COM_CmdDoNothing},      /// �����ϻ���ͷ����
    {0x00cb,    0,  60,COM_CmdDoNothing,   COM_CmdDoNothing},      /// �����»�ͷ����
    {0x00cc,    10, 60,COM_CmdDoNothing,   COM_CmdDoNothing},      	/// �����ϻ�ͷ����
    {0x00cd,    0,  60,COM_CmdDoNothing,   COM_CmdDoNothing},   	/// ���»�ͷ��֤
    {0x00ce,    0,  60,COM_CmdDoNothing,   COM_CmdDoNothing},   		/// ���ϻ�ͷ��֤
    {0x00cf,    10, 60,COM_CmdDoNothing,   COM_CmdDoNothing},    	///��֤���»�ͷ��ִ��
    
    {0x00d0,    5,  HOLD,COM_CmdDoNothing,  COM_9511FaultOff},      /// �ɹرչ��ϻ�ͷ            �Ϻ���  ����5
    {0x00d1,    2,  HOLD,COM_CmdDoNothing,  COM_CmdDoNothing},     /// ����У��                �Ϻ���  ����2
    {0x00d2,    5,  HOLD,COM_CmdDoNothing,  COM_CmdDoNothing},     /// ֪ͨ�ϻ�ͷ�¶ϼ�����  ��      ����5
    {0x00d3,    5,  HOLD,COM_CmdDoNothing,  COM_CmdDoNothing},     /// ���߼�ⷽʽ            �Ϻ���  ����5
    {0x00d4,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x00d5,    1,  HOLD,COM_CmdDoNothing,   COM_HeadSpeed},      /// ������
    {0x00d6,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},    /// ��ѯ��Ƭ�Ƿ�̧��        ��      ����0
    {0x00d7,    2,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ��ר�� H10��ADֵ    ��Ϊ������
    {0x00d8,    5,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// �»�ͷ��ʹ��            ��      ����5
    {0x00d9,    0,  HOLD,COM_CmdDoNothing,    COM_CmdDoNothing},      /// �����ջ�                ��      ����0
    {0x00da,    0,  HOLD,COM_CmdDoNothing,    COM_CmdDoNothing},      /// �������                ��      ����0
    {0x00db,    0,  HOLD,COM_CmdDoNothing,    COM_CmdDoNothing},      /// ����                    ��      ����0
    {0x00dc,    3,  HOLD,COM_CmdDoNothing,    COM_CmdDoNothing},      /// ���ü��߲���            ��      ����3
    {0x00dd,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},    /// ��ѯ�ϻ�ͷ��汾        ��      ����0
    {0x00de,    0,  HOLD,COM_CmdDoNothing,   COM_9511SetRedOdd},     /// ��ͷ�����, AD > �趨ֵ ��      ����0
    {0x00df,    0,  HOLD,COM_CmdDoNothing,   COM_9511SetGreOdd},     /// ��ͷ���̵�, AD== �趨ֵ ��      ����0
    
    {0x00e0,    0,  HOLD,COM_CmdDoNothing,   COM_9511SetOraOdd},     /// ��ͷ���Ƶ�, AD < �趨ֵ ��      ����0
    {0x00e1,    0,  HOLD,COM_CmdDoNothing,   COM_9511TestEnter},     /// �������״̬            ��      ����0
    {0x00e2,    0,  HOLD,COM_CmdDoNothing,   COM_9511TestExit},      /// �˳�����״̬            ��      ����0
    {0x00e3,    0,  HOLD,COM_CmdDoNothing,   COM_9511SetRedEven},    /// żͷ�����, AD > �趨ֵ ��      ����0
    {0x00e4,    0,  HOLD,COM_CmdDoNothing,   COM_9511SetGreEven},    /// żͷ���̵�, AD== �趨ֵ ��      ����0
    {0x00e5,    0,  HOLD,COM_CmdDoNothing,   COM_9511SetOraEven},    /// żͷ���Ƶ�, AD < �趨ֵ ��      ����0
    {0x00e6,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ͣ��ʱ���᲻����λ      ��      ����0
    {0x00e7,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},     /// ��ͷ��߲�ѯ            ��      ����0
    {0x00e8,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},    /// żͷ��߲�ѯ            ��      ����0
    {0x00e9,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ��ͷ��λ����ѯ          ��      ����0
    {0x00ea,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},     /// żͷ��λ����ѯ          ��      ����0
    {0x00eb,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},     /// �㶯                    ��      ����0
    {0x00ec,    0,  HOLD,COM_CmdDoNothing,   COM_9511SetGreALL},     /// ��Ƽ�ȫ��              ��      ����0
    {0x00ed,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������(?�иĶ�)
    {0x00ee,    0,  HOLD,COM_CmdDoNothing,   COM_9511PatchExit},     /// �˳�����                ��      ����0
    {0x00ef,    0,  HOLD,COM_CmdDoNothing,   COM_9511PatchEnter},    /// ���벹��                ��      ����0
    
    {0x00f0,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x00f1,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x00f2,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x00f3,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x00f4,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x00f5,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x00f6,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ������
    {0x00f7,    0,  HOLD,COM_CmdDoNothing,   COM_CmdDoNothing},      /// ���״̬(PatchIdle����) ��      ����0
    {0x00f8,    0,  HOLD,COM_CmdDoNothing,  COM_CmdDoNothing},      /// ������Ƿ��ԭ��        ��      ����0
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

/// ����ˢ��, �� �� ���
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
static  void  COM_ID_9511CutBack (BSP_ID_TypeDef id,u16 buf[], u8 len);//�����ջ�
static  void  COM_ID_9511CutHalf (BSP_ID_TypeDef id,u16 buf[], u8 len);//�������
static  void  COM_ID_9511CutFull (BSP_ID_TypeDef id,u16 buf[], u8 len);//��������
static  void  COM_ID_9511CutH (BSP_ID_TypeDef id,u16 buf[], u8 len);//�������
static  void  COM_ID_9511CutD (BSP_ID_TypeDef id,u16 buf[], u8 len);//��������
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
//���Ǹ��ݻ�ͷ��״̬�����»�ͷ���ذ�LED�õģ�ע�ⲻ�Ƿ������ص�����״̬
static  HDLED_ST_TypeDef  COM_GetHDLedSt (COM_UP_ST_TypeDef st,BSP_ID_TypeDef id)
{
    switch (st) {
        
    case COM_UP_ST_BROKEN:
    case COM_UP_ST_PATCH_BROKEN:
        return  HDLED_ST_RED; //���߾�Ӧ���Ǻ��
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
{//ע�������ǻ�ȡ�Ĵ����״̬�����塢�����塢ͣ�壬�Ǹ��������õġ���Ӧ��������0xF7����
    if (COM_IsFault[id] == FALSE) {
        switch (COM_UpStNow[id]) {
        case COM_UP_ST_PATCH_BROKEN:
        case COM_UP_ST_PATCH_NORMAL:
		
            return  BSP_EMB_ST_PATCH;//2
        case COM_UP_ST_PATCH_IDLE:
        case COM_UP_ST_NORMAL:
		case COM_UP_ST_BROKEN://������ߣ����Ƕ�������������������Ĵ���״̬��
			return  BSP_EMB_ST_NORMAL;//1��Ϊ������¶ϼ죬�����лὫ���߹رգ���������Ҫ������������Ӧ���м��߷�����������߶�һ��	
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
		case COM_UP_ST_BROKEN://�����Ƕ��ߣ������Ƭ���������Ѿ��ǲ����ˣ���Ϊ���������������
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
    
    
    if (COM_9511IsTset == FALSE) {                  /// �ǲ���״̬�Ÿ�����ʾ
        if (COM_IsFault[id] == FALSE) {             /// δ�رչ��ϻ�ͷ, ��״̬������ʾ
/// !!! cy 100905
//            led =   (HDLED_ST_TypeDef)((COM_UpStBits[COM_UpStNow[id]] >> COM_UP_BIT_HD_LED) & COM_UP_MSK_HD_LED);
            led =   COM_GetHDLedSt(COM_UpStNow[id],id);
/// !!! cy 100905
        } else {                                    /// �رչ��ϻ�ͷ, ��Ϊ��
            led =   (HDLED_ST_TypeDef)HDLED_ST_OFF;
        }
        HDLED_Set(id, led);                         /// ������ʾ
        

        if (BSP_DIP_RedIsSpkl) {
            if (led == HDLED_ST_RED) {
                HDLED_Sparkle(id, 1 << 30);
            } else {
                HDLED_Sparkle(id, 0);
            }
        }

    } else {                                        /// ����״̬��������ʾ
        ;
    }
}

static void MOT_CUT_Get_SwiDirIsStatus()
{
		u32 id;
		for(id=0;id<BSP_DEVICE_ID_N;id++)
		{
	    if ((BSP_SWI_FiGet(id) == BSP_SWITCH_LOW)||(COM_9511TangkongDir==FALSE)) /// ��û�л�λ, ���λ
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
			(IN_HDKey_GetSt((BSP_ID_TypeDef)id)!=HDKEY_ST_DN)){          /// ����, �������, �������˶�
/// !!! cy 100905
            cmd =   (MOTO_CUT_CMD_TypeDef)COM_Tangkong_WorkCmd;
        } else {                                                /// ���������, ����
            cmd =   (MOTO_HGH_CMD_TypeDef)MOTO_CUT_NONE;

        }
	/*	if((COM_Tangkong_WorkCmd==MOTO_CUT_D)||(COM_Tangkong_WorkCmd==MOTO_CUT_FULL))
			HeadUpDown_Status[id]=0;
		else if(COM_Tangkong_WorkCmd==MOTO_CUT_H)
			HeadUpDown_Status[id]=0;
		else if(COM_Tangkong_WorkCmd==MOTO_CUT_HALF)
			HeadUpDown_Status[id]=0; */ 
        MOTO_CutSetCmd(id, cmd);
        COM_IsHghRest = FALSE;                                  /// �ϵ������߲���������Ժ���Զ���

/// cy  110304
//    } else {                                                    /// ����, ������
//        ;
//    }
        return  TRUE;
   /* } else {                                                    /// ����, ������
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
		 {          /// ����, �������, �������˶�
/// !!! cy 100905
            cmd =   (MOTO_CUT_CMD_TypeDef)COM_Tangkong_WorkCmd;
        } else {                                                /// ���������, ����
            cmd =   (MOTO_HGH_CMD_TypeDef)MOTO_CUT_NONE;

        }
	/*	if((COM_Tangkong_WorkCmd==MOTO_CUT_D)||(COM_Tangkong_WorkCmd==MOTO_CUT_FULL))
			HeadUpDown_Status[id]=0;
		else if(COM_Tangkong_WorkCmd==MOTO_CUT_H)
			HeadUpDown_Status[id]=0;
		else if(COM_Tangkong_WorkCmd==MOTO_CUT_HALF)
			HeadUpDown_Status[id]=0; */
        MOTO_CutSetCmd(id, cmd);
        COM_IsHghRest = FALSE;                                  /// �ϵ������߲���������Ժ���Զ���

/// cy  110304
//    } else {                                                    /// ����, ������
//        ;
//    }
        return  TRUE;
   /* } else {                                                    /// ����, ������
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
/// @brief �����ϻ�ͷ���յ����λ�õ��������
///        ������  ���ݻ�ͷ״̬�������������
/// @param high  ���λ��ֵ
/// @return None
/// -----------------------------------------------------------------------------------------------------
////////
static  void  COM_HighCmd (u8 high)//��ߺ���
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
    /// ��Ϊ�з����ǰû�з����ᵽλ�����
    /// "��Ծ����"������Ϊ��ʱ, Խ��ǰ, ���ط�����λ, Խ������������û�б�ִ��, ������λ��Խ����
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
/// @brief     ���������������ݵĺ�����
/// ������   �ҳ���Ӧ�ֽ��еĶ�Ӧλ��
///            ��������ͱ����� check_data_add ���ֽ�����Ϊ��check_data_num��
/// @return    segment_select����Ӧλ��ֵ��
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
/// @brief �����ϻ�ͷ���յ����λ��10���������
///        ���룺�ϻ�ͷ��
/// ������  ���ݻ�ͷ״̬�������������
/// @return �ޡ�
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
//���1��0λ��4��Ó�xλ2���\���^���Ј���һ��B
//�����Լ���ԭ�c����ʼ���ӽ��_�P����ԭ�c��3	 

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
   if(TK_Org[i]==0)//û�ҹ�
   {
   		if(i==0)
			rt=3;
		else
		{
			rt &=0x0F;
			rt|=(3<<4);
		}
   }
   else//�ҹ���
   {
        if ((COM_IsFault[i] == FALSE) && (COM_DnIsEnable[i] == TRUE)) 
		{
            MotoSt[i]   =   MOTO_CutGetState((BSP_ID_TypeDef)i);
        } 
		else 
		{
            MotoSt[i]   =   MOTO_ST_OK;
        }
    //�ɹ���
		if(MotoSt[i]  ==   MOTO_ST_OK)
		{
			if(i==0)
			{
			 	if(BSP_SWI_FiGet(BSP_ID_ODD)  != BSP_SWITCH_LOW)//����
				{
					rt=0x01;//
				}
				else
				{
					if(HeadUpDown_Status[0]==1)
						rt=0x4;	//����λ
					if(HeadUpDown_Status[0]==2)
						rt=0x02;//������λ
		
				}
			}
			if(i==1)
			{
				if(BSP_SWI_FiGet(BSP_ID_EVEN) != BSP_SWITCH_LOW)
				{
						rt &=0x0F;
				    	rt=rt|(1<<4); //ͣ��λ 
				}
				
				else
				{
					if(HeadUpDown_Status[1]==1)
					{
						rt &=0x0F;
						rt=rt|(1<<6);	//����λ
					}
					if(HeadUpDown_Status[1]==2)
					{
						rt &=0x0F;
						rt=rt|(1<<5);//������λ
					}
				}
			}
		 COM_Tangkong_Pos_Last=rt;
		}
		else//û�гɹ�
		{
			 if(TK_Org[i]==1) //�Ѿ��ҹ���
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
	}//�����ҹ���
	}
  }//����for
    C491_SendOne(rt, 0);
}
static  void  COM_Tangkong_Sta(u16 buf[], u8 len)
{
	u16 rt;
 	rt  =   BSP_DIP_9511Addr;               /// ��ȷ
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
/// @brief �����ͷ��رջ�ͷ�����������
///        ���룺�ϡ��»�ͷ��
///        ������  ȫ�ֱ��� COM_HD_valid_even��COM_HD_valid_odd
/// @param u8* p_buffer, u8 len
/// @return �ޡ�
/// -----------------------------------------------------------------------------------------------------
////////
static  void  COM_9511FaultOff (u16 buf[], u8 len) //����0xd0���COM_IsFault==TRUE��ʾ���عر��������ͷ
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
        
        /// �ɼӵȴ�0.3S        /// !!! cy  100908
        
//        COM_HghUpdate((BSP_ID_TypeDef)i);
        if (COM_IsHghRest == FALSE) {           /// Ϊ��ȫ�ϵ����߲���, ֱ������������򿪹ر仯
            
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
    /// ֮ǰ�������������, ����Ϊ��У��������Ч
    if (   (COM_9511CheckNum == 0)                                          
        || (   ((buf[0] & COM_MSK_9511_DATA) == COM_9511CheckNum)           /// ����У������������ȷ
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
    
    if (COM_BIT_ODD(decode)) {                  /// ��������
        COM_9511UpStByBroken(BSP_ID_ODD);
    } else {                                    /// �����߲�����
        ;
    }
    
    if (COM_BIT_EVE(decode)) {                  /// ��������
        COM_9511UpStByBroken(BSP_ID_EVEN);
    } else {                                    /// �����߲�����
        ;
    }
}

/// Byte0   Ϊ���߼�������:   0 - ��9511, 1 - ��9511, 2 - ef192, 3 - ef196; 
/// Byte1   Ϊ���߼�����ͣ�    0 - ��, 1 - ��������(���ߵ��� �� �ӽ�����), 2 - ��������(ն����)
/// Byte2~4 ����
extern u8 IN_TEN_MULTIPLE;
static  void  COM_9511BrkDetMod (u16 buf[], u8 len)//0xd3�����ִ�к�����
{//�����ص�init3���з��������
    bool    IsDn;
//    u32     i;        /// !!! cy 100908
     u8 q=2;
    
    if ((buf[0] & COM_MSK_9511_DATA) == 0) {
        IsDn    =   FALSE; //�ϻ�ͷ�ϼ���0
    } else {
        IsDn    =   TRUE;//�»�ͷ�ϼ���1
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
/// @brief �����»�ͷ�����ʹ�������������
///        ���룺�»�ͷ��
///        ������  ȫ�ֱ��� COM_cut_eab_odd��COM_cut_eab_even
/// @param u8* p_buffer, u8 len
/// @return �ޡ�
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

static  void  COM_9511CutBack (u16 buf[], u8 len)//�����ջ�
{
    u32     i;
    
    
    for (i = 0; i < BSP_DEVICE_ID_N; i++) {
        if ((COM_IsFault[i] == FALSE)&&(COM_GetIsDrop(COM_UpStNow[i]) != FALSE)) {
            MOTO_CutSetCmd((BSP_ID_TypeDef)i, MOTO_CUT_BACK);
        }
    }
}

static  void  COM_9511CutHalf (u16 buf[], u8 len)//�������
{
    u32     i;
    
    
    for (i = 0; i < BSP_DEVICE_ID_N; i++) {
        if ((COM_IsFault[i] == FALSE) && (COM_DnIsEnable[i] == TRUE)&&(COM_GetIsDrop(COM_UpStNow[i]) != FALSE)) {
            MOTO_CutSetCmd((BSP_ID_TypeDef)i, MOTO_CUT_HALF);
        }
    }
}

static  void  COM_9511CutFull (u16 buf[], u8 len)//��������
{
    u32     i;
    
    
    for (i = 0; i < BSP_DEVICE_ID_N; i++) {
        if ((COM_IsFault[i] == FALSE) && (COM_DnIsEnable[i] == TRUE)&&(COM_GetIsDrop(COM_UpStNow[i]) != FALSE)) {
            MOTO_CutSetCmd((BSP_ID_TypeDef)i, MOTO_CUT_FULL);
        }
    }
}
static  void  COM_ID_9511CutBack (BSP_ID_TypeDef id,u16 buf[], u8 len)//�����ջ�
{
    //u32     i;
    
    
   // for (i = 0; i < BSP_DEVICE_ID_N; i++) {
        if ((COM_IsFault[id] == FALSE)&&(COM_GetIsDrop(COM_UpStNow[id]) != FALSE)) {
            MOTO_CutSetCmd(id, MOTO_CUT_BACK);
       // }
    }
}
static  void  COM_ID_9511CutH (BSP_ID_TypeDef id,u16 buf[], u8 len)//�����ջ�
{
    //u32     i;
    
    
   // for (i = 0; i < BSP_DEVICE_ID_N; i++) {
        if ((COM_IsFault[id] == FALSE)&&(COM_GetIsDrop(COM_UpStNow[id]) != FALSE)) {
            MOTO_CutSetCmd(id, MOTO_CUT_H);
       // }
    }
}static  void  COM_ID_9511CutD (BSP_ID_TypeDef id,u16 buf[], u8 len)//�����ջ�
{
    //u32     i;
    
    
   // for (i = 0; i < BSP_DEVICE_ID_N; i++) {
        if ((COM_IsFault[id] == FALSE)&&(COM_GetIsDrop(COM_UpStNow[id]) != FALSE)) {
            MOTO_CutSetCmd(id, MOTO_CUT_D);
       // }
    }
}
static  void  COM_ID_9511CutHalf (BSP_ID_TypeDef id,u16 buf[], u8 len)//�������
{
   // u32     i;
    
    
   // for (i = 0; i < BSP_DEVICE_ID_N; i++) {
        if ((COM_IsFault[id] == FALSE) && (COM_DnIsEnable[id] == TRUE)&&(COM_GetIsDrop(COM_UpStNow[id]) != FALSE)) {
            MOTO_CutSetCmd(id, MOTO_CUT_HALF);
     //   }
    }
}

static  void  COM_ID_9511CutFull (BSP_ID_TypeDef id,u16 buf[], u8 len)//��������
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
        /// ����3������, ���ڴ��жϷ�Χ
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
/// !!! cy 100908   ����״̬�ɸı����, ���˳�����ˢ��
////        COM_HghUpdate((BSP_ID_TypeDef)i);
//        if (COM_IsHghRest == FALSE) {           /// Ϊ��ȫ�ϵ����߲���, ֱ������������򿪹ر仯
//            COM_HghUpdate((BSP_ID_TypeDef)i);
//        }
/// !!! cy 100908   ����״̬�ɸı����, ���˳�����ˢ��
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
        rt  =   BSP_POT_GetAve(BSP_ID_ODD) >> 8;               /// �ظ�8λ
    }
    C491_SendOne(rt, 0);
}

static  void  COM_9511GetADEven (u16 buf[], u8 len)
{
    u16     rt;
    
    
    if (MOTO_HghGetIsActing(BSP_ID_EVEN) != FALSE) {
        rt  =   COM_8_RES_BUSY;
    } else {
        rt  =   BSP_POT_GetAve(BSP_ID_EVEN) >> 8;               /// �ظ�8λ
    }
    C491_SendOne(rt, 0);
}

/// ����������Ϣ, ��������߼����Ϣ
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

/// ! ���ܲ�����
static  void  COM_9511SetGreALL (u16 buf[], u8 len)//ȫ���̵ƣ����ط�ָ��
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

static  void  COM_9511PatchEnter (u16 buf[], u8 len) //���벹��
{
    COM_9511UpStByEnterP();	//�������ֻ�Ǹı����ϻ�ͷ��״̬��û�иı��ͷ����ĸ߶ȣ�Ϊʲô��
}

/// ?��ѯ״̬ �� ��ѯ��״̬ �Կ�? PatchIdle ����
/// ?��-���ߣ���-�������
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
         if (COM_IsHghRest == FALSE) {           /// Ϊ��ȫ�ϵ����߲���, ֱ������������򿪹ر仯
            
/// cy  110304
//            COM_HghUpdate((BSP_ID_TypeDef)i);
            if (COM_HghIsAim[i] == FALSE) {
                COM_HghIsAim[i] =   COM_HghUpdate((BSP_ID_TypeDef)i);
            }
/// cy  110304

        }
    }
}



/// ����������Ϣ, Ҳ������߼����Ϣ
/// ��Ϊ������߼����Ϣ, ���Զ��»�ͷ��Ч
static  void  COM_9511MAStartUp (u16 buf[], u8 len)
{
    u32     i;
    
    COM_9511MAIsInOrg   =   FALSE;
    COM_9511MAIsRunning =   TRUE; 
    if (BSP_DIP_BDFunc == BSP_DIP_BD_FUNC_UP) {//�ϻ�ͷ
       // COM_9511MAIsInOrg   =   FALSE;
       // COM_9511MAIsRunning =   TRUE;               /// cy  101115  added
        
        COM_9511UpStByMARun();
        
        for (i = 0; i < BSP_DEVICE_ID_N; i++) {
            if (COM_IsFault[i] == FALSE) {

                HDLED_Sparkle((BSP_ID_TypeDef)i, 3);

            }
        }
    }
    
    /// ���߼����Ϣ����
    IN_BRK_SampleBegin();
}

/// ������������Ϣ, ������߼����Ϣ
static  void  COM_9511MAStop (u16 buf[], u8 len)//0xfd����ͣ��������
{
    COM_9511MAIsRunning =   FALSE;          /// cy  101115  added
    
    /// ���߼����Ϣ����
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
//�߼�����0xFF����Ϊֻ�����ط����߼��źŲŻ�ı�IN_BrkIsBroken[i]��ֵ�����س���������û�п���������߼����   
static  void  COM_9511BrkDetect (u16 buf[], u8 len)//��emb��������SendDJtoHead�ĺ궨�壬�������з��ϼ��źŵġ��ڶ��߲��������Լ�����������0xFF
{

    u32                 i;
    COM_9511MAIsInOrg   =   FALSE;
    COM_9511MAIsRunning =   TRUE; 
    
    if (BSP_DIP_BDFunc == BSP_DIP_BD_FUNC_UP) {//�ϻ�ͷ
       // COM_9511MAIsInOrg   =   FALSE;
       // COM_9511MAIsRunning =   TRUE;               /// cy  101115  added
        
/// cy  110226  modify
//        COM_9511UpStByMARun();                      /// �����״̬
/// cy  110226  modify

/// cy  110227  modify        
//        IN_BRK_SentXJ();
        COM_XJ_3();
/// cy  110227  modify
    
        for (i = 0; i < BSP_DEVICE_ID_N; i++) {
/// !!! cy 100905
//            if (   (COM_IsFault[i] == FALSE)        /// ! ���п���
//                && (COM_GetIsDrop(COM_UpStNow[i]) == TRUE)
//                && (IN_BRK_GetIsBrk((BSP_ID_TypeDef)i) != FALSE)   ) {
            if (   (COM_GetIsDrop(COM_UpStNow[i]) == TRUE)
                	&& (IN_BRK_GetIsBrk((BSP_ID_TypeDef)i) != FALSE)   ) { //��ȡIN_BrkIsBroken[i]�ϻ�ͷ��鵽�˶���
/// !!! cy 100905
                		COM_9511UpStByBroken((BSP_ID_TypeDef)i);//�����ϻ�ͷ��
            }
        }
    } else {//�»�ͷ
/// cy  110227  modify
//        IN_BRK_SentXJ();
        COM_XJ_3();//��������������IN_BRK_SendXJ��������IN_BrkIsBroken[i]������ֵ
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
            broken[i]   =   COM_GetIsBroken(COM_UpStNow[i]);//ͨ���ж��ϻ�ͷ��״̬����
/// !!! cy 100905��IN_BRK_GetIsBrk��ͬ�����Ǹ��ݲ���ֵ����
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
        rt  =   COM_8_RES_BUSY;                /// æ
    } else if ((MotoSt[BSP_ID_ODD]  == MOTO_ST_PRO) || (MotoSt[BSP_ID_ODD]  == MOTO_ST_OVTIME)) {
        rt  =   COM_8_RES_ODD_OVERTIM;         /// �泬ʱ
    } else if ((MotoSt[BSP_ID_EVEN] == MOTO_ST_PRO) || (MotoSt[BSP_ID_EVEN] == MOTO_ST_OVTIME)) {
        rt  =   COM_8_RES_EVE_OVERTIM;         /// ż��ʱ
    } else if  (MotoSt[BSP_ID_ODD]  == MOTO_ST_REVERSE) {
        rt  =   COM_8_RES_ODD_REVERSE;         /// �淴ת
    } else if  (MotoSt[BSP_ID_EVEN] == MOTO_ST_REVERSE) {
        rt  =   COM_8_RES_EVE_REVERSE;         /// ż��ת
    } else if  (KeyErr[BSP_ID_ODD] == TRUE) {
        rt  =   COM_8_RES_ODD_KEYERR;          /// �濪�ػ�
    } else if  (KeyErr[BSP_ID_EVEN] == TRUE) {
        rt  =   COM_8_RES_EVE_KEYERR;          /// ż���ػ�
    } else if ((broken[BSP_ID_ODD] == TRUE) && (COM_9511MAIsRunning != FALSE)) {
        rt  =   COM_8_RES_ODD_BREAK;           /// �����
    } else if ((broken[BSP_ID_EVEN] == TRUE) && (COM_9511MAIsRunning != FALSE)) {
        rt  =   COM_8_RES_EVE_BREAK;           /// ż����
/// !!! cy 101115

//    } else if  (HghErr[BSP_ID_ODD] == TRUE) {         /// cy  101015  m
    } else if ((HghErr[BSP_ID_ODD]  == TRUE) && (COM_9511MAIsInOrg == TRUE)) {
        rt  =   COM_8_RES_ODD_NOTPOS;          /// ������쳣
//    } else if  (HghErr[BSP_ID_EVEN] == TRUE) {        /// cy  101015  m
    } else if ((HghErr[BSP_ID_EVEN] == TRUE) && (COM_9511MAIsInOrg == TRUE)) {
        rt  =   COM_8_RES_EVE_NOTPOS;          /// ż����쳣
    } else if  (COM_9511CheckErr == TRUE) {
        rt  =   COM_8_RES_DAT_ERR;             /// ����У�����
    } else {
        rt  =   BSP_DIP_9511Addr;               /// ��ȷ
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
    
    /// �ɼӵȴ�0.3S        /// !!! cy  100908
    
//    COM_HghUpdate(id);    /// cy  110304
}

/// ���˳�������ı�״̬
static  void  COM_9511UpStByExitP (void)
{
    u32     i;
 COM_9511SetGreALL(0,0);   
//��Ϊֻ�в���״̬������ֻ�Բ����״̬���д���    
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
        default:                                /// ����״̬����
			COM_UpStNow[i]=COM_UP_ST_NORMAL;	
            break;
        }
		COM_9511TangkongPatch[i]=0;
    }
}

/// ����벹����ı�״̬���Ǳ���������õģ�
static  void  COM_9511UpStByEnterP (void)
{
    u32     i;
    
    
    for (i = 0; i < BSP_DEVICE_ID_N; i++) {
        switch (COM_UpStNow[i]) {
        case COM_UP_ST_LOCK:
            COM_9511UpStChange((BSP_ID_TypeDef)i, COM_UP_ST_PATCH_LOCK);//�������ֻ�ı���LED�ͽ�Ƭ���LED״̬��û�иı���ߡ�
            break;
        case COM_UP_ST_NORMAL:
            COM_9511UpStChange((BSP_ID_TypeDef)i, COM_UP_ST_PATCH_IDLE);
            break;
        case COM_UP_ST_BROKEN:
//            COM_9511UpStChange((BSP_ID_TypeDef)i, COM_UP_ST_PATCH_NORMAL);    !!! cy 100904
            COM_9511UpStChange((BSP_ID_TypeDef)i, COM_UP_ST_PATCH_BROKEN);
            break;
        default:                                /// ����״̬����
            break;
        }
		COM_9511TangkongPatch[i]=2;
    }
}

/// ������ת�����ı�״̬
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
        default:                                /// ����״̬����
            break;
        }
    }
}

/// ����߶��ı�״̬
static  void  COM_9511UpStByBroken (BSP_ID_TypeDef id)
{
    switch (COM_UpStNow[id]) {
    case COM_UP_ST_NORMAL:
        COM_9511UpStChange(id, COM_UP_ST_BROKEN);
        break;
    case COM_UP_ST_PATCH_NORMAL:
        COM_9511UpStChange(id, COM_UP_ST_PATCH_BROKEN);
        break;
    default:                                /// ����״̬����
        break;
    }
}
u8 Test_Status[2]={1,1};
/// ���ͷ������λ���ı�״̬
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
    default:                                /// ����״̬����
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

/// ���ͷ������λ���ı�״̬
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
    default:                                /// ����״̬����
	if((TK_Org[id]==0)&&(BSP_SWI_FiGet(id)  == BSP_SWITCH_LOW)&&(IN_HDKey_GetSt((BSP_ID_TypeDef)id)!=HDKEY_ST_DN))
	{	COM_TKIsEnable[id]=TRUE;
		COM_Tangkong_WorkCmd=MOTO_CUT_D;
        COM_HghIsAim[id]    =   COM_HghUpdate_key(id);
		TK_Org[id]=1;	
	}
		
        break;
    }
}

/// ���ͷ������λ���ı�״̬
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
    default:                                /// ����״̬����
        break;
    }
   //if(COM_9511IsTset==TRUE)
   
}



/// -----------------------------------------------------------------------------------------------------
/// @brief  �ж��յ��������Ƿ�Ϊ��ַ, ����9511
/// @param  data : ��Ҫ�жϵ�����
/// @retval : �Ƿ�
/// -----------------------------------------------------------------------------------------------------
static  bool  COM_9511IsAddr (u16 data)
{
    data   &=   COM_MSK_9511_WHOLE;
    if ((data >> COM_BIT_ADDR_9511) == 0) {	//9511�İ��ַ��0x01~0x0f����������7Ϊ��0��924�ĵ�ַ�Ǵ�0xda~0xff
        return  TRUE;
    } else {
        return  FALSE;
    }
}

///// -----------------------------------------------------------------------------------------------------
///// @brief  �ж��յ��������Ƿ�Ϊ��ַ, ����924
///// @param  data : ��Ҫ�жϵ�����
///// @retval : �Ƿ�
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
/// @brief  �ж��յ��������Ƿ�Ϊ��Ч��ַ, ����9511
/// @param  data : ��Ҫ�жϵ�����
/// @retval : �Ƿ�
/// -----------------------------------------------------------------------------------------------------
static  bool  COM_9511IsValidAddr (u16 data,u16 data2)
{
    data   &=   COM_MSK_9511_WHOLE;	//���������0xFF,��0x71~0x7F�����ܹ�����ס���ֵ
    if ((data == COM_9511_ADDR_ALL) || (data == BSP_DIP_9511Addr)) {//COM_9511_ADDR_ALL��0,9511������ַ
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

/// ��У���(�ۼƺ͵ĵ�7λ)
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
    /// iFrame ��ʾ�յ���֡���ݳ��ȣ�
    ///     0                       -   ֡��ʼ, ��ʱӦ���յ�ַ, ������Ч���ݺ� +1
    ///     1 ~ (���ݳ��� + 1)      -   ��ʱӦ�������������, ����� +1
    ///     (���ݳ��� + 2)          -   ֡������ȫ, ����ִ�к���������
    static  u32     iFrame  =   0;
            u16     cmd;
            u32     iTable;
            u8      len;
            u16    *pData;
            bool    IsMyCmd;
    static  u16     buf[COM_MAX_FRAME_LEN];
    
    
    if (COM_9511IsAddr(rx)) {                                   /// �յ���ַ
        if (iFrame == 0) {                                      /// �������, ֡ͬ��
            ;
        } else {                                                /// �쳣���, ֡δͬ��
            iFrame  =   0;                                      /// ǿ��ͬ��, ��֡��ʼ, ������
            COM_ERR_ONE_TIME;                                   /// ָʾ�쳣һ��
        }
        buf[iFrame++]   =   rx;                                 /// �����ַ, ֡����+1
        
    } else {                                                    /// �յ��ǵ�ַ, �����������
        if (iFrame != 0) {                                      /// �������֡��ʼ, ���������, ֡ͬ��
            buf[iFrame++]   =   rx;                             /// �������ݻ�����
            cmd             =   buf[1];                         /// ����Ϊ�յ��ĵ�2����
            
/// ------- cy 101007   a   -------                             /// ��ΪЭ����0x80��0xFF, ���Բ��Ӵ˴�Ҳ��
//            if ((cmd >= COM_9511Tbl[0].cmd) && (cmd <= COM_9511Tbl[COM_LEN_TAB9511 - 1].cmd)) {
/// ------- cy 101007   a   -------
            
            iTable          =   cmd - COM_9511Tbl[0].cmd;       /// ͨ������, �õ����λ��
            len             =   COM_9511Tbl[iTable].len;        /// ���ݳ���
            if (iFrame == len + 2) {                            /// ֡������ȫ
                               
                /// !!! ��Ϊ�����н�Ƭ����, ���Բ����жϴ��������ٶ��ϻ���һ�ֻ�ͷ����Ч
                /// Ҳ�����жϵ�ַ, ��Ƭ��ͻ�ͷ���ַ��ͬ
                if (COM_9511IsValidAddr(buf[0],buf[1])) {              /// ��Ч��ַ��ִ��
                    pData   =   &buf[2];
/// �����°幦�ܵ���ִ�к���������ֻ���Ƕ�ѡһ������˵һ������ȥͬʱִ�����»�ͷ�ĺ���
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
                    /// ����У����ֵ, �����ݸ�����Ϊ��, ����У������, �����Ǳ�������
                    if ((len != 0) && (cmd != 0x00d1) && (IsMyCmd != FALSE)) {
                        COM_9511CheckSum    =   COM_9511GetCheckSum(pData, len);
                        COM_9511CheckNum    =   len & COM_MSK_9511_DATA;
                    } else {
                        COM_9511CheckSum    =   0;
                        COM_9511CheckNum    =   0;
                    }
                } else {                                        /// ��Ч��ַ��ִ��
                        ;
                }
                
/// !!! cy 100906

                iFrame  =   0;                                  /// ��Ϊ֡��ʼ
            } else {                                            /// ֡����δ��ȫ, �򲻴���
                ;
            }
            
/// ------- cy 101007   a   -------                             /// ��ΪЭ����0x80��0xFF, ���Բ��Ӵ˴�Ҳ��
//            } else {
//                COM_ERR_ONE_TIME;                               /// ��ΪЭ���0xFF, ���Բ�������Э��, ������
//            }
/// ------- cy 101007   a   -------
            
        } else {                                                /// ��֡��ʼ, ���쳣���, ֡δͬ��
            COM_ERR_ONE_TIME;                                   /// ָʾ�쳣һ��
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
    
    
    if (BSP_DIP_BDFunc == BSP_DIP_BD_FUNC_UP) {     /// �ϻ�ͷ��Ŵ�����
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
                default:                                /// ����״̬����
                    break;
                }
            }
        }
    }
}

static  void  COM_9511Init (void)
{
    u32     i;
    
    
    if (BSP_DIP_BDFunc == BSP_DIP_BD_FUNC_UP) {     /// �ϻ�ͷ�����Ҫ��ʼ��״̬
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
            /// ��ˢ�����, ��߲��ܶ�, ֱ�����ظı������
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
						if (COM_IsFault[id] ==FALSE)//��ͷû�йر�
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
	BitStreamDecode(ds3_src,8,buf_tmp); //����
	crypto_des3_decrypt(&ds3_crypto_des3_ctx_t,
                    ds3_dst,
                    ds3_src);//����
	//�ж��Ƿ����
	 if((ds3_dst[4]!=0xff)||(ds3_dst[5]!=0xff)||(ds3_dst[6]!=0xff)||(ds3_dst[7]!=0xff))//�Ǽ�������
	 {
	 	//�ӵ����ڶ���ҳȡ���ݣ����9511�Ѿ������ˣ��ж������Ƿ�һ��,��һ�����ý�ֹ����
		//9511û�м��ܣ�ֱ���˳�
		MAIN_SECURITY_STAT=TRUE;
	 }
	 for(i=0;i<4;i++)
	 {
	 	Main_Securit[i]=ds3_dst[4+i];
	 }
	 if(SECURITY_9511_STAT==TRUE)//9511����
	 {
	 	//�޸�����Ȩ��
		//����һ��,�޸���������Ϊ0,9511����
		//���벻һ�£����޸�����Ȩ�ޣ�9511������
			if((MAIN_Sec_ID[0] !=Main_Securit[0])||(MAIN_Sec_ID[1]!=Main_Securit[1])||
			   (MAIN_Sec_ID[2]!=Main_Securit[2])||(MAIN_Sec_ID[3]!=Main_Securit[3]))
				{
					TreahHold_9511=5;
					Secrit_For_LED=1;
				}	
	//	 }
		 else //����һ������
		 {
		 	TreahHold_9511=0;
		 	Secrit_For_LED=0;
		 }
	// }
	 }
	 //9511���ܣ����ز����ܣ����޸�����Ȩ�ޣ�9511��������
	for(i=0;i<4;i++)
	{
		ds3_src_tmp[i]=~ds3_dst[i];
	}
	Confirm_status=1;
	
}
extern void  	Get_Main_ID(void);//������ؼ���ʱ��д���4�ֽ�����
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
	for(i=0;i<10;i++)//��ȡ����
	 {
		tmp_in_buf[i]=*(u8*)(buf+i);
	 }
	BitStreamDecode(ds3_src,8,tmp_in_buf); //����
	crypto_des3_decrypt(&ds3_crypto_des3_ctx_t,
                    ds3_dst,
                    ds3_src);//����
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
		 case 3://��һ�������Ч���ڶ��鲻����
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
