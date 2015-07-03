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
 * @file    bitstream.c
 * @ingroup
 * @brief
 *
 * @author  Damien(min.dai.chn@gmail.com)
 * @date    2013/11/14
 * @version 0.1
 */


/*******************************************************************************
 *
 *  header files.
 *
 *******************************************************************************
 */

#include "bitstream.h"


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
 * @param pStream 位流保存地址
 */
void BitStreamEncode(const unsigned char* pData, unsigned char* pStream)
{
  unsigned int Bits  = 0;
  unsigned int Data  = 0;
  unsigned int Index = 0;
  unsigned int Count = 0;


  while (Index < 8) {
    Bits  += 8;
    Data <<= 8;
    Data  += pData[Index++];
    while (Bits >= 7) {
      pStream[Count++] = 0x80 | (Data >> (Bits - 7));
      Bits -= 7;
    }
  }

  if (Bits > 0) {
    pStream[Count++] = 0x80 | (Data << (7 - Bits));
  }
}

/**
 * @brief 将位流解码为数据
 *
 * @author Damien(2013/11/14)
 *
 * @param pData   数据保存地址
 * @param pStream 位流保存地址
 */
void BitStreamDecode(unsigned char* pData, const unsigned char* pStream)
{
  unsigned int Bits  = 0;
  unsigned int Data  = 0;
  unsigned int Index = 0;
  unsigned int Count = 0;


  while (Index < 8) {
    Bits  += 7;
    Data <<= 7;
    Data  += pStream[Count++] & 0x7F;
    while (Bits >= 8) {
      pData[Index++] = Data >> (Bits - 8);
      Bits -= 8;
    }
  }
}


/*******************************************************************************
 *
 *  end of file.
 *
 *******************************************************************************
 */
