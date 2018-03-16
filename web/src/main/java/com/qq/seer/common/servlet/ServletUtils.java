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

package com.qq.seer.common.servlet;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import javax.servlet.ServletContext;
import java.io.UnsupportedEncodingException;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLDecoder;
import java.nio.charset.Charset;

public class ServletUtils {
    private static Logger log = LoggerFactory.getLogger(ServletUtils.class);

    public static String getFileRealPath(ServletContext context, String path) {
        String realPath = context.getRealPath("/" + path);
        if (null == realPath) {
            try {
                URL url = context.getResource("/" + path);
                if (null == url) {
                    log.error("no file found, path=[" + path + "]");
                } else {
                    try {
                        realPath = URLDecoder.decode(url.getFile(), Charset.defaultCharset().name());
                    } catch (UnsupportedEncodingException e) {
                        e.printStackTrace();
                    }
                }
            } catch (MalformedURLException e) {
                e.printStackTrace();
            }
        }
        return realPath;
    }


}
