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

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

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
import com.qq.cloud.router.client.faulttolerant.RouterNodeAliveChecker;
import com.qq.cloud.router.client.faulttolerant.RouterNodeAliveConfig;
import com.qq.cloud.router.client.faulttolerant.RouterNodeAliveStat;
import com.qq.cloud.router.client.loadblance.LoadBlanceContext;
import com.qq.cloud.router.client.loadblance.LoadBlanceStrategy;
import com.qq.cloud.router.client.loadblance.impl.ConsistentHashing;
import com.qq.cloud.router.client.loadblance.impl.RandomSelectStrategy;
import com.qq.cloud.router.client.loadblance.impl.RoundRobinStrategy;
import com.qq.cloud.router.client.loadblance.impl.StaticWeightRoundRobin;
import com.qq.cloud.router.client.util.AssemblyRouteResponseUtil;
import com.qq.cloud.router.client.util.CacheNodeCollection;
import com.qq.cloud.router.client.util.LogUtils;
import com.qq.cloud.router.client.util.cache.ApiCacheNodeManager;
import com.qq.cloud.router.client.util.cache.CacheNodeManager;
import com.qq.cloud.router.client.util.stat.DefaultApiStatManager;
import com.qq.cloud.router.client.util.stat.StatManager;
import com.qq.tars.client.Communicator;
import com.qq.tars.client.CommunicatorConfig;
import com.qq.tars.client.CommunicatorFactory;
import com.qq.tars.support.query.prx.QueryFPrx;

public class ApiRouterImpl implements Router {

	private Communicator communicator;
	private RouterNodeAliveConfig routerNodeAliveConfig;
	private volatile boolean isInited = false;
	private QueryFPrx fPrx;
	
	private static CacheNodeManager<RouterRequest> node_manager = null;
	private static StatManager stat_manager = null;

	private LoadBlanceStrategy loadBlanceStrategy;
	private LoadBlanceContext loadBlanceContext;

	@Override
	public void init(RouterConfig cfg) {
		if (isInited == true) {
			return;
		}
		CommunicatorConfig communicatorConfig = new CommunicatorConfig();
		communicatorConfig.setLocator(cfg.getRouterLocater());
		communicatorConfig.setStat(cfg.getStatObj());
		communicator = CommunicatorFactory.getInstance().getCommunicator(communicatorConfig);
		fPrx = communicator.stringToProxy(QueryFPrx.class, communicator.getCommunicatorConfig().getLocator());
		routerNodeAliveConfig = cfg.getRouterNodeAliveConfig();
		node_manager = ApiCacheNodeManager.getInstance(fPrx);
		if (cfg.getStatManagerImpl() == null) {
			stat_manager = DefaultApiStatManager.getInstance(communicator.getStatHelper());
		} else {
			stat_manager = cfg.getStatManagerImpl();
		}
		isInited = true;
	}

	@Override
	public Result<RouterResponse> getRouter(RouterRequest req) {
		if (!isInited) {
			return AssemblyRouteResponseUtil.assemblyRouteResponse(-1, AssemblyRouteResponseUtil.CALL_INIT, null);
		}

		if (req == null || req.getObj() == null || req.getLbGetType() == null || req.getLbType() == null) {
			return AssemblyRouteResponseUtil.assemblyRouteResponse(-1, AssemblyRouteResponseUtil.ILLEGAL_PARAM, null);
		}

		if (req.getLbGetType() == LBGetType.LB_GET_SET && req.getSetInfo() == null) {
			return AssemblyRouteResponseUtil.assemblyRouteResponse(-1, AssemblyRouteResponseUtil.LACK_SETINFO, null);
		}

		int ret = node_manager.init(req);
		if (ret != 0) {
			return AssemblyRouteResponseUtil.assemblyRouteResponse(-1, AssemblyRouteResponseUtil.INIT_FAIL, null);
		}
		
		CacheNodeCollection cacheNodeCollection = node_manager.getCacheNodeCollection(req);
		
		Collection<ServerNode> availableNodes = getAvilableNodes(cacheNodeCollection);

		if (availableNodes.isEmpty()) {
			LogUtils.info(ApiRouterImpl.class.getSimpleName(), AssemblyRouteResponseUtil.NODE_NULL);
			return AssemblyRouteResponseUtil.assemblyRouteResponse(-1, AssemblyRouteResponseUtil.NODE_NULL, null);
		}

		setLBStrategyAndContext(req.getLbType(), cacheNodeCollection, req.getHashKey());
		Result<RouterResponse> targetNode = loadBlanceStrategy.selectRouteNode(availableNodes, loadBlanceContext);
		return targetNode;
	}

