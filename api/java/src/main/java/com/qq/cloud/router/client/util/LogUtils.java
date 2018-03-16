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

package com.qq.cloud.router.client.util;

import java.io.File;

import com.qq.tars.support.log.Logger;
import com.qq.tars.support.log.Logger.LogType;
import com.qq.tars.support.log.LoggerFactory;

public class LogUtils {
	
	static {
		File log_dir = new File(Constants.DEFAULE_LOG_PATH);
		if (!log_dir.exists()) {
			log_dir.mkdir();
		}
		LoggerFactory.config("info", Constants.DEFAULE_LOG_PATH);
	}
	
	private static final Logger logger = Logger.getLogger("Seer", LogType.LOCAL);
	
	public static void info(Object... args) {
		logger.info(join(args));
	}
	
	public static void debug(Object... args) {
		logger.debug(join(args));
	}
	
	public static void error(Throwable e, Object... args) {
		logger.error(join(args), e);
	}
	
	private static String join(Object... args) {
        return joinAll("|", true, args);
    }

    private static String joinAll(String separator, boolean startFirst, Object... args) {
        StringBuilder format = new StringBuilder();
        if (startFirst) {
            format.append(separator);
        }
        for (int i = 0, len = args.length; i < len; i++) {
            format.append("%s");
            if (i < (len - 1)) {
                format.append(separator);
            }
        }
        return String.format(format.toString(), args);
    }
}
