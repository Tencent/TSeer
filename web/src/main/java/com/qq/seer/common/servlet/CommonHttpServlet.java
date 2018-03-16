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

import java.io.File;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.lang.reflect.InvocationTargetException;
import java.net.URLDecoder;
import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import javax.servlet.ServletConfig;
import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathConstants;
import javax.xml.xpath.XPathFactory;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import com.qq.seer.common.codec.CodecUtils;

public class CommonHttpServlet extends HttpServlet {


	private static final long serialVersionUID = 5071427594009672177L;

    private static Logger log = LoggerFactory.getLogger(CommonHttpServlet.class);

	private List<String> configPathList = new ArrayList<String>();

	private Map<String, String> parameters = new HashMap<String, String>();
	private Map<String, String> forwards = new HashMap<String, String>();
	private Map<String, String> redirects = new HashMap<String, String>();

	private Map<String, CommonServletBean> servlets = new HashMap<String, CommonServletBean>();

	private int DEFAULT_LOAD_GAP = 1 * 60 * 1000;

	private long lastLoadTime = 0;
	private boolean needLoad = true;

	public CommonHttpServlet() {
		super();
	}
	
	@Override
	public void init(ServletConfig config) throws ServletException {
		super.init(config);
	}

	@Override
	public void init() throws ServletException {
		super.init();
		
		String configPath = this.getInitParameter("CONFIG_PATH");
		if (null == configPath || configPath.length() == 0) {
			throw new ServletException("can't find parameter CONFIG_PATH");
		}

		String[] tokens = configPath.split("\\|");
		for (String t : tokens) {
			if (null == t || t.length() == 0) {
				continue;
			}
			try {
				String realPathResult = ServletUtils.getFileRealPath(this.getServletContext(), t);
				if (null == realPathResult) {
					log.error("invalid path, path=[" + realPathResult + "]");
					continue;
				}
				
				if (new File(realPathResult).exists()) {
					configPathList.add(realPathResult);
					if (log.isDebugEnabled()) {
						log.debug("add path, path=[" + realPathResult + "]");
					}
					continue;
				}
				
				String decodedRealPathResult = URLDecoder.decode(realPathResult, Charset.defaultCharset().name());
				if (new File(decodedRealPathResult).exists()) {
					configPathList.add(decodedRealPathResult);
					if (log.isDebugEnabled()) {
						log.debug("add path, path=[" + decodedRealPathResult + "]");
					}
					continue;
				}
				
				log.error("invalid path, path=[" + realPathResult + "]");
			} catch (UnsupportedEncodingException e) {
				throw new ServletException(e);
			}
		}

		if (!loadServletConfig()) {
			throw new ServletException("can't load servlet config");
		}
	}

