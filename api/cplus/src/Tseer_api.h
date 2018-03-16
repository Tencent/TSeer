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

#ifndef __TSEER_API_AGENT_ROUTER_API_H_
#define __TSEER_API_AGENT_ROUTER_API_H_

/******************************************************************************
* 文件定义路由产品API的接口，通过该接口用户可以:
* 1.使用随机、轮询、权重、一致性哈希负载均衡的方式，来获取某服务指定IDC、SET或全部节点中的一个节点
* 2.获取某服务指定IDC、SET或全部节点的所有节点
* 3.上报模块间调用结果、时延；产品根据这些数据对被调服务节点做超时、错误屏蔽
*
*
* 只能选择使用Agent API或纯API的两种方式之一，不明确初始化时默认使用Agent API方式；
* 初始化方法只需要调用一次
* 1.使用Agent API:
*   a.调用ApiSetAgentIpInfo初始化Tseer Agent的本地Ip，不调用时使用默认值
* 2.使用纯API:
*   b.调用ApiSetRegistryInfo初始化Tseer服务端的"域名、端口，以及本地DNS配置文件"或"IP、端口"
******************************************************************************/

#include <string>
#include "Tseer_comm.h"

namespace Tseerapi
{
    /** 初始化纯API信息，分为以下两种：
     *  设置Tseer服务端的域名、Tseer服务端的端口、DNS配置文件;只需调用一次，成功返回0，失败返回-1
     *  如果使用纯API，必须显式调用该方法初始化；否则默认使用Agent API方式
     *
     *  InitRawApiParams结构描述：
     *      domainName:                Tseer服务端域名
     *      registryPort:              Tseer服务端的端口，没有特殊情况使用默认值即可
     *      dnsFile:                   DNS配置文件，如果domainNameOrRegistryIp填写IP地址，则不需要填写该参数
     *      serviceKey:                用户在管理平台上为当前业务注册的key     
     *
     *  errMsg:调用错误时返回的错误信息
     */
    int ApiSetRegistryInfo(const InitRawApiParams &initParams, std::string &errMsg);

    /**
     * 初始化Agent的IP，只需调用一次，成功返回0，失败返回-1
     *
     *  InitAgentApiParams结构描述：
     *      ip：                Agent本地IP地址，默认值：127.0.0.1
     *      serviceKey:        API标识        
     *
     *  errStr：调用错误时返回的错误信息
     */
    int ApiSetAgentIpInfo(const InitAgentApiParams &initParams, std::string &errMsg);

    /**
     *  按指定方式获取服务的单个节点，成功返回0，失败返回-1
     *  req:    需要获取的服务的信息，同时作为入参和出参
     *  errMsg:   失败时返回的错误信息
     */
    int ApiGetRoute(RouterRequest &req, std::string &errMsg);

    /**
     *  按指定方式获取服务的全部节点，成功返回0，失败返回-1
     *  req:    需要获取的服务的信息，同时作为入参和出参
     *  errMsg:   失败时返回的错误信息
     */
    int ApiGetRoutes(RoutersRequest &req, std::string &errMsg);

    /**
     *  上报调用服务结果，成功返回0，失败返回-1
     *  req：        被调服务的信息，用户应该使用ApiGetRoute接口的出参req作为此接口的入参
     *  ret：       调用被调服务的结果，0为成功，-1为异常，-2为超时（异常是除成功、超时以外的所有情况）
     *  timeCost：  调用服务时长
     *  errStr：       失败时返回的错误信息
     */
    int ApiRouteResultUpdate(const RouterRequest &req, int ret, int timeCost, std::string &errMsg);

}

#endif
