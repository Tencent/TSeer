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

#ifndef _UTIL_H
#define _UTIL_H

#include <netinet/in.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include "util/tc_common.h"
#include "RollLogger.h"


inline bool popen_sendMsg(const string& sCommand)
{
    FILE *fp;
    int rc = 0; // 用于接收命令返回值

    /*执行预先设定的命令，并读出该命令的标准输出*/
    fp = popen(sCommand.c_str(), "r");
    if(NULL == fp)
    {
        TSEER_LOG(UPDATE_LOG)->error() << FILE_FUN << "popen error,send message failed" << endl;
        return false;
    }

    /*等待命令执行完毕并关闭管道及文件指针*/
    rc = pclose(fp);

    return true;
}

template<typename T>
inline string display(const T& t)
{
    stringstream stream;
    stream.str("");
    t.displaySimple(stream);
    return(stream.str());
}

// get default network interface
inline bool get_default_if(char *iface, int size) {
    if (size < 8) return false;
    memset(iface, 0, size);

    FILE *f = fopen("/proc/net/route", "r");
    if (!f) return false;

    char dest[64]  = {0, };
    while (!feof(f)) {
        if (fscanf(f, "%s %s %*[^\r\n]%*c", iface, dest) != 2) continue;
        if (strcmp(dest, "00000000") == 0) {
            break;
        }
    }

    return strlen(iface) ? true : false;
}

// get eth0's ipv4 address
inline bool get_ip(const char *iface, char* ip, int size) {
    if (size < INET_ADDRSTRLEN) return false;
    memset(ip, 0, size);

    struct ifaddrs * ifAddrStruct = NULL;
    struct ifaddrs * ifa = NULL;
    void * tmpAddrPtr = NULL;

    getifaddrs(&ifAddrStruct);
    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if ((ifa->ifa_addr->sa_family == AF_INET) && (strcmp(ifa->ifa_name,iface) == 0)) {
            tmpAddrPtr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            inet_ntop(AF_INET, tmpAddrPtr, ip, INET_ADDRSTRLEN);
        }
    }

    if (ifAddrStruct != NULL) freeifaddrs(ifAddrStruct);

    return strlen(ip) ? true : false;
}

#endif