	private boolean loadServletConfig() {
		if (lastLoadTime == 0 || System.currentTimeMillis() >= lastLoadTime + DEFAULT_LOAD_GAP) {
			needLoad = true;
		}

		if (needLoad) {
			try {
				for (String configPath : configPathList) {
					File configFile = new File(configPath);
					if (!configFile.exists()) {
						return false;
					}

					DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
					DocumentBuilder builder = factory.newDocumentBuilder();
					Document doc = builder.parse(configFile);

					XPath xpath = XPathFactory.newInstance().newXPath();

					Object result = xpath.evaluate("/config/global/parameters/parameter", doc, XPathConstants.NODESET);
					if (null != result) {
						NodeList nodes = (NodeList) result;
						for (int i = 0; i < nodes.getLength(); i++) {
							Node n = nodes.item(i);
							String name = n.getAttributes().getNamedItem("name").getNodeValue();
							String value = n.getAttributes().getNamedItem("value").getNodeValue();
							parameters.put(name, value);
						}
					}

					result = xpath.evaluate("/config/global/forwards/forward", doc, XPathConstants.NODESET);
					if (null != result) {
						NodeList nodes = (NodeList) result;
						for (int i = 0; i < nodes.getLength(); i++) {
							Node n = nodes.item(i);
							String name = n.getAttributes().getNamedItem("name").getNodeValue();
							String url = n.getAttributes().getNamedItem("url").getNodeValue();
							forwards.put(name, url);
						}
					}

					result = xpath.evaluate("/config/global/redirects/redirect", doc, XPathConstants.NODESET);
					if (null != result) {
						NodeList nodes = (NodeList) result;
						for (int i = 0; i < nodes.getLength(); i++) {
							Node n = nodes.item(i);
							String name = n.getAttributes().getNamedItem("name").getNodeValue();
							String url = n.getAttributes().getNamedItem("url").getNodeValue();
							redirects.put(name, url);
						}
					}

					result = xpath.evaluate("/config/servlet", doc, XPathConstants.NODESET);
					if (null != result) {
						NodeList nodes = (NodeList) result;
						for (int i = 0; i < nodes.getLength(); i++) {
							Node n = nodes.item(i);
							CommonServletBean bean = new CommonServletBean();

							String path = "";
							NodeList pathNode = (NodeList) xpath.evaluate(".//path", n, XPathConstants.NODESET);
							if (pathNode.getLength() == 0) {
								return false;
							}
							path = pathNode.item(0).getFirstChild().getNodeValue().trim();
							bean.setPathName(path);

							NodeList packageNode = (NodeList) xpath.evaluate(".//package", n, XPathConstants.NODESET);
							if (packageNode.getLength() == 0) {
								return false;
							}
							bean.setPackageName(packageNode.item(0).getFirstChild().getNodeValue().trim());

							NodeList classNode = (NodeList) xpath.evaluate(".//class", n, XPathConstants.NODESET);
							if (classNode.getLength() == 0) {
								return false;
							}
							bean.setClassName(classNode.item(0).getFirstChild().getNodeValue().trim());

							NodeList methodNode = (NodeList) xpath.evaluate(".//method", n, XPathConstants.NODESET);
							if (methodNode.getLength() == 0) {
								return false;
							}
							bean.setMethodName(methodNode.item(0).getFirstChild().getNodeValue().trim());

							NodeList parameterNode = (NodeList) xpath.evaluate(".//parameters/parameter", n, XPathConstants.NODESET);
							if (null != parameterNode) {
								for (int j = 0; j < parameterNode.getLength(); j++) {
									Node p = parameterNode.item(j);
									String name = p.getAttributes().getNamedItem("name").getNodeValue();
									String value = p.getAttributes().getNamedItem("value").getNodeValue();
									bean.setParameter(name, value);
								}
							}

							NodeList forwardNode = (NodeList) xpath.evaluate(".//forwards/forward", n, XPathConstants.NODESET);
							if (null != forwardNode) {
								for (int j = 0; j < forwardNode.getLength(); j++) {
									Node p = forwardNode.item(j);
									String name = p.getAttributes().getNamedItem("name").getNodeValue();
									String url = p.getAttributes().getNamedItem("url").getNodeValue();
									bean.setForward(name, url);
								}
							}

							NodeList redirectNode = (NodeList) xpath.evaluate(".//redirects/redirect", n, XPathConstants.NODESET);
							if (null != redirectNode) {
								for (int j = 0; j < redirectNode.getLength(); j++) {
									Node p = redirectNode.item(j);
									String name = p.getAttributes().getNamedItem("name").getNodeValue();
									String url = p.getAttributes().getNamedItem("url").getNodeValue();
									bean.setRedirect(name, url);
								}
							}

							servlets.put(path, bean);
						}
					}
				}
			} catch (Exception e) {
                log.error("CommonHttpServlet, loadServletConfig, load config error", e);
				return false;
			}

			lastLoadTime = System.currentTimeMillis();
			needLoad = false;
		}

		return true;
	}

