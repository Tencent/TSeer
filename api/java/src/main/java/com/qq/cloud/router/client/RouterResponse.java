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



public class RouterResponse {
	
	private ServerNode serverNode;
	private String responseMsg;
	
	public ServerNode getServerNode() {
		return serverNode;
	}
	public void setServerNode(ServerNode serverNode) {
		this.serverNode = serverNode;
	}
	public String getResponseMsg() {
		return responseMsg;
	}
	public void setResponseMsg(String responseMsg) {
		this.responseMsg = responseMsg;
	}
	public String getIP() {
		if (serverNode == null)
			return null;
		return serverNode.getsIP();
	}
	public int getPort() {
		if (serverNode == null)
			return -1;
		return serverNode.getPort();
	}
	public int getTimeout() {
		if (serverNode == null)
			return -1;
		return serverNode.getTimeout();
	}
	public boolean isbTcp() {
		if (serverNode == null)
			return false;
		return serverNode.isbTcp();
	}
	public int getWeight() {
		if (serverNode == null)
			return 0;
		return serverNode.getWeight();
	}
	public String getSet() {
		if (serverNode == null)
			return null;
		return serverNode.getSet();
	}	
}
