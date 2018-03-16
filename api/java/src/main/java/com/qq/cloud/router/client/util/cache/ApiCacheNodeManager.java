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

import java.util.ArrayList;
import java.util.Collection;
import java.util.LinkedList;
import java.util.List;
import java.util.concurrent.TimeUnit;

import com.qq.cloud.router.client.LBType;
import com.qq.cloud.router.client.RouterRequest;
import com.qq.cloud.router.client.ServerNode;
import com.qq.cloud.router.client.util.AssemblyRouteResponseUtil;
import com.qq.cloud.router.client.util.CacheNodeCollection;
import com.qq.cloud.router.client.util.Constants;
import com.qq.cloud.router.client.util.LogUtils;
import com.qq.tars.common.support.Holder;
import com.qq.tars.support.query.prx.EndpointF;
import com.qq.tars.support.query.prx.QueryFPrx;

public class ApiCacheNodeManager extends AbstractCacheNodeManager<RouterRequest> {
	
	private static final ApiCacheNodeManager instance = new ApiCacheNodeManager();

	private QueryFPrx fPrx;
	
	private ApiCacheNodeManager() {}
	
	public void setQueryPrx(QueryFPrx fPrx) {
		this.fPrx = fPrx;
	}
	
	public static ApiCacheNodeManager getInstance(QueryFPrx fPrx) {
		instance.setQueryPrx(fPrx);
		return instance;
	}
	@Override
	public int init(RouterRequest req) {
		requestLock.putIfAbsent(req.getObj(), new Object());
		if (!cacheNodeLists.containsKey(req)) {
			synchronized (requestLock.get(req.getObj())) {
				if (!cacheNodeLists.containsKey(req)) {
					Collection<ServerNode> nodes = null;
					String filepath = Constants.API_CACHE_FILE_PATH + req.getObj();
					if (file_manager.isfile(filepath)) {
						nodes = file_manager.readCacheFile(filepath, req.getLbGetType(), req.getSetInfo());
					}

					if (!file_manager.isfile(filepath) || nodes.isEmpty()) {
						Holder<List<ServerNode>> activeNodes = new Holder<List<ServerNode>>();
						Holder<List<ServerNode>> inactNodes = new Holder<List<ServerNode>>();
						try {
							getNodesFromRegirest(req, activeNodes, inactNodes);
						} catch (Exception e) {
							LogUtils.error(e, ApiCacheNodeManager.class.getSimpleName());
						}
						nodes = activeNodes.getValue();
						if (nodes == null || nodes.isEmpty()) {
							return -1;
						}

						boolean ret = file_manager.createNewFile(filepath);
						if (ret) {
							file_manager.updateCacheFile(filepath, req.getLbGetType(), req.getSetInfo(), activeNodes.getValue(), inactNodes.getValue());
						}
					}

					if (nodes == null || nodes.isEmpty()) {
						return -1;
					}

					if (req.getLbType() == LBType.LB_TYPE_STATIC_WEIGHT) {
						boolean checkSum = false;
						for (ServerNode node : nodes) {
							if (node.getWeight() > 0)
								checkSum = true;
						}
						if (checkSum == false) {
							for (ServerNode node : nodes) {
								node.setWeight(1);
							}
						}
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
	public void refresh(RouterRequest req) {
		Holder<List<ServerNode>> activeNodes = new Holder<List<ServerNode>>();
		Holder<List<ServerNode>> inactNodes = new Holder<List<ServerNode>>();
		int ret = getNodesFromRegirest(req, activeNodes, inactNodes);
		if (ret != 0) {
			return;
		}

		if (req.getLbType() == LBType.LB_TYPE_STATIC_WEIGHT) {
			boolean checkSum = false;
			for (ServerNode node : activeNodes.getValue()) {
				if (node.getWeight() > 0)
					checkSum = true;
			}
			if (checkSum == false) {
				for (ServerNode node : activeNodes.getValue()) {
					node.setWeight(1);
				}
			}
		}
		String filepath = Constants.API_CACHE_FILE_PATH + req.getObj();
		synchronized (requestLock.get(req.getObj())) {
			file_manager.updateCacheFile(filepath, req.getLbGetType(), req.getSetInfo(), activeNodes.getValue(), inactNodes.getValue());
		}
		
		CacheNodeCollection node_collection = new CacheNodeCollection(activeNodes.getValue());
		cacheNodeLists.put(req.clone(), node_collection);
	}
	
	private int getNodesFromRegirest(RouterRequest req, Holder<List<ServerNode>> activeNodes, Holder<List<ServerNode>> inactivateNodes) {
		Holder<List<EndpointF>> activeEp = new Holder<List<EndpointF>>(new ArrayList<EndpointF>());
		Holder<List<EndpointF>> inactiveEp = new Holder<List<EndpointF>>(new ArrayList<EndpointF>());
		int ret = 0;
		switch (req.getLbGetType()) {
			case LB_GET_ALL:
				ret = fPrx.findObjectById4Any(req.getObj(), activeEp, inactiveEp, Constants.defaultApiCtx);
				break;
			case LB_GET_IDC:
				ret = fPrx.findObjectByIdInSameGroup(req.getObj(), activeEp, inactiveEp, Constants.defaultApiCtx);
				break;
			case LB_GET_SET:
				ret = fPrx.findObjectByIdInSameSet(req.getObj(), req.getSetInfo(), activeEp, inactiveEp, Constants.defaultApiCtx);
				break;
			default:
				ret = fPrx.findObjectById4Any(req.getObj(), activeEp, inactiveEp, Constants.defaultApiCtx);
				break;
		}
		if(ret != 0) {
			return -1;
		}
		List<ServerNode> nodes = new LinkedList<ServerNode>();
		if (activeEp != null && activeEp.getValue() != null) {
			for (EndpointF endpointF : activeEp.getValue()) {
				nodes.add(AssemblyRouteResponseUtil.convert2ServerNode(endpointF));
			}
		}
		activeNodes.setValue(nodes);
		List<ServerNode> inNodes = new LinkedList<ServerNode>();
		if (inactiveEp != null && inactiveEp.getValue() != null) {
			for (EndpointF endpointF : inactiveEp.getValue()) {
				inNodes.add(AssemblyRouteResponseUtil.convert2ServerNode(endpointF));
			}
		}
		inactivateNodes.setValue(inNodes);
		return 0;
	}
}
