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
int BitStreamEncode(const unsigned char* pData, int DataLen, unsigned char* pStream);

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
int BitStreamDecode(unsigned char* pData, int DataLen, const unsigned char* pStream);


/*******************************************************************************
 *
 *  end of file.
 *
 *******************************************************************************
 */
#endif /* __BITSTREAM_H__ */
