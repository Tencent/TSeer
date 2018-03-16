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

package com.qq.seer.servlet;

import java.io.IOException;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.json.JSONArray;
import org.json.JSONObject;

import com.qq.seer.agent.SeerAgentModule;
import com.qq.seer.common.log.LogUtils;

public class InterfaceServlet extends HttpServlet {
	
	
	/**
	 * 
	 */
	private static final long serialVersionUID = -6394790452570157472L;


	protected void doGet(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
		process(request, response);
	}

	protected void doPost(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
		doGet(request, response);
	}
	
	
	protected void process(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
		
		try {
			long logId = System.currentTimeMillis();
			
			String interfaceName=request.getParameter("interface_name");
			String interfaceParams=request.getParameter("interface_params");
			LogUtils.localLogInfo(logId, "interfaceName=" + interfaceName);
			LogUtils.localLogInfo(logId, "interfaceParams=" + interfaceParams);
			
			if("routerapi_get_service_group_list".equals(interfaceName)) {
				routerApiGetServiceGroupList(logId, request, response, interfaceName, interfaceParams);
			} else if("routerapi_get_server".equals(interfaceName)) {
				routerApiGetServer(logId, request, response, interfaceName, interfaceParams);
			} else if("routerapi_get_idc_group_list".equals(interfaceName)) {
				routerApiGetIdcGroupList(logId, request, response, interfaceName, interfaceParams);
			} else if("routerapi_add_service_group".equals(interfaceName)) {
				routerApiAddServiceGroup(logId, request, response, interfaceName, interfaceParams);
			} else if("routerapi_add_ip_port".equals(interfaceName)) {
				routerApiAddIpPort(logId, request, response, interfaceName, interfaceParams);
			} else if("routerapi_del_ip_port".equals(interfaceName)) {
				routerApiDelIpPort(logId, request, response, interfaceName, interfaceParams);
			} else if("routerapi_update_ip_port".equals(interfaceName)) {
				routerApiUpdateIpPort(logId, request, response, interfaceName, interfaceParams);
			} else if("routerapi_update_agent_locator".equals(interfaceName)) {
				routerApiUpdateAgentLocator(logId, request, response, interfaceName, interfaceParams);
			} else if("delete_agent".equals(interfaceName)) {
				deleteAgent(logId, request, response, interfaceName, interfaceParams);
			} else {
				JSONObject jsonObj = new JSONObject();
				jsonObj.put("ret_code", 500);
				jsonObj.put("err_msg", "Unknow interface，interface_name=" + interfaceName);
				jsonObj.put("data", new JSONArray());
				response.getWriter().write(jsonObj.toString());
			}
		} catch (Exception e) {
			String error = e.getMessage();
			JSONObject jsonObj = new JSONObject();
			jsonObj.put("ret_code", 500);
			jsonObj.put("err_msg", error);
			jsonObj.put("data", new JSONArray());
			response.getWriter().write(jsonObj.toString());
		}
	}
	
	
	public void routerApiGetServiceGroupList(long logId, HttpServletRequest request, HttpServletResponse response, String interfaceName, String interfaceParams) throws ServletException, IOException {
		//JSONObject paramsObj = new JSONObject(interfaceParams);
		
		JSONObject resultObj = SeerAgentModule.getServiceGroupList(logId);
		response.getWriter().write(resultObj.toString());
		
	}
	
	
	public void routerApiGetServer(long logId, HttpServletRequest request, HttpServletResponse response, String interfaceName, String interfaceParams) throws ServletException, IOException {
		JSONObject paramsObj = new JSONObject(interfaceParams);
		
		JSONObject resultObj = SeerAgentModule.getServerList(logId, paramsObj);
		response.getWriter().write(resultObj.toString());
		
	}
	
	
	public void routerApiGetIdcGroupList(long logId, HttpServletRequest request, HttpServletResponse response, String interfaceName, String interfaceParams) throws ServletException, IOException {
		JSONObject paramsObj = new JSONObject(interfaceParams);
		
		JSONObject resultObj = SeerAgentModule.getIdcGroupList(logId, paramsObj);
		response.getWriter().write(resultObj.toString());
		
	}
	
	
	public void routerApiAddServiceGroup(long logId, HttpServletRequest request, HttpServletResponse response, String interfaceName, String interfaceParams) throws ServletException, IOException {
		JSONObject paramsObj = new JSONObject(interfaceParams);
		
		JSONObject resultObj = SeerAgentModule.addServiceGroup(logId, paramsObj);
		response.getWriter().write(resultObj.toString());
		
	}
	
