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

package com.qq.seer.agent;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.json.JSONArray;
import org.json.JSONObject;

import com.qq.seer.http.SeerInterface;

public class SeerAgentModule {
	
	public static JSONObject updateAgentPackageInfo(long logId, String packageName, String ostype, String packageType) {
		String interfaceName = "updateagentpackageinfo";
		JSONObject paramsObj = new JSONObject();
		paramsObj.put("package_name", packageName);
		paramsObj.put("ostype", ostype);
		paramsObj.put("package_type", packageType);
		
		JSONObject resultObj = SeerInterface.callSeerInterface(logId, paramsObj, interfaceName);
		return resultObj;
	}
	
	public static JSONObject deleteAgentPackage(long logId, String packageName, String ostype) {
		String interfaceName = "deleteagentpackage";
		JSONObject paramsObj = new JSONObject();
		paramsObj.put("package_name", packageName);
		paramsObj.put("ostype", ostype);
		
		JSONObject resultObj = SeerInterface.callSeerInterface(logId, paramsObj, interfaceName);
		return resultObj;
	}
	
	
	public static JSONObject updateAgenGrayState(long logId, String iplist, String ostype, String grayscale) {
		String interfaceName = "updateagentgraystate";
		JSONObject paramsObj = new JSONObject();
		paramsObj.put("iplist", iplist);
		paramsObj.put("ostype", ostype);
		paramsObj.put("grayscale_state", grayscale);
		
		JSONObject resultObj = SeerInterface.callSeerInterface(logId, paramsObj, interfaceName);
		return resultObj;
	}
	
	public static List<Map<String, String>> getOSList(long logId) {
        List<Map<String, String>> osList = new ArrayList<Map<String, String>>();
        JSONObject resultObj = getAgentOsTypeInfo(logId);
        int retCode = resultObj.getInt("ret_code");
        if(retCode==0) {
        	JSONArray array = resultObj.getJSONArray("data");
        	for(int i=0; i<array.length(); i++) {
        		JSONObject osObj = array.getJSONObject(i);
        		String ostype = osObj.getString("ostype");
        		Map<String,String> map = new HashMap<String,String>();
                map.put("os_version", ostype);
                map.put("os_desc", ostype);
                osList.add(map);
        	}
        }
        return osList;
	}
	
	
	 public static JSONObject getAgentOsTypeInfo(long logId) {
		String interfaceName = "getagentostypeinfo";
		JSONObject paramsObj = new JSONObject();
		JSONObject resultObj = SeerInterface.callSeerInterface(logId, paramsObj, interfaceName);
		return resultObj;
	 }
	 
	 
	 public static List<Map<String, String>> getAgentPackageList(long logId, String ostype) {
        List<Map<String, String>> packageList = new ArrayList<Map<String, String>>();
        JSONObject resultObj = getAgentOsTypeInfo(logId, ostype);
        int retCode = resultObj.getInt("ret_code");
        if(retCode==0) {
        	JSONArray array = resultObj.getJSONArray("data");
        	for(int i=0; i<array.length(); i++) {
        		JSONObject packageObj = array.getJSONObject(i);
        		String os = packageObj.getString("ostype");
        		String version = packageObj.getString("version");
        		String packageType = packageObj.getString("package_type");
        		String packageName = packageObj.getString("package_name");
        		
        		
        		Map<String,String> map = new HashMap<String,String>();
                map.put("id", "");
                map.put("package_type", packageType);
                map.put("filename", packageName);
                map.put("nodeOrProxy", "router_agent");
                map.put("version", version);
                map.put("os_version", os);
                packageList.add(map);
        	}
        }
        return packageList;
	}
	
	
	 public static JSONObject getAgentOsTypeInfo(long logId, String ostype) {
		String interfaceName = "getagentpackageinfo";
		JSONObject paramsObj = new JSONObject();
		if(ostype != null && ostype.length() > 0) {
			paramsObj.put("ostype", ostype);
        }
		JSONObject resultObj = SeerInterface.callSeerInterface(logId, paramsObj, interfaceName);
		return resultObj;
	 }
	 
	 
	 
