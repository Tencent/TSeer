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

#include "lb_loop.h"
#include <arpa/inet.h>
#include <ctime>
#include <stdlib.h>
#include <sstream>
#include "global.h"

using namespace std;
using Tseer::RouterNodeInfo;

namespace Tseerapi
{

LBLoop::LBLoop(): _lastIndex(0){}

LBLoop::~LBLoop() {}

int LBLoop::addRouter(const RouterNodeInfo& nodeInfo)
{
    unsigned int ip = inet_addr(nodeInfo.ip.c_str());
    unsigned int tcp = (nodeInfo.isTcp ? 1 : 0); 
    unsigned long long id = (((unsigned long long)ip) << 32) | (((unsigned long long)nodeInfo.port) << 16) | tcp;

    std::map<unsigned long long, RouterNodeInfo>::iterator it = _normalMap.find(id);
    if(it == _normalMap.end())
    {
        _normalMap.insert(make_pair(id, nodeInfo));

        _statMap[id].succNum = 0;
        _statMap[id].errNum = 0;
        _statMap[id].timeCost = 0;
        _statMap[id].isAvailable = true;
        _statMap[id].nextRetryTime    = 0;
        _statMap[id].lastCheckTime    = 0;
        _statMap[id].continueErrNum    = 0;
        _statMap[id].continueErrTime = 0;
    }
    else
    {
        it->second = nodeInfo;
    }

    return 0;
}

int LBLoop::getRouter(RouterNodeInfo& nodeInfo, string &errMsg)
{
    const size_t normalSize = _normalNodeInfoVec.size();
    if(normalSize == 0)
    {
        ostringstream os;
        os << FILE_FUN << "getRouter has no avail node.";
        errMsg = os.str();
        return -1;
    }

    if(_lastIndex >= normalSize)
    {
        _lastIndex = 0;
    }

    vector<RouterNodeInfo> tempVec;

    for(size_t i = 0; i < _normalNodeInfoVec.size(); i++)
    {
        if(checkActive(_normalNodeInfoVec[_lastIndex]))
        {
            nodeInfo = _normalNodeInfoVec[_lastIndex++];
            return 0;
        }
        else
        {
            tempVec.push_back(_normalNodeInfoVec[_lastIndex]);
        }

        _lastIndex++;
        if(_lastIndex >= normalSize)
            _lastIndex = 0;
    }

    if(tempVec.empty())
    {
        ostringstream os;
        os << FILE_FUN << "getRouter has no avail node.";
        errMsg = os.str();
        return -1;
    }

    size_t nodeIndex = random() % tempVec.size();

    nodeInfo = tempVec[nodeIndex];

    return 0;
}

void LBLoop::clear()
{
    //当节点获取频率低于或接近节点更新频率时，那么会导致负载不均衡（总是获取头一两个节点）
    //因此在执行clear时，不把_lastIndex置为0
    //_lastIndex = 0;        
    _normalNodeInfoVec.clear();
}

int LBLoop::rebuild()
{
    std::map<unsigned long long, RouterNodeInfo>::iterator it = _normalMap.begin();
    while(it != _normalMap.end())
    {
        _normalNodeInfoVec.push_back(it->second);
        ++it;
    }
    return 0;
}

int LBLoop::statResult(const RouterNodeInfo& nodeInfo, int ret, int timeCost)
{
    unsigned int ip = inet_addr(nodeInfo.ip.c_str());
    unsigned int tcp = (nodeInfo.isTcp ? 1 : 0); 
    unsigned long long id = (((unsigned long long)ip) << 32) | (((unsigned long long)nodeInfo.port) << 16) | tcp;

    std::map<unsigned long long, RouterNodeStat>::iterator it =    _statMap.find(id);
    if(it == _statMap.end())
    {
        return -1;
    }

    size_t now = time(NULL);

    bool isFail = ((ret >= 0) ? false : true);

    RouterNodeStat &stat = it->second;

    //如果之前节点是不可用的
    if(!stat.isAvailable)
    {
        if(!isFail)
        {
            stat.isAvailable = true;
            stat.succNum = 1;
            stat.errNum = 0;
            stat.timeCost = timeCost;
            stat.continueErrNum = 0;
            stat.continueErrTime = now + _checkTimeoutInfo.minFrequenceFailTime;
            stat.lastCheckTime = now + _checkTimeoutInfo.checkTimeoutInterval;
        }
        else
        {
            stat.errNum++;
        }

        return 0;
    }

    if(!isFail)
    {
        stat.succNum++;
    }
    else
    {
        stat.errNum++;
    }

    if(isFail)
    {
        if(stat.continueErrNum == 0)
        {
            stat.continueErrTime = now + _checkTimeoutInfo.minFrequenceFailTime;
        }

        stat.continueErrNum++;

        //在minFrequenceFailTime时间内，错误次数超过frequenceFailInvoke次
        if(stat.continueErrNum >= _checkTimeoutInfo.frequenceFailInvoke && now >= stat.continueErrTime)
        {
            stat.isAvailable = false;
            stat.nextRetryTime = now + _checkTimeoutInfo.tryTimeInterval;

            return 0;
        }
    }
    else
    {
        stat.continueErrNum = 0;
    }

    //进行一轮统计
    if(now >= stat.lastCheckTime)
    {
        stat.lastCheckTime = now + _checkTimeoutInfo.checkTimeoutInterval;

        if(isFail && (stat.succNum + stat.errNum) >= _checkTimeoutInfo.minTimeoutInvoke && 
            stat.errNum >= _checkTimeoutInfo.timeoutRatio * (stat.succNum + stat.errNum))
        {
            stat.isAvailable = false;
            stat.nextRetryTime = now + _checkTimeoutInfo.tryTimeInterval;
        }
        else
        {
            stat.succNum = 0;
            stat.errNum = 0;
        }
    }

    return 0;
}

int LBLoop::del(const vector<RouterNodeInfo> &nodeInfo)
{
    //新获取的全部节点
    set<unsigned long long> setNodeInfo;
    for(size_t i = 0; i < nodeInfo.size(); i++)
    {
        unsigned int ip = inet_addr(nodeInfo[i].ip.c_str());
        unsigned int tcp = (nodeInfo[i].isTcp ? 1 : 0); 
        unsigned long long id = (((unsigned long long)ip) << 32) | (((unsigned long long)nodeInfo[i].port) << 16) | tcp;

        setNodeInfo.insert(id);
    }

    std::map<unsigned long long, RouterNodeInfo>::iterator it = _normalMap.begin();
    while(it != _normalMap.end())
    {
        //在新节点里找不到旧的节点就删掉
        if (setNodeInfo.count(it->first) == 0)
        {
            _statMap.erase(it->first);
            _normalMap.erase(it++);
        }
        else
        {
            it++;
        }
    }

    return 0;
}

bool LBLoop::checkActive(const RouterNodeInfo& nodeInfo)
{
    unsigned int ip = inet_addr(nodeInfo.ip.c_str());
    unsigned int tcp = (nodeInfo.isTcp ? 1 : 0); 
    unsigned long long id = (((unsigned long long)ip) << 32) | (((unsigned long long)nodeInfo.port) << 16) | tcp;

    std::map<unsigned long long, RouterNodeStat>::iterator it =    _statMap.find(id);
    if(it == _statMap.end())
    {
        return false;
    }

    size_t now = time(NULL);

    if(!(it->second.isAvailable) && now < it->second.nextRetryTime)
    {
        return false;
    }

    if(!(it->second.isAvailable))
    {
        it->second.nextRetryTime = now + _checkTimeoutInfo.tryTimeInterval;
    }

    return true;
}

}
