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

#ifndef __TSEER_API_ENDPOINT_H_
#define __TSEER_API_ENDPOINT_H_

/**    文件定义了网络端口
 *    eg.    tcp -h 127.0.0.1 -p 2345 -t 10000 -w 100
 */

#include <string>
#include <sstream>
#include "TseerAgentComm.h"

namespace Tseerapi
{ 

struct EndPoint
{
public:
    EndPoint();

    EndPoint(bool isTcp, const std::string& host, int port, int timeout, 
        const std::string &set, int weight = -1);

    EndPoint(const EndPoint& l);

    EndPoint& operator = (const EndPoint& l);

    bool operator == (const EndPoint& l);

    /**    用字符串描述来构造;成功返回0，失败返回-1
    *    格式:(tcp|udp) -h xx -p xx -t xx [-s xx] [-w xx]
    */
    int parse(const std::string& str, std::string &errMsg);

    std::string toString() const;

    Tseer::RouterNodeInfo toRouterNodeInfo();

    //类型
    bool         _istcp;

    //ip
    std::string _host;

    //端口
    int         _port;

    //超时时间
    int         _timeout;

    ///节点的静态权重值
    int            _weight;

    //节点的set信息
    std::string _set;
};


}

#endif
