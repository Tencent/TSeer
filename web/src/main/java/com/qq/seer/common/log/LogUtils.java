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

package com.qq.seer.common.log;

import java.io.PrintWriter;
import java.io.StringWriter;

/**
 * 日志处理
 *
 */
public class LogUtils {
	
	public static void localLogInfo(long logId, String info) {
		long tid = Thread.currentThread().getId();
		String logInfo = "[" + logId + "][" + tid + "]" + info;
		
		System.out.println(logInfo);
	}
	
	
	public static void localLogError(long logId, String error, Throwable th) {
		long tid = Thread.currentThread().getId();
		String logError = "[" + logId + "][" + tid + "]" + error;
		System.out.println(logError);
		
		StringWriter sw = new StringWriter();
		th.printStackTrace(new PrintWriter(sw, true));
		String rtnValue = sw.toString();
		System.out.println(rtnValue);
	}
	
	
	public static void localLogError(long logId, String error) {
		long tid = Thread.currentThread().getId();
		String logError = "[" + logId + "][" + tid + "]" + error;
		System.out.println(logError);
	}
	
	public static void localLogInfo(String info) {
		long logId = 0;
		localLogInfo(logId, info);
	}
	
	
	public static void localLogError(String error, Throwable th) {
		long logId = 0;
		localLogError(logId, error,  th);
	}
	
	public static void localLogError(String error) {
		long logId = 0;
		localLogError(logId, error);
	}

}
