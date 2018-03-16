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

package com.qq.seer.common.http;


import org.apache.http.HttpEntity;
import org.apache.http.client.config.RequestConfig;
import org.apache.http.client.methods.CloseableHttpResponse;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.config.RegistryBuilder;
import org.apache.http.conn.DnsResolver;
import org.apache.http.conn.HttpClientConnectionManager;
import org.apache.http.conn.socket.ConnectionSocketFactory;
import org.apache.http.conn.socket.PlainConnectionSocketFactory;
import org.apache.http.conn.ssl.SSLConnectionSocketFactory;
import org.apache.http.entity.ContentType;
import org.apache.http.entity.StringEntity;
import org.apache.http.entity.mime.MultipartEntityBuilder;
import org.apache.http.entity.mime.content.FileBody;
import org.apache.http.entity.mime.content.StringBody;
import org.apache.http.impl.client.CloseableHttpClient;
import org.apache.http.impl.client.HttpClients;
import org.apache.http.impl.conn.PoolingHttpClientConnectionManager;
import org.apache.http.util.EntityUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.qq.seer.common.codec.CodecUtils;
import com.qq.seer.common.collection.Pair;
import com.qq.seer.common.exception.ThrowableInfo;
import com.qq.seer.common.string.StringUtils;

import javax.servlet.http.HttpServletRequest;
import java.io.File;
import java.util.Map;

public class HttpUtils {
	


    private static Logger log = LoggerFactory.getLogger(HttpUtils.class);

    /**
     * 上传文件
     *
     * @param url            请求的地址
     * @param file           需要发送的文件
     * @param queryStringMap 查询字符串，key为参数名称，value为参数值
     * @param headers        请求头，可以覆盖默认请求头
     * @param receiveCharset 接收使用的字符集
     * @param connectTimeOut 连接超时，单位：毫秒
     * @param dataTimeOut    读取超时，单位：毫秒
     * @return 返回的内容，状态码&内容
     */
    public static Pair<Integer, String> upload(String url, File file, Map<String, String> queryStringMap,
                                               Map<String, String> headers, String receiveCharset,
                                               int connectTimeOut, int dataTimeOut) throws Exception {
        long tid = System.currentTimeMillis();

        if (log.isInfoEnabled()) {
            StringBuilder sb = new StringBuilder();
            sb.append("tid=").append(tid);
            sb.append(", post");
            sb.append(", url=").append(url);
            sb.append(", file=").append(file.getCanonicalPath());
            sb.append(", receive charset=").append(receiveCharset);
            sb.append(", connect timeout=").append(connectTimeOut);
            sb.append(", data timeout=").append(dataTimeOut);

            log.info(sb.toString());
        }

        DnsResolver dnsResolver = new CachedDnsResolver();
        HttpClientConnectionManager connectionManager = new PoolingHttpClientConnectionManager(
                RegistryBuilder.<ConnectionSocketFactory>create()
                        .register("http", PlainConnectionSocketFactory.getSocketFactory())
                        .register("https", SSLConnectionSocketFactory.getSocketFactory())
                        .build(), dnsResolver
        );
        CloseableHttpClient httpclient = HttpClients.custom()
                .setConnectionManager(connectionManager)
                .build();
        try {
            HttpPost httpPost = new HttpPost(url);
            RequestConfig config = RequestConfig.custom()
                    .setConnectionRequestTimeout(connectTimeOut)
                    .setConnectTimeout(connectTimeOut)
                    .setSocketTimeout(dataTimeOut).build();
            httpPost.setConfig(config);
            httpPost.setHeader("Connection", "close");

            if (null != headers) {
                for (String name : headers.keySet()) {
                    httpPost.setHeader(name, headers.get(name));
                }
            }

            FileBody binary = new FileBody(file);
            MultipartEntityBuilder builder = MultipartEntityBuilder.create();
            builder.addPart("binary", binary);

            if (null != queryStringMap) {
                for (String k : queryStringMap.keySet()) {
                    builder.addPart(k, new StringBody(queryStringMap.get(k), ContentType.TEXT_PLAIN));
                }
            }

            HttpEntity reqEntity = builder.build();
            httpPost.setEntity(reqEntity);

            CloseableHttpResponse response = httpclient.execute(httpPost);
            int code = response.getStatusLine().getStatusCode();

            if (log.isDebugEnabled()) {
                log.debug("tid=" + tid + ", code=" + code);
            }

            String responseText;
            try {
                responseText = EntityUtils.toString(response.getEntity(), receiveCharset);
            } catch (Exception e) {
                if (log.isInfoEnabled()) {
                    log.info("tid=" + tid + ", get response text error", e);
                }
                responseText = new ThrowableInfo(e).getThrowableStr();
            }

            if (log.isDebugEnabled()) {
                log.debug("tid=" + tid + ", response=" + responseText);
            }

            return Pair.of(code, responseText);
        } finally {
            httpclient.close();
        }
    }

