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

package com.qq.seer.http;

import java.util.Date;
import java.util.HashMap;
import java.util.Map;

import org.json.JSONException;
import org.json.JSONObject;

import com.qq.seer.common.http.HttpUtils;
import com.qq.seer.common.log.LogUtils;
import com.qq.seer.common.prop.SystemProperty;
import com.qq.seer.common.string.StringUtils;

public class SeerInterface {
	private static final String url = SystemProperty.getValueByName("seer.api.url");
	private static String int_name = "interface_name";
	private static String int_params = "interface_params";
	
	
	public static JSONObject callSeerInterface(long logId, JSONObject paramsObj, String interfaceName) throws JSONException {
		
		try {
			long time = 0;
			Map<String, String> paramsMap = new HashMap<String, String>();
			paramsMap.put(int_name, interfaceName);
			paramsMap.put(int_params, paramsObj.toString());
			
			LogUtils.localLogInfo(logId, "[callSeerInterface] url : " +  url );

			LogUtils.localLogInfo(logId, "[callSeerInterface][" + interfaceName + "] paramsMap :  " + paramsMap.toString());

			Date beginTime = new Date();
			String result = StringUtils.trim(HttpUtils.post(url, paramsMap, "UTF-8", "UTF-8"));
			Date endTime = new Date();
			time = Math.abs(endTime.getTime() - beginTime.getTime());
			
			LogUtils.localLogInfo(logId, "[callSeerInterface] [" + interfaceName + "] result :  " + result + ",time=" + time);

			if (null == result || result.length() < 1) {
				JSONObject resultObj = new JSONObject();
				resultObj.put("ret_code", -1);
				resultObj.put("err_msg", "调用callSeerInterface失败,result = " + result + ",interface_name=" + interfaceName );
				resultObj.put("data", new JSONObject());
				return resultObj;
			} else {
				JSONObject resultObj = new JSONObject(result);
				return resultObj;
			}
		} catch (Exception e) {
			String error = "[callSeerInterface] Exception : " + e.getMessage();
			LogUtils.localLogError(logId, error, e);
			JSONObject resultObj = new JSONObject();
			resultObj.put("ret_code", -1);
			resultObj.put("err_msg", error);
			resultObj.put("data", new JSONObject());
			return resultObj;
		}
	}

}
