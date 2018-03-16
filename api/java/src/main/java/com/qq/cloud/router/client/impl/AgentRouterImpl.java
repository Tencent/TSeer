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

package com.qq.cloud.router.client.impl;

import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import com.qq.cloud.router.client.LBGetType;
import com.qq.cloud.router.client.LBType;
import com.qq.cloud.router.client.Result;
import com.qq.cloud.router.client.Router;
import com.qq.cloud.router.client.RouterConfig;
import com.qq.cloud.router.client.RouterRequest;
import com.qq.cloud.router.client.RouterResponse;
import com.qq.cloud.router.client.RoutersRequest;
import com.qq.cloud.router.client.RoutersResponse;
import com.qq.cloud.router.client.ServerNode;
import com.qq.cloud.router.client.loadblance.LoadBlanceContext;
import com.qq.cloud.router.client.loadblance.LoadBlanceStrategy;
import com.qq.cloud.router.client.loadblance.impl.ConsistentHashing;
import com.qq.cloud.router.client.loadblance.impl.RoundRobinStrategy;
import com.qq.cloud.router.client.share.tseer.AgentRouterRequest;
import com.qq.cloud.router.client.share.tseer.AgentRouterResponse;
import com.qq.cloud.router.client.share.tseer.RouterNodeInfo;
import com.qq.cloud.router.client.share.tseer.RouterPrx;
import com.qq.cloud.router.client.util.AssemblyRouteResponseUtil;
import com.qq.cloud.router.client.util.CacheNodeCollection;
import com.qq.cloud.router.client.util.CommonUtils;
import com.qq.cloud.router.client.util.Constants;
import com.qq.cloud.router.client.util.LogUtils;
import com.qq.cloud.router.client.util.ScheduledExecutorManager;
import com.qq.cloud.router.client.util.cache.AgentCacheNodeManager;
import com.qq.cloud.router.client.util.cache.AgentCacheNodesManager;
import com.qq.cloud.router.client.util.cache.CacheFileManager;
import com.qq.cloud.router.client.util.cache.CacheNodeManager;
import com.qq.cloud.router.client.util.stat.AgentStatManager;
import com.qq.cloud.router.client.util.stat.StatManager;
import com.qq.tars.client.Communicator;
import com.qq.tars.client.CommunicatorConfig;
import com.qq.tars.client.CommunicatorFactory;
import com.qq.tars.common.support.Holder;

public class AgentRouterImpl implements Router {

	private Communicator routerCommunicator;
	private String routerLocator;
	private String statObj;
	private RouterPrx rPrx;
	private RouterPrx rPrxS;
	
	private LoadBlanceStrategy loadBlanceStrategy;
	private LoadBlanceContext loadBlanceContext;

	private volatile boolean isInited = false;

	private static CacheFileManager file_manager = null;	
	private static CacheNodeManager<RoutersRequest> nodes_manager = null;
	private static CacheNodeManager<RouterRequest> node_manager = null;	
	private static StatManager stat_manager = null;
	private static ScheduledExecutorManager executor_manager = ScheduledExecutorManager.getInstance();

	@Override
	public void init(RouterConfig cfg) {
		if (isInited == true) {
			return;
		}
		CommunicatorConfig routerConfig = new CommunicatorConfig();
		routerCommunicator = CommunicatorFactory.getInstance().getCommunicator(routerConfig);
		routerLocator = cfg.getRouterLocater();
		statObj = cfg.getStatObj();
		rPrx = routerCommunicator.stringToProxy(RouterPrx.class, routerLocator);
		rPrxS = routerCommunicator.stringToProxy(RouterPrx.class, statObj);
		nodes_manager = AgentCacheNodesManager.getInstance(rPrx);
		stat_manager = AgentStatManager.getInstance(rPrxS);
		node_manager = AgentCacheNodeManager.getInstance();
		file_manager = CacheFileManager.getInstance();
		isInited = true;
	}

