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
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.json.JSONObject;

import com.qq.seer.agent.SeerAgentModule;
import com.qq.seer.common.log.LogUtils;
import com.qq.seer.common.prop.SystemProperty;
import com.qq.seer.common.servlet.CommonHttpServlet;
import com.qq.seer.common.servlet.CommonServletContext;

public class PageServlet extends CommonHttpServlet {
	
	
	/**
	 * 
	 */
	private static final long serialVersionUID = -8368043604479034927L;
	
	private static final String agentInstallUrl = SystemProperty.getValueByName("seer.agent.onekey.install.url");

	public static String packageManage(CommonServletContext context) {
		HttpServletRequest request = context.getRequest();
		HttpServletResponse response = context.getResponse();
		
		long logId = System.currentTimeMillis();
		
		try {
			
			LogUtils.localLogInfo(logId, "packageManage ...");
			String osVersion = request.getParameter("os_version");
			LogUtils.localLogInfo(logId, "packageManage osVersion=" + osVersion);
			
			List<Map<String, String>> osList = SeerAgentModule.getOSList(logId);
			List<Map<String, String>> fileList = SeerAgentModule.getAgentPackageList(logId, osVersion);
			
			request.setAttribute("fileList", fileList);
			request.setAttribute("osList", osList);
			
			
			request.getRequestDispatcher("/pages/ftl/package/manage.ftl").forward(request, response);
			return "1";
		} catch (Exception e) {
			String errMsg = "packageManage Exception : " + e.getMessage();
			LogUtils.localLogError(logId, errMsg, e);
			try {
				request.setAttribute("ret_code", 500);
				request.setAttribute("err_msg", errMsg);
				request.getRequestDispatcher("/pages/ftl/package/manage.ftl").forward(request, response);
			} catch (Exception e1) {
				LogUtils.localLogError(logId, "Exception: " + e1.getMessage(), e1);
			}
			return "-1";
		}
	}
	
