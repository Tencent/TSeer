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

package com.qq.cloud.router.client.util.cache;

import java.util.Collection;
import java.util.LinkedList;
import java.util.List;
import java.util.concurrent.TimeUnit;

import com.qq.cloud.router.client.LBType;
import com.qq.cloud.router.client.RoutersRequest;
import com.qq.cloud.router.client.ServerNode;
import com.qq.cloud.router.client.share.tseer.AgentRouterRequest;
import com.qq.cloud.router.client.share.tseer.AgentRouterResponse;
import com.qq.cloud.router.client.share.tseer.RouterNodeInfo;
import com.qq.cloud.router.client.share.tseer.RouterPrx;
import com.qq.cloud.router.client.util.AssemblyRouteResponseUtil;
import com.qq.cloud.router.client.util.CacheNodeCollection;
import com.qq.cloud.router.client.util.CommonUtils;
import com.qq.cloud.router.client.util.Constants;
import com.qq.cloud.router.client.util.LogUtils;
import com.qq.tars.common.support.Holder;

public class AgentCacheNodesManager extends AbstractCacheNodeManager<RoutersRequest> {
	
	private static final AgentCacheNodesManager instance = new AgentCacheNodesManager();
	
	private RouterPrx rPrx;
	
	private AgentCacheNodesManager() {}
	
	private void setRouterPrx(RouterPrx rPrx) {
		this.rPrx = rPrx;
	}
	
	public static AgentCacheNodesManager getInstance(RouterPrx rPrx) {
		instance.setRouterPrx(rPrx);
		return instance;
	}

	@Override
	public int init(RoutersRequest req) {
		requestLock.putIfAbsent(req.getObj(), new Object());
		if (!cacheNodeLists.containsKey(req)) {
			synchronized (requestLock.get(req.getObj())) {
				if (!cacheNodeLists.containsKey(req)) {
					Collection<ServerNode> nodes = null;
					String filepath = Constants.AGENT_ROUTERS_CACHE_FILE_PATH + req.getObj();
					if (file_manager.isfile(filepath)) {
						nodes = file_manager.readCacheFile(filepath, req.getLbGetType(), req.getSetInfo());
					}
					if (!file_manager.isfile(filepath) || nodes.isEmpty()) {
						try {
							nodes = getRoutersFromAgent(req);
						} catch (Exception e) {
							LogUtils.error(e, AgentCacheNodeManager.class.getSimpleName());
						}
					}
					
					if (nodes == null ||nodes.isEmpty()) {
						return -1;
					}
					CacheNodeCollection node_collection = new CacheNodeCollection(nodes);
					cacheNodeLists.putIfAbsent(req.clone(), node_collection);
					synchronized (isRefreshNode) {
						if (!isRefreshNode.contains(req)) {
							isRefreshNode.add(req.clone());
							executor_manager.scheduleAtFixedRate(new RefreshCacheNodeListThread(req.clone()), Constants.THREAD_CALL_INTERVAL, Constants.THREAD_CALL_INTERVAL, TimeUnit.MILLISECONDS);
						}
					}
				}
			}
		}
		return 0;
	}

	@Override
	public void refresh(RoutersRequest req) {
		List<ServerNode> nodes = getRoutersFromAgent(req);
		CacheNodeCollection node_list = new CacheNodeCollection(nodes);
	    cacheNodeLists.put(req.clone(), node_list);
	}
	
	private List<ServerNode> getRoutersFromAgent(RoutersRequest req) {
		AgentRouterRequest areq = new AgentRouterRequest(req.getLbGetType().ordinal(), LBType.LB_TYPE_ALL.ordinal(), req.getObj(), req.getSetInfo(),
				Thread.currentThread().getId(), Constants.SEER_AGENT_VERSION, 0, CommonUtils.getCurrentPid());
		AgentRouterResponse ares = new AgentRouterResponse();
		Holder<AgentRouterResponse> holder = new Holder<AgentRouterResponse>(ares);
		int RPC_result = rPrx.getRouterNode(areq, holder, Constants.defaultAgentCtx);
		if (RPC_result == -1) {
			return null;
		}
		List<RouterNodeInfo> nodeInfos = holder.getValue().getResultList();
        List<ServerNode> nodes = null;
		if (nodeInfos != null) {
			if (nodes == null)
				nodes = new LinkedList<ServerNode>();
			for (RouterNodeInfo nodeInfo : nodeInfos) {
				nodes.add(AssemblyRouteResponseUtil.convert2ServerNode(nodeInfo));
			}
		}
		return nodes;
	}
}
