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
 * @brief   ��ȫϵͳ��Կ����ģ��
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
 * @brief ��ʼ��ϵͳ��Χ��Կ
 *
 * @author Damien(2013/11/18)
 *
 * @param pPeriKey ��Χ��Կ�����ַ
 */
void Des3PeriKeyInit(crypto_des3_ctx_t* pPeriKey,unsigned char *Key,unsigned char KeySize);


/*******************************************************************************
 *
 *  end of file.
 *
 *******************************************************************************
 */
#endif /* __DES3KEY_H__ */