    /**
     * POST请求
     *
     * @param url            请求的地址
     * @param content        需要发送的内容
     * @param headers        请求头，可以覆盖默认请求头
     * @param sendCharset    发送使用的字符集
     * @param receiveCharset 接收使用的字符集
     * @param connectTimeOut 连接超时，单位：毫秒
     * @param dataTimeOut    读取超时，单位：毫秒
     * @return 返回的内容，状态码&内容
     */
    public static Pair<Integer, String> post(String url, String content, Map<String, String> headers,
                                             String sendCharset, String receiveCharset,
                                             int connectTimeOut, int dataTimeOut) throws Exception {
        long tid = System.currentTimeMillis();

        if (log.isInfoEnabled()) {
            StringBuilder sb = new StringBuilder();
            sb.append("tid=").append(tid);
            sb.append(", post");
            sb.append(", url=").append(url);
            sb.append(", content=").append(content);
            sb.append(", send charset=").append(sendCharset);
            sb.append(", receive charset=").append(receiveCharset);
            sb.append(", connect timeout=").append(connectTimeOut);
            sb.append(", data timeout=").append(dataTimeOut);

            log.info(sb.toString());
        }

        DnsResolver dnsResolver = new CachedDnsResolver();
        HttpClientConnectionManager connectionManager = new PoolingHttpClientConnectionManager(
                RegistryBuilder.<ConnectionSocketFactory>create()
                        .register("http", PlainConnectionSocketFactory.getSocketFactory())
                        .register("https", SSLConnectionSocketFactory.getSocketFactory())
                        .build(), dnsResolver
        );
        CloseableHttpClient httpclient = HttpClients.custom()
                .setConnectionManager(connectionManager)
                .build();
        try {
            HttpPost httpPost = new HttpPost(url);
            RequestConfig config = RequestConfig.custom()
                    .setConnectionRequestTimeout(connectTimeOut)
                    .setConnectTimeout(connectTimeOut)
                    .setSocketTimeout(dataTimeOut).build();
            httpPost.setConfig(config);
            httpPost.setHeader("Content-type", "application/x-www-form-urlencoded");
            httpPost.setHeader("Connection", "close");

            if (null != headers) {
                for (String name : headers.keySet()) {
                    httpPost.setHeader(name, headers.get(name));
                }
            }

            httpPost.setEntity(new StringEntity(content, sendCharset));

            CloseableHttpResponse response = httpclient.execute(httpPost);
            int code = response.getStatusLine().getStatusCode();

            if (log.isDebugEnabled()) {
                log.debug("tid=" + tid + ", code=" + code);
            }

            String responseText;
            try {
                responseText = EntityUtils.toString(response.getEntity(), receiveCharset);
            } catch (Exception e) {
                if (log.isInfoEnabled()) {
                    log.info("tid=" + tid + ", get response text error", e);
                }
                responseText = new ThrowableInfo(e).getThrowableStr();
            }

            if (log.isDebugEnabled()) {
                log.debug("tid=" + tid + ", response=" + responseText);
            }

            return Pair.of(code, responseText);
        } finally {
            httpclient.close();
        }
    }

