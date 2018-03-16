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
import java.util.List;
import java.util.concurrent.ConcurrentHashMap;

import com.qq.cloud.router.client.util.CacheNodeCollection;
import com.qq.cloud.router.client.util.ScheduledExecutorManager;

public abstract class AbstractCacheNodeManager<T> implements CacheNodeManager<T> {
	 
	protected final ConcurrentHashMap<T, CacheNodeCollection> cacheNodeLists = new ConcurrentHashMap<T, CacheNodeCollection>();
	protected final List<T> isRefreshNode = new ArrayList<T>();
	protected final ConcurrentHashMap<String, Object> requestLock = new ConcurrentHashMap<String, Object>();  //写文件同步锁
	protected final CacheFileManager file_manager = CacheFileManager.getInstance();
	protected final ScheduledExecutorManager executor_manager = ScheduledExecutorManager.getInstance();
	
	public CacheNodeCollection getCacheNodeCollection(T req) {
		CacheNodeCollection cacheNodeList = cacheNodeLists.get(req);
		if (cacheNodeList == null) {
			return null;
		}
		return new CacheNodeCollection(cacheNodeList);
	}
	
	public void setCacheNodeCollection(T req, CacheNodeCollection value) {
		CacheNodeCollection cacheNodeList = cacheNodeLists.get(req);
		if (cacheNodeList == null) {
			cacheNodeLists.putIfAbsent(req, value);
		} else {
			System.out.println(cacheNodeLists.keySet().size());
			cacheNodeLists.put(req, value);
		}
	}
	
	public abstract int init(T req);
	
	public abstract void refresh(T req);
	
	protected class RefreshCacheNodeListThread implements Runnable {

		private T req;

		public RefreshCacheNodeListThread(T req) {
			this.req = req;
		}

		@Override
		public void run() {
			try {
				refresh(req);
			} catch (Exception e) {
				e.printStackTrace();
			}
		}

	}
}
