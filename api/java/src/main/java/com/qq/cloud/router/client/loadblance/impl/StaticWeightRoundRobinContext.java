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

import java.util.Map;

import com.qq.cloud.router.client.ServerNode;

public class StaticWeightRoundRobinContext extends GenericLoadBlanceContext {
	
	public StaticWeightRoundRobinContext() {}
	
	public StaticWeightRoundRobinContext(Map<ServerNode, Integer> current_weights) {
		setAttachment("current_weights", current_weights);
	}

	public void setCurrent_weights(Map<ServerNode, Integer> current_weights) {
		setAttachment("current_weights", current_weights);
	}
	
	@SuppressWarnings("unchecked")
	public <T> Map<T, Integer> getCurrent_weights() {
		return (Map<T, Integer>) getAttachment("current_weights", null);
	}
}