	public void routerApiAddIpPort(long logId, HttpServletRequest request, HttpServletResponse response, String interfaceName, String interfaceParams) throws ServletException, IOException {
		JSONObject paramsObj = new JSONObject(interfaceParams);
		
		JSONObject resultObj = new JSONObject();
		
		JSONObject jsonObj = SeerAgentModule.addServer(logId, paramsObj);
		int retCode = jsonObj.getInt("ret_code");
		if(retCode==0) {
			resultObj.put("ret_code", 200);
			resultObj.put("err_msg", "");
			resultObj.put("data", new JSONArray());
		} else {
			String errMsg = jsonObj.getString("err_msg");
			resultObj.put("ret_code", 500);
			resultObj.put("err_msg", errMsg);
			resultObj.put("data", new JSONArray());
		}
		response.getWriter().write(resultObj.toString());
		
	}
	
	public void routerApiDelIpPort(long logId, HttpServletRequest request, HttpServletResponse response, String interfaceName, String interfaceParams) throws ServletException, IOException {
		JSONObject paramsObj = new JSONObject(interfaceParams);
		
		JSONObject resultObj = new JSONObject();
		
		JSONObject jsonObj = SeerAgentModule.delServer(logId, paramsObj);
		int retCode = jsonObj.getInt("ret_code");
		if(retCode==0) {
			resultObj.put("ret_code", 200);
			resultObj.put("err_msg", "");
			resultObj.put("data", new JSONArray());
		} else {
			String errMsg = jsonObj.getString("err_msg");
			resultObj.put("ret_code", 500);
			resultObj.put("err_msg", errMsg);
			resultObj.put("data", new JSONArray());
		}
		response.getWriter().write(resultObj.toString());
		
	}
	
	public void routerApiUpdateIpPort(long logId, HttpServletRequest request, HttpServletResponse response, String interfaceName, String interfaceParams) throws ServletException, IOException {
		JSONObject paramsObj = new JSONObject(interfaceParams);
		
		JSONObject resultObj = new JSONObject();
		
		JSONObject jsonObj = SeerAgentModule.updateServer(logId, paramsObj);
		int retCode = jsonObj.getInt("ret_code");
		if(retCode==0) {
			resultObj.put("ret_code", 200);
			resultObj.put("err_msg", "");
			resultObj.put("data", new JSONArray());
		} else {
			String errMsg = jsonObj.getString("err_msg");
			resultObj.put("ret_code", 500);
			resultObj.put("err_msg", errMsg);
			resultObj.put("data", new JSONArray());
		}
		response.getWriter().write(resultObj.toString());
		
	}
	
	
	public void routerApiUpdateAgentLocator(long logId, HttpServletRequest request, HttpServletResponse response, String interfaceName, String interfaceParams) throws ServletException, IOException {
		JSONObject paramsObj = new JSONObject(interfaceParams);
		
		JSONObject resultObj = new JSONObject();
		
		JSONObject jsonObj = SeerAgentModule.updateAgentLocator(logId, paramsObj);
		int retCode = jsonObj.getInt("ret_code");
		if(retCode==0) {
			resultObj.put("ret_code", 200);
			resultObj.put("err_msg", "");
			resultObj.put("data", new JSONArray());
		} else {
			String errMsg = jsonObj.getString("err_msg");
			resultObj.put("ret_code", 500);
			resultObj.put("err_msg", errMsg);
			resultObj.put("data", new JSONArray());
		}
		response.getWriter().write(resultObj.toString());
		
	}
	
	
	public void deleteAgent(long logId, HttpServletRequest request, HttpServletResponse response, String interfaceName, String interfaceParams) throws ServletException, IOException {
		JSONObject paramsObj = new JSONObject(interfaceParams);
		
		JSONObject resultObj = new JSONObject();
		
		JSONObject jsonObj = SeerAgentModule.deleteAgent(logId, paramsObj);
		int retCode = jsonObj.getInt("ret_code");
		if(retCode==0) {
			resultObj.put("ret_code", 200);
			resultObj.put("err_msg", "");
			resultObj.put("data", new JSONArray());
		} else {
			String errMsg = jsonObj.getString("err_msg");
			resultObj.put("ret_code", 500);
			resultObj.put("err_msg", errMsg);
			resultObj.put("data", new JSONArray());
		}
		response.getWriter().write(resultObj.toString());
		
	}

}
