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

import com.qq.cloud.router.client.ServerNode;

public class CacheFileStringParseUtil {
	
	public static Collection<ServerNode> parseIPListString(String ip_list) {
		List<ServerNode> IPs = new LinkedList<ServerNode>();
		if (ip_list == null || ip_list.equals("")) {
			return IPs;
		}
		String[] ips = ip_list.split(":");
		for (String ip : ips) {
			IPs.add(parseIpString(ip));
		}
		return IPs;
	}
	
	public static String parseServerNodes(Collection<ServerNode> nodes) {
		StringBuffer sb = new StringBuffer();
		int index = 0;
		for (ServerNode node : nodes) {
			if (index > 0)
				sb.append(":");
			
			if (node.isbTcp()) {
				sb.append("tcp ");
			} else {
				sb.append("udp ");
			}
			sb.append("-h ").append(node.getsIP()).append(" ");
			sb.append("-p ").append(node.getPort()).append(" ");
			sb.append("-t ").append(node.getTimeout());
			if (node.getSet() != null && !node.getSet().equals("")) {
			    sb.append(" -s ").append(node.getSet());
			}
			if (node.getWeight() >= 0) {
				sb.append(" -w ").append(node.getWeight());
			}
			index++;
		}
		return sb.toString();
	}
	
	private static ServerNode parseIpString(String  ip) {
		ServerNode node = new ServerNode(); 
		String[] elements = ip.split("\\s+-\\w\\s+");
		if (elements[0].equalsIgnoreCase("tcp")) {
			node.setbTcp(true);
		} else {
			node.setbTcp(false);
		}
		node.setsIP(elements[1]);
		node.setPort(Integer.parseInt(elements[2]));
		node.setTimeout(Integer.parseInt(elements[3]));
		if (elements.length > 4 && ip.contains("-s")) {
			node.setSet(elements[4]);
		} else if (elements.length > 4) {
			node.setWeight(Integer.parseInt(elements[4]));
			node.setSet("");
		}
		if (elements.length > 5){
			node.setWeight(Integer.parseInt(elements[5]));
			node.setSet("");
		}
		return node;
	}
}
