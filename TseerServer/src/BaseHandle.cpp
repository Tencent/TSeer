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

#include "BaseHandle.h"
#include "util.h"

BaseHandle::BaseHandle(){}

BaseHandle::~BaseHandle(){}

int BaseHandle::init(TC_Config * pconf)
{
    TLOGDEBUG(FILE_FUN  << "Unsupported method!" << endl);
    _nodeTimeout = pconf->get("/tars/reap<nodeTimeout>", "250");
    
        /* 主控心跳超时时间 */
    _registryTimeout = pconf->get("/tars/reap<reigistryTimeout>", "120");
    return -1;
}

/***********************************Server服务************************************************/

int BaseHandle::addServers(const vector<RouterData> &routerDataList,tars::TarsCurrentPtr current)
{
    TLOGDEBUG(FILE_FUN  << "Unsupported method!" << endl);
    return -1;
}
int BaseHandle::updateServers(const vector<RouterData>& needUpdateRouterList,tars::TarsCurrentPtr &current)
{
    TLOGDEBUG(FILE_FUN  << "Unsupported method!" << endl);
    return -1;
}
int BaseHandle::deleteServers(const vector<RouterDataRequest>& delSrvList,tars::TarsCurrentPtr &current)
{
    TLOGDEBUG(FILE_FUN  << "Unsupported method!" << endl);
    return -1;
}
int BaseHandle::getServers(const RouterDataRequest & reqInfo, tars::TarsCurrentPtr current)
{
    TLOGDEBUG(FILE_FUN  << "Unsupported method!" << endl);
    return -1;
}
int BaseHandle::getAllServers(vector<map<string, string> > &routerDataMap)
{
    TLOGDEBUG(FILE_FUN  << "Unsupported method!" << endl);
    return -1;
}

int BaseHandle::updateServerState(const RouterData &routerDataInfo,tars::TarsCurrentPtr& current)
{
    TLOGDEBUG(FILE_FUN  << "Unsupported method!" << endl);
    return -1;
}
int BaseHandle::updateServerStates(const vector<RouterData>& needUpdateRouterList,tars::TarsCurrentPtr &current)
{
    TLOGDEBUG(FILE_FUN  << "Unsupported method!" << endl);
    return -1;
}
int BaseHandle::updateNodeServerStates(const string& ip, const string& presentState)
{
    TLOGDEBUG(FILE_FUN  << "Unsupported method!" << endl);
    return -1;
}

int BaseHandle::keepServerAlive(const vector<RouterData>& needUpdateRouterList,tars::TarsCurrentPtr &current)
{
    TLOGDEBUG(FILE_FUN  << "Unsupported method!" << endl);
    return -1;
}

/***********************************基础服务************************************************/

int BaseHandle::addBaseServiceInfo(const string &locatorId, const map<string, string> &mapServantEndpoint)
{
    TLOGDEBUG(FILE_FUN  << "Unsupported method!" << endl);
    return -1;
}
int BaseHandle::updateBaseServiceInfo(const string &locatorId, const map<string, string> &mapServantEndpoint)
{
    TLOGDEBUG(FILE_FUN  << "Unsupported method!" << endl);
    return -1;
}

/*********************************Node类服务************************************************/

int BaseHandle::addNodeInfo(const NodeServiceInfo &nodeInfo)
{
    TLOGDEBUG(FILE_FUN  << "Unsupported method!" << endl);
    return -1;
}
int BaseHandle::delNodeInfo(const string &nodeName, const string &networkId)
{
    TLOGDEBUG(FILE_FUN  << "Unsupported method!" << endl);
    return -1;
}
int BaseHandle::keepAlive(const string& name, const LoadInfo& li)
{
    TLOGDEBUG(FILE_FUN  << "Unsupported method!" << endl);
    return -1;
}
int BaseHandle::keepNodeAlive(const NodeInstanceInfo& stNodeInstanceInfo, const string &networkId)
{
    TLOGDEBUG(FILE_FUN  << "Unsupported method!" << endl);
    return -1;
}

int BaseHandle::updateNodeStateBatch(vector<NodeServiceInfo> &nodeList)
{
    TLOGDEBUG(FILE_FUN  << "Unsupported method!" << endl);
    return -1;
}

