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

#ifndef __MYSQL_HANDLE_H__
#define __MYSQL_HANDLE_H__

#include "util.h"

#ifdef USE_MYSQL
#include "util/tc_mysql.h"
#endif

#include "util/tc_config.h"
#include "util/tc_monitor.h"
#include "servant/TarsLogger.h"
#include "BaseHandle.h"

class MysqlHandle : public BaseHandle
{
public:

#ifdef USE_MYSQL

    int init(TC_Config *pconf);
    
    /***********************************Server服务************************************************/

    int addServers(const vector<RouterData> & routerDataList,tars::TarsCurrentPtr current);
    int updateServers(const vector<RouterData>& needUpdateRouterList,tars::TarsCurrentPtr &current);
    int deleteServers(const vector<RouterDataRequest>& delSrvList,tars::TarsCurrentPtr &current);
    int getServers(const RouterDataRequest & reqInfo,tars::TarsCurrentPtr current);
    int getAllServers(vector<map<string, string> > &routerDataMap);

    int updateServerState(const RouterData &routerDataInfo,tars::TarsCurrentPtr& current);
    int updateServerStates(const vector<RouterData>& needUpdateRouterList,tars::TarsCurrentPtr &current);
    int updateNodeServerStates(const string& ip,const string& presentState);

    int keepServerAlive(const vector<RouterData>& needUpdateRouterList,tars::TarsCurrentPtr &current);

    /***********************************基础服务************************************************/

    int addBaseServiceInfo(const string &locatorId, const map<string, string> &mapServantEndpoint);
    int updateBaseServiceInfo(const string &locatorId, const map<string, string> &mapServantEndpoint);
    int checkRegistryTimeout();
    /*********************************Node类服务************************************************/

    int addNodeInfo(const NodeServiceInfo &nodeInfo);
    int delNodeInfo(const string &nodeName, const string &networkId);
    int keepAlive(const string& name, const LoadInfo& li);
    int keepNodeAlive(const NodeInstanceInfo& stNodeInstanceInfo, const string &networkId);

    int updateNodeStateBatch(vector<NodeServiceInfo> &nodeList);

    int getAllNodes(vector<NodeServiceInfo> &nodeInfo, const string &networkId);
    int getAllTimeOutNodes(vector<NodeServiceInfo> &nodeList, const string &networkId, const unsigned int interval);
    int getOneSeerAgent(const string &nodeName, NodeServiceInfo &nodeServiceInfo);
    int getAllAliveSeerAgents(std::set<string> &aliveNodes);

    /*********************************IDC相关操作*************************************************/

    int addIdcGroupRule(const string &groupName, const vector<string> &allowIps, TarsCurrentPtr current);
    int modifyIdcGroupRule(const string &groupId, const vector<string> &allowIps, TarsCurrentPtr current);
    int delIdcGroupRule(const vector<string> &groupsIds, TarsCurrentPtr current);
    int getAllIdcGroupRule(vector<IDCGroupInfo> &idcGroupInfoList);

    int addIdcPriority(int order, const string &priorityName, const vector<string> &groupIdList, TarsCurrentPtr current);
    int modifyIdcPriority(const string &priorityId, int order, const string &priorityName, const vector<string> &groupIdList, TarsCurrentPtr current);
    int delIdcPriority(const vector<string> &priorityIds, TarsCurrentPtr current);
    int getAllIdcPriority(vector<IDCPriority> &idcPriorityInfo);

    /*********************************ServiceGroup相关操作*************************************************/

    int addServiceGroup(const string& srvGrp, const string& key, const string& userListStr);
    int updateServiceGroup(const string& srvGrp,const string& key, const string& userListStr);
    int getAllServiceGroup(vector<ServiceGroupInfo>& srvGrpInfoList);
    int getServiceGroupKey(const string& srvGrp,string& key);


   /*******************************SeerAgent管理********************************/
    int get_agent_baseinfo(const AgentBaseInfoReq& req,vector<AgentBaseInfo>& agentBaseInfos);
    int update_agent_graystate(const UpdateAgentInfoReq& req);
    int get_agent_packageinfo(const AgentPackageReq& req,vector<AgentPackageInfo>& agentPackageInfos);
    int update_agent_packageinfo(const UpdatePackageInfoReq& req);
    int delete_agent_package(const AgentPackageReq& req);
    int add_agent_packageinfo(const AgentPackageInfo & reqInfo);
private:
    /**
      * tc_mysql组件的线程安全封装,如果出现异常会抛出
      */
    string escapeString(const string& str);
    size_t updateR(const string &tableName, const map<string, pair<TC_Mysql::FT, string> > &columns, const string &condition);
    size_t insertR(long &id, const string &tableName, const map<string, pair<TC_Mysql::FT, string> > &columns);
    size_t insertR(const string &tableName, const map<string, pair<TC_Mysql::FT, string> > &columns);
    size_t replaceR(const string &tableName, const map<string, pair<TC_Mysql::FT, string> > &columns);
    size_t deleteR(const string &tableName, const string &condition);
    TC_Mysql::MysqlData queryR(const string& sql);
    size_t executeR(const string& sql);
private:
    /* mysql连接对象 */
    TC_Mysql _mysqlReg;

    /* 数据库组件不是线程安全，需要加锁 */
    TC_ThreadLock _mysqlLock;
#endif

};

//取db查询结果数据
#define RES(i,x)  res[i][(x)]
#define RESEX(s) (res[i][(s)])
#define FIRSTRES(s)  RES(0,(s))


#endif