	@Override
	public Result<RouterResponse> getRouter(RouterRequest req) {
		if (!isInited) {
			LogUtils.info(AgentRouterImpl.class.getSimpleName(), AssemblyRouteResponseUtil.CALL_INIT);
			return AssemblyRouteResponseUtil.assemblyRouteResponse(-1, AssemblyRouteResponseUtil.CALL_INIT, null);
		}

		if (req == null || req.getObj() == null || req.getLbGetType() == null || req.getLbType() == null) {
			return AssemblyRouteResponseUtil.assemblyRouteResponse(-1, AssemblyRouteResponseUtil.ILLEGAL_PARAM, null);
		}

		if (req.getLbGetType() == LBGetType.LB_GET_SET && req.getSetInfo() == null) {
			return AssemblyRouteResponseUtil.assemblyRouteResponse(-1, AssemblyRouteResponseUtil.LACK_SETINFO, null);
		}
		try {
			Holder<AgentRouterResponse> value = new Holder<AgentRouterResponse>();
			AgentRouterRequest request = new AgentRouterRequest(req.getLbGetType().ordinal(), req.getLbType().ordinal(), req.getObj(),
					req.getSetInfo(), Thread.currentThread().getId(), Constants.SEER_AGENT_VERSION, req.getHashKey(), CommonUtils.getCurrentPid());
			int RPC_result = rPrx.getRouterNode(request, value);
			if (RPC_result == 0) {
				ServerNode serverNode = null;
				LogUtils.info(AgentRouterImpl.class.getSimpleName(), value.getValue().getResultList().isEmpty() ? "empty" : "no empty");
				if (!value.getValue().getResultList().isEmpty()) {
					for (RouterNodeInfo nodeInfo : value.getValue().getResultList()) {
						serverNode = AssemblyRouteResponseUtil.convert2ServerNode(nodeInfo);
					}
					asyc_setMemCache(req, serverNode);
					int ret = node_manager.init(req);
					if (ret != 0) {
						return AssemblyRouteResponseUtil.assemblyRouteResponse(-1, AssemblyRouteResponseUtil.INIT_FAIL, null);
					}
					return AssemblyRouteResponseUtil.assemblyRouteResponse(0, AssemblyRouteResponseUtil.CALL_SUCCESS, serverNode);
				}
				LogUtils.info(AgentRouterImpl.class.getSimpleName(), AssemblyRouteResponseUtil.NODE_NULL);
				return AssemblyRouteResponseUtil.assemblyRouteResponse(-1, AssemblyRouteResponseUtil.NODE_NULL, serverNode);
			}
		} catch (Exception e) {
			LogUtils.error(e, AgentRouterImpl.class.getSimpleName());
		}
		
		CacheNodeCollection cacheNode = node_manager.getCacheNodeCollection(req);
		Collection<ServerNode> avilableNodes = null;
		String filepath = Constants.AGENT_ROUTER_CACHE_FILE_PATH + req.getObj();
		
		if (cacheNode == null && file_manager.isfile(filepath)) {
			synchronized (req.getObj()) {
				if ((cacheNode = node_manager.getCacheNodeCollection(req)) == null) {
					avilableNodes = file_manager.readCacheFile(filepath, req.getLbGetType(), req.getSetInfo());
					if (avilableNodes.isEmpty()) {
						LogUtils.info(AgentRouterImpl.class.getSimpleName(), AssemblyRouteResponseUtil.NODE_NULL);
						return AssemblyRouteResponseUtil.assemblyRouteResponse(-1, AssemblyRouteResponseUtil.NODE_NULL, null);
					}
					cacheNode = new CacheNodeCollection(avilableNodes);
					node_manager.setCacheNodeCollection(req.clone(), cacheNode);
				}
			}     
		} else if (cacheNode == null || !file_manager.isfile(filepath)){
			LogUtils.info(AgentRouterImpl.class.getSimpleName(), AssemblyRouteResponseUtil.NODE_NULL);
			return AssemblyRouteResponseUtil.assemblyRouteResponse(-1, AssemblyRouteResponseUtil.NODE_NULL, null);
		}
		
		if (req.getLbType() == LBType.LB_TYPE_CST_HASH) {
			loadBlanceStrategy = ConsistentHashing.getInstance();
			cacheNode.getHashingContext().setHashKey(req.getHashKey());
			loadBlanceContext = cacheNode.getHashingContext();
		} else {
			loadBlanceStrategy = RoundRobinStrategy.getInstance();
			loadBlanceContext = cacheNode.getRRContext();
		}
		Result<RouterResponse> targetNode = loadBlanceStrategy.selectRouteNode(cacheNode.getNodeCollection(), loadBlanceContext);
		return targetNode;
	}

