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

#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include "global.h"
#include "Tseer_api.h"
#include "conn.h"

namespace Tseerapi
{

int Conn::NodeStatSendAgent(const char *sendBuff, unsigned int sendLen, int timeOut, std::string &errMsg)
{
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(-1 == fd)
    {
        std::stringstream buffer;
        buffer << FILE_FUN << "create udp socket error|errno:" << errno << "|info:" << strerror(errno);
        errMsg = buffer.str();
        return -1;
    }

    struct sockaddr_in     addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(g_agent_addr.c_str());
    addr.sin_port = htons(g_agent_router_port);

    if(timeOut > 0)
    {
        struct timeval tv = {timeOut * 1000 / 1000000, (timeOut * 1000) % 1000000 };
        setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    }

    int ret = sendto(fd, sendBuff, sendLen, 0, (struct sockaddr*)&addr, sizeof(addr));
    
    if(ret < 0 || ret != (int)sendLen)
    {
        close(fd);

        std::ostringstream buffer;
        buffer << FILE_FUN << "socket sendto error|ip:" << g_agent_addr << "|port:" << g_agent_router_port 
            << "|ret:" << ret << "|timeOut:" << timeOut << "|fd:" << fd << "|errno:" << errno << "|info:" << strerror(errno);
        errMsg = buffer.str();
        return -1;
    }

    close(fd);
    return 0;
}

int Conn::QueryAndRecvRouterFromAgent(const char *sendBuff, unsigned int sendLen, int timeOut, char *recvBuff, unsigned int& recvSize, std::string &errMsg)
{
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(-1 == fd)
    {
        std::ostringstream buffer;
        buffer << FILE_FUN << "create udp socket error|errno:" << errno << "|info:" << strerror(errno);
        errMsg = buffer.str();
        return -1;
    }

    struct sockaddr_in     addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(g_agent_addr.c_str());
    addr.sin_port = htons(g_agent_router_port);

    if(timeOut > 0)
    {
        struct timeval tv = {timeOut * 1000 / 1000000, (timeOut * 1000) % 1000000 };
        setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    }

    int ret = sendto(fd, sendBuff, sendLen, 0, (struct sockaddr*)&addr, sizeof(addr));
    
    if(ret < 0 || ret != (int)sendLen)
    {
        close(fd);
        std::ostringstream buffer;
        buffer << FILE_FUN << "socket sendto error|ip:" << g_agent_addr << "|port:" << g_agent_router_port 
            << "|ret:" << ret << "|timeOut:" << timeOut << "|fd:" << fd << "|errno:" << errno << "|info:" << strerror(errno);
        errMsg = buffer.str();
        return -1;
    }

    if(timeOut > 0)
    {
        struct timeval tv = {timeOut * 1000 / 1000000, (timeOut * 1000) % 1000000 };
        setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    }

    char *tmpPtr = recvBuff;
    unsigned int tmpLen = recvSize;
    unsigned int recvLen = 0;

    struct sockaddr_in recv_addr;
    socklen_t addr_len = 0;

    do
    {
        recv_addr.sin_family = AF_INET;
        recv_addr.sin_addr.s_addr = inet_addr(g_agent_addr.c_str());
        recv_addr.sin_port = htons(g_agent_router_port);

        addr_len = sizeof(addr);

        ret = recvfrom(fd, tmpPtr, tmpLen, 0, (struct sockaddr*)&recv_addr, &addr_len);
        if(ret < 0)
        {
            close(fd);

            std::ostringstream buffer;
            buffer << FILE_FUN << "socket recvfrom error|ip:" << g_agent_addr << "|port:" << g_agent_router_port 
                << "|ret:" << ret << "|timeOut:" << timeOut << "|fd:" << fd << "|errno:" << errno << "|info:" << strerror(errno);

            errMsg = buffer.str();

            return -1;
        }

        recvLen += ret;
        tmpPtr += ret;
        tmpLen -= ret;

        if(recvLen < sizeof(uint32_t))
        {
            continue;
        }

        uint32_t headerLen = ntohl(*(uint32_t*)(recvBuff));

        if(recvLen < headerLen)
        {
            continue;
        }

        if(recvLen > headerLen)
        {
            close(fd);

            std::ostringstream buffer;
            buffer << FILE_FUN << "SendAndRecv socket recvfrom packet error|ip:" << g_agent_addr << "|port:" << g_agent_router_port 
                << "|headerLen:" << headerLen << "|recvLen:" << recvLen;
            errMsg = buffer.str();
            return -1;
        }

        break;
    }while(1);

    recvSize = recvLen;

    close(fd);    
    return 0;
}

int Conn::QueryAndRecvRouterFromRegistry(const char *sendBuff, unsigned int sendLen, int timeOut, 
    char *recvBuff, unsigned int& recvSize, const std::string &registryIp, std::string &errMsg)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == fd)
    {
        std::ostringstream buffer;
        buffer << FILE_FUN << "create tcp socket error|errno:" << errno << "|info:" << strerror(errno);
        errMsg = buffer.str();
        return -1;
    }

    struct sockaddr_in     addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(registryIp.c_str());
    addr.sin_port = htons(g_registry_port);

    if (timeOut > 0)
    {
        struct timeval tv = { timeOut * 1000 / 1000000, (timeOut * 1000) % 1000000 };
        setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    }

    int ret = connect(fd, (sockaddr*)&addr, sizeof(addr));
    if (ret)
    {
        close(fd);
        std::ostringstream buffer; 
        buffer << FILE_FUN << "socket connect error|ip:" << registryIp << "|port:" << g_registry_port
            << "|ret:" << ret << "|timeOut:" << timeOut << "|fd:" << fd << "|errno:" << errno << "|info:" << strerror(errno);
        errMsg = buffer.str();
        return -1;
    }

    ret = write(fd, sendBuff, sendLen);

    if (ret < 0 || ret != (int)sendLen)
    {
        close(fd);
        std::ostringstream buffer;
        buffer << FILE_FUN << "|socket send error|ip:" << registryIp << "|port:" << g_registry_port
            << "|ret:" << ret << "|timeOut:" << timeOut << "|fd:" << fd << "|errno:" << errno << "|info:" << strerror(errno);
        errMsg = buffer.str();
        return -1;
    }


    if (timeOut > 0)
    {
        struct timeval tv = { timeOut * 1000 / 1000000, (timeOut * 1000) % 1000000 };
        setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    }

    char *tmpPtr = recvBuff;
    unsigned int tmpLen = recvSize;
    unsigned int recvLen = 0;

    do
    {
        ret = read(fd, tmpPtr, tmpLen);
        if (ret < 0)
        {
            close(fd);

            std::ostringstream buffer;
            buffer << FILE_FUN << "recv from registry error |ip:" << registryIp  << "|port:" << g_registry_port 
                << "|ret:" << ret << "|timeOut:" << timeOut << "|fd:" << fd << "|errno:" << errno << "|info:" << strerror(errno);
            errMsg = buffer.str();

            return -1;
        }

        recvLen += ret;
        tmpPtr += ret;
        tmpLen -= ret;

        if (recvLen < sizeof(uint32_t))
        {
            continue;
        }

        uint32_t headerLen = ntohl(*(uint32_t*)(recvBuff));

        if (recvLen < headerLen)
        {
            continue;
        }

        if (recvLen > headerLen)
        {
            close(fd);

            std::ostringstream buffer;
            buffer << FILE_FUN << "recv from registry error |ip:" << registryIp << "|port:" << g_registry_port 
                << "|ret:" << ret << "|timeOut:" << timeOut << "|fd:" << fd << "|errno:" << errno << "|info:" << strerror(errno);
            errMsg = buffer.str();
            return -1;
        }

        break;
    } while (1);

    recvSize = recvLen;

    close(fd);
    return 0;
}

}