int BaseHandle::getAllNodes(vector<NodeServiceInfo> &nodeInfo, const string &networkId)
{
    TLOGDEBUG(FILE_FUN  << "Unsupported method!" << endl);
    return -1;
}
int BaseHandle::getAllTimeOutNodes(vector<NodeServiceInfo> &nodeList, const string &networkId, const unsigned int interval)
{
    TLOGDEBUG(FILE_FUN  << "Unsupported method!" << endl);
    return -1;
}
int BaseHandle::getOneSeerAgent(const string &nodeName, NodeServiceInfo &nodeServiceInfo)
{
    TLOGDEBUG(FILE_FUN  << "Unsupported method!" << endl);
    return -1;
}
int BaseHandle::getAllAliveSeerAgents(std::set<string> &aliveNodes)
{
    TLOGDEBUG(FILE_FUN  << "Unsupported method!" << endl);
    return -1;
}

/*********************************IDC相关操作*************************************************/

int BaseHandle::addIdcGroupRule(const string &groupName, const vector<string> &allowIps, TarsCurrentPtr current)
{
    TLOGDEBUG(FILE_FUN  << "Unsupported method!" << endl);
    return -1;
}
int BaseHandle::modifyIdcGroupRule(const string &groupId, const vector<string> &allowIps, TarsCurrentPtr current)
{
    TLOGDEBUG(FILE_FUN  << "Unsupported method!" << endl);
    return -1;
}
int BaseHandle::delIdcGroupRule(const vector<string> &groupsIds, TarsCurrentPtr current)
{
    TLOGDEBUG(FILE_FUN  << "Unsupported method!" << endl);
    return -1;
}
int BaseHandle::getAllIdcGroupRule(vector<IDCGroupInfo> &idcGroupInfoList)
{
    TLOGDEBUG(FILE_FUN  << "Unsupported method!" << endl);
    return -1;
}

int BaseHandle::addIdcPriority(int order, const string &priorityName, const vector<string> &groupIdList, TarsCurrentPtr current)
{
    TLOGDEBUG(FILE_FUN  << "Unsupported method!" << endl);
    return -1;
}
int BaseHandle::modifyIdcPriority(const string &priorityId, int order, const string &priorityName, const vector<string> &groupIdList, TarsCurrentPtr current)
{
    TLOGDEBUG(FILE_FUN  << "Unsupported method!" << endl);
    return -1;
}
int BaseHandle::delIdcPriority(const vector<string> &priorityIds, TarsCurrentPtr current)
{
    TLOGDEBUG(FILE_FUN  << "Unsupported method!" << endl);
    return -1;
}
int BaseHandle::getAllIdcPriority(vector<IDCPriority> &idcPriorityInfo)
{
    TLOGDEBUG(FILE_FUN  << "Unsupported method!" << endl);
    return -1;
}

/*********************************ServiceGroup相关操作*************************************************/

int BaseHandle::addServiceGroup(const string& srvGrp, const string& key, const string& userListStr)
{
    TLOGDEBUG(FILE_FUN  << "Unsupported method!" << endl);
    return -1;
}
int BaseHandle::updateServiceGroup(const string& srvGrp,const string& key, const string& userListStr)
{
    TLOGDEBUG(FILE_FUN  << "Unsupported method!" << endl);
    return -1;
}

int BaseHandle::getAllServiceGroup(vector<ServiceGroupInfo>& srvGrpInfoList)
{
    TLOGDEBUG(FILE_FUN  << "Unsupported method!" << endl);
    return -1;
}
int BaseHandle::getServiceGroupKey(const string& srvGrp,string& key)
{
    TLOGDEBUG(FILE_FUN  << "Unsupported method!" << endl);
    return -1;
}

/*******************************SeerAgent管理********************************/
int BaseHandle::get_agent_baseinfo(const AgentBaseInfoReq& req,vector<AgentBaseInfo>& AgentBaseInfos)
{
    TLOGDEBUG(FILE_FUN  << "Unsupported method!" << endl);
    return -1;   
}

int BaseHandle::update_agent_graystate(const UpdateAgentInfoReq& req)
{
    TLOGDEBUG(FILE_FUN  << "Unsupported method!" << endl);
    return -1;       
}

int BaseHandle::get_agent_packageinfo(const AgentPackageReq& req,vector<AgentPackageInfo>& AgentPackageInfos)
{
    TLOGDEBUG(FILE_FUN  << "Unsupported method!" << endl);
    return -1;       
}

int BaseHandle::update_agent_packageinfo(const UpdatePackageInfoReq& req)
{
    TLOGDEBUG(FILE_FUN  << "Unsupported method!" << endl);
    return -1;       
}

int BaseHandle::delete_agent_package(const AgentPackageReq& req)
{
    TLOGDEBUG(FILE_FUN  << "Unsupported method!" << endl);
    return -1;      
}

int BaseHandle::add_agent_packageinfo(const AgentPackageInfo & reqInfo)
{
    TLOGDEBUG(FILE_FUN  << "Unsupported method!" << endl);
    return -1;          
}