/********************************************************************************
 *                  Watchdata Platform Development Center                       *
 *                         Watchdata Confidential                               *
 *              Copyright (c) Watchdata System CO., Ltd. 2013 -                 *
 *                          All Rights Reserved                                 *
 *                        http://www.watchdata.com                              *
 ********************************************************************************/

/* Create Date: 2013/11/18
 *
 * Modify Record:
 *  <date>      <author>    <version>       <desc>
 * -------------------------------------------------------------------
 *  2013/11/18  Damien      0.1             Create The File.
 */

/**
 * @file    des3key.c
 * @ingroup
 * @brief   安全系统公钥管理模块
 *
 * @author  Damien(min.dai.chn@gmail.com)
 * @date    2013/11/18
 * @version 0.1
 */


/*******************************************************************************
 *
 *  header files.
 *
 *******************************************************************************
 */

#include "../Key/des3key.h"


/*******************************************************************************
 *
 *  internal data.
 *
 *******************************************************************************
 */

/* ---------------------------------------------------------------------------- */
// 密钥生成数据
static const unsigned long DES3KEY[] = {
  0x424A33F4, 0x38E8C881, 0x79CD42E0, 0x06B303D4,
  0x5E2F29AA, 0x46B18ACA, 0x3E27D0D9, 0x7D76FDD7,
  0x40CAE7F0, 0xC3401613, 0xD644AD4F, 0xC2403D0D,
  0xD26C7D28, 0xA466B54A, 0x41D916CB, 0xC70145FC,
  0x6AC9B8D8, 0x57FD9E89, 0x6C097B1B, 0x64A47F1C,
  0x43D1AABA, 0x3A007EDE, 0x43D13818, 0x5157E830,
  0x7ABDF7F3, 0x544A3392, 0x7AC928DB, 0x2623EA5F,
  0x5E32FFCE, 0x4B1084F1, 0x78B01005, 0x34C8BE02,
  0xD2ABB304, 0x2D20E259, 0x6CDE406E, 0xA5C48D28,
  0x780A2F37, 0x4838D1BE, 0x6BD0D891, 0x1D23070F,
  0x6BF042B3, 0xE59FEBF7, 0x7B177C20, 0x37F3A9AD,
  0x4271F75A, 0xE7BB5426, 0x3E3D060A, 0xE8517EEC,
  0x7B7DD223, 0x38F11047, 0x5E28F73C, 0xEB247DED,
  0x40EE12C1, 0xEA017B33, 0x5DF3141C, 0xC9F5ADC8,
  0x6D9EE08C, 0x1F2BFE8E, 0x3E6DCAB5, 0xC61FF498,
};

#define COUNT_OF(array)   (sizeof(array) / sizeof(array[0]))


/*******************************************************************************
 *
 *  public functions.
 *
 *******************************************************************************
 */

/**
 * @brief 初始化系统外围公钥
 *
 * @author Damien(2013/11/18)
 *
 * @param pPeriKey 外围公钥保存地址
 */
void Des3PeriKeyInit(crypto_des3_ctx_t* pPeriKey,unsigned char *Key,unsigned char KeySize)
{
  unsigned long Index;
  unsigned long Value = 9;
  //unsigned long Key[CRYPTO_DES3_KEY_SIZE / sizeof(unsigned long)];


  for (Index = 0; Index < KeySize; Index++) {
    Value = DES3KEY[Value % COUNT_OF(DES3KEY)];
    ((unsigned char*)Key)[Index] = (Value >> ((Value % 4) * 8)) & 0xFF;

    if (Value == (Value % COUNT_OF(DES3KEY))) {
      Value += 3;
    }
  }

  crypto_des3_set_key(pPeriKey, Key, KeySize);   // 设置系统外围公钥
}


/*******************************************************************************
 *
 *  end of file.
 *
 *******************************************************************************
 */
