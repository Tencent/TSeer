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

package com.qq.cloud.router.client;

import com.qq.cloud.router.client.faulttolerant.RouterNodeAliveConfig;
import com.qq.cloud.router.client.util.stat.StatManager;

public class RouterConfig {

	private String RouterLocater = "Tseer.TseerServer.QueryObj@tcp -h routerserver.Tseer.qq.com -p 9903 -t 50000";
	private String statObj = "tars.TseerStat.StatObj";
	public static String seerApiKey = "";
	private StatManager statManagerImpl = null;
	private RouterNodeAliveConfig routerNodeAliveConfig = new RouterNodeAliveConfig();

	public String getRouterLocater() {
		return RouterLocater;
	}
	public void setRouterLocater(String routerLocater) {
		RouterLocater = routerLocater;
	}
	public String getStatObj() {
		return statObj;
	}
	public void setStatObj(String statObj) {
		this.statObj = statObj;
	}
	public RouterNodeAliveConfig getRouterNodeAliveConfig() {
		return routerNodeAliveConfig;
	}
	public void setRouterNodeAliveConfig(RouterNodeAliveConfig routerNodeAliveConfig) {
		this.routerNodeAliveConfig = routerNodeAliveConfig;
	}
	public static String getSeerApiKey() {
		return seerApiKey;
	}
	public static void setSeerApiKey(String seerApiKey) {
		RouterConfig.seerApiKey = seerApiKey;
	}
	public StatManager getStatManagerImpl() {
		return statManagerImpl;
	}
	public void setStatManagerImpl(StatManager statManagerImpl) {
		this.statManagerImpl = statManagerImpl;
	}
}
