/********************************************************************************
 *                  Watchdata Platform Development Center                       *
 *                         Watchdata Confidential                               *
 *              Copyright (c) Watchdata System CO., Ltd. 2013 -                 *
 *                          All Rights Reserved                                 *
 *                        http://www.watchdata.com                              *
 ********************************************************************************/

/* Create Date: 2013/11/14
 *
 * Modify Record:
 *  <date>      <author>    <version>       <desc>
 * -------------------------------------------------------------------
 *  2013/11/14  Damien      0.1             Create The File.
 */

/**
 * @file    bitstream.h
 * @ingroup
 * @brief
 *
 * @author  Damien(min.dai.chn@gmail.com)
 * @date    2013/11/14
 * @version 0.1
 */

#ifndef __BITSTREAM_H__
#define __BITSTREAM_H__


/*******************************************************************************
 *
 *  public functions.
 *
 *******************************************************************************
 */

/**
 * @brief 将数据编码为位流
 *
 * @author Damien(2013/11/14)
 *
 * @param pData   数据保存地址
 * @param DataLen 数据长度
 * @param pStream 位流保存地址
 *
 * @retval int 返回位流长度
 */
int BitStreamEncode(const unsigned char* pData, int DataLen, unsigned char* pStream);

/**
 * @brief 将位流解码为数据
 *
 * @author Damien(2013/11/14)
 *
 * @param pData   数据保存地址
 * @param DataLen 数据长度
 * @param pStream 位流保存地址
 *
 * @retval int 返回位流长度
 */
int BitStreamDecode(unsigned char* pData, int DataLen, const unsigned char* pStream);


/*******************************************************************************
 *
 *  end of file.
 *
 *******************************************************************************
 */
#endif /* __BITSTREAM_H__ */
