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

#include "util/tc_common.h"

#include "StoreProxy.h"
#include "EtcdHandle.h"
#include "MysqlHandle.h"

StoreProxy::StoreProxy() {}

StoreProxy::~StoreProxy() {}

int StoreProxy::init(TC_Config * pconf)
{
    string storeType = pconf->get("/tars/application/server<store>", "mysql");
    if(storeType == "etcd")
    {
        _baseHandle = new EtcdHandle();
    }
    else
    {
        _baseHandle = new MysqlHandle();
    }

    return _baseHandle->init(pconf);
}

int StoreProxy::updateNodeServerStates(const string& ip,const string& presentState)
{
    return _baseHandle->updateNodeServerStates(ip, presentState);
}

int StoreProxy::updateServerStates(const vector<RouterData>& needUpdateRouterList, TarsCurrentPtr &current)
{
    return _baseHandle->updateServerStates(needUpdateRouterList, current);
}

int StoreProxy::updateServerState(const RouterData &routerDataInfo,TarsCurrentPtr &current)
{
    return _baseHandle->updateServerState(routerDataInfo, current);
}

int StoreProxy::addServers(const vector<RouterData> & routerDataList,TarsCurrentPtr current)
{
    return _baseHandle->addServers(routerDataList, current);
}

int StoreProxy::updateServers(const vector<RouterData>& needUpdateRouterList,TarsCurrentPtr &current)
{
    return _baseHandle->updateServers(needUpdateRouterList, current);     
}

int StoreProxy::keepServerAlive(const vector<RouterData>& needUpdateRouterList,TarsCurrentPtr &current)
{
    return _baseHandle->keepServerAlive(needUpdateRouterList, current); 
}

int StoreProxy::getServers(const RouterDataRequest & reqInfo,TarsCurrentPtr current)
{
    return _baseHandle->getServers(reqInfo, current);
}

int StoreProxy::getAllServers(vector<map<string, string> > &routerDataMap)
{
    return _baseHandle->getAllServers(routerDataMap);
}


int StoreProxy::deleteServers(const vector<RouterDataRequest>& delSrvList,TarsCurrentPtr &current)
{
    return _baseHandle->deleteServers(delSrvList, current);
}

int StoreProxy::addBaseServiceInfo(const string &locatorId, const map<string, string> &mapServantEndpoint)
{
    return _baseHandle->addBaseServiceInfo(locatorId, mapServantEndpoint);
}

int StoreProxy::updateBaseServiceInfo(const string &locatorId, const map<string, string> &mapServantEndpoint)
{
    return _baseHandle->updateBaseServiceInfo(locatorId, mapServantEndpoint);
}
int StoreProxy::addNodeInfo(const NodeServiceInfo &nodeInfo)
{
    return _baseHandle->addNodeInfo(nodeInfo);
}

int StoreProxy::registerNodeExt(const NodeInstanceInfo & nodeInfo)
{
    //如果该节点上已经有SeerAgent，则直接返回，否则新增
    NodeServiceInfo oldNodeInfo;
    if (STOREPROXY->getOneSeerAgent(nodeInfo.nodeName, oldNodeInfo) == 0)
    {
        return 0;
    }

    NodeServiceInfo newNodeInfo = NodeServiceInfo::fromNodeInstanceInfo(nodeInfo);
    TLOGDEBUG(FILE_FUN << "add new tseer agent|" << newNodeInfo.toStringSeer() << endl);
    if (STOREPROXY->addNodeInfo(newNodeInfo))
    {
        TLOGERROR(FILE_FUN << "add new tseer agent node to etcd failed" << endl);
        return -1;
    }

    return 0;
}
int StoreProxy::destroyAgent(const string& name)
{
    TLOGDEBUG(FILE_FUN << "delete tseer agent in node:" << name << endl);
    if (STOREPROXY->delNodeInfo(name, TSEER_AGENT_NET_WORK_ID))
    {
        TLOGERROR(FILE_FUN << "delete tseer agent failed" << endl);
        return -1;
    }
    return 0;
}
int StoreProxy::delNodeInfo(const string &nodeName, const string &networkId)
{
    return _baseHandle->delNodeInfo(nodeName, networkId);
}
int StoreProxy::updateNodeStateBatch(vector<NodeServiceInfo> &nodeList)
{
    return _baseHandle->updateNodeStateBatch(nodeList);
}
int StoreProxy::getAllNodes(vector<NodeServiceInfo> &nodeInfo, const string &networkId)
{
    return _baseHandle->getAllNodes(nodeInfo, networkId);
}

