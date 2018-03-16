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
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;

import com.qq.seer.admin.SeerAdmin;
import com.qq.seer.common.log.LogUtils;
import com.qq.tars.common.support.Holder;
import com.qq.tars.tseer.PushPackageInfo;
import com.qq.tars.tseer.RegistryPrx;

public class SeerAgentProxy {
	
	
	public static void pushPackage(long logId, File file, String packageName, String osType, String version, int intPackageType, String md5) throws Exception {
		
		List<RegistryPrx> regList = SeerAdmin.getInstance().getRegistryPrx();
		if(regList==null) {
			LogUtils.localLogInfo(logId, "pushPackage regList=" + regList);
		} else {
			int size = regList.size();
			if(size == 0) {
				LogUtils.localLogInfo(logId, "pushPackage regList.size=" + size);
			} else {
				for(int i=0; i<size; i++) {
					RegistryPrx prx = regList.get(i);
					pushPackage(logId, prx, file, packageName, osType, version, intPackageType, md5);
				}
			}
		}
	}
	
	
	public static void pushPackage(long logId, RegistryPrx prx, File file, String packageName, String osType, String version, int intPackageType, String md5) throws Exception {
		long size = getFileSize(file);
        int bytesum = 0;
        int byteread = 0;
        int b = 1024 * 1024;
        LogUtils.localLogInfo(logId, "pushPackage packageName=" + packageName + ",md5=" + md5  + ",size=" + size);

        if (file.exists()) { //文件存在时 
            InputStream inStream = new FileInputStream(file);
            byte[] buffer = new byte[b];

            while ((byteread = inStream.read(buffer)) != -1) {

                if (byteread < buffer.length) {

                    byte[] lastBuffer = new byte[byteread];

                    for (int i = 0; i < byteread; i++) {
                        lastBuffer[i] = buffer[i];
                    }
                    buffer = lastBuffer;
                }
                
                pushPackage(logId, prx, file, packageName, osType, version, intPackageType, md5,  bytesum, size, buffer);

                bytesum += byteread; //字节数 文件大小 
                LogUtils.localLogInfo(logId, "bytesum    " + bytesum + "   byteread  " + byteread + "  byte " + buffer.length);

            }
            inStream.close();
        } else {
        	LogUtils.localLogInfo(logId, "pushPackage packageName=" + packageName + ",file.exists=false");
        }
	}
	
	
	public static void pushPackage(long logId, RegistryPrx prx, File file, String packageName, String osType, String version, int intPackageType, String md5, long pos, long size, byte[] vb) throws Exception {
		
		PushPackageInfo pushPackageInfo = new PushPackageInfo();
		pushPackageInfo.setPackageName(packageName);
		pushPackageInfo.setPackage_type(intPackageType);
		pushPackageInfo.setOstype(osType);
		pushPackageInfo.setVersion(version);
		pushPackageInfo.setMd5(md5);
		pushPackageInfo.setUser("seer");
		pushPackageInfo.setPos(pos);
		pushPackageInfo.setSize(size);
		pushPackageInfo.setVb(vb);
	        
		Holder<String> result = new Holder<String>();
		
		LogUtils.localLogInfo(logId, "pushPackage pushPackageInfo=" + pushPackageInfo + ",packageName=" + packageName  + ",pos=" + pos + ",size=" + size);
		int ret = prx.pushPackage(pushPackageInfo, result);
		String resultInfo = result.getValue();
		
		LogUtils.localLogInfo(logId, "pushPackage ret=" + ret + ",resultInfo=" + resultInfo );
		if(ret ==0) {
			
		} else {
			 throw new Exception(" pushPackage fail, ret=" + ret + ",result=" + resultInfo);
		}
	}
	
	
	public static long getFileSize(File file) throws Exception {
        long size = 0;
        size = file.length();
        return size;
    }

}
