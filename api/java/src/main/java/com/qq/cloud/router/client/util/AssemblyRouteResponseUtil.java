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

import java.util.Collection;
import java.util.LinkedList;
import java.util.List;

import com.qq.cloud.router.client.Result;
import com.qq.cloud.router.client.RouterResponse;
import com.qq.cloud.router.client.RoutersResponse;
import com.qq.cloud.router.client.ServerNode;
import com.qq.cloud.router.client.share.tseer.RouterNodeInfo;
import com.qq.tars.support.query.prx.EndpointF;

public class AssemblyRouteResponseUtil {
	
	public static final String CALL_SUCCESS = "call success";
	
	public static final String CALL_INIT = "need init before call the method";
	
	public static final String INIT_FAIL = "init fail";
	
	public static final String ILLEGAL_PARAM = "Illegal parameter";
	
	public static final String LACK_SETINFO = "lack setinfo";
	
	public static final String NODE_NULL = "node is inavilable";
	
	public static Result<RouterResponse> assemblyRouteResponse(int ret, String message, Object obj) {
		Result<RouterResponse> result = new Result<RouterResponse>();
		RouterResponse routerResponse = new RouterResponse();
		if (obj == null) {
			routerResponse.setResponseMsg(message);
			result.setRet(ret);
			result.setData(routerResponse);
			return result;
		}
		routerResponse.setServerNode(convert2ServerNode(obj));
		routerResponse.setResponseMsg(message);
		result.setData(routerResponse);
		result.setRet(ret);
		return result;
	}
	
	public static Result<RoutersResponse> assemblyRoutersResponse(int ret, String message, Collection<? extends Object> objs) {
		Result<RoutersResponse> result = new Result<RoutersResponse>();
		RoutersResponse routersResponse = new RoutersResponse();
		if (objs == null) {
			routersResponse.setResponseMsg(message);
			result.setRet(ret);
			result.setData(routersResponse);
			return result;
		}
		List<ServerNode> nodes  = new LinkedList<ServerNode>();
		for (Object obj : objs) {
			nodes.add(convert2ServerNode(obj));
		}
		routersResponse.setNode(nodes);
		routersResponse.setResponseMsg(message);
	    result.setData(routersResponse);
	    result.setRet(ret);
	    return result;
	}
	
	public static Result<String> assemblyStatResponse(int ret, String message) {
		Result<String> result = new Result<String>();
		result.setRet(ret);
		result.setData(message);
		return result;
	}
	
	public static ServerNode convert2ServerNode(Object obj) {
		if (obj instanceof ServerNode) {
			return (ServerNode)obj;
		}
		ServerNode serverNode = null;
		if (obj instanceof RouterNodeInfo) {
			RouterNodeInfo routerNodeInfo = (RouterNodeInfo)obj;
			serverNode = new ServerNode(routerNodeInfo.getIp(), routerNodeInfo.getPort(), 3000,routerNodeInfo.isTcp(), routerNodeInfo.getWeight(), routerNodeInfo.getSetname());
			return serverNode;
		}
		if (obj instanceof EndpointF) {
			EndpointF endpointF = (EndpointF)obj;
			serverNode = new ServerNode(endpointF.getHost(), endpointF.getPort(), endpointF.getTimeout(), endpointF.getIstcp() == 0 ? false : true, endpointF.getWeight(), endpointF.getSetId());
			return serverNode;
		}
		return serverNode;
	}
}