    /**
     * POST请求
     *
     * @param url            请求的地址
     * @param content        需要发送的内容
     * @param sendCharset    发送使用的字符集
     * @param receiveCharset 接收使用的字符集
     * @param timeout        连接超时和读取超时，单位：毫秒
     * @return 返回的内容
     */
    public static String post(String url, String content, String sendCharset, String receiveCharset, int timeout) {
        Pair<Integer, String> response;
        try {
            response = post(url, content, null, sendCharset, receiveCharset, timeout, timeout);
        } catch (Exception e) {
            if (log.isInfoEnabled()) {
                log.info("http get error, url=" + url + ", return null", e);
            }
            return null;
        }

        if (response.first != 200) {
            if (log.isInfoEnabled()) {
                log.info("http get status code not 200, url=" + url + ", return null");
            }
            return null;
        } else {
            return response.second;
        }
    }

    /**
     * POST请求
     *
     * @param url     请求的地址
     * @param content 需要发送的内容
     * @param charset 发送和接收使用的字符集
     * @param timeout 连接超时和读取超时，单位：毫秒
     * @return 返回的内容
     */
    public static String post(String url, String content, String charset, int timeout) {
        return post(url, content, charset, charset, timeout);
    }

    /**
     * POST请求，发送和接收使用使用UTF-8
     *
     * @param url     请求的地址
     * @param content 需要发送的内容
     * @param timeout 连接超时和读取超时，单位：毫秒
     * @return 返回的内容
     */
    public static String post(String url, String content, int timeout) {
        return post(url, content, "UTF-8", timeout);
    }

    /**
     * POST请求
     *
     * @param url            请求的地址
     * @param content        需要发送的内容
     * @param sendCharset    发送使用的字符集
     * @param receiveCharset 接收使用的字符集
     * @return 返回的内容
     */
    public static String post(String url, String content, String sendCharset, String receiveCharset) {
        return post(url, content, sendCharset, receiveCharset, 0);
    }

    /**
     * POST请求
     *
     * @param url     请求的地址
     * @param content 需要发送的内容
     * @param charset 发送和接收使用的字符集
     * @return 返回的内容
     */
    public static String post(String url, String content, String charset) {
        return post(url, content, charset, charset);
    }

    /**
     * POST请求，发送和接收使用使用UTF-8
     *
     * @param url     请求的地址
     * @param content 需要发送的内容
     * @return 返回的内容
     */
    public static String post(String url, String content) {
        return post(url, content, "UTF-8");
    }

    /**
     * POST请求
     *
     * @param url            请求的地址
     * @param queryStringMap 查询字符串，key为参数名称，value为参数值
     * @param sendCharset    发送使用的字符集
     * @param receiveCharset 接收使用的字符集
     * @param timeout        连接超时和读取超时，单位：毫秒
     * @return 返回的内容
     */
    public static String post(String url, Map<String, String> queryStringMap, String sendCharset, String receiveCharset,
                              int timeout) {
        String content = "";
        for (String k : queryStringMap.keySet()) {
            if (content.length() > 0) {
                content += "&";
            }
            content += k + "=" + CodecUtils.encodeWebURL(queryStringMap.get(k), sendCharset);
        }
        return post(url, content, sendCharset, receiveCharset, timeout);
    }

    /**
     * POST请求
     *
     * @param url            请求的地址
     * @param queryStringMap 查询字符串，key为参数名称，value为参数值
     * @param charset        发送和接收使用的字符集
     * @param timeout        连接超时和读取超时，单位：毫秒
     * @return 返回的内容
     */
    public static String post(String url, Map<String, String> queryStringMap, String charset, int timeout) {
        return post(url, queryStringMap, charset, charset, timeout);
    }

