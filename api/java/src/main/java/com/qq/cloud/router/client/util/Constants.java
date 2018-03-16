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

import java.util.HashMap;
import java.util.Map;

import com.qq.cloud.router.client.RouterConfig;

public class Constants {

	public static final String LOGGER_NAME = "Seer";

	public static final String AGENT_ROUTERS_CACHE_FILE_PATH = "/usr/local/app/Seer/SeerAgent/data/routersCache/";

	public static final String API_CACHE_FILE_PATH = "/usr/local/app/Seer/SeerJavaApi/data/routersCache/" + (RouterConfig.seerApiKey.isEmpty() ? "temp/" : RouterConfig.seerApiKey +  "/");
	
	public static final String AGENT_ROUTER_CACHE_FILE_PATH = "/usr/local/app/Seer/SeerJavaApi/data/routerCache/" + (RouterConfig.seerApiKey.isEmpty() ? "temp/" : RouterConfig.seerApiKey +  "/");

	public static final String CACHE_FILE_ROOT = "/cache";

	public static final String DEFAULE_LOG_PATH = "/usr/local/app/Seer/SeerJavaApi/log/";

	public static final long THREAD_CALL_INTERVAL = 15000;

	public static final String SEER_API_VERSION = "SeerRawJavaAPI_v1.0";
	public static final String SEER_AGENT_VERSION = "SeerAgentJavaAPI_v1.0";
	
	public static final Map<String, String> defaultAgentCtx = new HashMap<String, String>();
	static {
		defaultAgentCtx.put("SEERAPI_KEY", RouterConfig.seerApiKey);
		defaultAgentCtx.put("SEERAPI_VERSION", Constants.SEER_AGENT_VERSION);
	}
	
	public static final Map<String, String> defaultApiCtx = new HashMap<String, String>();
	static {
		defaultApiCtx.put("SEERAPI_KEY", RouterConfig.seerApiKey);
		defaultApiCtx.put("SEERAPI_VERSION", Constants.SEER_API_VERSION);
	}
}
