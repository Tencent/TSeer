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

#include "router.h"

#include "global.h"
#include <iostream>
#include <sstream>
using std::ostringstream;
using std::string;
using std::vector;
using Tseer::RouterNodeInfo;

namespace Tseerapi
{

Router::Router(LB_TYPE type)
: _expreTime(0)
, _lastVisit(0)
, _lb(NULL)
{
    _lb = LoadBalanceFactory::CreateLoadBalance(type);
}

Router::~Router()
{
    if(_lb)
    {
        delete _lb;
        _lb = NULL;
    }
}
int Router::getRouters(vector<RouterNodeInfo>& nodeInfoVec, string &errMsg)
{
    size_t now = time(NULL);

    if(_expreTime < now)
    {
        int ret = _lb->getRouters(nodeInfoVec, errMsg);
        if(ret == 0)
        {
            return -2;
        }

        return -1;
    }
    _lastVisit = now;

    return _lb->getRouters(nodeInfoVec, errMsg);
}

int Router::getRouter(RouterNodeInfo& nodeInfo, string &errMsg)
{
    size_t now = time(NULL);

    if(_expreTime < now)
    {
        int ret = _lb->getRouter(nodeInfo, errMsg);
        if(ret == 0)
        {
            return -2;
        }

        return -1;
    }
    _lastVisit = now;

    return _lb->getRouter(nodeInfo, errMsg);
}

int Router::getRouter(unsigned long long key, RouterNodeInfo& nodeInfo, string &errMsg)
{
    size_t now = time(NULL);

    if(_expreTime < now)
    {
        int ret = _lb->getRouter(key, nodeInfo, errMsg);
        if(ret == 0)
        {
            return -2;
        }

        return -1;
    }
    _lastVisit = now;

    return _lb->getRouter(key, nodeInfo, errMsg);
}

int Router::updateRouterNodes(const vector<RouterNodeInfo> &RouterNodeInfoVec, string &errMsg)
{
    if(_lb == NULL)
    {
        ostringstream os;
        os << FILE_FUN << "lb is not created";
        errMsg = os.str();
        return -1;
    }

    _lb->clear();
    _lb->del(RouterNodeInfoVec);

    for(size_t i = 0; i < RouterNodeInfoVec.size(); ++i)
    {
        int ret = _lb->addRouter(RouterNodeInfoVec[i]);
        if(ret != 0)
        {
            ostringstream os;
            os << FILE_FUN << "add node error";
            errMsg = os.str();
            return ret;
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

int Router::statReqResult(const InnerRouterRequest& req, int ret, int timeCost)
{
    RouterNodeInfo nodeInfo;
    nodeInfo.ip = req.ip;
    nodeInfo.port = req.port;
    nodeInfo.isTcp = req.isTcp;

    if (req.isAgent)
    {
        return _statMgr.reportStat(req, ret, timeCost);
    }
    else
    {
        return _lb->statResult(nodeInfo, ret, timeCost);
    }
}

void Router::updateExpretime(time_t interval)
{
    size_t now = time(NULL);
    _expreTime = now + interval;
}

}