    /**
     * POST请求，发送和接收使用使用UTF-8
     *
     * @param url            请求的地址
     * @param queryStringMap 查询字符串，key为参数名称，value为参数值
     * @param timeout        连接超时和读取超时，单位：毫秒
     * @return 返回的内容
     */
    public static String post(String url, Map<String, String> queryStringMap, int timeout) {
        return post(url, queryStringMap, "UTF-8", timeout);
    }

    /**
     * POST请求
     *
     * @param url            请求的地址
     * @param queryStringMap 查询字符串，key为参数名称，value为参数值
     * @param sendCharset    发送使用的字符集
     * @param receiveCharset 接收使用的字符集
     * @return 返回的内容
     */
    public static String post(String url, Map<String, String> queryStringMap, String sendCharset,
                              String receiveCharset) {
        return post(url, queryStringMap, sendCharset, receiveCharset, 0);
    }

    /**
     * POST请求
     *
     * @param url            请求的地址
     * @param queryStringMap 查询字符串，key为参数名称，value为参数值
     * @param charset        发送和接收使用的字符集
     * @return 返回的内容
     */
    public static String post(String url, Map<String, String> queryStringMap, String charset) {
        return post(url, queryStringMap, charset, charset);
    }

    /**
     * POST请求，发送和接收使用使用UTF-8
     *
     * @param url            请求的地址
     * @param queryStringMap 查询字符串，key为参数名称，value为参数值
     * @return 返回的内容
     */
    public static String post(String url, Map<String, String> queryStringMap) {
        return post(url, queryStringMap, "UTF-8");
    }

    /**
     * GET请求
     *
     * @param url            请求的地址
     * @param headers        请求头，可以覆盖默认请求头
     * @param charset        接收使用的字符集
     * @param connectTimeOut 连接超时，单位：毫秒
     * @param dataTimeOut    读取超时，单位：毫秒
     * @return 返回的内容
     */
    public static Pair<Integer, String> get(String url, Map<String, String> headers, String charset,
                                            int connectTimeOut, int dataTimeOut) throws Exception {
        long tid = System.currentTimeMillis();

        if (log.isInfoEnabled()) {
            StringBuilder sb = new StringBuilder();
            sb.append("tid=").append(tid);
            sb.append(", get");
            sb.append(", url=").append(url);
            sb.append(", charset=").append(charset);
            sb.append(", connect timeout=").append(connectTimeOut);
            sb.append(", data timeout=").append(dataTimeOut);

            log.info(sb.toString());
        }

        DnsResolver dnsResolver = new CachedDnsResolver();
        HttpClientConnectionManager connectionManager = new PoolingHttpClientConnectionManager(
                RegistryBuilder.<ConnectionSocketFactory>create()
                        .register("http", PlainConnectionSocketFactory.getSocketFactory())
                        .register("https", SSLConnectionSocketFactory.getSocketFactory())
                        .build(), dnsResolver
        );
        CloseableHttpClient httpclient = HttpClients.custom()
                .setConnectionManager(connectionManager)
                .build();

        try {
            HttpGet httpGet = new HttpGet(url);
            RequestConfig config = RequestConfig.custom()
                    .setConnectionRequestTimeout(connectTimeOut)
                    .setConnectTimeout(connectTimeOut)
                    .setSocketTimeout(dataTimeOut).build();
            httpGet.setConfig(config);
            httpGet.setHeader("Content-type", "application/x-www-form-urlencoded");
            httpGet.setHeader("Connection", "close");

            if (null != headers) {
                for (String name : headers.keySet()) {
                    httpGet.setHeader(name, headers.get(name));
                }
            }

            CloseableHttpResponse response = httpclient.execute(httpGet);
            int code = response.getStatusLine().getStatusCode();

            if (log.isDebugEnabled()) {
                log.debug("tid=" + tid + ", code=" + code);
            }

            String responseText;
            try {
                responseText = EntityUtils.toString(response.getEntity(), charset);
            } catch (Exception e) {
                if (log.isInfoEnabled()) {
                    log.info("tid=" + tid + ", get response text error", e);
                }
                responseText = new ThrowableInfo(e).getThrowableStr();
            }

            if (log.isDebugEnabled()) {
                log.debug("tid=" + tid + ", response=" + responseText);
            }

            return Pair.of(code, responseText);
        } finally {
            httpclient.close();
        }
    }

