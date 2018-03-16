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

import com.qq.cloud.router.client.impl.AgentRouterImpl;
import com.qq.cloud.router.client.impl.ApiRouterImpl;
import com.qq.cloud.router.client.util.stat.StatManager;

public class RouterFactory {

	private static final RouterFactory instance = new RouterFactory();
	
	private RouterFactory() {}
	
	public static RouterFactory getInstance() {
		return instance;
	}
	
	public Router createAgentRouter() {
		return createAgentRouter(new RouterConfig());
	}
	
	public Router createAgentRouter(RouterConfig config) {
		AgentRouterImpl agentRouterImpl = new AgentRouterImpl();
		config.setRouterLocater("Tseer.TseerAgent.RouterObj@udp -h 127.0.0.1 -p 8865 -t 60000");
		config.setStatObj("Tseer.TseerAgent.RouterObj@udp -h 127.0.0.1 -p 8865 -t 60000");
		agentRouterImpl.init(config);
		return agentRouterImpl;
	}
	
	public Router createApiRouter() {
		return createApiRouter(new RouterConfig());
	}
	
	public Router createApiRouter(StatManager statManager) {
		RouterConfig config = new RouterConfig();
		config.setStatManagerImpl(statManager);
		return createAgentRouter(config);
	}
	
	public Router createApiRouter(RouterConfig config) {
		ApiRouterImpl apiRouterImpl = new ApiRouterImpl();
		apiRouterImpl.init(config);
		return apiRouterImpl;
	}
}
