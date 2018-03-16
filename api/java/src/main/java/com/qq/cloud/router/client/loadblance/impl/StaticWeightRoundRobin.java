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

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Map;

import com.qq.cloud.router.client.Result;
import com.qq.cloud.router.client.RouterResponse;
import com.qq.cloud.router.client.ServerNode;
import com.qq.cloud.router.client.loadblance.LoadBlanceContext;
import com.qq.cloud.router.client.loadblance.LoadBlanceStrategy;
import com.qq.cloud.router.client.util.AssemblyRouteResponseUtil;
import com.qq.cloud.router.client.util.Constants;
import com.qq.tars.support.log.Logger;

public class StaticWeightRoundRobin implements LoadBlanceStrategy {
	
	private static StaticWeightRoundRobin instance;
	
	private StaticWeightRoundRobin() {
	}
	
	public static StaticWeightRoundRobin getInstance() {
		if (instance == null) {
			synchronized (StaticWeightRoundRobin.class) {
				if (instance == null) {
					instance = new StaticWeightRoundRobin();
				}
			}
		}
		return instance;
	}
	
	@SuppressWarnings("unchecked")
	@Override
	public Result<RouterResponse> selectRouteNode(Collection<? extends ServerNode> nodes,
			LoadBlanceContext context) {
		if (nodes == null || nodes.isEmpty()) {
			return AssemblyRouteResponseUtil.assemblyRouteResponse(-1, AssemblyRouteResponseUtil.NODE_NULL, null);
		}
		Result<RouterResponse> result = null;
		Map<ServerNode, Integer> current_weights = ((StaticWeightRoundRobinContext)context).getCurrent_weights();
		List<ServerNode> node_list = null;
	    if (!(nodes instanceof List)) {
		    node_list = new ArrayList<ServerNode>();
		    node_list.addAll(nodes);
	    } else {
	    	node_list = (List<ServerNode>)(nodes);
	    }
		ServerNode targetNode = null;
		int i;
		int total = 0;
		int index = -1;
			
		synchronized (current_weights) {
			for (i = 0; i < node_list.size(); i++) {
				ServerNode node = node_list.get(i);
				int current_weight = current_weights.get(node);
				current_weights.put(node, current_weight+node.getWeight());
				total+=node.getWeight();
				if (index == -1 || current_weights.get(node_list.get(index)) < current_weights.get(node)) {
					index = i;
				}
			}
			targetNode = node_list.get(index);
			result = AssemblyRouteResponseUtil.assemblyRouteResponse(0, AssemblyRouteResponseUtil.CALL_SUCCESS, targetNode);
			current_weights.put(targetNode, current_weights.get(targetNode)-total);
		}
		return result;
	}
}
