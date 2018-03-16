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

package com.qq.cloud.router.client.loadblance.impl;

import java.util.Collection;
import java.util.SortedMap;



import java.util.concurrent.ConcurrentSkipListMap;

import com.qq.cloud.router.client.Result;
import com.qq.cloud.router.client.RouterResponse;
import com.qq.cloud.router.client.ServerNode;
import com.qq.cloud.router.client.loadblance.LoadBlanceContext;
import com.qq.cloud.router.client.loadblance.LoadBlanceStrategy;
import com.qq.cloud.router.client.util.AssemblyRouteResponseUtil;
import com.qq.cloud.router.client.util.LogUtils;

public class ConsistentHashing implements LoadBlanceStrategy {

	private final ConsistentHashingFunction hashFunction = new ConsistentHashingFunction();

	private static final int NUMBER_OF_REPLICAS = 5;
	
	private static ConsistentHashing instance;
	
	private ConsistentHashing() {
	}
	
	public void addVirtualNode(ConcurrentSkipListMap<Long, ServerNode> circle, ServerNode node) {
		for (int i = 0; i < NUMBER_OF_REPLICAS; i++) {
			circle.putIfAbsent(hashFunction.hash(node.getsIP()+node.getPort()+node.isbTcp()+i), node); 
		}
	}
	
	public void deleteVirtualNode(ConcurrentSkipListMap<Long, ServerNode> circle, ServerNode node) {
		for (int i = 0; i < NUMBER_OF_REPLICAS; i++) {
			circle.remove(hashFunction.hash(node.getsIP()+node.getPort()+node.isbTcp()+i));
		}
	}
	
	public boolean isNodeOnCircle(ConcurrentSkipListMap<Long, ServerNode> circle, ServerNode node) {
		return circle.containsKey(hashFunction.hash(node.getsIP()+node.getPort()+node.isbTcp()+0));
	}
	
	public static ConsistentHashing getInstance() {
		if (instance == null) {
			synchronized (ConsistentHashing.class) {
				if (instance == null) {
					instance = new ConsistentHashing();
				}
			}
		}
		return instance;
	}
	
	@Override
	public Result<RouterResponse> selectRouteNode(Collection<? extends ServerNode> nodes,
			LoadBlanceContext context) {
		Long hashKey = ((ConsistentHashingContext)context).getHashKey();
		SortedMap<Long, ServerNode> circle = ((ConsistentHashingContext)context).getCircle();
		if (hashKey < (long)0 || hashKey > (long)((1L << 32) -1)) {
			LogUtils.info(ConsistentHashing.class.getSimpleName(), "hash key is invalid");
			return AssemblyRouteResponseUtil.assemblyRouteResponse(-1, "hashKey is invalid", null);
		}
		ServerNode targetNodeInfo = null;
		long targetHash = hashKey;
		if (!circle.containsKey(hashKey)) {
			SortedMap<Long, ServerNode> tailMap = circle.tailMap(hashKey);
			 targetHash = tailMap.isEmpty() ? circle.firstKey() : tailMap.firstKey();
		}
		targetNodeInfo = circle.get(targetHash);

		return AssemblyRouteResponseUtil.assemblyRouteResponse(0, AssemblyRouteResponseUtil.CALL_SUCCESS, targetNodeInfo);
	}
}
