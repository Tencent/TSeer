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

package com.qq.cloud.router.client.util.stat;

import com.qq.cloud.router.client.util.ApiStatUtils;
import com.qq.tars.common.util.StringUtils;
import com.qq.tars.support.stat.ProxyStat;
import com.qq.tars.support.stat.ProxyStatHead;

public class ApiProxyStat {
	
	private static final ApiProxyStat instance = new ApiProxyStat();
	
	
	public static ApiProxyStat getInstance() {
		return instance;
	}
	
	private ApiProxyStat() {
	}

	public void addInvokeTime(ProxyStat proxyStat, String moduleName, String objectName, String setDivision, String masterSet, String methodName,
            String masterIp, String slaveIp, int slavePort, int result, long costTimeMill) {
		ProxyStatHead head = null;
		if (masterSet != null) {
			String[] masterSetInfo  = StringUtils.split(masterSet, ".");
			if (masterSetInfo != null && masterSetInfo.length == 3) {
				head = ApiStatUtils.getHead(moduleName, objectName, methodName, masterIp, slaveIp, slavePort, result, masterSetInfo[0], masterSetInfo[1], masterSetInfo[2], setDivision);
			} else {
				head = ApiStatUtils.getHead(moduleName, objectName, methodName, masterIp, slaveIp, slavePort, result, "", "", "", setDivision);
			}
		} else {
			head = ApiStatUtils.getHead(moduleName, objectName, methodName, masterIp, slaveIp, slavePort, result, "", "", "", setDivision);
		}
		proxyStat.addInvokeTime(head, costTimeMill, result);
	}
}