	 public static List<Map<String, String>> getAgentBaseList(long logId, String ip, String ostype, String presentState) {
        List<Map<String, String>> agentList = new ArrayList<Map<String, String>>();
        JSONObject resultObj = getAgentBaseInfo(logId, ip,  ostype, presentState);
        int retCode = resultObj.getInt("ret_code");
        if(retCode==0) {
        	JSONArray array = resultObj.getJSONArray("data");
        	for(int i=0; i<array.length(); i++) {
        		JSONObject agentObj = array.getJSONObject(i);
        		String locator = agentObj.getString("locator");
        		String grayscale_state = agentObj.getString("grayscale_state");
        		String last_reg_time = agentObj.getString("last_reg_time");
        		String os = agentObj.getString("ostype");
        		String formal_version = agentObj.getString("formal_version");
        		String present_state = agentObj.getString("present_state");
        		String gray_version = agentObj.getString("gray_version");
        		String last_heartbeat_time = agentObj.getString("last_heartbeat_time");
        		String version = agentObj.getString("version");
        		String agentip = agentObj.getString("ip");
        		
        		if(grayscale_state ==null || grayscale_state.length() < 1) {
        			grayscale_state="0";
        		}
        		
        		
        		Map<String,String> map = new HashMap<String,String>();
        		map.put("locator", locator);
        		map.put("last_reg_time", last_reg_time);
        		map.put("present_state", present_state);
        		map.put("formal_ver", formal_version);
        		map.put("locatorIp", locator);
        		map.put("cur_ver", version);
        		map.put("ip", agentip);
        		map.put("os_version", os);
        		map.put("product", "");
        		map.put("grayscale_state", grayscale_state);
        		map.put("last_heartbeat", last_heartbeat_time);
        		map.put("bg_name", "");
        		map.put("gray_ver", gray_version);
        		map.put("chargename", "");
                agentList.add(map);
        	}
        }
        return agentList;
	}
	
	
	 public static JSONObject getAgentBaseInfo(long logId, String ip, String ostype, String presentState) {
		String interfaceName = "getagentbaseinfo";
		JSONObject paramsObj = new JSONObject();
		if(ip != null && ip.length() > 0) {
			paramsObj.put("ip", ip);
        }
        
        if(ostype != null && ostype.length() > 0) {
        	paramsObj.put("ostype", ostype);
        }
 
        if(presentState != null && presentState.length() > 0) {
        	paramsObj.put("present_state", presentState);
        }
	        
	        
		JSONObject resultObj = SeerInterface.callSeerInterface(logId, paramsObj, interfaceName);
		return resultObj;
	 }
	 
	 
	 public static JSONObject getServiceGroupList(long logId) {
       
		JSONObject jsonObj = new JSONObject();
		 
        JSONObject resultObj = getServiceGroup(logId);
        int retCode = resultObj.getInt("ret_code");
        if(retCode==0) {
        	
        	JSONArray dataArray = new JSONArray();
        	
        	JSONArray array = resultObj.getJSONArray("data");
        	for(int i=0; i<array.length(); i++) {
        		JSONObject serObj = array.getJSONObject(i);
        		String serviceGroupName = serObj.getString("service_group");
        		//String rtx = serObj.getString("rtx");
        		String key = serObj.getString("key");
        		
        		JSONObject dataObj = new JSONObject();
        		dataObj.put("service_group", serviceGroupName);
        		dataObj.put("rtx", "");
        		dataObj.put("key", key);
        		dataObj.put("hasPrivileges", "1");
        		
        		JSONArray moduleArray = new JSONArray();
				JSONObject serverObj = getServer(logId, serviceGroupName, null, null);
				int serCode = serverObj.getInt("ret_code");
				if(serCode==0) {
					JSONArray serverArray = serverObj.getJSONArray("data");
					for(int j=0; j<serverArray.length(); j++) {
						JSONObject serDataObj = serverArray.getJSONObject(j);
						String appName = serDataObj.getString("app_name");
						String serverName = serDataObj.getString("servername");
						String moduleName = appName + "." + serverName;	 
						moduleArray.put(moduleName);
					 }
				}
        		dataObj.put("module_array", moduleArray);
        		
        		dataArray.put(dataObj);
           
        	}
        	
        	jsonObj.put("ret_code", 200);
        	jsonObj.put("err_msg", "");
        	jsonObj.put("data", dataArray);
        	
        } else {
        	String errMsg = resultObj.getString("err_msg");
        	jsonObj.put("ret_code", 500);
        	jsonObj.put("err_msg", errMsg);
        	jsonObj.put("data", new JSONArray());
        }
        return jsonObj;
	}
	 
	 
	 public static JSONObject getServiceGroup(long logId) {
		String interfaceName = "getservicegroup";
		JSONObject paramsObj = new JSONObject();
		paramsObj.put("rtx", "adminrtx");
		JSONObject resultObj = SeerInterface.callSeerInterface(logId, paramsObj, interfaceName);
		return resultObj;
	 }
	 
	 
	 public static JSONObject getServer(long logId, String serviceGroup, String appName, String servername) {
		String interfaceName = "getserver";
		JSONObject paramsObj = new JSONObject();
		paramsObj.put("service_group", serviceGroup);
		
		if(appName != null && appName.length() > 0) {
			paramsObj.put("app_name", appName);
		}
		
		if(servername != null && servername.length() > 0) {
			paramsObj.put("servername", servername);
		}

		JSONObject resultObj = SeerInterface.callSeerInterface(logId, paramsObj, interfaceName);
		return resultObj;
	 }
	 
	 
	 public static JSONObject getServerList(long logId, JSONObject paramsObj) {
		String serviceGroupName = paramsObj.getString("service_group");
		String appName = paramsObj.getString("app_name");
		String servername = paramsObj.getString("servername");
		JSONObject jsonObj = new JSONObject();
		JSONObject serverObj = getServer(logId, serviceGroupName, appName, servername);
		int serCode = serverObj.getInt("ret_code");
		if(serCode==0) {
			JSONArray serverArray = serverObj.getJSONArray("data");
			jsonObj.put("ret_code", 200);
        	jsonObj.put("err_msg", "");
        	jsonObj.put("data", serverArray);
		} else {
			String errMsg = serverObj.getString("err_msg");
        	jsonObj.put("ret_code", 500);
        	jsonObj.put("err_msg", errMsg);
        	jsonObj.put("data", new JSONArray());
		}
        return jsonObj;
	}
	 
	 
	 public static JSONObject getIdcGroup(long logId) {
		String interfaceName = "getidcgroup";
		JSONObject paramsObj = new JSONObject();
		JSONObject resultObj = SeerInterface.callSeerInterface(logId, paramsObj, interfaceName);
		return resultObj;
	 }
	 
