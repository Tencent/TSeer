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

#include <stdlib.h>
#include <syscall.h>
#include <arpa/inet.h>
#include <time.h>
#include <ctime>
#include <sstream>

#include "ApiLb_random.h"
#include "TseerAgentServer.h"

LBRandom::LBRandom()
{
    struct timespec now = {0, 0};
    clock_gettime(CLOCK_REALTIME, &now);
    srandom(now.tv_nsec);
    _normalNodeInfoVec.reserve(32);
}

LBRandom::~LBRandom() {}

int LBRandom::addRouter(const RouterNodeInfo& nodeInfo)
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
        _statMap[id].available = true;
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

bool LBRandom::checkActive(const RouterNodeInfo& nodeInfo)
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

    if(!(it->second.available) && now < it->second.nextRetryTime)
    {
        return false;
    }

    if(!(it->second.available))
    {
        it->second.nextRetryTime = now + g_app.getTryTimeInterval();
    }

    return true;
}

int LBRandom::getRouter(RouterNodeInfo& nodeInfo, string &errMsg)
{
    size_t normalSize = _normalNodeInfoVec.size();
    if(normalSize == 0)
    {
        ostringstream os;
        os << FILE_FUN << "getRouter has no active node.";
        errMsg = os.str();
        return -1;
    }

    size_t nodeIndex = random() % normalSize;

    vector<RouterNodeInfo> tmpVec;

    for(size_t i = 0; i < _normalNodeInfoVec.size(); i++)
    {
        if(checkActive(_normalNodeInfoVec[nodeIndex]))
        {
            nodeInfo = _normalNodeInfoVec[nodeIndex];
            return 0;
        }
        else
        {
            tmpVec.push_back(_normalNodeInfoVec[nodeIndex]);
        }

        nodeIndex++;
        if(nodeIndex >= normalSize)
            nodeIndex = 0;
    }

    if(tmpVec.size() <= 0)
    {
        ostringstream os;
        os << FILE_FUN << "getRouter has no avail node.";
        errMsg = os.str();
        return -1;
    }

    nodeIndex = random() % tmpVec.size();

    nodeInfo = tmpVec[nodeIndex];

    return 0;
}

void LBRandom::clear()
{
    _normalNodeInfoVec.clear();
}

int LBRandom::rebuild()
{
    std::map<unsigned long long, RouterNodeInfo>::iterator it = _normalMap.begin();
    while(it != _normalMap.end())
    {
        _normalNodeInfoVec.push_back(it->second);
        ++it;
    }
    return 0;
}

int LBRandom::statResult(const RouterNodeInfo& nodeInfo, int ret, int timeCost)
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

    bool fail = ((ret >= 0) ? false : true);

    RouterNodeStat &stat = it->second;

    if(!stat.available)
    {
        if(!fail)
        {
            stat.available = true;
            stat.succNum = 1;
            stat.errNum = 0;
            stat.timeCost = timeCost;
            stat.continueErrNum = 0;
            stat.continueErrTime = now + g_app.getMinFrequenceFailTime();
            stat.lastCheckTime = now + g_app.getCheckTimeoutInterval();
        }
        else
        {
            stat.errNum++;
        }

        return 0;
    }

    if(!fail)
    {
        stat.succNum++;
    }
    else
    {
        stat.errNum++;
    }

    if(fail)
    {
        if(stat.continueErrNum == 0)
        {
            stat.continueErrTime = now + g_app.getMinFrequenceFailTime();
        }

        stat.continueErrNum++;

        if(stat.continueErrNum >= g_app.getFrequenceFailInvoke() && now >= stat.continueErrTime)
        {
            stat.available = false;
            stat.nextRetryTime = now + g_app.getTryTimeInterval();

            return 0;
        }
    }
    else
    {
        stat.continueErrNum = 0;
    }

    if(now >= stat.lastCheckTime)
    {
        stat.lastCheckTime = now + g_app.getCheckTimeoutInterval();

        if(fail && (stat.succNum + stat.errNum) >= g_app.getMinTimeoutInvoke() && 
            stat.errNum >= g_app.getRadio() * (stat.succNum + stat.errNum))
        {
            stat.available = false;
            stat.nextRetryTime = now + g_app.getTryTimeInterval();
        }
        else
        {
            stat.succNum = 0;
            stat.errNum = 0;
        }
    }

    return 0;
}

int LBRandom::del(const vector<RouterNodeInfo> &nodeInfoVec)
{
    set<unsigned long long> setNodeInfo;
    for(size_t i = 0; i < nodeInfoVec.size(); i++)
    {
        unsigned int ip = inet_addr(nodeInfoVec[i].ip.c_str());
        unsigned int tcp = (nodeInfoVec[i].isTcp ? 1 : 0); 
        unsigned long long id = (((unsigned long long)ip) << 32) | (((unsigned long long)nodeInfoVec[i].port) << 16) | tcp;

        setNodeInfo.insert(id);
    }

    std::map<unsigned long long, RouterNodeInfo>::iterator it = _normalMap.begin();
    while(it != _normalMap.end())
    {
        set<unsigned long long>::iterator it_find = setNodeInfo.find(it->first);
        if(it_find == setNodeInfo.end())
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


