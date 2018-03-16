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

package com.qq.seer.common.exception;

import java.io.IOException;
import java.io.PrintWriter;
import java.io.StringWriter;

public class ThrowableInfo {
	

	
	private transient Throwable throwable;

	public ThrowableInfo(Throwable throwable) {
		this.throwable = throwable;
	}

	public Throwable getThrowable() {
		return throwable;
	}

	/**
	 * 查看异常的堆栈信息
	 * @return 堆栈详细信息
	 */
	public String getThrowableStr() {
		StringWriter sw = new StringWriter();
		throwable.printStackTrace(new PrintWriter(sw, true));
		String rtnValue = sw.toString();
		try {
			sw.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
		return rtnValue;
	}

}
