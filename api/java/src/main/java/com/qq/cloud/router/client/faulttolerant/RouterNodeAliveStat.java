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

package com.qq.cloud.router.client.faulttolerant;

import java.math.BigDecimal;

import com.qq.cloud.router.client.CallResponse;
import com.qq.cloud.router.client.ServerNode;
import com.qq.cloud.router.client.util.LogUtils;

public class RouterNodeAliveStat {

    private ServerNode node;
    private boolean lastCallSucess = true;
    private long cycle_startTime = System.currentTimeMillis();
    private long frequenceFailInvoke = 0;
    private long frequenceFailInvoke_startTime = 0;
    private long timeoutCount = 0;
    private long failedCount = 0;
    private long succCount = 0;
    private volatile boolean alive = true;
    private long lastRetryTime = 0;
    
    public RouterNodeAliveStat(ServerNode node) {
        this.node = node;
    }
    
    public boolean isAlive() {
        return this.alive;
    }
    
    public synchronized void statisticCallResult(int ret, RouterNodeAliveConfig config) {
    	if(ret == CallResponse.CALL_SUCCESS.getCallResponse()) {
    		succCount++;
    		frequenceFailInvoke = 0;
    		frequenceFailInvoke_startTime = 0;
    		lastCallSucess = true;
    	} else if (ret == CallResponse.CALL_FAIL.getCallResponse()) {
    		failedCount++;
    		if (lastCallSucess) {
    			lastCallSucess = false;
    			frequenceFailInvoke = 1;
    			frequenceFailInvoke_startTime = System.currentTimeMillis();
    		} else {
    			frequenceFailInvoke++;
    		}
    	} else if (ret == CallResponse.CALL_TIMEOUT.getCallResponse()) {
    		timeoutCount++;
    		if (lastCallSucess) {
    			lastCallSucess = false;
    			frequenceFailInvoke = 1;
    			frequenceFailInvoke_startTime = System.currentTimeMillis();
    		} else {
    			frequenceFailInvoke++;
    		}
    	}
    	
    	if (cycle_startTime+config.getCheckInterval() < System.currentTimeMillis()) {
    		succCount = 0;
    		failedCount = 0;
    		timeoutCount = 0;
    		cycle_startTime = System.currentTimeMillis();
    	}
    	
    	if (alive) {
    		if (timeoutCount >= config.getMinTimeoutInvoke()) {
        		long totalCount = succCount+failedCount+timeoutCount;
        		double timeout_ratio = div(timeoutCount, totalCount, 2);
        		if (timeout_ratio > config.getFrequenceFailRadio()) {
        			alive = false;
        			if (lastRetryTime == 0)
        				lastRetryTime = System.currentTimeMillis();
        			LogUtils.info(Thread.currentThread().getName(), RouterNodeAliveStat.class.getSimpleName(), "node " + node.toString() + "is invaliad");
        		}
    		}
	    	if (alive) {
	    		if (frequenceFailInvoke >= config.getFrequenceFailInvoke() &&
	    				(frequenceFailInvoke_startTime+5000) > System.currentTimeMillis()) {
	    			alive = false;
	    			if (lastRetryTime == 0)
	    				lastRetryTime = System.currentTimeMillis();
	    			LogUtils.info(Thread.currentThread().getName(), RouterNodeAliveStat.class.getSimpleName(), "node " + node.toString() + "is invaliad");
	    		}
	    	}
    	} else {
    		if (ret == CallResponse.CALL_SUCCESS.getCallResponse()) {
    			alive = true;
    			LogUtils.info(Thread.currentThread().getName(), RouterNodeAliveStat.class.getSimpleName(), "node " + node.toString() + " recive");
    		}
    	}
    }
    
    private double div(double dividend, double divisor, int scale) {
    	if (scale < 0) {
    		throw new IllegalArgumentException("scale must >= 0");
    	}
    	BigDecimal b1 = new BigDecimal(Double.valueOf(dividend));
    	BigDecimal b2 = new BigDecimal(Double.valueOf(divisor));
    	return b1.divide(b2, scale , BigDecimal.ROUND_HALF_UP).doubleValue();
    }

	public long getLastRetryTime() {
		return lastRetryTime;
	}

	public void setLastRetryTime(long lastRetryTime) {
		this.lastRetryTime = lastRetryTime;
	}
}
