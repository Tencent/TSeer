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

#ifndef __TSEER_API_STAT_MANAGER_H_
#define __TSEER_API_STAT_MANAGER_H_

/*********************************************************
* 文件定义了模调数据管理类，该类维护所有主调服务的模调数据，并按时上报
*********************************************************/

#include <map>
#include "Tseer_inner_comm.h"
#include "registry_ep_manager.h"
#include "TseerAgentComm.h"

namespace Tseerapi
{

struct ModuleStat {
    std::string masterName;
    std::string masterIp;
    std::string slaveName;
    std::string slaveIp;
    int            port;
    std::string funcName;
    int            allocNum;
    int            succNum;
    long        succDelay;
    int            errNum;
    long        errDelay;
    int            timeOutNum;
    long        timeOutDelay;
};

//全部模调数据的管理者
class StatManager
{
public:

    StatManager();

    virtual ~StatManager();

    //供Agent Api方式使用
    int reportStat(const InnerRouterRequest &req, int ret, int timeCost);

private:

    //禁用函数
    StatManager(const StatManager&);
    StatManager& operator=(const StatManager&);

    //上报单个服务的调用数据
    //int reportStat(const InnerRouterRequest &req, int ret, int timeCost);

    //通过TUP与Agent通信
    int reportToAgent(const Tseer::NodeStat &nodeStat, std::string &errMsg);

private:
    //线程id
    unsigned int _tid;

    //本机IP
    std::string _localIp;
};

}

#endif
