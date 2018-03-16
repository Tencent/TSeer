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

package com.qq.seer.admin;

import java.util.ArrayList;
import java.util.List;

import com.qq.seer.common.log.LogUtils;
import com.qq.seer.common.resource.ResourceUtils;
import com.qq.tars.tseer.RegistryPrx;
import com.qq.tars.client.Communicator;
import com.qq.tars.client.CommunicatorConfig;
import com.qq.tars.client.CommunicatorFactory;
import com.qq.tars.common.util.Config;
import com.qq.tars.support.query.prx.EndpointF;

public class SeerAdmin {
	
	private static final SeerAdmin seerAdmin = new SeerAdmin();
	private static String communicatorPath = ResourceUtils.getResourcePath("seer.conf");
	private static String seerServerRegistryObj = "Tseer.TseerServer.RegistryObj";
	
	private Communicator communicator = null;
	//private RegistryPrx registryPrx = null;
	
	
	private SeerAdmin() {
		init();
	}
	
	/**
	 * 单实例
	 * @return
	 */
	public static SeerAdmin getInstance(){
		return seerAdmin;
	}
	
	/**
	 * 初始化通讯器
	 */
	private void init(){
		try {
			LogUtils.localLogInfo("SeerAdmin init, communicatorPath=" + communicatorPath);
			CommunicatorConfig config = new CommunicatorConfig().load(Config.parseFile(communicatorPath));

			LogUtils.localLogInfo("SeerAdmin  init  config :" + config.getCharsetName());
			communicator = CommunicatorFactory.getInstance().getCommunicator(config);
		} catch (Exception e) {
			String error = "SeerAdmin init Exception : " + e.getMessage();
			LogUtils.localLogError(error, e);
		}
	}
	
	/**
	 * 获取通讯器
	 * @return
	 */
	private Communicator getCommunicator(){
		try {
			if (communicator == null) {
				if(communicatorPath==null || communicatorPath.length() < 1) {
					communicatorPath = ResourceUtils.getResourcePath("seer.conf");
				}
				
				LogUtils.localLogInfo("SeerAdmin getCommunicator,communicator=" + communicator + ",communicatorPath=" + communicatorPath);
				CommunicatorConfig config = new CommunicatorConfig().load(Config.parseFile(communicatorPath));

				LogUtils.localLogInfo("SeerAdmin  getCommunicator  config  chartName b :" + config.getCharsetName());
				LogUtils.localLogInfo("SeerAdmin  getCommunicator  config  getLocator :" + config.getLocator());

				communicator = CommunicatorFactory.getInstance().getCommunicator(config);

				LogUtils.localLogInfo("SeerAdmin  getCommunicator  config  chartName a :" + communicator.getCommunicatorConfig().getCharsetName());
			}
			LogUtils.localLogInfo("SeerAdmin  getCommunicator  communicator : " + communicator);
			return communicator;
		} catch (Exception e) {
			String error = "getCommunicator Exception : " + e.getMessage();
			LogUtils.localLogError(error, e);
			return null;
		}
	}
	
	
	private List<EndpointF> getEndpoint4All() {
		LogUtils.localLogInfo("SeerAdmin  getEndpoint4All  ... ");
		List<EndpointF> list = null;
		try {
			Communicator c = getCommunicator();
			LogUtils.localLogInfo("SeerAdmin  getEndpoint4All  c : " + c);
			LogUtils.localLogInfo("SeerAdmin  getEndpoint4All  c getLocator : " + c.getCommunicatorConfig().getLocator());
			LogUtils.localLogInfo("SeerAdmin  getEndpoint4All  seerServerRegistryObj : " + seerServerRegistryObj);
			list = c.getEndpoint4All(seerServerRegistryObj);
			LogUtils.localLogInfo("SeerAdmin  getEndpoint4All  list : " + list);
			
		} catch (Exception e) {
			String error = "getEndpoint4All Exception : " + e.getMessage();
			LogUtils.localLogError(error, e);
		}
		return list;
	}
	
	
	public List<RegistryPrx> getRegistryPrx() {
		
		
		List<RegistryPrx> regList = new ArrayList<RegistryPrx>();
		List<EndpointF> list = getEndpoint4All();
		for (EndpointF endpoint : list) {
            String ip = endpoint.getHost();
            int port = endpoint.getPort();
            String objectName = seerServerRegistryObj + "@tcp -h " + ip + " -p " + port;
            LogUtils.localLogInfo("SeerAdmin  getRegistryPrx=" + objectName);
            
            RegistryPrx registryPrx = getCommunicator().stringToProxy(RegistryPrx.class, objectName);
            regList.add(registryPrx);
            
		}
		
		return regList;
	}

}
