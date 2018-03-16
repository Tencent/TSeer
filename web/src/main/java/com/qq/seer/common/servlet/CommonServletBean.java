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

public class CommonServletBean {
	


    private String pathName = "";
    private String packageName = "";
    private String className = "";
    private String methodName = "";

    private Map<String, String> parameters = new HashMap<String, String>();
    private Map<String, String> forwards = new HashMap<String, String>();
    private Map<String, String> redirects = new HashMap<String, String>();

    public String getPathName() {
        return pathName;
    }

    public void setPathName(String pathName) {
        this.pathName = pathName;
    }

    public String getPackageName() {
        return packageName;
    }

    public void setPackageName(String packageName) {
        this.packageName = packageName;
    }

    public String getClassName() {
        return className;
    }

    public void setClassName(String className) {
        this.className = className;
    }

    public String getMethodName() {
        return methodName;
    }

    public void setMethodName(String methodName) {
        this.methodName = methodName;
    }

    public Map<String, String> getParameters() {
        return parameters;
    }

    public void setParameters(Map<String, String> parameters) {
        this.parameters = parameters;
    }

    public Map<String, String> getForwards() {
        return forwards;
    }

    public void setForwards(Map<String, String> forwards) {
        this.forwards = forwards;
    }

    public Map<String, String> getRedirects() {
        return redirects;
    }

    public void setRedirects(Map<String, String> redirects) {
        this.redirects = redirects;
    }
    
    public String getParameter(String k) {
        return parameters.get(k);
    }

    public void setParameter(String k, String v) {
        parameters.put(k, v);
    }

    public String getForward(String k) {
        return forwards.get(k);
    }

    public void setForward(String k, String v) {
        forwards.put(k, v);
    }
    
    public String getRedirect(String k) {
        return redirects.get(k);
    }

    public void setRedirect(String k, String v) {
        redirects.put(k, v);
    }


}
