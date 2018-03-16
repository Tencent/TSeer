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

import java.util.SortedMap;
import java.util.TreeMap;

import com.qq.cloud.router.client.ServerNode;

public class ConsistentHashingContext extends GenericLoadBlanceContext {
	
	public ConsistentHashingContext() {
	}
	
	public ConsistentHashingContext(long hashKey, SortedMap<Long, ServerNode> circle) {
		setHashKey(hashKey);
		setCircle(circle);
	}
	
	public void setHashKey(long hashKey) {
		setAttachment("hashKey", hashKey);
	}
	
	public long getHashKey() {
		return (Long)getAttachment("hashKey", 0L);
	}
	
	public void setCircle(SortedMap<Long, ServerNode> circle) {
		setAttachment("circle", circle);
	}
	
	@SuppressWarnings("unchecked")
	public SortedMap<Long, ServerNode> getCircle() {
		return (SortedMap<Long, ServerNode>)getAttachment("circle", new TreeMap<Long, ServerNode>());
	}
}
