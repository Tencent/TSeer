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

package com.qq.cloud.router.client.util;

import com.qq.tars.client.ClientVersion;
import com.qq.tars.common.util.StringUtils;
import com.qq.tars.support.stat.ProxyStatHead;

public class ApiStatUtils {
	
    public final static int MAX_MASTER_NAME_LEN = 127;
	
	public static ProxyStatHead getHead(String masterName, String slaveName, String interfaceName, String masterIp, String slaveIp, int slavePort, int returnValue, String masterSetName, String masterSetArea, String masterSetID, String slaveSetDivision) {
		 if (masterSetName == null || masterSetName.length() == 0) {
	            masterName = masterName + "@" + ClientVersion.getVersion();
	        } else {
	            String shortMasterName = getShortModuleName(masterName);
	            masterName = shortMasterName + "." + masterSetName + masterSetArea + masterSetID + "@" + ClientVersion.getVersion();
	        }

	        String slaveSetName = "";
	        String slaveSetArea = "";
	        String slaveSetID = "";
	        int index = slaveName.lastIndexOf(".");
	        if (index > 0) {
               slaveName = slaveName.substring(0, slaveName.lastIndexOf("."));
	        }
	        boolean setExist = false;
	        if (slaveSetDivision != null && slaveSetDivision.length() > 0) {
	            String[] tempStrs = StringUtils.split(slaveSetDivision, ".");
	            if (tempStrs.length == 3) {
	                slaveSetName = tempStrs[0];
	                slaveSetArea = tempStrs[1];
	                slaveSetID = tempStrs[2];
	                setExist = true;
                    String shortSlaveName = getShortModuleName(slaveName);
	                slaveName = shortSlaveName + "." + slaveSetName + slaveSetArea + slaveSetID;
	            }
	        }

	        if (!setExist) {
	            slaveName = trimAndLimit(slaveName, MAX_MASTER_NAME_LEN);
	        }
	        ProxyStatHead head = new ProxyStatHead(masterName, slaveName, interfaceName, masterIp, slaveIp, slavePort, returnValue, slaveSetName, slaveSetArea, slaveSetID, ClientVersion.getVersion());
	        return head;
	}
	
	private static String getShortModuleName(String moduleName) {
        String shortModuleName = "";
        if (moduleName != null) {
            int pos = moduleName.indexOf('.');
            if (pos >= 0) {
                shortModuleName = moduleName.substring(pos + 1);
            }
        }
        return shortModuleName;
    }

    private static String trimAndLimit(String str, int limit) {
        String ret = "";
        if (str != null) {
            str = str.trim();
            if (str.length() > limit) {
                str = str.substring(0, limit);
            }
            ret = str;
        }
        return ret;
    }
}