int StoreProxy::getOneSeerAgent(const string &nodeName, NodeServiceInfo &nodeServiceInfo)
{
    return _baseHandle->getOneSeerAgent(nodeName, nodeServiceInfo);
}

int StoreProxy::getAllAliveSeerAgents(std::set<string> &aliveNodes)
{
    return _baseHandle->getAllAliveSeerAgents(aliveNodes);
}
int StoreProxy::getAllTimeOutNodes(vector<NodeServiceInfo> &nodeList, const string &networkId, const unsigned int interval)
{
    return _baseHandle->getAllTimeOutNodes(nodeList, networkId, interval);
}
int StoreProxy::keepAliveExt(const Tseer::NodeInstanceInfo& stNodeInstanceInfo)
{
    if (stNodeInstanceInfo.nodeName.empty())
    {
        TLOGERROR(FILE_FUN << "empty node name" << endl);
        return -1;
    }

    if (STOREPROXY->keepNodeAlive(stNodeInstanceInfo, TSEER_AGENT_NET_WORK_ID))
    {
        TLOGERROR(FILE_FUN << "set alive to etcd failed" << endl);
        return -1;
    }

    return 0;
}
int StoreProxy::keepNodeAlive(const NodeInstanceInfo& stNodeInstanceInfo, const string &networkId)
{
    return _baseHandle->keepNodeAlive(stNodeInstanceInfo, networkId);
}
int StoreProxy::keepAlive(const string& name, const LoadInfo& li)
{
    return _baseHandle->keepAlive(name, li);
}

int StoreProxy::addIdcGroupRule(const string &groupName, const vector<string> &allowIps, TarsCurrentPtr current)
{
    return _baseHandle->addIdcGroupRule(groupName, allowIps, current);
}

int StoreProxy::modifyIdcGroupRule(const string &groupId, const vector<string> &allowIps, TarsCurrentPtr current)
{
    return _baseHandle->modifyIdcGroupRule(groupId, allowIps, current);
}

int StoreProxy::delIdcGroupRule(const vector<string> &groupsIds, TarsCurrentPtr current)
{
    return _baseHandle->delIdcGroupRule(groupsIds, current);
}

int StoreProxy::getAllIdcGroupRule(vector<IDCGroupInfo> &idcGroupInfoList)
{
    return _baseHandle->getAllIdcGroupRule(idcGroupInfoList);
}
int StoreProxy::addIdcPriority(int order, const string &priorityName, const vector<string> &groupIdList, TarsCurrentPtr current)
{
    return _baseHandle->addIdcPriority(order, priorityName, groupIdList, current);
}

int StoreProxy::modifyIdcPriority(const string &priorityId, int order, const string &priorityName, const vector<string> &groupIdList, TarsCurrentPtr current)
{
    return _baseHandle->modifyIdcPriority(priorityId, order, priorityName, groupIdList, current);
}

int StoreProxy::delIdcPriority(const vector<string> &priorityIds, TarsCurrentPtr current)
{
    return _baseHandle->delIdcPriority(priorityIds, current);
}

int StoreProxy::getAllIdcPriority(vector<IDCPriority> &idcPriorityInfo)
{
    return _baseHandle->getAllIdcPriority(idcPriorityInfo);
}
int StoreProxy::addServiceGroup(const string& srvGrp, const string& key, const string& userListStr)
{
    return _baseHandle->addServiceGroup(srvGrp, key, userListStr);
}
int StoreProxy::updateServiceGroup(const string& srvGrp, const string& key,const string& userListStr)
{
    return _baseHandle->updateServiceGroup(srvGrp, key,userListStr);
}


