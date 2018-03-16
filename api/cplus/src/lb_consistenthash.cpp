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

#include "lb_consistenthash.h"

#include <stdlib.h>
#include <arpa/inet.h>
#include <syscall.h>
#include <time.h>
#include <ctime>
#include <sstream>
#include "global.h"

using namespace std;
using Tseer::RouterNodeInfo;

namespace Tseerapi
{

LBConsistentHash::LBConsistentHash()
{
    struct timespec now = {0, 0};
    clock_gettime(CLOCK_REALTIME, &now);
    srandom(now.tv_nsec);
}

LBConsistentHash::~LBConsistentHash() {}

int LBConsistentHash::addRouter(const RouterNodeInfo& nodeInfo)
{
    unsigned int ip = inet_addr(nodeInfo.ip.c_str());
    unsigned int tcp = (nodeInfo.isTcp ? 1 : 0); 
    unsigned long long id = (((unsigned long long)ip) << 32) | (((unsigned long long)nodeInfo.port) << 16) | tcp;

    std::map<unsigned long long, RouterNodeInfo>::iterator it = _normalMap.find(id);
    if(it == _normalMap.end())
    {
        _normalMap.insert(make_pair(id, nodeInfo));

        for(int i = 0; i < nodeInfo.weight; i++)
        {
            ostringstream buffer;
            buffer << nodeInfo.ip << ":" << i << ":" << nodeInfo.port;

            string hashStr = buffer.str();

            unsigned int hashNum = murmurHash32(hashStr.c_str(), hashStr.size(), 16);

            std::map<unsigned int, RouterNodeInfo>::iterator it_hash = _tableMap.find(hashNum);
            if(it_hash != _tableMap.end())
            {
                if(nodeInfo.weight > it_hash->second.weight)
                {
                    it_hash->second = nodeInfo;
                }
            }
            else
            {
                _tableMap.insert(make_pair(hashNum, nodeInfo));
            }
        }

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
        if(nodeInfo.weight != it->second.weight)
        {
            if(nodeInfo.weight > it->second.weight)
            {
                for(int i = it->second.weight; i < nodeInfo.weight; i++)
                {
                    ostringstream buffer;
                    buffer << nodeInfo.ip << ":" << i << ":" << nodeInfo.port;

                    string hashStr = buffer.str();

                    unsigned int hashNum = murmurHash32(hashStr.c_str(), hashStr.size(), 16);

                    std::map<unsigned int, RouterNodeInfo>::iterator it_hash = _tableMap.find(hashNum);
                    if(it_hash != _tableMap.end())
                    {
                        if(nodeInfo.weight > it_hash->second.weight)
                        {
                            it_hash->second = nodeInfo;
                        }
                    }
                    else
                    {
                        _tableMap.insert(make_pair(hashNum, nodeInfo));
                    }

                }
            }
            else
            {
                for(int i = nodeInfo.weight; i < it->second.weight; i++)
                {
                    ostringstream buffer;
                    buffer << nodeInfo.ip << ":" << i << ":" << nodeInfo.port;

                    string hashStr = buffer.str();

                    unsigned int hashNum = murmurHash32(hashStr.c_str(), hashStr.size(), 16);

                    std::map<unsigned int, RouterNodeInfo>::iterator it_hash = _tableMap.find(hashNum);
                    if(it_hash != _tableMap.end())
                    {
                        if(it->second.weight == it_hash->second.weight)
                        {
                            _tableMap.erase(it_hash);
                        }
                    }

                }
            }
        }
    }

    return 0;
}

bool LBConsistentHash::checkActive(const RouterNodeInfo& nodeInfo)
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
int LBConsistentHash::getRouter(RouterNodeInfo& nodeInfo, string &errMsg)
{
    return getRouter(random(), nodeInfo, errMsg);
}

int LBConsistentHash::getRouter(unsigned long long key, RouterNodeInfo& nodeInfo, string &errMsg)
{
    size_t normalSize = _normalMap.size();
    if(normalSize == 0)
    {
        ostringstream os;
        os << FILE_FUN << "getRouter has no active node.";
        errMsg = os.str();
        return -1;
    }

    bool isActive = true;
    unsigned int hashNum = murmurHash32((const char*)&key, sizeof(key), 16);
    std::map<unsigned int, RouterNodeInfo>::iterator it_hash = _tableMap.lower_bound(hashNum);
    if(it_hash != _tableMap.end())
    {
        if(checkActive(it_hash->second))
        {
            nodeInfo = it_hash->second;
            return 0;
        }
        else
        {
            isActive = false;
        }
    }
    else
    {
        if(_tableMap.size())
        {
            if(checkActive(_tableMap.begin()->second))
            {
                nodeInfo = _tableMap.begin()->second;
                return 0;
            }
            else
            {
                isActive = false;
            }
        }
    }

    if(!isActive)
    {
        std::vector<RouterNodeInfo>        normalNodeInfoVec;

        std::map<unsigned int, RouterNodeInfo>::iterator it = _tableMap.begin();
        while(it != _tableMap.end())
        {
            normalNodeInfoVec.push_back(it->second);
            ++it;
        }

        if(normalNodeInfoVec.size() > 0)
        {
            int iNum = normalNodeInfoVec.size();
            while(iNum > 0)
            {
                hashNum = hashNum % normalNodeInfoVec.size();
                if(checkActive(normalNodeInfoVec[hashNum]))
                {
                    nodeInfo = normalNodeInfoVec[hashNum];
                    return 0;
                }

                --iNum;
                hashNum++;
            }

            size_t nodeIndex = random() % normalNodeInfoVec.size();

            nodeInfo = normalNodeInfoVec[nodeIndex];
        }
    }

    return -1;
}

void LBConsistentHash::clear(){}

unsigned int LBConsistentHash::murmurHash32(const char *key, unsigned int len, unsigned int seed)
{
    const int nblocks = len / 4;
    const unsigned char *tail = (const unsigned char*)key + nblocks * 4;
    const unsigned int *blocks = (const unsigned int*)key;
    unsigned int k;

    unsigned hash = seed;

    int index;
    for(index = 0; index < nblocks; index++)
    {
        k = *blocks++;
        k *= 0xcc9e2d51;
        k = (k << 15) | (k >> 17);
        k *=  0x1b873593;
        
        hash ^= k;
        hash = ( (hash << 13) | (hash >> 19)) * 5 + 0xe6546b64;
    }

    k = 0;
    switch(len & 3 )
    {
    case 3:
        k ^= tail[2] << 16;
    case 2:
        k ^= tail[1] << 8;
    case 1:
        k ^= tail[0];

        k *= 0xcc9e2d51;
        k = (k << 15) | (k >> 17);
        k *=  0x1b873593;
        
        hash ^= k;
    }

    hash ^= len;
    hash ^= (hash >> 16);
    hash *= 0x85ebca6b;
    hash ^= (hash >> 13);
    hash *= 0xc2b2ae35;
    hash ^= (hash >> 16);

    return hash;
}

int LBConsistentHash::rebuild()
{
    return 0;
}

int LBConsistentHash::statResult(const RouterNodeInfo& nodeInfo, int ret, int timeCost)
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

    if(now >= stat.lastCheckTime)
    {
        stat.lastCheckTime = now + _checkTimeoutInfo.checkTimeoutInterval;

        if(isFail && (stat.succNum + stat.errNum) >= _checkTimeoutInfo.minTimeoutInvoke && stat.errNum >= _checkTimeoutInfo.timeoutRatio * (stat.succNum + stat.errNum))
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

int LBConsistentHash::del(const vector<RouterNodeInfo> &nodeInfoVec)
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

            for(int i = 0; i < it->second.weight; i++)
            {
                ostringstream buffer;
                buffer << it->second.ip << ":" << i << ":" << it->second.port;

                string hashStr = buffer.str();

                unsigned int hashNum = murmurHash32(hashStr.c_str(), hashStr.size(), 16);

                std::map<unsigned int, RouterNodeInfo>::iterator it_hash = _tableMap.find(hashNum);
                if(it_hash != _tableMap.end())
                {
                    if(it->second.weight == it_hash->second.weight)
                    {
                        _tableMap.erase(it_hash);
                    }
                }

            }

            _normalMap.erase(it++);
        }
        else
        {
            it++;
        }
    }
    return 0;
}

}