	 public static JSONObject getIdcGroupList(long logId, JSONObject paramsObj) {
		
		JSONObject jsonObj = new JSONObject();
		JSONObject serverObj = getIdcGroup(logId);
		int serCode = serverObj.getInt("ret_code");
		if(serCode==0) {
			
			JSONArray dataArray = new JSONArray();
			
			JSONArray groupArray = serverObj.getJSONArray("data");
			for(int i=0; i<groupArray.length(); i++) {
				JSONObject dataObj = groupArray.getJSONObject(i);
				String groupName = dataObj.getString("group_name");
				String cn = dataObj.getString("group_name_cn");
				
				String key = groupName;
				String value = groupName + "," + cn;
				
				JSONObject groupObj = new JSONObject();
				groupObj.put("key", key);
				groupObj.put("value", value);
				
				dataArray.put(groupObj);
			}
			jsonObj.put("ret_code", 200);
        	jsonObj.put("err_msg", "");
        	jsonObj.put("data", dataArray);
		} else {
			String errMsg = serverObj.getString("err_msg");
        	jsonObj.put("ret_code", 500);
        	jsonObj.put("err_msg", errMsg);
        	jsonObj.put("data", new JSONArray());
		}
        return jsonObj;
	}
	 
	 
	 public static JSONObject addServiceGroup(long logId, String serviceGroup, String manager) {
		String interfaceName = "addservicegroup";
		JSONObject paramsObj = new JSONObject();
		paramsObj.put("service_group", serviceGroup);
		paramsObj.put("rtx", manager);
		
		JSONObject resultObj = SeerInterface.callSeerInterface(logId, paramsObj, interfaceName);
		return resultObj;
	 }
	 
	 
	 public static JSONObject addServiceGroup(long logId, JSONObject paramsObj) {
			
		JSONObject jsonObj = new JSONObject();
		
		String serviceGroup = paramsObj.getString("service_group");
		String manager = paramsObj.optString("manager");
		
		JSONObject serverObj = addServiceGroup(logId, serviceGroup, manager);
		int retCode = serverObj.getInt("ret_code");
		if(retCode==0) {
			jsonObj.put("ret_code", 200);
        	jsonObj.put("err_msg", "");
        	jsonObj.put("data", new JSONArray());
		} else {
			String errMsg = serverObj.getString("err_msg");
        	jsonObj.put("ret_code", 500);
        	jsonObj.put("err_msg", errMsg);
        	jsonObj.put("data", new JSONArray());
		}
        return jsonObj;
	}
	 
	 
	 public static JSONObject addServer(long logId, JSONObject paramsObj) {
		String interfaceName = "addserver";		
		JSONObject resultObj = SeerInterface.callSeerInterface(logId, paramsObj, interfaceName);
		return resultObj;
	 }
	 
	 
	 public static JSONObject delServer(long logId, JSONObject paramsObj) {
		String interfaceName = "delserver";		
		JSONObject resultObj = SeerInterface.callSeerInterface(logId, paramsObj, interfaceName);
		return resultObj;
	 }
	 
	 
	 public static JSONObject updateServer(long logId, JSONObject paramsObj) {
		String interfaceName = "updateserver";		
		JSONObject resultObj = SeerInterface.callSeerInterface(logId, paramsObj, interfaceName);
		return resultObj;
	 }
	 
	 
	 public static JSONObject updateAgentLocator(long logId, JSONObject paramsObj) {
		String interfaceName = "updateagentlocator";		
		JSONObject resultObj = SeerInterface.callSeerInterface(logId, paramsObj, interfaceName);
		return resultObj;
	 }
	 
	 
	 public static JSONObject deleteAgent(long logId, JSONObject jsonObj) {
		String interfaceName = "deleteagent";
		
		String ipList = jsonObj.getString("ipList");
		
		if(ipList != null && ipList.length() > 0) {
			ipList.replaceAll(";$", "");
		}
		
		JSONObject paramsObj = new JSONObject();
		paramsObj.put("iplist", ipList);
		paramsObj.put("key", "webadmin");
		
		JSONObject resultObj = SeerInterface.callSeerInterface(logId, paramsObj, interfaceName);
		return resultObj;
	 }
	 
	 

}
