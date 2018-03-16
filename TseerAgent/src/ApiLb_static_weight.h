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

#ifndef __API_LB_STATIC_WEIGHT_H_
#define __API_LB_STATIC_WEIGHT_H_

#include "ApiLb.h"

class LBStaticWeight : public LoadBalance
{
public:
    LBStaticWeight();

    ~LBStaticWeight();

    virtual int addRouter(const Tseer::RouterNodeInfo& nodeInfo);

    virtual int getRouter(Tseer::RouterNodeInfo& nodeInfo, string &errMsg);

    virtual void clear();

    virtual int rebuild();

    virtual int del(const vector<Tseer::RouterNodeInfo> &nodeInfoVec);

    virtual int statResult(const Tseer::RouterNodeInfo& nodeInfo, int ret, int timeCost);

private:
    bool checkActive(const Tseer::RouterNodeInfo& nodeInfo);

    //计算全部节点权重的最大公约数，min为已知最小权重
    int gcdWeights(const std::vector<Tseer::RouterNodeInfo> nodes, int min);

    //节点列表
    std::vector<Tseer::RouterNodeInfo>        _normalNodeInfoVec;

    //节点列表，KEY由服务的IP、Port、TCP/UDP组成
    std::map<unsigned long long, Tseer::RouterNodeInfo> _normalMap;

    //节点状态，KEY由服务的IP、Port、TCP/UDP组成
    std::map<unsigned long long, RouterNodeStat>    _statMap;

    //各节点权重最大公约数
    int _gcdWeight;

    //各节点最大权重
    int _maxWeight;

    //权重筛选值，权重至少大于此值的机器才会被选中
    int _atLeastWeight;

    //上一轮选择的机器下标
    int _lastIndex;
};


#endif