	public static String uploadPage(CommonServletContext context) {
		HttpServletRequest request = context.getRequest();
		HttpServletResponse response = context.getResponse();
		
		long logId = System.currentTimeMillis();
		
		try {
			LogUtils.localLogInfo(logId, "uploadPage ...");
			
			List<Map<String, String>> osList = SeerAgentModule.getOSList(logId);
			request.setAttribute("osList", osList);
			
			request.getRequestDispatcher("/pages/ftl/package/upload.ftl").forward(request, response);
			return "1";
		} catch (Exception e) {
			String errMsg = "uploadPage Exception : " + e.getMessage();
			LogUtils.localLogError(logId, errMsg, e);
			try {
				request.setAttribute("ret_code", 500);
				request.setAttribute("err_msg", errMsg);
				request.getRequestDispatcher("/pages/ftl/package/upload.ftl").forward(request, response);
			} catch (Exception e1) {
				LogUtils.localLogError(logId, "Exception: " + e1.getMessage(), e1);
			}
			return "-1";
		}
	}
	
	
	public static String updatePackage(CommonServletContext context) throws IOException {
		HttpServletRequest request = context.getRequest();
		HttpServletResponse response = context.getResponse();
		
		long logId = System.currentTimeMillis();
		
		
		
		try {
			LogUtils.localLogInfo(logId, "updatePackage ...");
			
			JSONObject jsonObj = new JSONObject();
			
			String packageName = request.getParameter("package_name");
			String osVersion = request.getParameter("os_version");
			String packageType = request.getParameter("package_type");
			
			
			
			JSONObject resultObj = SeerAgentModule.updateAgentPackageInfo(logId, packageName, osVersion, packageType);
			int retCode = resultObj.getInt("ret_code");
		    if(retCode==0) {
		    	jsonObj.put("ret_code", 200);
				jsonObj.put("err_msg", "");
				jsonObj.put("data", new JSONObject());
		    } else {
		    	String errMsg = resultObj.getString("err_msg");
		    	jsonObj.put("ret_code", 500);
				jsonObj.put("err_msg", errMsg);
				jsonObj.put("data", new JSONObject());
		    }

			
			response.getWriter().write(jsonObj.toString());
			return "1";
		} catch (Exception e) {
			String errMsg = "updatePackage Exception : " + e.getMessage();
			LogUtils.localLogError(logId, errMsg, e);
			JSONObject jsonObj = new JSONObject();
			jsonObj.put("ret_code", 500);
			jsonObj.put("err_msg", errMsg);
			jsonObj.put("data", new JSONObject());
			response.getWriter().write(jsonObj.toString());
			return "-1";
		}
	}
	
	
	public static String delPackage(CommonServletContext context) throws IOException {
		HttpServletRequest request = context.getRequest();
		HttpServletResponse response = context.getResponse();
		
		long logId = System.currentTimeMillis();
		
		try {
			LogUtils.localLogInfo(logId, "delPackage ...");
			
			JSONObject jsonObj = new JSONObject();
			
			String packageName = request.getParameter("package_name");
			String osVersion = request.getParameter("os_version");
			
			JSONObject resultObj = SeerAgentModule.deleteAgentPackage(logId, packageName, osVersion);
			int retCode = resultObj.getInt("ret_code");
		    if(retCode==0) {
		    	jsonObj.put("ret_code", 200);
				jsonObj.put("err_msg", "");
				jsonObj.put("data", new JSONObject());
		    } else {
		    	String errMsg = resultObj.getString("err_msg");
		    	jsonObj.put("ret_code", 500);
				jsonObj.put("err_msg", errMsg);
				jsonObj.put("data", new JSONObject());
		    }

			
			response.getWriter().write(jsonObj.toString());
			return "1";
		} catch (Exception e) {
			String errMsg = "delPackage Exception : " + e.getMessage();
			LogUtils.localLogError(logId, errMsg, e);
			JSONObject jsonObj = new JSONObject();
			jsonObj.put("ret_code", 500);
			jsonObj.put("err_msg", errMsg);
			jsonObj.put("data", new JSONObject());
			response.getWriter().write(jsonObj.toString());
			return "-1";
		}
	}
	