	@Override
	public Result<String> resultReport(RouterRequest req, RouterResponse res, int ret, long timecost) {
		if (!isInited) {
			return AssemblyRouteResponseUtil.assemblyStatResponse(-1, AssemblyRouteResponseUtil.CALL_INIT);
		}
		if (req == null || res == null || req.getObj() == null || res.getIP() == null || res.getPort() <= 0
				|| (ret != 0 && ret != -1 && ret != -2) || timecost <= 0) {
			LogUtils.info(Thread.currentThread().getName(), ApiRouterImpl.class.getSimpleName(), AssemblyRouteResponseUtil.ILLEGAL_PARAM);
			return AssemblyRouteResponseUtil.assemblyStatResponse(-1, AssemblyRouteResponseUtil.ILLEGAL_PARAM);
		}

		boolean isAlive = RouterNodeAliveChecker.statisticCallResult(ret, res.getServerNode(), routerNodeAliveConfig);
		ServerNode node = new ServerNode(res.getIP(), res.getPort(), res.getTimeout(), res.isbTcp(), res.getWeight(), res.getSet());

		CacheNodeCollection cacheNodeList = node_manager.getCacheNodeCollection(req);
		if (!isAlive) {
			if (cacheNodeList != null)
				cacheNodeList.deleteNodeOnCircle(node);
		} else if (cacheNodeList != null && cacheNodeList.getNodeCollection().contains(node)){
			cacheNodeList.addNodeOnCircle(node);
		}
		
		stat_manager.reportStat(req, res, ret, timecost);

		return AssemblyRouteResponseUtil.assemblyStatResponse(0, AssemblyRouteResponseUtil.CALL_SUCCESS);
	}

	@Override
	public Result<RoutersResponse> getRouters(RoutersRequest req) {
		RouterRequest _req = new RouterRequest();
		_req.setObj(req.getObj());
		_req.setSetInfo(req.getSetInfo());
		_req.setLbGetType(req.getLbGetType());
		_req.setLbType(LBType.LB_TYPE_ALL);
		int ret = node_manager.init(_req);
		if (ret != 0) {
			return AssemblyRouteResponseUtil.assemblyRoutersResponse(-1, AssemblyRouteResponseUtil.CALL_INIT, null);
		}
		List<ServerNode> nodes = (List<ServerNode>) node_manager.getCacheNodeCollection(_req).getNodeCollection();
		return AssemblyRouteResponseUtil.assemblyRoutersResponse(0, AssemblyRouteResponseUtil.CALL_SUCCESS, nodes);
	}
	
	private Collection<ServerNode> getAvilableNodes(CacheNodeCollection cacheNodeList) {
		List<ServerNode> availableNodes = new ArrayList<ServerNode>();
		for (ServerNode node : cacheNodeList.getNodeCollection()) {
			if (RouterNodeAliveChecker.isAlive(node)) {
				availableNodes.add(node);
			} else {
				RouterNodeAliveStat nodeStat = RouterNodeAliveChecker.get(node);
				if (nodeStat.isAlive() || (nodeStat.getLastRetryTime() + routerNodeAliveConfig.getTryTimeInterval() * 1000) < System.currentTimeMillis()) {
					availableNodes.add(node);
					cacheNodeList.addNodeOnCircle(node);
					nodeStat.setLastRetryTime(System.currentTimeMillis());
				}
			}
		}
		return availableNodes;
	}
	
	private void setLBStrategyAndContext(LBType lbType, CacheNodeCollection cacheNodeList, long hashKey) {
		switch(lbType) {
			case LB_TYPE_LOOP:
				loadBlanceStrategy = RoundRobinStrategy.getInstance();
				loadBlanceContext = cacheNodeList.getRRContext();
				break;
			case LB_TYPE_RANDOM:
				loadBlanceStrategy = RandomSelectStrategy.getInstance();
				loadBlanceContext = cacheNodeList.getRandomContext();
				break;
			case LB_TYPE_STATIC_WEIGHT:
				loadBlanceStrategy = StaticWeightRoundRobin.getInstance();
				loadBlanceContext = cacheNodeList.getStaticWeightContext();
				break;
			case LB_TYPE_CST_HASH:
				loadBlanceStrategy = ConsistentHashing.getInstance();
				cacheNodeList.getHashingContext().setHashKey(hashKey);
				loadBlanceContext = cacheNodeList.getHashingContext();
				break;
			default :
				loadBlanceStrategy = RoundRobinStrategy.getInstance();
				loadBlanceContext =cacheNodeList.getRRContext();
				break;
	    }
	}
}
