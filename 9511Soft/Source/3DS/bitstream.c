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
 * @brief �����ݱ���Ϊλ��
 *
 * @author Damien(2013/11/14)
 *
 * @param pData   ���ݱ����ַ
 * @param DataLen ���ݳ���
 * @param pStream λ�������ַ
 *
 * @retval int ����λ������
 */
int BitStreamEncode(const unsigned char* pData, int DataLen, unsigned char* pStream)
{
  unsigned int Bits  = 0;
  unsigned int Data  = 0;
  unsigned int Index = 0;
  unsigned int Count = 0;


  while (Index < DataLen) {
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

  return (Count);
}

/**
 * @brief ��λ������Ϊ����
 *
 * @author Damien(2013/11/14)
 *
 * @param pData   ���ݱ����ַ
 * @param DataLen ���ݳ���
 * @param pStream λ�������ַ
 *
 * @retval int ����λ������
 */
int BitStreamDecode(unsigned char* pData, int DataLen, const unsigned char* pStream)
{
  unsigned int Bits  = 0;
  unsigned int Data  = 0;
  unsigned int Index = 0;
  unsigned int Count = 0;


  while (Index < DataLen) {
    Bits  += 7;
    Data <<= 7;
    Data  += pStream[Count++] & 0x7F;
    while (Bits >= 8) {
      pData[Index++] = Data >> (Bits - 8);
      Bits -= 8;
    }
  }

  return (Count);
}


/*******************************************************************************
 *
 *  end of file.
 *
 *******************************************************************************
 */
