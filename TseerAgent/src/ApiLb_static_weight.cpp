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

#include <arpa/inet.h>
#include <set>
#include <ctime>
#include <iostream>
#include <sstream>
#include "TseerAgentServer.h"
#include "ApiLb_static_weight.h"

LBStaticWeight::LBStaticWeight() : _lastIndex(0) {}

LBStaticWeight::~LBStaticWeight() {}

int LBStaticWeight::addRouter(const Tseer::RouterNodeInfo& nodeInfo)
{
    unsigned int ip = inet_addr(nodeInfo.ip.c_str());
    unsigned int tcp = (nodeInfo.isTcp ? 1 : 0);
    unsigned long long id = (((unsigned long long)ip) << 32) | (((unsigned long long)nodeInfo.port) << 16) | tcp;

    std::map<unsigned long long, RouterNodeInfo>::iterator it = _normalMap.find(id);
    if (it == _normalMap.end())
    {
        _normalMap.insert(make_pair(id, nodeInfo));

        _statMap[id].succNum = 0;
        _statMap[id].errNum = 0;
        _statMap[id].timeCost = 0;
        _statMap[id].available = true;
        _statMap[id].nextRetryTime = 0;
        _statMap[id].lastCheckTime = 0;
        _statMap[id].continueErrNum = 0;
        _statMap[id].continueErrTime = 0;
    }
    else
    {
        it->second = nodeInfo;
    }
    return 0;
}
int LBStaticWeight::getRouter(Tseer::RouterNodeInfo& nodeInfo, string &errMsg)
{
    while (true)
    {
        _lastIndex = (_lastIndex + 1) % _normalNodeInfoVec.size();
        if (_lastIndex == 0)    //大于权重筛选值的服务器都被选择了一遍
        {
            _atLeastWeight -= _gcdWeight;
            if (_atLeastWeight <= 0)    //所有筛选权重值都重复了一遍
            {
                _atLeastWeight = _maxWeight;
                if (_atLeastWeight == 0)
                {
                    //最大权重值为0或小于0，说明没有节点都没有设置静态权重或最大权重有错，此时报错
                    ostringstream os;
                    os << FILE_FUN << "INVALID MAX WEIGHT: 0";
                    errMsg = os.str();
                    return -1;
                }
            }
        }
        if (_normalNodeInfoVec[_lastIndex].weight >= _atLeastWeight)
        {
            //检查节点是否可用
            if (checkActive(_normalNodeInfoVec[_lastIndex]))
            {
                nodeInfo = _normalNodeInfoVec[_lastIndex];
                return 0;
            }
        }
    }
}

void LBStaticWeight::clear()
{
    //当节点获取频率低于或接近节点更新频率时，那么会导致负载不均衡（总是获取头一两个节点）
    //因此在执行clear时，不把_lastIndex置为0
    //_lastIndex = 0;
    _gcdWeight = 0;
    _maxWeight = 0;
    _atLeastWeight = 0;
    _normalNodeInfoVec.clear();
}

int LBStaticWeight::rebuild()
{
    //重建列表，找出最大、最小权重
    int minWeight = INT_MAX;
    for (std::map<unsigned long long, Tseer::RouterNodeInfo>::iterator it = _normalMap.begin(); 
        it != _normalMap.end(); ++it)
    {
        _normalNodeInfoVec.push_back(it->second);
        if (it->second.weight < minWeight)
        {
            minWeight = it->second.weight;
        }
        if (it->second.weight > _maxWeight)
        {
            _maxWeight = it->second.weight;
        }
    }

    _atLeastWeight = _maxWeight;
    
    //获取权重公约数
    _gcdWeight = gcdWeights(_normalNodeInfoVec, minWeight);
    return 0;
}

int LBStaticWeight::del(const vector<Tseer::RouterNodeInfo> &nodeInfoVec)
{
    set<unsigned long long> setNodeInfo;
    for (size_t i = 0; i < nodeInfoVec.size(); i++)
    {
        unsigned int ip = inet_addr(nodeInfoVec[i].ip.c_str());
        unsigned int tcp = (nodeInfoVec[i].isTcp ? 1 : 0);
        unsigned long long id = (((unsigned long long)ip) << 32) | (((unsigned long long)nodeInfoVec[i].port) << 16) | tcp;

        setNodeInfo.insert(id);
    }

    std::map<unsigned long long, RouterNodeInfo>::iterator it = _normalMap.begin();
    while (it != _normalMap.end())
    {
        set<unsigned long long>::iterator it_find = setNodeInfo.find(it->first);
        if (it_find == setNodeInfo.end())
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

int LBStaticWeight::statResult(const Tseer::RouterNodeInfo& nodeInfo, int ret, int timeCost)
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

    //如果之前节点是不可用的
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

        //在iMinFrequenceFailTime时间内，错误次数超过iFrequenceFailInvoke次
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

    //进行一轮统计
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

bool LBStaticWeight::checkActive(const Tseer::RouterNodeInfo& nodeInfo)
{
    unsigned int ip = inet_addr(nodeInfo.ip.c_str());
    unsigned int tcp = (nodeInfo.isTcp ? 1 : 0);
    unsigned long long id = (((unsigned long long)ip) << 32) | (((unsigned long long)nodeInfo.port) << 16) | tcp;

    std::map<unsigned long long, RouterNodeStat>::iterator it = _statMap.find(id);
    if (it == _statMap.end())
    {
        return false;
    }

    size_t now = time(NULL);

    if (!(it->second.available) && now < it->second.nextRetryTime)
    {
        return false;
    }

    if (!(it->second.available))
    {
        it->second.nextRetryTime = now + g_app.getTryTimeInterval();
    }
    
    return true;
}

int LBStaticWeight::gcdWeights(const std::vector<Tseer::RouterNodeInfo> nodes, int min)
{
    int result = 1;
    for (int gcd = 2; gcd <= min; ++gcd)
    {
        bool getGcd = true;
        for (std::vector<Tseer::RouterNodeInfo>::const_iterator iter = nodes.begin(); iter != nodes.end(); ++iter)
        {
            if (iter->weight % gcd != 0)
            {
                getGcd = false;
                break;
            }
        }
        if (getGcd)
        {
            result = gcd;
        }
    }

    return result;
}


