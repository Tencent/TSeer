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

import org.apache.http.conn.DnsResolver;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.ConcurrentHashMap;

public class CachedDnsResolver implements DnsResolver {
	


    private Logger log = LoggerFactory.getLogger(CachedDnsResolver.class);

    private static ConcurrentHashMap<String, InetAddress[]> dnsMap = new ConcurrentHashMap<String, InetAddress[]>();

    private static Timer timer = null;

    private static final byte[] lock = new byte[0];

    public CachedDnsResolver() {
        synchronized (lock) {
            if (null == timer) {
                timer = new Timer("CachedDnsResolver Refresh Timer");
                timer.schedule(new TimerTask() {
                    @Override
                    public void run() {
                        for (String host : dnsMap.keySet()) {
                            try {
                                InetAddress[] addresses = InetAddress.getAllByName(host);
                                dnsMap.put(host, addresses);
                            } catch (UnknownHostException e) {
                                log.error("unknown host, host={}", host, e);
                            }
                        }
                    }
                }, 60000, 60000);
            }
        }
    }

    @Override
    public InetAddress[] resolve(final String host) throws UnknownHostException {
        if (!dnsMap.containsKey(host)) {
            InetAddress[] addresses = InetAddress.getAllByName(host);
            StringBuilder ips = new StringBuilder();
            for (InetAddress address : addresses) {
                ips.append(address.getHostAddress()).append(",");
            }
            if (addresses.length > 0) {
                ips.deleteCharAt(ips.length() - 1);
            }
            log.info("get from dns, host={}, ips={}", host, ips);
            dnsMap.put(host, addresses);
        }
        return dnsMap.get(host);
    }


}
