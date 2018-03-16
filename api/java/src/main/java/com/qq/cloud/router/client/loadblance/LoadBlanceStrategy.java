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

package com.qq.cloud.router.client.loadblance;

import java.util.Collection;

import com.qq.cloud.router.client.Result;
import com.qq.cloud.router.client.RouterResponse;
import com.qq.cloud.router.client.ServerNode;

public interface LoadBlanceStrategy {
	
   /**
    * 选取服务节点
    * @param nodes  本地缓存的服务节点列表
    * @param context 负载均衡的上下文
    * @return 选择的节点，使用Result包装，可以获取结果编号
    */
	public Result<RouterResponse> selectRouteNode(Collection<? extends ServerNode> nodes, LoadBlanceContext context);

}
