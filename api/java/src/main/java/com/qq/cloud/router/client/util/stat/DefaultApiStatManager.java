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

package com.qq.cloud.router.client.util.stat;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.TimeUnit;

import com.qq.cloud.router.client.RouterRequest;
import com.qq.cloud.router.client.RouterResponse;
import com.qq.cloud.router.client.util.Constants;
import com.qq.cloud.router.client.util.ScheduledExecutorManager;
import com.qq.tars.support.stat.InvokeStatHelper;
import com.qq.tars.support.stat.ProxyStat;
import com.qq.tars.support.stat.ProxyStatUtils;
import com.qq.tars.support.stat.StatHelper;

public class DefaultApiStatManager implements StatManager {

	private static final DefaultApiStatManager instance = new DefaultApiStatManager();
	private static final List<String> isStatReported = new ArrayList<String>();
	private static final ScheduledExecutorManager executorManager = ScheduledExecutorManager.getInstance();
	
	private StatHelper statHelper;
	
	private DefaultApiStatManager() {
	}
	
	private void setStatHelper(StatHelper statHelper) {
		this.statHelper = statHelper;
	}
	
	public static DefaultApiStatManager getInstance(StatHelper statHelper) {
		instance.setStatHelper(statHelper);
		return instance;
	}
	
	@Override
	public void reportStat(RouterRequest req, RouterResponse res, int ret,
			long costtime) {
		ProxyStat targetStat = InvokeStatHelper.getInstance().addProxyStat(req.getObj());
		ApiProxyStat.getInstance().addInvokeTime(targetStat, req.getCallModuleName() == null ? ProxyStatUtils.getLocalIP() : req.getCallModuleName(), req.getObj(), res.getSet(), req.getSetInfo(),
				"seer", ProxyStatUtils.getLocalIP(), res.getIP(), res.getPort(), ret, costtime);
		if (!isStatReported.contains(req.getObj())) {			
			synchronized (isStatReported) {
				if (!isStatReported.contains(req.getObj())) {
					isStatReported.add(req.getObj());
					executorManager.scheduleAtFixedRate(new ReportCallResultThread(req.getObj()), Constants.THREAD_CALL_INTERVAL, Constants.THREAD_CALL_INTERVAL, TimeUnit.MILLISECONDS);
				}
			}
		}
	}
	
	private void remoteReportStat(String obj) {
		try {
			statHelper.report(InvokeStatHelper.getInstance().getProxyStat(obj));
		} catch (Exception e) {
		}
	}
	
	private class ReportCallResultThread implements Runnable {
		private String objName;

		public ReportCallResultThread(String objName) {
			this.objName = objName;
		}

		@Override
		public void run() {
			remoteReportStat(objName);
		}

	}
}
