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
 * @brief 将 32 位宽处理器格式数值转换为小端格式。
 *
 * @author Damien(2011/12/22)
 *
 * @param word 输入数值。
 *
 * @return unsigned long 返回转换后的数值。
 */
static INLINE unsigned long cpu_to_le32(unsigned long word)
{
  return (word);
}

/**
 * @brief 将 32 位宽的小端格式数值转换为处理器格式。
 *
 * @author Damien(2011/12/22)
 *
 * @param word 输入数值。
 *
 * @return unsigned long 返回转换后的数值。
 */
static INLINE unsigned long le32_to_cpu(unsigned long word)
{
  return (word);
}

/**
 * @brief 向右循环位移 32 位宽的数值。
 *
 * @author Damien(2011/12/22)
 *
 * @param word  目标数值。
 * @param shift 位移位数。
 *
 * @return unsigned long 返回位移后的数值。
 */
static INLINE unsigned long ror32(unsigned long word, unsigned int shift)
{
  return ((word >> shift) | (word << (32 - shift)));
}

/**
 * @brief 向左循环位移 32 位宽的数值。
 *
 * @author Damien(2011/12/22)
 *
 * @param word  目标数值。
 * @param shift 位移位数。
 *
 * @return unsigned long 返回位移后的数值。
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
