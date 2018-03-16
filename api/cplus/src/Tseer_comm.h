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

#ifndef __TSEER_API_AGENT_ROUTER_COMM_H_
#define __TSEER_API_AGENT_ROUTER_COMM_H_

/*********************************************************
* 文件定义了与路由产品API接口所需的数据类型
**********************************************************/

#include <string>
#include <vector>

namespace Tseerapi
{
    //负载均衡类型
    enum LB_TYPE
    {
        LB_TYPE_LOOP,               //轮询
        LB_TYPE_RANDOM,             //随机
        LB_TYPE_STATIC_WEIGHT,      //静态权重
        LB_TYPE_CST_HASH,           //一致性哈希
        LB_TYPE_ALL                 //获取所有节点
    };
    
    //获取方式
    enum LB_GET_TYPE
    {
        LB_GET_IDC,             //按IDC
        LB_GET_SET,             //按set
        LB_GET_ALL,             //全部获取
    };

    //初始化纯API入参
    struct InitRawApiParams
    {
     std::string domainName;                //Registry域名
        std::string dnsFile;                   //DNS配置文件
     int registryPort;                        //Tseer服务端的端口，没有特殊情况使用默认值即可
        std::string serviceKey;                //用户在管理平台上为当前业务注册的key
        std::string reserved;                  //保留字段

        InitRawApiParams()
        {
            domainName = "localhost";
            dnsFile = "/etc/resolv.conf";
         registryPort = 9903;
            serviceKey = "";
            reserved = "";
        }
    };

    //初始化Agent API入参
    struct InitAgentApiParams
    {
        std::string serviceKey;        //用户在管理平台上为当前业务注册的key
        std::string reserved;          //保留字段

        InitAgentApiParams()
        {
            serviceKey = "";
            reserved = "";
        }
    };

    //路由请求的入参与出参
    struct RouterRequest
    {
        std::string     obj;                       //请求服务名称，必填
        LB_GET_TYPE     lbGetType;                  //按IDC、set或all获取，选填，默认按all获取
        std::string     setInfo;                   //set信息，当按set方式获取时必填
        LB_TYPE         type;                       //负载均衡类型，选填，默认使用轮询方式
        long            hashKey;                   //hash key，选择哈希负载均衡时必填
        std::string     moduleName;                //主调模块名，模调数据上报时需要该信息；若为空API会自动用本地Ip替代，选填（推荐填写）
        std::string     ip;                        //服务Ip，出参
        int             port;                      //服务端口，出参
        bool            isTcp;                       //服务是否为TCP，否则为UDP，出参
        std::string     slaveSet;                  //服务的set信息，出参（当服务启动set的时候返回）

        RouterRequest()
        {
            obj = "";
            lbGetType = LB_GET_ALL;             //默认按all获取
            setInfo = "";
            type = LB_TYPE_LOOP;                //默认使用轮询方式
            hashKey = -1;
            moduleName = "";
            ip = "";
            port = -1;
            isTcp = true;
            slaveSet = "";
        }
    };

    //路由节点信息
    struct NodeInfo
    {
        std::string     ip;                        //服务Ip，出参
        int             port;                      //服务端口，出参
        bool            isTcp;                       //服务是否为TCP，否则为UDP，出参
        std::string     slaveSet;                  //服务的set信息，出参（当服务启动set的时候返回） 
        NodeInfo()
        {
            ip = "";
            port = -1;
            isTcp = true;
            slaveSet = "";
        }       
    };   
    //路由请求的入参与出参
    struct RoutersRequest
    {
        std::string     obj;                       //请求服务名称，必填
        LB_GET_TYPE     lbGetType;                  //按IDC、set或all获取，选填，默认按all获取
        std::string     setInfo;                   //set信息，当按set方式获取时必填

        std::vector<NodeInfo> nodeInfoVec;
        RoutersRequest()
        {
            obj = "";
            lbGetType = LB_GET_ALL;             //默认按all获取
            setInfo = "";
        }
    };

    //ApiGetRoute返回值
    enum GET_ROUTER_RET
    {
        GET_ROUTER_SUCCESS      =   0,              //成功
        GET_ROUTER_NOT_EXIST    =   -1,             //服务不存在（常因Obj名字错误或服务已经下线引起）
        GET_ROUTER_FAILED       =   -2,             //失败
        GET_ROUTER_INNER_ERR    =   -3,             //内部错误
        GET_ROUTER_PARAM_ERR    =   -4,             //入参错误
        
    };
}

#endif
