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

package com.qq.seer.common.config;

import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.UnsupportedEncodingException;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.Map;
import java.util.Properties;

import com.qq.seer.common.lang.Charsets;
import com.qq.seer.common.log.LogUtils;
import com.qq.seer.common.resource.ResourceUtils;

public class PropertiesUtils {
	
	
	public static Map<String, String> getPropertyMap(String file) {
        InputStream is = ResourceUtils.getResourceAsStream(file);
        if (null == is) {
        	LogUtils.localLogError("cat't get resource as stream, file=" + file);
            return null;
        }

        InputStreamReader isr;
        try {
            isr = new InputStreamReader(is, Charsets.UTF8);
        } catch (UnsupportedEncodingException e) {
        	LogUtils.localLogError("unsupported encoding=" + Charsets.UTF8, e);
            return null;
        }

        Properties properties = new Properties();
        try {
            properties.load(isr);
        } catch (IOException e) {
        	LogUtils.localLogError("load error, file=[" + file + "]", e);
            return null;
        } finally {
            if (null != isr) {
                try {
                    isr.close();
                } catch (IOException e) {
                	LogUtils.localLogError("close input stream reader error", e);
                    return null;
                }
            }

            if (null != is) {
                try {
                    is.close();
                } catch (IOException e) {
                	LogUtils.localLogError("close input stream error", e);
                    return null;
                }
            }
        }

        Map<String, String> rtnValue = new HashMap<String, String>();
        Enumeration<?> keys = properties.propertyNames();
        while (keys.hasMoreElements()) {
            String key = (String) keys.nextElement();
            rtnValue.put(key, properties.getProperty(key));
        }
        return rtnValue;
    }

}
