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

import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.SerializationFeature;
import com.qq.seer.common.log.LogUtils;

import freemarker.ext.servlet.FreemarkerServlet;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.json.JSONObject;

import java.io.IOException;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.Map;

public class DebugableFreeMarkerServlet extends FreemarkerServlet {
	
    /**
	 * 
	 */
	private static final long serialVersionUID = 6667897666248895624L;

	@Override
    public void doPost(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        String debug = request.getParameter("_debug");
        
        LogUtils.localLogInfo("DebugableFreeMarkerServlet doPost debug: " + debug);
        
        if ("true".equalsIgnoreCase(debug)) {
            debug(request, response);
        } else {
            super.doPost(request, response);
        }
    }

    @Override
    public void doGet(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        String debug = request.getParameter("_debug");
        
        LogUtils.localLogInfo("DebugableFreeMarkerServlet doGet debug: " + debug);
        
        if ("true".equalsIgnoreCase(debug)) {
        	 LogUtils.localLogInfo("DebugableFreeMarkerServlet doGet debug ... ");
        	
            debug(request, response);
        } else {
            super.doGet(request, response);
        }
    }

    private void debug(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
        try {
        	
        	LogUtils.localLogInfo("DebugableFreeMarkerServlet debug ... ");
        	
			Map<String, Object> model = new HashMap<String, Object>();
			Enumeration<String> e = request.getAttributeNames();
			while (e.hasMoreElements()) {
			    String key = e.nextElement();
			    model.put(key, request.getAttribute(key));
			}
			
			/*
			System.out.println("1111");
			ObjectMapper mapper = new ObjectMapper();
			System.out.println("2222");
			LogUtils.localLogInfo("DebugableFreeMarkerServlet debug mapper=" + mapper);
			
			mapper.configure(SerializationFeature.FAIL_ON_EMPTY_BEANS, false);
			mapper.configure(SerializationFeature.INDENT_OUTPUT, true);
			LogUtils.localLogInfo("DebugableFreeMarkerServlet : " + mapper.writeValueAsString(model));
			response.getWriter().write(mapper.writeValueAsString((model)));
			*/
			
			JSONObject jsonObj = new JSONObject(model);
			//jsonObj.put(SerializationFeature.FAIL_ON_EMPTY_BEANS.toString(), false);
			//jsonObj.put(SerializationFeature.INDENT_OUTPUT.toString(), true);
			
			response.getWriter().write(jsonObj.toString());
		} catch (Exception e) {
			String error = "debug Exception : " + e.getMessage();
			System.out.println(error);
			LogUtils.localLogError(error, e);
			e.printStackTrace();
			response.getWriter().write(error);
		}
    }
}