	protected void process(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
		if (!loadServletConfig()) {
			throw new ServletException("can't load servlet config");
		}

		String servletPath = request.getServletPath();
		String requestURI = request.getRequestURI();
		int idx = requestURI.indexOf(servletPath);
		requestURI = requestURI.substring(idx);

		CommonServletBean servlet = servlets.get(requestURI);
		if (null == servlet) {
			throw new ServletException("can't find servlet with path \"" + requestURI + "\"");
		}

		String globalEncoding = parameters.get("ENCODING");
		String servletEncoding = servlet.getParameter("ENCODING");
		if (null != servletEncoding && servletEncoding.length() > 0) {
			setHttpEncoding(request, response, servletEncoding);
		} else if (null != globalEncoding && globalEncoding.length() > 0) {
			setHttpEncoding(request, response, globalEncoding);
		}

		Map<String, String> rqp = new HashMap<String, String>();

		// 设置了解析标志后才对请求的数据进行解析，防止取不到InputStream的情况
		String parseRequest = servlet.getParameter("PARSE_REQUEST");
		if (null == parseRequest || "true".equalsIgnoreCase(parseRequest)) {
			for (Object k : request.getParameterMap().keySet()) {
				String[] v = (String[])request.getParameterMap().get(k);
				if (null != v && v.length > 0) {
					rqp.put((String) k, v[0]);
				}
			}
		}

		rqp.putAll(parameters);
		rqp.putAll(servlet.getParameters());

		Map<String, String> rqf = new HashMap<String, String>();
		rqf.putAll(forwards);
		rqf.putAll(servlet.getForwards());

		Map<String, String> rqr = new HashMap<String, String>();
		rqr.putAll(redirects);
		rqr.putAll(servlet.getRedirects());

		CommonServletContext context = new CommonServletContext();
		context.setRequest(request);
		context.setResponse(response);
		context.setServletContext(getServletContext());

		context.setParameterMap(rqp);
		context.setForwardMap(rqf);
		context.setRedirectMap(rqr);

		String pakageName = servlet.getPackageName();
		String className = servlet.getClassName();
		String methodName = servlet.getMethodName();

		try {
			String classFullName = pakageName + "." + className;
			Class c = Class.forName(classFullName);
			Class[] mp = new Class[] { CommonServletContext.class };
			String result = (String) c.getMethod(methodName, mp).invoke(c.newInstance(), context);

			String forward = context.getForward(result);
			if (null != forward && forward.length() > 0) {
				forward(request, response, forward);
				return;
			}

			String redirect = context.getRedirect(result);
			if (null != redirect && redirect.length() > 0) {
				response.sendRedirect(encodeURL(context, redirect));
				return;
			}
		} catch (ClassNotFoundException e) {
			e.printStackTrace();
		} catch (IllegalArgumentException e) {
			e.printStackTrace();
		} catch (SecurityException e) {
			e.printStackTrace();
		} catch (IllegalAccessException e) {
			e.printStackTrace();
		} catch (InvocationTargetException e) {
			e.printStackTrace();
			e.getTargetException().printStackTrace();
		} catch (NoSuchMethodException e) {
			e.printStackTrace();
		} catch (InstantiationException e) {
			e.printStackTrace();
		}
	}

	protected void doGet(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
		process(request, response);
	}

	protected void doPost(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
		doGet(request, response);
	}

	private void forward(HttpServletRequest request, HttpServletResponse response, String path) throws ServletException, IOException {
		request.getRequestDispatcher(path).forward(request, response);
	}

	private void setHttpEncoding(HttpServletRequest request, HttpServletResponse response, String encode) throws IOException {
		request.setCharacterEncoding(encode);
		response.setContentType("text/html; charset=" + encode);
		response.setCharacterEncoding(encode);
	}

	private String encodeURL(CommonServletContext context, String url) {
		Pattern pattern = Pattern.compile("\\$\\{.+?\\}");
		Matcher matcher = pattern.matcher(url);

		Set<String> ps = new HashSet<String>();

		while (true) {
			if (!matcher.find()) {
				break;
			}
			ps.add(url.substring(matcher.start(), matcher.end()));
		}

		Iterator<String> it = ps.iterator();
		while (it.hasNext()) {
			String pn = it.next();
			String p = pn.substring(2, pn.length() - 1);
			String v = context.getParameter(p);
			if (null == v) {
				Object obj = context.getRequest().getAttribute(p);
				if (null != obj) {
					v = obj.toString();
				}
			}
			url = url.replace(pn, v == null ? "" : CodecUtils.encodeWebURL(v));
		}

		return url;
	}

}
