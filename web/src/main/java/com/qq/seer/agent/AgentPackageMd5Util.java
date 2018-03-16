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

package com.qq.seer.agent;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

import com.qq.seer.common.log.LogUtils;

public class AgentPackageMd5Util {
	
	public static String getFileMd5(File file) {
		String md5 = "";
		try {
			md5 = streamToMD5(new FileInputStream(file));
		} catch (Exception e) {
			String error = "AgentPackageMd5Util getFileMd5 Exception : " + e.getMessage();
			LogUtils.localLogError(error, e);
		}
		return md5;
	}
	
	
	public static String streamToMD5(InputStream inputStream) throws NoSuchAlgorithmException, IOException {
		String md5 = "";
		MessageDigest mdTemp = MessageDigest.getInstance("MD5");
		byte[] buffer = new byte[1024];
		int numRead = 0;
		while ((numRead = inputStream.read(buffer)) > 0) {
			mdTemp.update(buffer, 0, numRead);
		}
		md5 = toHexString(mdTemp.digest());
		return md5;
	}
	
	private static String toHexString(byte[] md) {
		char hexDigits[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
				'a', 'b', 'c', 'd', 'e', 'f' };
		int j = md.length;
		char str[] = new char[j * 2];
		for (int i = 0; i < j; i++) {
			byte byte0 = md[i];
			str[2 * i] = hexDigits[byte0 >>> 4 & 0xf];
			str[i * 2 + 1] = hexDigits[byte0 & 0xf];
		}
		return new String(str);
	}

}