	public static String release(CommonServletContext context) throws IOException {
		HttpServletRequest request = context.getRequest();
		HttpServletResponse response = context.getResponse();
		
		long logId = System.currentTimeMillis();
		
		try {
			LogUtils.localLogInfo(logId, "release ...");
			
			JSONObject jsonObj = new JSONObject();
			
			String ips = request.getParameter("ips");
			String osVersion = request.getParameter("os_version");
			String packageType = request.getParameter("package_type");
			
			LogUtils.localLogInfo(logId, "ips=" + ips);
			LogUtils.localLogInfo(logId, "osVersion=" + osVersion);
			LogUtils.localLogInfo(logId, "packageType=" + packageType);
			
			
			JSONObject resultObj = SeerAgentModule.updateAgenGrayState(logId, ips, osVersion, packageType);
			int retCode = resultObj.getInt("ret_code");
		    if(retCode==0) {
		    	jsonObj.put("ret_code", 200);
				jsonObj.put("err_msg", "");
				jsonObj.put("data", new JSONObject());
		    } else {
		    	String errMsg = resultObj.getString("err_msg");
		    	jsonObj.put("ret_code", 500);
				jsonObj.put("err_msg", errMsg);
				jsonObj.put("data", new JSONObject());
		    }

			
			response.getWriter().write(jsonObj.toString());
			return "1";
		} catch (Exception e) {
			String errMsg = "release Exception : " + e.getMessage();
			LogUtils.localLogError(logId, errMsg, e);
			JSONObject jsonObj = new JSONObject();
			jsonObj.put("ret_code", 500);
			jsonObj.put("err_msg", errMsg);
			jsonObj.put("data", new JSONObject());
			response.getWriter().write(jsonObj.toString());
			return "-1";
		}
	}
	
	
	public static String agentRouterInstallPage(CommonServletContext context) {
		HttpServletRequest request = context.getRequest();
		HttpServletResponse response = context.getResponse();
		
		long logId = System.currentTimeMillis();
		
		try {
			LogUtils.localLogInfo(logId, "agentRouterInstallPage ...");
			
			List<Map<String, String>> osList = SeerAgentModule.getOSList(logId);
			request.setAttribute("osList", osList);
			
			request.getRequestDispatcher("/pages/ftl/router_manager/agentRouterInstall.ftl").forward(request, response);
			return "1";
		} catch (Exception e) {
			String errMsg = "agentRouterInstallPage Exception : " + e.getMessage();
			LogUtils.localLogError(logId, errMsg, e);
			try {
				request.setAttribute("ret_code", 500);
				request.setAttribute("err_msg", errMsg);
				request.getRequestDispatcher("/pages/ftl/router_manager/agentRouterInstall.ftl").forward(request, response);
			} catch (Exception e1) {
				LogUtils.localLogError(logId, "Exception: " + e1.getMessage(), e1);
			}
			return "-1";
		}
	}
	
	
	public static String agentRouterInstallCommand(CommonServletContext context) throws IOException {
		HttpServletRequest request = context.getRequest();
		HttpServletResponse response = context.getResponse();
		
		long logId = System.currentTimeMillis();
		
		try {
			LogUtils.localLogInfo(logId, "agentRouterInstallCommand ...");
			
			String ip = request.getParameter("ip");
			if(ip== null || ip.length() < 1) {
				JSONObject jsonObj = new JSONObject();
				jsonObj.put("ret_code", 500);
				jsonObj.put("err_msg", "ip=" + ip);
				jsonObj.put("data", new JSONObject());
				response.getWriter().write(jsonObj.toString());
			}
			
			String command = "wget " + agentInstallUrl + " && python installscript --innerip=" + ip;
			
			JSONObject dataObj = new JSONObject();
			dataObj.put("ip", ip);
			dataObj.put("command", command);
			
			JSONObject jsonObj = new JSONObject();
			jsonObj.put("ret_code", 200);
			jsonObj.put("err_msg", "");
			jsonObj.put("data", dataObj);
			response.getWriter().write(jsonObj.toString());
			return "1";
		} catch (Exception e) {
			String errMsg = "agentRouterInstallCommand Exception : " + e.getMessage();
			LogUtils.localLogError(logId, errMsg, e);
			JSONObject jsonObj = new JSONObject();
			jsonObj.put("ret_code", 500);
			jsonObj.put("err_msg", errMsg);
			jsonObj.put("data", new JSONObject());
			response.getWriter().write(jsonObj.toString());
			return "-1";
		}
	}
	
	
	public static String grayReleasedPage(CommonServletContext context) {
		HttpServletRequest request = context.getRequest();
		HttpServletResponse response = context.getResponse();
		long logId = System.currentTimeMillis();
		
		try {
			LogUtils.localLogInfo(logId, "grayReleasedPage ...");
			request.setAttribute("isAdmin", "true");
			
			request.getRequestDispatcher("/pages/ftl/package/grayReleasedPage.ftl").forward(request, response);
			return "1";
		} catch (Exception e) {
			String errMsg = "grayReleasedPage Exception : " + e.getMessage();
			LogUtils.localLogError(logId, errMsg, e);
			try {
				request.setAttribute("ret_code", 500);
				request.setAttribute("err_msg", errMsg);
				request.getRequestDispatcher("/pages/ftl/package/grayReleasedPage.ftl").forward(request, response);
			} catch (Exception e1) {
				LogUtils.localLogError(logId, "Exception: " + e1.getMessage(), e1);
			}
			return "-1";
		}
	}
	
