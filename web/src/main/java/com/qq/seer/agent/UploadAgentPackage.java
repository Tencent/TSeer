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

package com.qq.seer.agent;

import java.io.File;
import java.io.IOException;
import java.util.Calendar;
import java.util.Map;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.io.FileUtils;

import com.qq.seer.common.log.LogUtils;
import com.qq.seer.common.prop.SystemProperty;

/**
 * 上传Seer Agent包
 *
 */
public class UploadAgentPackage extends HttpServlet {
	
	/**
	 * 
	 */
	private static final long serialVersionUID = -8777487791664571656L;
	
	private static final String fileDir = SystemProperty.getValueByName("agent.package.dir");

	@Override
	protected void doPost(HttpServletRequest req, HttpServletResponse res) throws ServletException, IOException {
		long logId = System.currentTimeMillis();
		LogUtils.localLogInfo(logId, "UploadAgentPackage doPost ... ");
		doGet(req, res);
	}

	@Override
	protected void doGet(HttpServletRequest request, HttpServletResponse response)
	        throws ServletException, IOException {
		
		
		long logId = System.currentTimeMillis();
		
		LogUtils.localLogInfo(logId, "UploadAgentPackage doGet ... ");
		
		String resultObj = "";
		
		try {
			String file = request.getParameter("file");
			String fileFilename = request.getParameter("file.filename"); //CentOs-6.2-64
			String filename = request.getParameter("filename");//SeerAgent_20170614161336_CentOs-6.2-64_v0.16_65deceea1ec43e57f67a092d7eebc779.tgz
			String version = request.getParameter("version");//v0.16
			String md5 = request.getParameter("md5");
			String packageType = request.getParameter("package_type");//2-灰度，3-正式版本
			String osVersion = request.getParameter("os_version"); //CentOs-6.2-64
			
			LogUtils.localLogInfo(logId, "UploadAgentPackage file=" + file);
			LogUtils.localLogInfo(logId, "UploadAgentPackage fileFilename=" + fileFilename);
			LogUtils.localLogInfo(logId, "UploadAgentPackage filename=" + filename);
			LogUtils.localLogInfo(logId, "UploadAgentPackage version=" + version);
			LogUtils.localLogInfo(logId, "UploadAgentPackage md5=" + md5);
			LogUtils.localLogInfo(logId, "UploadAgentPackage packageType=" + packageType);
			LogUtils.localLogInfo(logId, "UploadAgentPackage osVersion=" + osVersion);
			// 校验文件名，禁止路径穿越
			if (filename.contains("..") || fileFilename.contains("..")) {
				resultObj = execScript("-1", "文件名校验不通过，禁止路径穿越"); 
				response.getWriter().println(resultObj);	
				return;
			}
			
			int intPackageType = 2;
			
			if(packageType !=null && packageType.length() > 0) {
				intPackageType = Integer.parseInt(packageType);
			}
			
			String suffix = "tgz";
			if(filename != null) {
				 int startInd = filename.lastIndexOf(".");
				 if (startInd > 0) {
			         suffix = filename.substring(startInd);
			     }
			}
			
			long sequence = getSequence();
			
			String fileName = "SeerAgent_" + sequence + "." + suffix;
			if(fileFilename != null && fileFilename.length() > 0) {
				fileName = fileFilename;
			}
			String filePath = fileDir + "/"+ fileName;
			LogUtils.localLogInfo(logId, "UploadAgentPackage fileName=" + fileName);
			LogUtils.localLogInfo(logId, "UploadAgentPackage fileDir=" + fileDir);
			LogUtils.localLogInfo(logId, "UploadAgentPackage filePath=" + filePath);
			new File(fileDir).mkdirs();
			File agentPackagFile = new File(filePath);
			// 校验文件是否已存在, 禁止覆盖
			if (agentPackagFile.exists()) {
				LogUtils.localLogError(logId, "File exist, filename=" + filePath);
				resultObj = execScript("-1", "文件已存在");
				response.getWriter().println(resultObj);
				return;
			}
			File tmpFile = new File(file);
			
			String filemd5 = AgentPackageMd5Util.getFileMd5(tmpFile);
			LogUtils.localLogInfo(logId, "UploadAgentPackage filemd5=" + filemd5);
			if(filemd5.equals(md5)) {
				FileUtils.copyFile(tmpFile, agentPackagFile);
				
				SeerAgentProxy.pushPackage(logId, agentPackagFile, fileName, osVersion, version, intPackageType, md5);
				agentPackagFile.delete();
				resultObj = execScript("0", "");
			} else {
				resultObj = execScript("-1", "文件MD5值校验未通过，文件计算的md5=[" + filemd5 + "],请重新上传。");
			}
			
			
		} catch (Exception e) {
			String error = "UploadAgentPackage Exception : " + e.getMessage();
			resultObj = execScript("-1", error);
			LogUtils.localLogError(logId, error, e);
		} finally {
			 response.getWriter().println(resultObj);
		}
		
	}
	
	private String execScript(String retCode, String data) {
        StringBuffer sb = new StringBuffer();
        sb.append("<script type=\"text/javascript\" src=\"/global-libs/js/jquery.min.js\"></script>");
        sb.append("<script type=\"text/javascript\" src=\"/global-libs//odfl/odfl.js\"></script>");

        sb.append("<script>");
        sb.append("odfl.crossJs({");
        sb.append("page: '" + "/pages/iframe/iframe.jsp?js_name=upload_back',");
        sb.append("params:['" + retCode + "', '" + data + "']");
        sb.append("})");
        sb.append("</script>");

        return sb.toString();
    }
	
	
	public static long getSequence() {
		return Calendar.getInstance().getTimeInMillis();
	}

}
