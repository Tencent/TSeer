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

#include "lb_all.h"
#include <arpa/inet.h>
#include <ctime>
#include <stdlib.h>
#include <sstream>
#include "global.h"

using namespace std;
using Tseer::RouterNodeInfo;

namespace Tseerapi
{

LBAll::LBAll() {}

LBAll::~LBAll() {}

int LBAll::addRouter(const RouterNodeInfo& nodeInfo)
{
    unsigned int ip = inet_addr(nodeInfo.ip.c_str());
    unsigned int tcp = (nodeInfo.isTcp ? 1 : 0); 
    unsigned long long id = (((unsigned long long)ip) << 32) | (((unsigned long long)nodeInfo.port) << 16) | tcp;

    std::map<unsigned long long, RouterNodeInfo>::iterator it = _normal.find(id);
    if(it == _normal.end())
    {
        _normal.insert(make_pair(id, nodeInfo));
    }
    else
    {
        it->second = nodeInfo;
    }

    return 0;
}

int LBAll::getRouters(vector<Tseer::RouterNodeInfo> &nodeInfoVec, string &errMsg)
{
    size_t normalSize = _normalNodeInfo.size();
    if(normalSize == 0)
    {
        ostringstream os;
        os << FILE_FUN << "getRouter has no avail node.";
        errMsg = os.str();
        return -1;
    }
    nodeInfoVec = _normalNodeInfo;
    return 0;
}
void LBAll::clear()
{
    _normalNodeInfo.clear();
}

int LBAll::rebuild()
{
    std::map<unsigned long long, RouterNodeInfo>::iterator it = _normal.begin();
    while(it != _normal.end())
    {
        _normalNodeInfo.push_back(it->second);
        ++it;
    }
    return 0;
}

int LBAll::statResult(const RouterNodeInfo& nodeInfo, int ret, int timeCost)
{
    return -1;
}

int LBAll::del(const vector<RouterNodeInfo> &nodeInfoVec)
{
    //新获取的全部节点
    set<unsigned long long> setNodeInfo;
    for(size_t i = 0; i < nodeInfoVec.size(); i++)
    {
        unsigned int ip = inet_addr(nodeInfoVec[i].ip.c_str());
        unsigned int tcp = (nodeInfoVec[i].isTcp ? 1 : 0); 
        unsigned long long id = (((unsigned long long)ip) << 32) | (((unsigned long long)nodeInfoVec[i].port) << 16) | tcp;

        setNodeInfo.insert(id);
    }

    std::map<unsigned long long, RouterNodeInfo>::iterator it = _normal.begin();
    while(it != _normal.end())
    {
        set<unsigned long long>::iterator it_find = setNodeInfo.find(it->first);
        //在新节点里找不到旧的节点就删掉
        if(it_find == setNodeInfo.end())
        {
            _normal.erase(it++);
        }
        else
        {
            it++;
        }
    }

    return 0;
}

}
