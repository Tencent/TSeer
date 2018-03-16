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

package com.qq.cloud.router.client.loadblance.impl;

import java.util.HashMap;
import java.util.Map;

import com.qq.cloud.router.client.loadblance.LoadBlanceContext;

public abstract class GenericLoadBlanceContext implements LoadBlanceContext {

	private Map<String, Object> attachment;
	
	@Override
	public Object getAttachment(String key) {
		if (attachment == null)
			return null;
		return attachment.get(key);
	}

	@Override
	public Object getAttachment(String key, Object defaultvalue) {
		if (attachment == null)
			return defaultvalue;
		if (!attachment.containsKey(key))
			return defaultvalue;
		return attachment.get(key);
	}

	@Override
	public Object[] getAttachments() {
		if (attachment == null)
			return null;
		return attachment.values().toArray();
	}

	public void setAttachment(String key, Object value) {
		if (attachment == null)
			attachment = new HashMap<String, Object>();
		attachment.put(key, value);
	}
	
	public void setAttachments(Map<String, Object> attachments) {
		if (attachments == null || attachments.size() == 0) {
			this.attachment = new HashMap<String, Object>();
		}
		this.attachment = attachments;
	}
	
	public void addAttachments(Map<String, Object> attachments) {
		if (attachments == null || attachments.size() == 0) {
			return;
		}
		attachments.putAll(attachments);
	}
}