    /**
     * GET请求
     *
     * @param url     请求的地址
     * @param charset 接收使用的字符集
     * @param timeout 连接超时和读取超时，单位：毫秒
     * @return 返回的内容
     */
    public static String get(String url, String charset, int timeout) {
        Pair<Integer, String> response;
        try {
            response = get(url, null, charset, timeout, timeout);
        } catch (Exception e) {
            if (log.isInfoEnabled()) {
                log.info("http get error, url=" + url + ", return null", e);
            }
            return null;
        }

        if (response.first != 200) {
            if (log.isInfoEnabled()) {
                log.info("http get status code not 200, url=" + url + ", return null");
            }
            return null;
        } else {
            return response.second;
        }
    }

    /**
     * GET请求，发送和接收使用使用UTF-8
     *
     * @param url     请求的地址
     * @param timeout 连接超时和读取超时，单位：毫秒
     * @return 返回的内容
     */
    public static String get(String url, int timeout) {
        return get(url, "UTF-8", timeout);
    }

    /**
     * GET请求
     *
     * @param url     请求的地址
     * @param charset 接收使用的字符集
     * @return 返回的内容
     */
    public static String get(String url, String charset) {
        return get(url, charset, 0);
    }

    /**
     * GET请求，发送和接收使用使用UTF-8
     *
     * @param url 请求的地址
     * @return 返回的内容
     */
    public static String get(String url) {
        return get(url, "UTF-8");
    }

    /**
     * GET请求
     *
     * @param url            请求的地址
     * @param queryStringMap 查询字符串，key为参数名称，value为参数值
     * @param charset        接收使用的字符集
     * @param timeout        连接超时和读取超时，单位：毫秒
     * @return 返回的内容
     */
    public static String get(String url, Map<String, String> queryStringMap, String charset, int timeout) {
        String condition = "";
        for (String k : queryStringMap.keySet()) {
            if (condition.length() > 0) {
                condition += "&";
            }
            condition += k + "=" + CodecUtils.encodeWebURL(queryStringMap.get(k), "UTF-8");
        }
        url += "?" + condition;
        return get(url, charset, timeout);
    }

    /**
     * GET请求，发送和接收使用使用UTF-8
     *
     * @param url            请求的地址
     * @param queryStringMap 查询字符串，key为参数名称，value为参数值
     * @param timeout        连接超时和读取超时，单位：毫秒
     * @return 返回的内容
     */
    public static String get(String url, Map<String, String> queryStringMap, int timeout) {
        return get(url, queryStringMap, "UTF-8", timeout);
    }

    /**
     * GET请求
     *
     * @param url            请求的地址
     * @param queryStringMap 查询字符串，key为参数名称，value为参数值
     * @param charset        接收使用的字符集
     * @return 返回的内容
     */
    public static String get(String url, Map<String, String> queryStringMap, String charset) {
        return get(url, queryStringMap, charset, 0);
    }

    /**
     * GET请求，发送和接收使用使用UTF-8
     *
     * @param url            请求的地址
     * @param queryStringMap 查询字符串，key为参数名称，value为参数值
     * @return 返回的内容
     */
    public static String get(String url, Map<String, String> queryStringMap) {
        return get(url, queryStringMap, "UTF-8");
    }

    /**
     * 获取客户端IP
     *
     * @param request Http请求
     * @return 客户端IP地址
     */
    public static String getRemoteIP(HttpServletRequest request) {
        String clientIp = request.getHeader("X-Forwarded-For-Pound");
        if (StringUtils.isBlank(clientIp)) {
            clientIp = request.getRemoteAddr();
        }
        return clientIp;
    }

}
