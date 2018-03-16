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

public class ServerNode {
	
	private String sIP;
	private int port;
	private int timeout;
	private boolean bTcp;
	private int weight = 1;
	private String set = "";
	
	public ServerNode() {}
	
	public ServerNode(String sIP, int port, int timeout, boolean bTcp, int weight, String set) {
		this.sIP = sIP;
		this.port = port;
		this.timeout = timeout;
		this.bTcp = bTcp;
		this.weight = weight;
		this.set = set;
	}
	public String getsIP() {
		return sIP;
	}
	public void setsIP(String sIP) {
		this.sIP = sIP;
	}
	public int getPort() {
		return port;
	}
	public void setPort(int port) {
		this.port = port;
	}
	public int getTimeout() {
		return timeout;
	}
	public void setTimeout(int timeout) {
		this.timeout = timeout;
	}

	public boolean isbTcp() {
		return bTcp;
	}
	public void setbTcp(boolean bTcp) {
		this.bTcp = bTcp;
	}	
	public int getWeight() {
		return weight;
	}
	public void setWeight(int weight) {
		this.weight = weight;
	}
	
	public String getSet() {
		return set;
	}

	public void setSet(String set) {
		this.set = set;
	}

	@Override
	public int hashCode() {
		int code = (sIP+port).hashCode();
		return code;
	}
	
	@Override
	public boolean equals(Object obj) {
		if (this == obj) {
			return true;
		}
		if (obj == null) {
			return false;
		}
		if (!(obj instanceof ServerNode)) {
			return false;
		}
		ServerNode serverNode = (ServerNode)obj;
		return sIP.equals(serverNode.getsIP()) &&
			   port == serverNode.getPort() &&
			   bTcp == serverNode.isbTcp();
	}
	
	@Override
	public String toString() {
		return this.sIP + ":" + this.port + " is tcp: " + this.bTcp;
	}
}
