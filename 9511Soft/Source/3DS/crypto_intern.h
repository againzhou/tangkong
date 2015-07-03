/********************************************************************************
 *                  Watchdata Platform Development Center                       *
 *                         Watchdata Confidential                               *
 *              Copyright (c) Watchdata System CO., Ltd. 2009 -                 *
 *                          All Rights Reserved                                 *
 *                        http://www.watchdata.com                              *
 ********************************************************************************/

/* Create Date: 2011/12/22
 *
 * Modify Record:
 *  <date>      <author>    <version>       <desc>
 * -------------------------------------------------------------------
 *  2011/12/22  Damien      0.1             Create The File.
 */

/**
 * @file    crypto.h
 * @ingroup crypto
 * @brief
 *
 * @author  Damien(min.dai.chn@gmail.com)
 * @date    2011/12/22
 * @version 0.1
 */

#ifndef _CRYPTO_H_
#define _CRYPTO_H_


/*******************************************************************************
 *
 *  global definitions.
 *
 *******************************************************************************
 */

#undef INLINE
#if defined(__ICCARM__)
#define INLINE inline
#else
#define INLINE
#endif


/*******************************************************************************
 *
 *  public functions.
 *
 *******************************************************************************
 */

/**
 * @brief �� 32 λ��������ʽ��ֵת��ΪС�˸�ʽ��
 *
 * @author Damien(2011/12/22)
 *
 * @param word ������ֵ��
 *
 * @return unsigned long ����ת�������ֵ��
 */
static INLINE unsigned long cpu_to_le32(unsigned long word)
{
  return (word);
}

/**
 * @brief �� 32 λ���С�˸�ʽ��ֵת��Ϊ��������ʽ��
 *
 * @author Damien(2011/12/22)
 *
 * @param word ������ֵ��
 *
 * @return unsigned long ����ת�������ֵ��
 */
static INLINE unsigned long le32_to_cpu(unsigned long word)
{
  return (word);
}

/**
 * @brief ����ѭ��λ�� 32 λ�����ֵ��
 *
 * @author Damien(2011/12/22)
 *
 * @param word  Ŀ����ֵ��
 * @param shift λ��λ����
 *
 * @return unsigned long ����λ�ƺ����ֵ��
 */
static INLINE unsigned long ror32(unsigned long word, unsigned int shift)
{
  return ((word >> shift) | (word << (32 - shift)));
}

/**
 * @brief ����ѭ��λ�� 32 λ�����ֵ��
 *
 * @author Damien(2011/12/22)
 *
 * @param word  Ŀ����ֵ��
 * @param shift λ��λ����
 *
 * @return unsigned long ����λ�ƺ����ֵ��
 */
static INLINE unsigned long rol32(unsigned long word, unsigned int shift)
{
  return ((word << shift) | (word >> (32 - shift)));
}


/*******************************************************************************
 *
 *  end of file.
 *
 *******************************************************************************
 */
#endif  /* _CRYPTO_H_ */
