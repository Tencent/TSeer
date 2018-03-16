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

package com.qq.cloud.router.client;

public interface Router {

	/**
	 * 初始化路由，如果无特殊需求不需要手动调用
	 * @param cfg 初始化配置
	 */
    void init(RouterConfig cfg);

    /**
     * 获取单个节点
     * @param req 路由请求
     * @return 包含目标节点的ip,port等信息和返回描述信息
     */
    Result<RouterResponse> getRouter(RouterRequest req);
    
    /**
     * 获取所有目标节点
     * @param req 路由请求
     * @return 目标节点列表
     */
    Result<RoutersResponse> getRouters(RoutersRequest req);

    /**
     * 模调数据上报接口
     * @param req 路由请求
     * @param res 路由回复
     * @param ret 调用被调服务的返回值
     * @param timecost 调用被调服务花费的时间，单位毫秒
     * @return 描述信息
     */
    Result<String> resultReport(RouterRequest req, RouterResponse res, int ret, long timecost);
}
