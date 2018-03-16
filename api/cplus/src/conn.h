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

#ifndef __TSEER_API_CONN_H_
#define __TSEER_API_CONN_H_

/*********************************************************
* 文件封装了使用TUP访问TARS服务的操作
*********************************************************/

#include <time.h>
#include <string>

namespace Tseerapi
{

class Conn
{
public:
    //模调数据上报到Agent
    static int NodeStatSendAgent(const char *sendBuff, unsigned int sendLen, int timeOut, std::string &errMsg);

    //与Agent通信获取节点信息
    static int QueryAndRecvRouterFromAgent(const char *sendBuff, unsigned int sendLen, int timeOut, char *recvBuff, unsigned int& recvSize, std::string &errMsg);

    //与Registry通信获取节点信息
    static int QueryAndRecvRouterFromRegistry(const char *sendBuff, unsigned int sendLen, int timeOut, char *recvBuff, unsigned int& recvSize, const std::string &registryIp, std::string &errMsg);
};

}

#endif
