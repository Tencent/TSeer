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

public class RoutersRequest {
	
	private String obj;
	private LBGetType lbGetType;
	private String setInfo;
	
	public String getObj() {
		return obj;
	}
	public void setObj(String obj) {
		this.obj = obj;
	}
	public LBGetType getLbGetType() {
		return lbGetType;
	}
	public void setLbGetType(LBGetType lbGetType) {
		this.lbGetType = lbGetType;
	}
	public String getSetInfo() {
		return setInfo;
	}
	public void setSetInfo(String setInfo) {
		this.setInfo = setInfo;
	}
	
    public RoutersRequest clone() {
    	RoutersRequest request = new RoutersRequest();
    	request.setObj(getObj());
    	request.setLbGetType(getLbGetType());
    	request.setSetInfo(getSetInfo());
    	return request;
    }
	
	@Override
	public int hashCode() {
		return (this.obj+this.lbGetType.getLBGetType()+(this.setInfo == null ? "" : this.setInfo)).hashCode();
	}
	
	@Override
	public boolean equals(Object other) {
		if (obj == null) {
			return false;
		}
		if (!(other instanceof RoutersRequest)) {
			return false;
		}
		RoutersRequest request = (RoutersRequest)other;
		boolean set = true;
	    if (this.lbGetType == LBGetType.LB_GET_SET && request.getLbGetType() == LBGetType.LB_GET_SET) {
	    	if (!this.setInfo.equals(request.getSetInfo())) {
	    		set = false;
	    	}
	    }
		return this.obj.equals(request.getObj()) &&
			   this.lbGetType.equals(request.getLbGetType()) &&
			   set;
	}
}
