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

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.Collection;
import java.util.Date;

import com.qq.cloud.router.client.LBGetType;
import com.qq.cloud.router.client.ServerNode;
import com.qq.cloud.router.client.util.CacheFileConfig;
import com.qq.cloud.router.client.util.CacheFileStringParseUtil;
import com.qq.cloud.router.client.util.ConcurrentDateUtil;
import com.qq.cloud.router.client.util.Constants;
import com.qq.cloud.router.client.util.LogUtils;

public class CacheFileManager {
	
	private static final CacheFileManager instance = new CacheFileManager();
	
	private CacheFileManager() {}
	
	public static CacheFileManager getInstance() {
		return instance;
	}
	
	public Collection<ServerNode> readCacheFile(String filepath, LBGetType lbGetType, String setInfo) {
		try {
			CacheFileConfig cacheFileConfig = CacheFileConfig.parseFile(filepath);
			String active_ip = null;
			String path = null;
			switch (lbGetType) {
			case LB_GET_ALL:
				path = Constants.CACHE_FILE_ROOT + "/all";
				break;
			case LB_GET_IDC:
				path = Constants.CACHE_FILE_ROOT + "/idc";
				break;
			case LB_GET_SET:
				path = Constants.CACHE_FILE_ROOT + "/" + setInfo;
                break;
			default:
				path = Constants.CACHE_FILE_ROOT + "/all";
				break;
			}
			active_ip = cacheFileConfig.get(path + "<active_ip>");
			return CacheFileStringParseUtil.parseIPListString(active_ip);
		} catch (FileNotFoundException e) {
			e.printStackTrace();
			return null;
		} catch (IOException e) {
			e.printStackTrace();
			return null;
		}
	}
	
	public void updateCacheFile(String filepath, LBGetType lbGetType, String setInfo, Collection<ServerNode> activeNodes, Collection<ServerNode> inactiveNodes) {
		try {
			CacheFileConfig cacheFileConfig = CacheFileConfig.parseFile(filepath);
			String path = null;
			switch (lbGetType) {
			case LB_GET_ALL:
				path = Constants.CACHE_FILE_ROOT + "/all";
				break;
			case LB_GET_IDC:
				path = Constants.CACHE_FILE_ROOT + "/idc";
				break;
			case LB_GET_SET:
				path = Constants.CACHE_FILE_ROOT + "/" + setInfo;
                break;
			default:
				path = Constants.CACHE_FILE_ROOT + "/all";
				break;
			}
			cacheFileConfig.addKey(Constants.CACHE_FILE_ROOT + "<last_update>", ConcurrentDateUtil.format(new Date()));
			String key = path + "<last_update>";
			String value = ConcurrentDateUtil.format(new Date());
			cacheFileConfig.addKey(key, value);
			if (inactiveNodes != null) {
				key = path + "<inactive_ip>";
				value = CacheFileStringParseUtil.parseServerNodes(inactiveNodes);
				cacheFileConfig.addKey(key, value);
			}
			if (activeNodes != null) {
				key = path + "<active_ip>";
				value = CacheFileStringParseUtil.parseServerNodes(activeNodes);
				cacheFileConfig.addKey(key, value);
			}
			cacheFileConfig.updateFile(filepath);
		} catch (FileNotFoundException e) {
			LogUtils.error(e, CacheFileManager.class.getSimpleName());
		} catch (IOException e) {
			LogUtils.error(e, CacheFileManager.class.getSimpleName());
		}
	}
	
	public boolean isfile(String path) {
		File file = new File(path);
		return file.isFile();
	}
	
	public boolean createNewFile(String path) {
		File file = new File(path);

		if (file.isFile()) {
			return true;
		}
		if (!file.getParentFile().exists()) {
			if (!file.getParentFile().mkdirs()) {
				return false;
			}
		}
		try {
			if (file.createNewFile()) {
				return true;
			} else {
				return false;
			}
		} catch (IOException e) {
			LogUtils.error(e, CacheFileManager.class.getSimpleName());
			return false;
		}
	}
}
