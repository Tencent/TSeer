/**
 * Tencent is pleased to support the open source community by making Tseer available.
 *
 * Copyright (C) 2018 THL A29 Limited, a Tencent company. All rights reserved.
 * 
 * Licensed under the BSD 3-Clause License (the "License"); you may not use this file except
 * in compliance with the License. You may obtain a copy of the License at
 * 
 * https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software distributed 
 * under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 */

package com.qq.seer.common.codec;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import javax.crypto.Cipher;
import javax.crypto.SecretKey;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.DESKeySpec;
import javax.crypto.spec.IvParameterSpec;
import java.io.UnsupportedEncodingException;
import java.net.URLDecoder;
import java.net.URLEncoder;

public class CodecUtils {
	


    private static Logger log = LoggerFactory.getLogger(CodecUtils.class);


    /**
     * URL编码
     *
     * @param url     URL
     * @param charset 字符集
     * @return 编码结果
     */
    public static String encodeWebURL(String url, String charset) {
        String result = null;
        try {
            result = URLEncoder.encode(url, charset);
        } catch (UnsupportedEncodingException e) {
            log.error("encode error, url=[" + url + "], charset=[" + charset + "]", e);
            return null;
        }

        return result.replace("+", "%20").replace("-", "%2d");
    }

    /**
     * URL解码
     *
     * @param url     URL
     * @param charset 字符集
     * @return 解码结果
     */
    public static String decodeWebURL(String url, String charset) {
        try {
            return URLDecoder.decode(url, charset);
        } catch (UnsupportedEncodingException e) {
            log.error("decode error, url=[" + url + "], charset=[" + charset + "]", e);
            return null;
        }
    }

    /**
     * URL编码
     *
     * @param url URL
     * @return 编码结果
     */
    public static String encodeWebURL(String url) {
        return encodeWebURL(url, "UTF-8");
    }




    /**
     * DES加密
     * 对应Perl代码
     * my $cipher = Crypt::CBC->new({
     * 'key' => $key,
     * 'cipher' => 'DES',
     * 'padding' => 'standard',
     * 'iv' => '01234567',
     * 'header' => 'none',
     * 'keysize' => '8',
     * 'blocksize' => '8',
     * 'literal_key' => '1'
     * });
     *
     * @param data 数据
     * @param key  密钥，通过UTF-8来获取字节，注意只能是8字节
     * @return 加密结果
     */
    public static byte[] encryptDES(byte[] data, String key) {
        try {
            byte[] keyBytes = key.getBytes("UTF-8");
            if (keyBytes.length != 8) {
                throw new Exception("length of key is not 8 in UTF-8");
            }

            byte[] iv = "01234567".getBytes();
            IvParameterSpec ivSpec = new IvParameterSpec(iv);

            DESKeySpec dks = new DESKeySpec(keyBytes);
            SecretKeyFactory keyFactory = SecretKeyFactory.getInstance("DES");
            SecretKey securekey = keyFactory.generateSecret(dks);
            Cipher cipher = Cipher.getInstance("DES/CBC/PKCS5Padding");
            cipher.init(Cipher.ENCRYPT_MODE, securekey, ivSpec);
            return cipher.doFinal(data);
        } catch (Exception e) {
            log.error("DES encrypt error", e);
            return null;
        }
    }

    /**
     * DES解密
     * Perl相关代码参见encryptDES函数
     *
     * @param data 数据
     * @param key  密钥，通过UTF-8来获取字节，注意只能是8字节
     * @return 解密结果
     */
    public static byte[] decryptDES(byte[] data, String key) {
        try {
            byte[] keyBytes = key.getBytes("UTF-8");
            if (keyBytes.length != 8) {
                throw new Exception("length of key is not 8 in UTF-8");
            }

            byte[] iv = "01234567".getBytes();
            IvParameterSpec ivSpec = new IvParameterSpec(iv);

            DESKeySpec dks = new DESKeySpec(keyBytes);
            SecretKeyFactory keyFactory = SecretKeyFactory.getInstance("DES");
            SecretKey securekey = keyFactory.generateSecret(dks);
            Cipher cipher = Cipher.getInstance("DES/CBC/PKCS5Padding");
            cipher.init(Cipher.DECRYPT_MODE, securekey, ivSpec);
            return cipher.doFinal(data);
        } catch (Exception e) {
            log.error("DES decrypt error", e);
            return null;
        }
    }




}
