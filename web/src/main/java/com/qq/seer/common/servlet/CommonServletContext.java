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

import java.util.HashMap;
import java.util.Map;

import javax.servlet.ServletContext;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

public class CommonServletContext {
    private HttpServletRequest request;
    private HttpServletResponse response;
    private ServletContext servletContext;
    
    private Map<String, String> parameterMap = new HashMap<String, String>();
    private Map<String, String> forwardMap = new HashMap<String, String>();
    private Map<String, String> redirectMap = new HashMap<String, String>();

    public HttpServletRequest getRequest() {
        return request;
    }

    public void setRequest(HttpServletRequest request) {
        this.request = request;
    }

    public HttpServletResponse getResponse() {
        return response;
    }

    public void setResponse(HttpServletResponse response) {
        this.response = response;
    }
    
    public ServletContext getServletContext() {
        return servletContext;
    }

    public void setServletContext(ServletContext servletContext) {
        this.servletContext = servletContext;
    }

    public Map<String, String> getParameterMap() {
        return parameterMap;
    }

    public void setParameterMap(Map<String, String> parameterMap) {
        this.parameterMap = parameterMap;
    }
    
    public Map<String, String> getForwardMap() {
        return forwardMap;
    }

    public void setForwardMap(Map<String, String> forwardMap) {
        this.forwardMap = forwardMap;
    }
    
    public Map<String, String> getRedirectMap() {
        return redirectMap;
    }

    public void setRedirectMap(Map<String, String> redirectMap) {
        this.redirectMap = redirectMap;
    }

    public String getParameter(String key) {
        return parameterMap.get(key);
    }
    
    public String getForward(String key) {
        return forwardMap.get(key);
    }
    
    public String getRedirect(String key) {
        return redirectMap.get(key);
    }


}
