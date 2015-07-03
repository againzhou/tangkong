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
 * @file    crypto_des3.h
 * @ingroup crypto
 * @brief   DES & Triple DES �����㷨��
 *
 * @author  Damien(min.dai.chn@gmail.com)
 * @date    2011/12/22
 * @version 0.1
 */

#ifndef _CRYPTO_DES3_H_
#define _CRYPTO_DES3_H_


/*******************************************************************************
 *
 *  global definitions.
 *
 *******************************************************************************
 */

#define CRYPTO_DES_KEY_SIZE            8
#define CRYPTO_DES_EXPKEY_WORDS        32
#define CRYPTO_DES_BLOCK_SIZE          8

#define CRYPTO_DES3_KEY_SIZE           (3 * CRYPTO_DES_KEY_SIZE)
#define CRYPTO_DES3_EXPKEY_WORDS       (3 * CRYPTO_DES_EXPKEY_WORDS)
#define CRYPTO_DES3_BLOCK_SIZE         CRYPTO_DES_BLOCK_SIZE

/**
 * @brief Triple DES �����㷨�ﾳ�ṹ��
 */
typedef struct {
  unsigned long expkey[CRYPTO_DES3_EXPKEY_WORDS];       //!< ��չ��Կ������
} crypto_des3_ctx_t;


/*******************************************************************************
 *
 *  public functions.
 *
 *******************************************************************************
 */

/**
 * @brief ���� Triple DES �����㷨��Կ��
 *
 * @author Damien(2011/12/22)
 *
 * @param p_ctx   Triple DES �����㷨�ﾳ��
 * @param p_key   ������Կ��
 * @param key_len ��Կ���ȡ�
 *
 * @return int ����ִ�н����
 * @retval  0 ִ�гɹ���
 * @retval !0 ִ�г���
 */
int
crypto_des3_set_key(crypto_des3_ctx_t*   p_ctx,
                    const unsigned char* p_key,
                    unsigned long        key_len);

/**
 * @brief �����ݶν��� Triple DES �㷨���ܡ�
 *
 * @author Damien(2011/12/22)
 *
 * @param p_ctx  Triple DES �����㷨�ﾳ��
 * @param p_dst  ������ı����ַ��
 * @param p_src  �������ı����ַ��
 *
 * @note ���ݶεĳ���Ϊ 8 ���ֽڡ�
 */
void
crypto_des3_encrypt(const crypto_des3_ctx_t* p_ctx,
                    unsigned char*           p_dst,
                    const unsigned char*     p_src);

/**
 * @brief �����ݶν��� Triple DES �㷨���ܡ�
 *
 * @author Damien(2011/12/22)
 *
 * @param p_ctx  Triple DES �����㷨�ﾳ��
 * @param p_dst  ������ı����ַ��
 * @param p_src  �������ı����ַ��
 *
 * @note ���ݶεĳ���Ϊ 8 ���ֽڡ�
 */
void
crypto_des3_decrypt(const crypto_des3_ctx_t* p_ctx,
                    unsigned char*           p_dst,
                    const unsigned char*     p_src);


/*******************************************************************************
 *
 *  end of file.
 *
 *******************************************************************************
 */
#endif  /* _CRYPTO_DES3_H_ */