int StoreProxy::getAllServiceGroup(vector<ServiceGroupInfo>& srvGrpInfoList)
{
    return _baseHandle->getAllServiceGroup(srvGrpInfoList);
}
int StoreProxy::getServiceGroupKey(const string& srvGrp,string& key)
{
    return _baseHandle->getServiceGroupKey(srvGrp, key);
}

int StoreProxy::get_agent_baseinfo(const AgentBaseInfoReq& req,vector<AgentBaseInfo>& agentBaseInfoList)
{
    return _baseHandle->get_agent_baseinfo(req, agentBaseInfoList);
}

int StoreProxy::update_agent_graystate(const UpdateAgentInfoReq& req)
{
    return _baseHandle->update_agent_graystate(req);
}

int StoreProxy::get_agent_packageinfo(const AgentPackageReq& req,vector<AgentPackageInfo>& agentPackageInfoList)
{
    return _baseHandle->get_agent_packageinfo(req, agentPackageInfoList);
}

int StoreProxy::update_agent_packageinfo(const UpdatePackageInfoReq& req)
{
    return _baseHandle->update_agent_packageinfo(req);
}

int StoreProxy::delete_agent_package(const AgentPackageReq& req)
{
    return _baseHandle->delete_agent_package(req);
}

void StoreProxy::convertStruct(const PushPackageInfo & pushPkInfo, AgentPackageInfo& stAgentInfo)
{
    stAgentInfo.ostype = pushPkInfo.ostype;
    stAgentInfo.package_type = TC_Common::tostr(pushPkInfo.package_type);
    stAgentInfo.package_name = pushPkInfo.packageName;
    stAgentInfo.md5 = pushPkInfo.md5;
    stAgentInfo.version = pushPkInfo.version;
    stAgentInfo.uploadUser = pushPkInfo.user;
    stAgentInfo.uploadTime = TC_Common::tm2str(TNOW,"%Y-%m-%d %H:%M:%S");
}

int StoreProxy::getAgentInfo(const string &packageType, vector <AgentPackageInfo> & agentPackageInfoList)
{
    AgentPackageReq req;
    vector<AgentPackageInfo> agentPackageInfos;
    int ret = _baseHandle->get_agent_packageinfo(req, agentPackageInfos);
    if(ret == 0)
    {
        for (size_t i = 0; i < agentPackageInfos.size(); ++i)
        {
            if (agentPackageInfos[i].package_type == packageType)
            {
                agentPackageInfoList.push_back(agentPackageInfos[i]);
            }
        }
        return ret;
    }
    else
    {
        return ret;
    }
}

int StoreProxy::getGrayAgentInfo(vector < GrayAgentInfo > & grayAgentInfoList)
{

    AgentBaseInfoReq req;
    vector<AgentBaseInfo> agentBaseInfos;
    int ret = _baseHandle->get_agent_baseinfo(req, agentBaseInfos);
    if(ret == 0)
    {
        for (size_t i = 0; i < agentBaseInfos.size(); ++i)
        {
            AgentBaseInfo& info = agentBaseInfos[i];
            if (info.grayscale_state == TSEER_PACKAGE_GRAY_TYPE)
            {
                GrayAgentInfo grayInfo;
                grayInfo.ip = info.ip;
                grayInfo.ostype = info.ostype;
                grayInfo.grayscale_version = info.gray_version;
                grayInfo.grayscale_state = info.grayscale_state;
                grayInfo.package_name = info.package_name;
                grayInfo.md5 = info.md5;
                grayAgentInfoList.push_back(grayInfo);
            }
        }
    }
    return ret;
}

int StoreProxy::add_agent_packageinfo(const AgentPackageInfo & stAgentInfo)
{
    return _baseHandle->add_agent_packageinfo(stAgentInfo);
}

