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

import com.qq.cloud.router.client.RouterRequest;
import com.qq.cloud.router.client.RouterResponse;
import com.qq.cloud.router.client.share.tseer.NodeStat;
import com.qq.cloud.router.client.share.tseer.RouterPrx;
import com.qq.cloud.router.client.util.CommonUtils;
import com.qq.cloud.router.client.util.Constants;
import com.qq.cloud.router.client.util.LogUtils;
import com.qq.tars.support.stat.ProxyStatUtils;

public class AgentStatManager implements StatManager {

	private static final AgentStatManager instance = new AgentStatManager();
	
	private RouterPrx rPrx;
	
	private void setRouterRrx(RouterPrx rPrx) {
		this.rPrx = rPrx;
	}
	
	private AgentStatManager() {}
	
	public static AgentStatManager getInstance(RouterPrx rPrx) {
		instance.setRouterRrx(rPrx);
		return instance;
	}
	
	@Override
	public void reportStat(RouterRequest req, RouterResponse res, int ret,
			long costtime) {
		NodeStat stat = new NodeStat(ProxyStatUtils.getLocalIP(), (int)costtime, ret, req.getCallModuleName(), res.getIP(),
				res.getPort(), req.getObj(), res.isbTcp(), "getRouter", Constants.SEER_AGENT_VERSION, req.getLbType().ordinal(),
				req.getLbGetType().ordinal(), CommonUtils.getCurrentPid());
		try {
			rPrx.async_reportStat(null, stat, Constants.defaultAgentCtx);
		} catch (Exception e) {
			LogUtils.error(e, Thread.currentThread().getName(), AgentStatManager.class.getSimpleName());
		}
	}
}
