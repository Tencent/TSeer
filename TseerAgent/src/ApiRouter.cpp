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

#include <iostream>
#include <sstream>
#include "ApiRouter.h"
#include "RollLogger.h"

ApiRouter::ApiRouter(Tseer::LB_TYPE lbType)
: _expireTime(0)
, _lastVisit(0)
, _lb(NULL)
{
    _lb = LoadBalanceFactory::CreateLoadBalance(lbType);
}

ApiRouter::~ApiRouter()
{
    if(_lb)
    {
        delete _lb;
        _lb = NULL;
    }
}
int ApiRouter::getRouter(Tseer::RouterNodeInfo& nodeInfo, string &errMsg)
{
    size_t now = TNOW;

    if(_expireTime < now)
    {
        int iRet = _lb->getRouter(nodeInfo, errMsg);
        if(iRet == 0)
        {
            return GET_ROUTER_TIMEOUT;
        }

        return GET_ROUTER_NOTFIND;
    }
    _lastVisit = now;

    return _lb->getRouter(nodeInfo, errMsg);
}


int ApiRouter::getRouters(vector<RouterNodeInfo>& nodeInfoList, string &errMsg)
{
    size_t now = TNOW;

    if(_expireTime < now)
    {
        int iRet = _lb->getRouters(nodeInfoList, errMsg);
        if(iRet == 0)
        {
            return GET_ROUTER_TIMEOUT;
        }

        return GET_ROUTER_NOTFIND;
    }
    _lastVisit = now;

    return _lb->getRouters(nodeInfoList, errMsg);
}


int ApiRouter::getRouter(unsigned long long key, Tseer::RouterNodeInfo& nodeInfo, string &errMsg)
{
    size_t now = TNOW;

    if(_expireTime < now)
    {
        int iRet = _lb->getRouter(key, nodeInfo, errMsg);
        if(iRet == 0)
        {
            return GET_ROUTER_TIMEOUT;
        }

        return GET_ROUTER_NOTFIND;
    }
    _lastVisit = now;

    return _lb->getRouter(key, nodeInfo, errMsg);
}

int ApiRouter::updateRouterNodes(const vector<RouterNodeInfo> &nodeInfoList, string &errMsg)
{
    if(_lb == NULL)
    {
        ostringstream os;
        os << FILE_FUN << "lb is not created";
        errMsg = os.str();
        return -1;
    }

    _lb->clear();
    _lb->del(nodeInfoList);

    for(size_t i = 0; i < nodeInfoList.size(); ++i)
    {
        int iRet = _lb->addRouter(nodeInfoList[i]);
        if(iRet != 0)
        {
            ostringstream os;
            os << FILE_FUN << "add node error";
            errMsg = os.str();
            return iRet;
        }
    }

    if(_lb->rebuild())
    {
        ostringstream os;
        os << FILE_FUN << "rebuild node error";
        errMsg = os.str();
        return -1;
    }

    return 0;
}

int ApiRouter::statReqResult(const Tseer::RouterNodeInfo &nodeInfoRsp, int ret, int timeCost)
{
    if(_lb)
    {
        return _lb->statResult(nodeInfoRsp, ret, timeCost);
    }
    return -1;
}

void ApiRouter::updateExpireTime(time_t interval)
{
    _expireTime = TNOW + interval;
}

