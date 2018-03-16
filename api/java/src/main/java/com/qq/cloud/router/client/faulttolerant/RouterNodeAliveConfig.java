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

public final class RouterNodeAliveConfig {
	
    private int checkInterval = 60 * 1000;
    private int tryTimeInterval = 30;
    private int minTimeoutInvoke = 20;
    private int frequenceFailInvoke = 50;
    private float frequenceFailRadio = 0.5f;
    
    public RouterNodeAliveConfig() {
    }
	
    public RouterNodeAliveConfig(int checkInterval, int tryTimeInterval, int minTimeoutInvoke, int frequenceFailInvoke, float frequenceFailRadio) {
    	this.checkInterval = checkInterval;
    	this.tryTimeInterval = tryTimeInterval;
    	this.minTimeoutInvoke = minTimeoutInvoke;
    	this.frequenceFailInvoke = frequenceFailInvoke;
    	this.frequenceFailRadio = frequenceFailRadio;
    }
    
	public int getCheckInterval() {
		return checkInterval;
	}
	
	public void setCheckInterval(int checkInterval) {
		if (checkInterval > 0) {
			this.checkInterval = checkInterval;
		}
	}
	
	public int getTryTimeInterval() {
		return tryTimeInterval;
	}
	
	public void setTryTimeInterval(int tryTimeInterval) {
		if (tryTimeInterval > 0) {
			this.tryTimeInterval = tryTimeInterval;
		}
	}
	
	public int getMinTimeoutInvoke() {
		return minTimeoutInvoke;
	}
	
	public void setMinTimeoutInvoke(int minTimeoutInvoke) {
		if (minTimeoutInvoke > 0) {
			this.minTimeoutInvoke = minTimeoutInvoke;
		}
	}
	
	public int getFrequenceFailInvoke() {
		return frequenceFailInvoke;
	}
	
	public void setFrequenceFailInvoke(int frequenceFailInvoke) {
		if (frequenceFailInvoke > 0) {
			this.frequenceFailInvoke = frequenceFailInvoke;
		}
	}
	
	public float getFrequenceFailRadio() {
		return frequenceFailRadio;
	}
	
	public void setFrequenceFailRadio(float frequenceFailRadio) {
		if (frequenceFailRadio > 0) {
			this.frequenceFailRadio = frequenceFailRadio;
		}
	}
}
