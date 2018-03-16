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

#ifndef __TSEER_API_AGENT_ROUTER_INNER_COMM_H_
#define __TSEER_API_AGENT_ROUTER_INNER_COMM_H_

#include <string>
#include <vector>
#include "Tseer_comm.h"

namespace Tseerapi
{
    //内部使用的路由请求参数
    struct InnerRouterRequest
    {
     bool             isAgent;                     //请求方式为纯api还是agent api
        std::string      obj;                       //请求服务名称
        LB_GET_TYPE      lbGetType;                  //按IDC、set或all获取，默认按all获取
        std::string      setInfo;                   //set信息
        LB_TYPE          type;                       //负载均衡类型，默认使用轮询方式
        long             hashKey;                   //hash key
        std::string      moduleName;                //主调模块名，模调数据上报时需要该信息；若为空API会自动用本地Ip替代

        std::string      ip;                        //服务Ip
        int              port;                      //服务端口
        bool             isTcp;                       //服务是否为TCP，否则为UDP
        std::string      slaveSet;                  //服务的set信息

        std::vector<NodeInfo> nodeInfoVec;             //所有节点信息数组

        InnerRouterRequest(const RouterRequest& r) {
            obj        = r.obj;
            lbGetType   = r.lbGetType;
            setInfo    = r.setInfo;
            type        = r.type;
            hashKey    = r.hashKey;
            moduleName = r.moduleName;
            ip         = r.ip;
            port       = r.port;
            isTcp        = r.isTcp;
            slaveSet   = r.slaveSet;
        }

        InnerRouterRequest(const RoutersRequest& r) {
            obj      = r.obj;
            lbGetType = r.lbGetType;
            setInfo  = r.setInfo;
            type      = LB_TYPE_ALL;
        }
    };
}

#endif
