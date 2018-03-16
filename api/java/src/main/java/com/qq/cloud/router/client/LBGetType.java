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

public enum LBGetType {
    LB_GET_IDC(0),
    LB_GET_SET(1),
    LB_GET_ALL(2);
    
    private final int lbGetType;
    
    private LBGetType(int lbGetType) {
    	this.lbGetType = lbGetType;
    }
    
    public int getLBGetType() {
    	return lbGetType;
    }
    
	public static LBGetType convert(int value) {
		for(LBGetType v : values()) {
			if(v.getLBGetType() == value) {
				return v;
			}
		}
		return null;
	}
}
