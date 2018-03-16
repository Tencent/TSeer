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

#include "servant/Application.h"
#include "RouterImp.h"
#include "RollLogger.h"
#include "ApiRouterFactory.h"

using namespace Tseer;

void RouterImp::initialize()
{
}

void RouterImp::destroy()
{
}

tars::Int32 RouterImp::getRouterNode(const Tseer::AgentRouterRequest &req,
                                     Tseer::AgentRouterResponse &rsp,
                                     tars::TarsCurrentPtr current)
{
    TSEER_LOG(COMMON_LOG)->debug() << FILE_FUN
        <<"obj:" << req.obj << "|" << Tseer::etos(req.getType) 
        <<"|"<< Tseer::etos(req.lbType)
        <<"|set:" << req.setname
        <<"|identifer:"<<req.identifier
        <<endl;
    
    int ret = GET_ROUTER_SUCCESS;
    string errMsg;
    do
    {
        if (req.obj.empty())
        {
            errMsg = "empty server obj name";
            ret = GET_ROUTER_PARAM_ERR;
            break;
        }

        if (req.getType != Tseer::GET_IDC && req.getType != Tseer::GET_SET && req.getType != Tseer::GET_ALL)
        {
            errMsg = "invalid LB_GET_TYPE";
            ret = GET_ROUTER_PARAM_ERR;
            break;
        }
        
        if (req.getType == Tseer::GET_SET && req.setname.empty())
        {
            errMsg = "empty set info while getting from set";
            ret = GET_ROUTER_PARAM_ERR;
            break;
        }
        
        if (req.lbType != LB_TYPE_LOOP && req.lbType != LB_TYPE_RANDOM && req.lbType != LB_TYPE_STATIC_WEIGHT
            && req.lbType != LB_TYPE_CST_HASH && req.lbType != LB_TYPE_ALL)
        {
            errMsg = "invalid LB_TYPE";
            ret = GET_ROUTER_PARAM_ERR;
            break;
        }

        ApiRouterManager*  apiRouter = ApiRouterFactory::getInstance()->getRouterMr(req);
        if(apiRouter)
        {
            ret = apiRouter->getRouter(req,rsp,errMsg);
        }
        else
        {
            errMsg = "get apiRouter null";
            ret = GET_ROUTER_INNER_ERR;
            break;
        }
    }while(0);
    
    TSEER_LOG(COMMON_LOG)->error() << FILE_FUN
        <<"obj:" << req.obj << "|" << Tseer::etos(req.getType) 
        <<"|"<< Tseer::etos(req.lbType)
        <<"|set:" << req.setname
        <<"|identifer:"<<req.identifier
        <<"|msg:"<<errMsg
        <<"|ret:"<<ret
        <<"|"<<rsp.resultList.size()
        <<endl;
    return ret;
}

tars::Int32 RouterImp::reportStat(const Tseer::NodeStat & stat, tars::TarsCurrentPtr current)
{
    //入参检查
    if (stat.obj.empty())
    {
        ostringstream msgBuffer;
        msgBuffer << FILE_FUN << "empty obj";
        return -1;
    }
    
    Tseer::AgentRouterRequest req;
    req.identifier = stat.identifier;
    req.getType = stat.getType;
    req.lbType = stat.lbType;
    req.obj = stat.obj;
    req.setname = stat.setname;
    
    int ret = GET_ROUTER_SUCCESS;
    string errMsg;
    ApiRouterManager*  apiRouter = ApiRouterFactory::getInstance()->getRouterMr(req);
    if(apiRouter)
    {
        Tseer::RouterNodeInfo nodeInfo;
        nodeInfo.ip = stat.ip;
        nodeInfo.port = stat.port;
        nodeInfo.isTcp = stat.isTcp;
        //更新nodeInfo 对应的调用统计信息，用来作为负载均衡和超时屏蔽依据
        ret = apiRouter->updateStat(req,nodeInfo,stat.ret,stat.timeCost,errMsg);
    }
    else
    {
        errMsg = "get apiRouter null";
        ret = GET_ROUTER_INNER_ERR;
    }
    
    TSEER_LOG(COMMON_LOG)->debug() << FILE_FUN
        <<"obj:" << req.obj << "|" << Tseer::etos(req.getType) 
        <<"|"<< Tseer::etos(req.lbType)
        <<"|set:" << req.setname
        <<"|identifer:"<<req.identifier
        <<"|ret:"<<ret
        <<"|errmsg:"<<errMsg
        <<endl;

    return ret;
}


