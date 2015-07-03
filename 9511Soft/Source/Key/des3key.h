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
 * @file    des3key.h
 * @ingroup
 * @brief   安全系统公钥管理模块
 *
 * @author  Damien(min.dai.chn@gmail.com)
 * @date    2013/11/18
 * @version 0.1
 */

#ifndef __DES3KEY_H__
#define __DES3KEY_H__


/*******************************************************************************
 *
 *  header files.
 *
 *******************************************************************************
 */

#include "../3ds/crypto_des3.h"


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
void Des3PeriKeyInit(crypto_des3_ctx_t* pPeriKey,unsigned char *Key,unsigned char KeySize);


/*******************************************************************************
 *
 *  end of file.
 *
 *******************************************************************************
 */
#endif /* __DES3KEY_H__ */
