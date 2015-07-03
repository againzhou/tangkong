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
 * @brief   DES & Triple DES 加密算法。
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
 * @brief Triple DES 加密算法语境结构。
 */
typedef struct {
  unsigned long expkey[CRYPTO_DES3_EXPKEY_WORDS];       //!< 扩展密钥保存区
} crypto_des3_ctx_t;


/*******************************************************************************
 *
 *  public functions.
 *
 *******************************************************************************
 */

/**
 * @brief 设置 Triple DES 加密算法密钥。
 *
 * @author Damien(2011/12/22)
 *
 * @param p_ctx   Triple DES 加密算法语境。
 * @param p_key   输入密钥。
 * @param key_len 密钥长度。
 *
 * @return int 返回执行结果。
 * @retval  0 执行成功。
 * @retval !0 执行出错。
 */
int
crypto_des3_set_key(crypto_des3_ctx_t*   p_ctx,
                    const unsigned char* p_key,
                    unsigned long        key_len);

/**
 * @brief 对数据段进行 Triple DES 算法加密。
 *
 * @author Damien(2011/12/22)
 *
 * @param p_ctx  Triple DES 加密算法语境。
 * @param p_dst  输出密文保存地址。
 * @param p_src  输入明文保存地址。
 *
 * @note 数据段的长度为 8 个字节。
 */
void
crypto_des3_encrypt(const crypto_des3_ctx_t* p_ctx,
                    unsigned char*           p_dst,
                    const unsigned char*     p_src);

/**
 * @brief 对数据段进行 Triple DES 算法解密。
 *
 * @author Damien(2011/12/22)
 *
 * @param p_ctx  Triple DES 加密算法语境。
 * @param p_dst  输出明文保存地址。
 * @param p_src  输入密文保存地址。
 *
 * @note 数据段的长度为 8 个字节。
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
