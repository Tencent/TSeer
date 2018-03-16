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

import java.util.concurrent.TimeUnit;

import com.qq.cloud.router.client.RouterRequest;
import com.qq.cloud.router.client.util.CacheNodeCollection;
import com.qq.cloud.router.client.util.Constants;

public class AgentCacheNodeManager extends AbstractCacheNodeManager<RouterRequest> {
	
	private static final AgentCacheNodeManager instance = new AgentCacheNodeManager();
	private static final CacheFileManager file_manager = CacheFileManager.getInstance();

	
	private AgentCacheNodeManager() {}
	
	public static AgentCacheNodeManager getInstance() {
		return instance;
	}

	@Override
	public int init(RouterRequest req) {
		synchronized (isRefreshNode) {
			if (!isRefreshNode.contains(req)) {
				isRefreshNode.add(req.clone());
				executor_manager.scheduleAtFixedRate(new RefreshCacheNodeListThread(req.clone()), Constants.THREAD_CALL_INTERVAL, 4 * Constants.THREAD_CALL_INTERVAL, TimeUnit.MILLISECONDS);
			}
		}
		return 0;
	}

	@Override
	public void refresh(RouterRequest req) {	
		CacheNodeCollection cacheNodeList = getCacheNodeCollection(req);
		String filepath = Constants.AGENT_ROUTER_CACHE_FILE_PATH + req.getObj();
		boolean ret = file_manager.createNewFile(filepath);
		if (ret) {
			synchronized (filepath) {
				file_manager.updateCacheFile(filepath, req.getLbGetType(), req.getSetInfo(), cacheNodeList.getNodeCollection(), null);
			}
		}
	}
}
