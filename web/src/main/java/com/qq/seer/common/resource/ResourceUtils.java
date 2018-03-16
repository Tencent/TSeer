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

package com.qq.seer.common.resource;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.UnsupportedEncodingException;
import java.net.URL;
import java.net.URLDecoder;
import java.nio.charset.Charset;
import java.util.concurrent.ConcurrentHashMap;

import org.apache.commons.io.FileUtils;

import com.qq.seer.common.log.LogUtils;

public class ResourceUtils {
	

    private static ConcurrentHashMap<String, String> cache = new ConcurrentHashMap<String, String>();
	
	
	public static String getResourcePath(String resource) {
   

        if (cache.containsKey(resource)) {
            String path = cache.get(resource);
            LogUtils.localLogInfo("get path from cache, resource=" + resource + ", path=" + path);
            return path;
        }

        String charset = Charset.defaultCharset().name();
        String path = null;
        try {
            ClassLoader loader = ResourceUtils.class.getClassLoader();
            if (null != loader) {
                URL url = loader.getResource(resource);
                if (null != url) {
                    try {
                        path = URLDecoder.decode(url.getFile(), charset);
                    } catch (UnsupportedEncodingException e) {
                    	LogUtils.localLogError("unsupported encoding, encoding=" + charset, e);
                    }
                }
            } else {
                URL url = ClassLoader.getSystemResource(resource);
                if (null != url) {
                    try {
                        path = URLDecoder.decode(url.getFile(), Charset.defaultCharset().name());
                    } catch (UnsupportedEncodingException e) {
                    	LogUtils.localLogError("unsupported encoding, encoding=" + charset, e);
                    }
                }
            }
        } finally {
            if (null != path) {
                cache.put(resource, path);
            }
        }
        LogUtils.localLogInfo("get resource from disk, resource=" + resource + ", path=" + path);
        return path;
    }
	
	public static InputStream getResourceAsStream(String resource) {
        InputStream is = ResourceUtils.class.getClassLoader().getResourceAsStream(resource);
        if (null != is) {
            return is;
        }

        is = ClassLoader.getSystemResourceAsStream(resource);
        if (null != is) {
            return is;
        }

        String path = getResourcePath(resource);
        if (null == path) {
            return null;
        }

        try {
            return FileUtils.openInputStream(new File(path));
        } catch (IOException e) {
        	LogUtils.localLogError(String.format("open input stream error, path=%s", path), e);
            return null;
        }
    }

}
