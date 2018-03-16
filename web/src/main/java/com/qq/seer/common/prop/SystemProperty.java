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

package com.qq.seer.common.prop;

import java.util.HashMap;
import java.util.Map;

import com.qq.seer.common.config.PropertiesUtils;

/**
 * 系统配置
 *
 */
public class SystemProperty extends PropertiesUtils {
	
	private static Map<String, String> systemConfig = new HashMap<String, String>();
	private static String fileName = "system.properties";
	
	
	static {
		init();
	}
	
	private static void init() {
		systemConfig = getPropertyMap(fileName);
	}
	
	
	public static String getValueByName(String name) {
		if (name ==null || name.length() < 1) {
			return "";
		}
		return systemConfig.get(name);
	}

}