	@Override
	public Result<String> resultReport(RouterRequest req, RouterResponse res, int ret, long timecost) {
		if (!isInited) {
			return AssemblyRouteResponseUtil.assemblyStatResponse(-1, AssemblyRouteResponseUtil.CALL_INIT);
		}

		if (req == null || res == null || req.getObj() == null || res.getIP() == null || res.getPort() <= 0
				|| (ret != 0 && ret != -1 && ret != -2) || timecost <= 0) {
			return AssemblyRouteResponseUtil.assemblyStatResponse(-1, AssemblyRouteResponseUtil.ILLEGAL_PARAM);
		}
		
		stat_manager.reportStat(req, res, ret, timecost);
		return AssemblyRouteResponseUtil.assemblyStatResponse(0, AssemblyRouteResponseUtil.CALL_SUCCESS);
	}

	@Override
	public Result<RoutersResponse> getRouters(RoutersRequest req) {
		int ret = nodes_manager.init(req);
		if (ret != 0) {
			LogUtils.info(AgentRouterImpl.class.getSimpleName(), AssemblyRouteResponseUtil.NODE_NULL);
			return AssemblyRouteResponseUtil.assemblyRoutersResponse(-1, AssemblyRouteResponseUtil.INIT_FAIL, null);
		}
		List<ServerNode> nodes = (List<ServerNode>) nodes_manager.getCacheNodeCollection(req).getNodeCollection();
		return AssemblyRouteResponseUtil.assemblyRoutersResponse(0, AssemblyRouteResponseUtil.CALL_SUCCESS, nodes);
	}
	
	private synchronized void setRouterMemCache(RouterRequest index, ServerNode node) {
		Set<ServerNode> nodes = null;
		CacheNodeCollection cacheNode = node_manager.getCacheNodeCollection(index);
		if (cacheNode == null) {
			nodes = new HashSet<ServerNode>();
			nodes.add(node);
			cacheNode = new CacheNodeCollection(nodes);
            node_manager.setCacheNodeCollection(index.clone(), cacheNode);
		} else if (!(cacheNode.getNodeCollection() instanceof Set)) {
			nodes = new HashSet<ServerNode>();
			nodes.addAll(cacheNode.getNodeCollection());
			nodes.add(node);
			cacheNode = new CacheNodeCollection(nodes);
			node_manager.setCacheNodeCollection(index.clone(), cacheNode);
		} else {
			nodes = (Set<ServerNode>) cacheNode.getNodeCollection();
			nodes.add(node);
			node_manager.setCacheNodeCollection(index.clone(), cacheNode);
		}
	}
	
	private void asyc_setMemCache(RouterRequest index, ServerNode node) {
		
		executor_manager.execute(new Runnable() {
			
			private RouterRequest index;
			private ServerNode node;
			
			public Runnable init(RouterRequest index, ServerNode node) {
				this.index = index;
				this.node = node;
				return this;
			}
			
			@Override
			public void run() {
				setRouterMemCache(index, node);
			}
		}.init(index, node));
	}
}
