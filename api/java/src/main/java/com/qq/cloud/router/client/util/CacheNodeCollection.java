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

package com.qq.cloud.router.client.util;

import java.io.Serializable;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.SortedMap;
import java.util.concurrent.ConcurrentSkipListMap;
import java.util.concurrent.atomic.AtomicInteger;

import com.qq.cloud.router.client.ServerNode;
import com.qq.cloud.router.client.faulttolerant.RouterNodeAliveChecker;
import com.qq.cloud.router.client.loadblance.impl.ConsistentHashing;
import com.qq.cloud.router.client.loadblance.impl.ConsistentHashingContext;
import com.qq.cloud.router.client.loadblance.impl.RandomSelectContext;
import com.qq.cloud.router.client.loadblance.impl.RoundRobinContext;
import com.qq.cloud.router.client.loadblance.impl.StaticWeightRoundRobinContext;

public class CacheNodeCollection implements Serializable{
	
	private static final long serialVersionUID = 2711042552568072529L;

	private final Collection<ServerNode> nodeCollection;
	
	private ConcurrentSkipListMap<Long, ServerNode> circle;
	
	private Map<ServerNode, Integer> current_weight;
	
	private final ConsistentHashingContext hashingContext = new ConsistentHashingContext();
	
	private final RandomSelectContext randomContext = new RandomSelectContext();
	
	private final StaticWeightRoundRobinContext staticWeightContext = new StaticWeightRoundRobinContext();
	
	private final RoundRobinContext RRContext = new RoundRobinContext();
	
	private volatile AtomicInteger index = new AtomicInteger(0);
	
	
	
	public CacheNodeCollection(Collection<ServerNode> nodeCollection) {
		this.nodeCollection = nodeCollection;
		current_weight = new HashMap<ServerNode, Integer>();
		circle = new ConcurrentSkipListMap<Long, ServerNode>();
		Iterator<ServerNode> iter = nodeCollection.iterator();
		while (iter.hasNext()) {
			ServerNode node = iter.next();
			current_weight.put(node, 0);
			if (RouterNodeAliveChecker.isAlive(node)) {
				addNodeOnCircle(node);
			}
		}
		hashingContext.setCircle(circle);
		staticWeightContext.setCurrent_weights(current_weight);
		RRContext.setIndex(index);
	}
	public CacheNodeCollection(CacheNodeCollection other) {
		this.nodeCollection = other.getNodeCollection();
		this.circle = (ConcurrentSkipListMap<Long, ServerNode>) other.getCircle();
		this.current_weight = other.getCurrent_weight();
		hashingContext.setCircle(other.getCircle());
		staticWeightContext.setCurrent_weights(other.getCurrent_weight());
		RRContext.setIndex(other.getIndex());
	}
	
	public void deleteNodeOnCircle(ServerNode node) {
		ConsistentHashing.getInstance().deleteVirtualNode(circle, node);
	}
	
	public void addNodeOnCircle(ServerNode node) {
		ConsistentHashing.getInstance().addVirtualNode(circle, node);
	}
	
	public boolean isNodeOnCircle(ServerNode node) {
		return ConsistentHashing.getInstance().isNodeOnCircle(circle, node);
	}
	
	public Collection<ServerNode> getNodeCollection() {
		return nodeCollection;
	}
    
	public Map<ServerNode, Integer> getCurrent_weight() {
		synchronized (current_weight) {
			return current_weight;
		}
	}

	public AtomicInteger getIndex() {
		return index;
	}

	public SortedMap<Long, ServerNode> getCircle() {
		return circle;
	}

	public void setIndex(AtomicInteger index) {
		this.index = index;
	}

	public ConsistentHashingContext getHashingContext() {
		return hashingContext;
	}

	public RandomSelectContext getRandomContext() {
		return randomContext;
	}

	public StaticWeightRoundRobinContext getStaticWeightContext() {
		return staticWeightContext;
	}

	public RoundRobinContext getRRContext() {
		return RRContext;
	}
}
