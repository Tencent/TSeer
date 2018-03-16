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

package com.qq.cloud.router.client.faulttolerant;

import java.util.concurrent.ConcurrentHashMap;


import com.qq.cloud.router.client.ServerNode;

public class RouterNodeAliveChecker {
	
	private static final ConcurrentHashMap<ServerNode, RouterNodeAliveStat> cache_state = new ConcurrentHashMap<ServerNode, RouterNodeAliveStat>();
	
	public static RouterNodeAliveStat get(ServerNode node) {
		RouterNodeAliveStat routerNodeAliveStat = cache_state.get(node);
		if (routerNodeAliveStat == null) {
			routerNodeAliveStat = new RouterNodeAliveStat(node);
			cache_state.putIfAbsent(node, routerNodeAliveStat);
			routerNodeAliveStat = cache_state.get(node);
		}
		return routerNodeAliveStat;
	}
	
	public static boolean statisticCallResult(int ret, ServerNode node, RouterNodeAliveConfig config) {
		RouterNodeAliveStat routerNodeAliveStat = get(node);
		routerNodeAliveStat.statisticCallResult(ret, config);
		return routerNodeAliveStat.isAlive();
	}

	public static boolean isAlive(ServerNode node) {
		RouterNodeAliveStat routerNodeAliveStat = get(node);
		return routerNodeAliveStat.isAlive();
	}
}