	public static String incGrayReleasedPage(CommonServletContext context) {
		HttpServletRequest request = context.getRequest();
		HttpServletResponse response = context.getResponse();
		long logId = System.currentTimeMillis();
		
		try {
			LogUtils.localLogInfo(logId, "incGrayReleasedPage ...");
			
			
			String ip =request.getParameter("ip");
			String osVersion=request.getParameter("os_version");
			String state=request.getParameter("present_state");
			
			List<Map<String, String>> ipList = SeerAgentModule.getAgentBaseList(logId, ip, osVersion, state);
			request.setAttribute("ipList", ipList);
			request.setAttribute("ip", ip);
			request.setAttribute("os_version", osVersion);
			request.setAttribute("present_state", state);
			request.setAttribute("isAdmin", "true");
			
			request.getRequestDispatcher("/pages/ftl/package/inc/incGrayReleasedPage.ftl").forward(request, response);
			return "1";
		} catch (Exception e) {
			String errMsg = "grayReleasedPage Exception : " + e.getMessage();
			LogUtils.localLogError(logId, errMsg, e);
			try {
				request.setAttribute("ret_code", 500);
				request.setAttribute("err_msg", errMsg);
				request.getRequestDispatcher("/pages/ftl/package/inc/incGrayReleasedPage.ftl").forward(request, response);
			} catch (Exception e1) {
				LogUtils.localLogError(logId, "Exception: " + e1.getMessage(), e1);
			}
			return "-1";
		}
	}
	
	
	public static String serviceGroupList(CommonServletContext context) {
		HttpServletRequest request = context.getRequest();
		HttpServletResponse response = context.getResponse();
		long logId = System.currentTimeMillis();
		
		try {
			LogUtils.localLogInfo(logId, "serviceGroupList ...");
			
			request.getRequestDispatcher("/pages/ftl/router_manager/serviceGroupList.ftl").forward(request, response);
			return "1";
		} catch (Exception e) {
			String errMsg = "serviceGroupList Exception : " + e.getMessage();
			LogUtils.localLogError(logId, errMsg, e);
			try {
				request.setAttribute("ret_code", 500);
				request.setAttribute("err_msg", errMsg);
				request.getRequestDispatcher("/pages/ftl/router_manager/serviceGroupList.ftl").forward(request, response);
			} catch (Exception e1) {
				LogUtils.localLogError(logId, "Exception: " + e1.getMessage(), e1);
			}
			return "-1";
		}
	}
	
	
	public static String ipPortList(CommonServletContext context) {
		HttpServletRequest request = context.getRequest();
		HttpServletResponse response = context.getResponse();
		long logId = System.currentTimeMillis();
		
		try {
			LogUtils.localLogInfo(logId, "ipPortList ...");
			
			String serviceGroup =request.getParameter("service_group");
			String moduleName=request.getParameter("module_name");
			String appName = "";
			String serverName = "";
			if(moduleName != null && moduleName.length() > 0) {
				String[] ms = moduleName.split("\\.");
				appName =  ms[0];
				serverName =  ms[1];
			}
			
			request.setAttribute("service_group", serviceGroup);
			request.setAttribute("app_name", appName);
			request.setAttribute("servername", serverName);
			request.setAttribute("hasPrivileges", true);
			
			request.getRequestDispatcher("/pages/ftl/router_manager/ipPortList.ftl").forward(request, response);
			return "1";
		} catch (Exception e) {
			String errMsg = "ipPortList Exception : " + e.getMessage();
			LogUtils.localLogError(logId, errMsg, e);
			try {
				request.setAttribute("ret_code", 500);
				request.setAttribute("err_msg", errMsg);
				request.getRequestDispatcher("/pages/ftl/router_manager/ipPortList.ftl").forward(request, response);
			} catch (Exception e1) {
				LogUtils.localLogError(logId, "Exception: " + e1.getMessage(), e1);
			}
			return "-1";
		}
	}
	
}
