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

#include "MysqlHandle.h"
#include "ApiRegImp.h"

using namespace Tseer;

#ifdef USE_MYSQL

int MysqlHandle::init(TC_Config * pconf)
{
    try
    {
        BaseHandle::init(pconf);

        map<string,string> dbConf = pconf->getDomainMap("/tars/application/mysql");
        if(dbConf.empty())
        {
             DB_LOGERROR<<FILE_FUN<<"db config is empty"<<endl;
            return -1;
        }
        TC_DBConf tcDBConf;
        tcDBConf.loadFromMap(dbConf);
        _mysqlReg.init(tcDBConf);
    }
    catch (TC_Config_Exception&ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    catch(TC_Mysql_Exception& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    return 0;
}
int MysqlHandle::keepAlive(const string& name, const LoadInfo& li)
{
    try
    {
        int64_t start  = TNOWMS;
        string condition = "where node_name=" + escapeString(name) + " and node_networkId=" + escapeString("dcnode");
        map<string, pair<TC_Mysql::FT, string> > m;
        m["last_heartbeat"] = make_pair(TC_Mysql::DB_STR, TC_Common::now2str("%Y-%m-%d %H:%M:%S"));
        m["load_avg1"] = make_pair(TC_Mysql::DB_STR, TC_Common::tostr<float>(li.avg1));
        m["load_avg5"] = make_pair(TC_Mysql::DB_STR, TC_Common::tostr<float>(li.avg5));
        m["load_avg15"] = make_pair(TC_Mysql::DB_STR, TC_Common::tostr<float>(li.avg15));
        m["present_state"] = make_pair(TC_Mysql::DB_STR, "active");
        size_t affected = updateR("t_node_info", m, condition);

        DB_LOG<<FILE_FUN<< "affected:" << affected << "|cost:" << (TNOWMS - start) <<"ms"<< endl;
        if(affected == 0)
        {
            return -2;
        }
    }
    catch(TC_Mysql_Exception& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    catch(TarsException& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    return 0;
}
int MysqlHandle::addNodeInfo(const NodeServiceInfo &nodeInfo)
{
    try
    {
        string selectSql =
            "select present_state, node_obj, template_name,last_reg_time "
            "from t_node_info "
            "where node_name=" + escapeString(nodeInfo.nodeName) + " and node_networkId=" + escapeString(nodeInfo.nodeNetworkId);

        TC_Mysql::MysqlData res = queryR(selectSql);
        DB_LOG<<FILE_FUN <<selectSql<<"|select node affected:" << res.size() << endl;
        string templateName;
        string last_reg_time;
        if(res.size() != 0)
        {
            /* 合法性判断，是否存在名字相同已经存活但node obj不同的注册节点 */
            if(res[0]["present_state"] == "active" && res[0]["node_obj"] != nodeInfo.nodeObj)
            {
                DB_LOGERROR<<FILE_FUN<<"registery node :" << nodeInfo.nodeName << " error, other node has registered." << endl;
                //暂时return 0解决线上问题 --2016.11.29
                return 0;
            }
            /* 传递已配置的模板名 */
            templateName = res[0]["template_name"];
            last_reg_time = res[0]["last_reg_time"];
        }

        map<string, pair<TC_Mysql::FT, string> > m;
        m["node_name"] = make_pair(TC_Mysql::DB_STR, nodeInfo.nodeName);
        m["node_obj"] = make_pair(TC_Mysql::DB_STR, nodeInfo.nodeObj);
        m["endpoint_ip"] = make_pair(TC_Mysql::DB_STR, nodeInfo.endpointIp);
        m["endpoint_port"] = make_pair(TC_Mysql::DB_STR, nodeInfo.endpointPort);
        m["data_dir"] = make_pair(TC_Mysql::DB_STR, nodeInfo.dataDir);
        m["load_avg1"] = make_pair(TC_Mysql::DB_INT, TC_Common::tostr<float>(TC_Common::strto<float>(nodeInfo.loadAvg1)));
        m["load_avg5"] = make_pair(TC_Mysql::DB_INT, TC_Common::tostr<float>(TC_Common::strto<float>(nodeInfo.loadAvg5)));
        m["load_avg15"] = make_pair(TC_Mysql::DB_INT, TC_Common::tostr<float>(TC_Common::strto<float>(nodeInfo.loadAvg15)));
        m["last_reg_time"] = make_pair(TC_Mysql::DB_STR, TC_Common::now2str("%Y-%m-%d %H:%M:%S"));
        m["last_heartbeat"] = make_pair(TC_Mysql::DB_STR, TC_Common::now2str("%Y-%m-%d %H:%M:%S"));
        m["setting_state"] = make_pair(TC_Mysql::DB_STR, "active");
        m["present_state"] = make_pair(TC_Mysql::DB_STR, "active");
        m["version"] = make_pair(TC_Mysql::DB_STR, nodeInfo.version);
        m["template_name"] = make_pair(TC_Mysql::DB_STR, nodeInfo.templateName);
        m["cpu_num"] = make_pair(TC_Mysql::DB_STR, nodeInfo.cpuNum);
        m["cpu_hz"] = make_pair(TC_Mysql::DB_STR, nodeInfo.cpuHz);
        m["memory_size"] = make_pair(TC_Mysql::DB_STR, nodeInfo.memorySize);
        m["node_networkId"] = make_pair(TC_Mysql::DB_STR, nodeInfo.nodeNetworkId);
        m["locator"] = make_pair(TC_Mysql::DB_STR, nodeInfo.locator);
        m["ostype"] = make_pair(TC_Mysql::DB_STR, nodeInfo.ostype);
        size_t affected = insertR("t_node_info", m);
        DB_LOG<<FILE_FUN<<"registery node affected:" << affected << endl;
    }
    catch(TC_Mysql_Exception& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    catch(TarsException& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    return 0;
}
int MysqlHandle::delNodeInfo(const string &nodeName, const string &networkId)
{
    try
    {
        string condition = " where node_name=" + escapeString(nodeName) + " and node_networkId=" + escapeString(networkId);
        size_t affected = deleteR("t_node_info", condition);
        DB_LOG<<FILE_FUN  <<condition<<"|affected:" << affected << endl;
    }
    catch(TC_Mysql_Exception& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    catch(TarsException& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    return 0;
}
int MysqlHandle::keepNodeAlive(const NodeInstanceInfo& stNodeInstanceInfo, const string &networkId)
{
    try
    {
        map<string, pair<TC_Mysql::FT, string> > m;
        m["last_heartbeat"] = make_pair(TC_Mysql::DB_STR, TC_Common::now2str("%Y-%m-%d %H:%M:%S"));
        m["present_state"] = make_pair(TC_Mysql::DB_STR, "active");
        m["version"] = make_pair(TC_Mysql::DB_STR, stNodeInstanceInfo.version);
        string condition = " where node_name=" + escapeString(stNodeInstanceInfo.nodeName) + " and node_networkId=" + escapeString("router_agent");
        size_t affected = updateR("t_node_info", m, condition);
        DB_LOG<<FILE_FUN  << "affected:" << affected << endl;
        if(affected == 0)
        {
            return -2;
        }
    }
    catch(TC_Mysql_Exception& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    catch(TarsException& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    return 0;
}
int MysqlHandle::updateNodeStateBatch(vector<NodeServiceInfo> &nodeList)
{
    try
    {
        for(size_t i = 0; i < nodeList.size(); i++)
        {
            map<string, pair<TC_Mysql::FT, string> > m;
            m["present_state"] = make_pair(TC_Mysql::DB_STR, "inactive");
            string condition = " where node_name=" + escapeString(nodeList[i].nodeName);
            size_t affected = updateR("t_node_info", m, condition);
            DB_LOG<<FILE_FUN  << "affected:" << affected << endl;
        }
    }
    catch(TC_Mysql_Exception& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    catch(TarsException& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    return 0;
}
int MysqlHandle::getAllNodes(vector<NodeServiceInfo> &nodeInfoArr, const string &networkId)
{
    try
    {
        string condition = " where node_networkId=" + escapeString(networkId);
        string selectSql = "select node_name,present_state,setting_state,last_heartbeat from t_node_info " + condition;
        TC_Mysql::MysqlData res = queryR(selectSql);
        DB_LOG<<FILE_FUN  << "affected:" << res.size() << endl;
        for(size_t i = 0; i < res.size(); i++)
        {
            NodeServiceInfo nodeInfo;

            nodeInfo.nodeName = res[i]["node_name"];
            nodeInfo.presentState = res[i]["present_state"];
            nodeInfo.settingState = res[i]["setting_state"];

            nodeInfoArr.push_back(nodeInfo);
        }
    }
    catch(TC_Mysql_Exception& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    catch(TarsException& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    return 0;
}

int MysqlHandle::getAllTimeOutNodes(vector<NodeServiceInfo> &nodeList, const string &networkId, const unsigned int interval)
{
    try
    {
        string selectSql = "select node_name from t_node_info ";
        string condition = " where  node_networkId = " + escapeString(networkId) + 
                           " and setting_state = 'active' and last_heartbeat < date_sub(now(), INTERVAL " + 
                           TC_Common::tostr(interval) + " SECOND)";
        selectSql += condition;
        TC_Mysql::MysqlData res = queryR(selectSql);
        for (size_t i = 0; i < res.size(); i++)
        {
            NodeServiceInfo nodeInfo;
            nodeInfo.nodeName = res[i]["node_name"];
            nodeList.push_back(nodeInfo);
        }
    }
    catch(TC_Mysql_Exception& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    catch(TarsException& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    return 0;
}

int MysqlHandle::getOneSeerAgent(const string &nodeName, NodeServiceInfo &nodeServiceInfo)
{
    try
    {
        string condition = " where node_name=" + escapeString(nodeName) + " and node_networkId=" + escapeString("router_agent");
        string selectSql = "select * from t_node_info " + condition;
        TC_Mysql::MysqlData res = queryR(selectSql);
        DB_LOG<<FILE_FUN  << "affected:" << res.size() << endl;
        if(res.size() == 0)
        {
            return -2;
        }
        nodeServiceInfo.nodeObj = res[0]["node_obj"];
        nodeServiceInfo.presentState = res[0]["present_state"];
    }
    catch(TC_Mysql_Exception& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    catch(TarsException& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    return 0;
}
int MysqlHandle::getAllAliveSeerAgents(std::set<string> &aliveNodes)
{
    try
    {
        string condition = " where present_state=" + escapeString("active");
        string selectSql = "select node_name from t_node_info " + condition;
        TC_Mysql::MysqlData res = queryR(selectSql);
        DB_LOG<<FILE_FUN  << "affected:" << res.size() << endl;
        for(size_t i = 0; i < aliveNodes.size(); i++)
        {
            aliveNodes.insert(res[i]["node_name"]);
        }
    }
    catch(TC_Mysql_Exception& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    catch(TarsException& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    return 0;
}

int MysqlHandle::updateServerState(const RouterData &routerData,tars::TarsCurrentPtr& current)
{
    try
    {
        string condition = string(" where") +
                           " moduletype=" + escapeString(routerData.moduletype) +
                           " and application=" + escapeString(routerData.application) +
                           " and service_name=" + escapeString(routerData.service_name) +
                           " and node_name=" + escapeString(routerData.node_name) +
                           " and container_name=" + escapeString(routerData.container_name);
        map<string, pair<TC_Mysql::FT, string> > m;
        m["present_state"] = make_pair(TC_Mysql::DB_STR, routerData.present_state);
        size_t affected = updateR("t_server_conf", m, condition);
        DB_LOG<<FILE_FUN  << "affected:" << affected << endl;
    }
    catch(TC_Mysql_Exception& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    catch(TarsException& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    return 0;
}
int MysqlHandle::updateServerStates(const vector<RouterData>& needUpdateRouterList,tars::TarsCurrentPtr &current)
{
    for(size_t i = 0; i < needUpdateRouterList.size(); i++)
    {
        int ret = updateServerState(needUpdateRouterList[i], current);
        if(ret == -1)
        {
            DB_LOGERROR<<FILE_FUN <<needUpdateRouterList[i].moduletype<<"."<<needUpdateRouterList[i].application<<"."<<needUpdateRouterList[i].service_name
                      <<"."<<needUpdateRouterList[i].node_name<<"_"<<needUpdateRouterList[i].container_name<<"|update failure"<<endl;
            continue;
        }
    }
    return 0;
}
int MysqlHandle::keepServerAlive(const vector<RouterData>& needUpdateRouterList,tars::TarsCurrentPtr &current)
{
    try {
        map<string, pair<TC_Mysql::FT, string> > m;
        for(size_t i = 0; i < needUpdateRouterList.size(); i++)
        {
            RouterData routerData = needUpdateRouterList[i];

            string condition = string(" where") +
                               " moduletype=" + escapeString(routerData.moduletype) +
                               " and application=" + escapeString(routerData.application) +
                               " and service_name=" + escapeString(routerData.service_name) +
                               " and node_name=" + escapeString(routerData.node_name) +
                               " and container_name=" + escapeString(routerData.container_name) +
                               " and enable_heartbeat<>" + escapeString("N");
            m["present_state"] = make_pair(TC_Mysql::DB_STR, "active");
            size_t affected = updateR("t_server_conf", m, condition);
            DB_LOG<<FILE_FUN  << "affected:" << affected << endl;
        }
    }
    catch(TC_Mysql_Exception& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    catch(TarsException& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    return 0;
}
int MysqlHandle::updateNodeServerStates(const string& ip,const string& presentState)
{
    try
    {
        string condition = string(" where") +
                           " node_name=" + escapeString(ip);
        map<string, pair<TC_Mysql::FT, string> > m;
        m["present_state"] = make_pair(TC_Mysql::DB_STR, presentState);
        size_t affected = updateR("t_server_conf", m, condition);
        DB_LOG<<FILE_FUN  << "affected:" << affected << endl;
    }
    catch(TC_Mysql_Exception& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    catch(TarsException& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    return 0;
}

int MysqlHandle::addServers(const vector<RouterData> & routerDataList,tars::TarsCurrentPtr current)
{
    try
    {
        map<string, pair<TC_Mysql::FT, string> > m;
        for(size_t i = 0; i < routerDataList.size(); i++)
        {
            const RouterData &routerData = routerDataList[i];
            string servantInfo;
            for(map<string,string>::const_iterator it = routerData.servant.begin(); it != routerData.servant.end(); it++)
            {
                if(servantInfo.size() > 0)
                {
                    servantInfo += ":";
                }
                servantInfo += it->first;
                servantInfo += "@";
                servantInfo += it->second;
            }
            m["servant"] = make_pair(TC_Mysql::DB_STR, servantInfo);
            m["moduletype"] = make_pair(TC_Mysql::DB_STR, routerData.moduletype);
            m["application"] = make_pair(TC_Mysql::DB_STR, routerData.application);
            m["service_name"] = make_pair(TC_Mysql::DB_STR, routerData.service_name);
            m["node_name"] = make_pair(TC_Mysql::DB_STR, routerData.node_name);
            m["container_name"] = make_pair(TC_Mysql::DB_STR, routerData.container_name);
            m["bak_flag"] = make_pair(TC_Mysql::DB_STR, routerData.bak_flag);
            m["setting_state"] = make_pair(TC_Mysql::DB_STR, routerData.setting_state);
            m["present_state"] = make_pair(TC_Mysql::DB_STR, routerData.present_state);
            m["process_id"] = make_pair(TC_Mysql::DB_STR, routerData.process_id);
            m["tars_version"] = make_pair(TC_Mysql::DB_STR, TARS_VERSION);
            m["enable_group"] = make_pair(TC_Mysql::DB_STR, routerData.enable_group);
            m["enable_set"] = make_pair(TC_Mysql::DB_STR, routerData.enable_set);
            m["set_name"] = make_pair(TC_Mysql::DB_STR, routerData.set_name);
            m["set_area"] = make_pair(TC_Mysql::DB_STR, routerData.set_area);
            m["set_group"] = make_pair(TC_Mysql::DB_STR, routerData.set_group);
            m["ip_group_name"] = make_pair(TC_Mysql::DB_STR, routerData.ip_group_name);

            m["node_port"] = make_pair(TC_Mysql::DB_STR, routerData.node_port);
            m["container_port"] = make_pair(TC_Mysql::DB_STR, routerData.container_port);
            m["weight"] = make_pair(TC_Mysql::DB_STR, routerData.weight);
            m["grid_flag"] = make_pair(TC_Mysql::DB_STR, routerData.grid_flag);
            m["weighttype"] = make_pair(TC_Mysql::DB_STR, routerData.weighttype);
            m["enable_heartbeat"] = make_pair(TC_Mysql::DB_STR, routerData.enable_heartbeat);

            size_t affected = insertR("t_server_conf", m);
            DB_LOG<<FILE_FUN << "affected rows:" << affected << endl;
        }
        string errMsg = "addServers success!";
        ApiRegImp::doResponse(current, TSEER_SERVERSUCCESS, errMsg);
    }
    catch(TC_Mysql_Exception& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    catch(TarsException& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    return 0;
}
int MysqlHandle::updateServers(const vector<RouterData>& needUpdateRouterList,tars::TarsCurrentPtr &current)
{
    try
    {
        map<string, pair<TC_Mysql::FT, string> > m;
        for(size_t i = 0; i < needUpdateRouterList.size(); i++)
        {
            RouterData routerData = needUpdateRouterList[i];
            string condition = "where container_name=" + escapeString(routerData.container_name);
                     condition + " and moduletype=" + escapeString(routerData.moduletype);  
                     condition + " and application=" + escapeString(routerData.application);  
                     condition + " and service_name=" + escapeString(routerData.service_name);  
            string selectSql = "select * from t_server_conf " + condition;
            TC_Mysql::MysqlData res = queryR(selectSql);
            if(res.size() == 0)
            {
                DB_LOGERROR<<FILE_FUN << current->getIp() << "|has no such data|" << toStr(routerData) << endl;
                return -2;
            }
            
            string servantInfo;
            for(map<string,string>::const_iterator it = routerData.servant.begin(); it != routerData.servant.end(); it++)
            {
                if(servantInfo.size() > 0)
                {
                    servantInfo += ":";
                }
                servantInfo += it->first;
                servantInfo += "@";
                servantInfo += it->second;
            }
            
            if(servantInfo != res[0]["servant"])
            {
                DB_LOGERROR<<FILE_FUN << current->getIp() << "|has no such portname|" << servantInfo << endl;
                return -2;
            }
            
            EMPTYSETVALUE(routerData.enable_group, res[0]["enable_group"]);
            if(TC_Common::lower(routerData.enable_group) == "n")
            {
                routerData.ip_group_name = "";
            }
            else
            {
                EMPTYSETVALUE(routerData.ip_group_name, res[0]["ip_group_name"]);
            }
            
            EMPTYSETVALUE(routerData.enable_set, res[0]["enable_set"]);
            if(TC_Common::lower(routerData.enable_set) == "n")
            {
                routerData.set_name = routerData.set_area = routerData.set_group="";
            }
            else
            {
                EMPTYSETVALUE(routerData.set_name, res[0]["set_name"]);
                EMPTYSETVALUE(routerData.set_area, res[0]["set_area"]);
                EMPTYSETVALUE(routerData.set_group, res[0]["set_group"]);
            }
            EMPTYSETVALUE(routerData.setting_state, res[0]["setting_state"]);
            EMPTYSETVALUE(routerData.enable_heartbeat, res[0]["enable_heartbeat"]);

            if (TC_Common::lower(routerData.enable_heartbeat) == "n")
            {
                routerData.present_state = "active";
            }
            else
            {
                EMPTYSETVALUE(routerData.present_state, res[0]["present_state"]);
            }
            
            EMPTYSETVALUE(routerData.grid_flag, res[0]["grid_flag"]);
            EMPTYSETVALUE(routerData.weight, res[0]["weight"]);
            EMPTYSETVALUE(routerData.weighttype, res[0]["weighttype"]);

            m["bak_flag"] = make_pair(TC_Mysql::DB_STR, routerData.bak_flag);
            m["setting_state"] = make_pair(TC_Mysql::DB_STR, routerData.setting_state);
            m["present_state"] = make_pair(TC_Mysql::DB_STR, routerData.present_state);
            m["process_id"] = make_pair(TC_Mysql::DB_STR, routerData.process_id);
            m["tars_version"] = make_pair(TC_Mysql::DB_STR, TARS_VERSION);
            m["ip_group_name"] = make_pair(TC_Mysql::DB_STR, routerData.ip_group_name);
            m["enable_group"] = make_pair(TC_Mysql::DB_STR, routerData.enable_group);
            m["enable_set"] = make_pair(TC_Mysql::DB_STR, routerData.enable_set);
            m["set_name"] = make_pair(TC_Mysql::DB_STR, routerData.set_name);
            m["set_area"] = make_pair(TC_Mysql::DB_STR, routerData.set_area);
            m["set_group"] = make_pair(TC_Mysql::DB_STR, routerData.set_group);
            
            m["node_port"] = make_pair(TC_Mysql::DB_STR, routerData.node_port);
            m["container_port"] = make_pair(TC_Mysql::DB_STR, routerData.container_port);
            m["weight"] = make_pair(TC_Mysql::DB_STR, routerData.weight);
            m["grid_flag"] = make_pair(TC_Mysql::DB_STR, routerData.grid_flag);
            m["weighttype"] = make_pair(TC_Mysql::DB_STR, routerData.weighttype);
            m["enable_heartbeat"] = make_pair(TC_Mysql::DB_STR, routerData.enable_heartbeat);

            size_t affected = updateR("t_server_conf", m, condition);
            DB_LOG<<FILE_FUN << "affected rows:" << affected << endl;
        }
        string errMsg = "updateServers success!";
        ApiRegImp::doResponse(current, TSEER_SERVERSUCCESS, errMsg);
    }
    catch(TC_Mysql_Exception& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    catch(TarsException& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    return 0;
}
int MysqlHandle::deleteServers(const vector<RouterDataRequest>& delSrvList,tars::TarsCurrentPtr &current)
{
    try
    {
        map<string, pair<TC_Mysql::FT, string> > m;
        for(size_t i = 0; i < delSrvList.size(); i++)
        {
            const RouterDataRequest &reqInfo = delSrvList[i];
            string condition = "where moduletype=" + escapeString(reqInfo.moduletype) + "and application=" + escapeString(reqInfo.application) +
                               "and service_name=" + escapeString(reqInfo.service_name);
            if(!reqInfo.node_name.empty())
            {
                condition += "and node_name=" + escapeString(reqInfo.node_name);
                if(!reqInfo.container_name.empty())
                {
                    condition += "and container_name=" + escapeString(reqInfo.container_name);
                }
            }
            size_t affected = deleteR("t_server_conf", condition);
            DB_LOG<<FILE_FUN << "affected rows:" << affected << endl;
        }
        string errMsg = "deleteServers success!";
        ApiRegImp::doResponse(current, TSEER_SERVERSUCCESS, errMsg);
    }
    catch(TC_Mysql_Exception& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    catch(TarsException& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    return 0;
}
int MysqlHandle::getServers(const RouterDataRequest & reqInfo,tars::TarsCurrentPtr current)
{
    try
    {
        if(reqInfo.moduletype.empty())
        {
            DB_LOGERROR<<FILE_FUN << toStr(reqInfo) << " | param invalid" << endl;
            return TSEER_REGISTRY_PARAM_ERROR;
        }
        string condition = "where moduletype=" + escapeString(reqInfo.moduletype);
        if(!reqInfo.application.empty())
        {
            condition += "and application=" + escapeString(reqInfo.application);
        }
        if(!reqInfo.service_name.empty())
        {
            condition += "and service_name=" + escapeString(reqInfo.service_name);
        }
        if(!reqInfo.node_name.empty())
        {
            condition += "and node_name=" + escapeString(reqInfo.node_name);
        }
        if(!reqInfo.container_name.empty())
        {
            condition += "and container_name=" + escapeString(reqInfo.container_name);
        }
        string selectSql ="select * from t_server_conf " + condition;
        TC_Mysql::MysqlData res = queryR(selectSql);
        vector<RouterData> routerDataArr;
        for(size_t i = 0; i < res.size(); i++)
        {
            RouterData routerData;

            routerData.moduletype = res[i]["moduletype"];
            routerData.application = res[i]["application"];
            routerData.service_name = res[i]["service_name"];
            routerData.node_name = res[i]["node_name"];
            routerData.container_name = res[i]["container_name"];
            routerData.bak_flag = res[i]["bak_flag"];
            routerData.setting_state = res[i]["setting_state"];
            routerData.present_state = res[i]["present_state"];
            routerData.process_id = res[i]["process_id"];
            routerData.enable_group = res[i]["enable_group"];
            routerData.enable_set = res[i]["enable_set"];
            routerData.set_name = res[i]["set_name"];
            routerData.set_area = res[i]["set_area"];
            routerData.set_group = res[i]["set_group"];
            routerData.ip_group_name = res[i]["ip_group_name"];

            routerData.node_port = res[i]["node_port"];
            routerData.container_port = res[i]["container_port"];
            routerData.weight = res[i]["weight"];
            routerData.grid_flag = res[i]["grid_flag"];
            routerData.weighttype = res[i]["weighttype"];
            routerData.enable_heartbeat = res[i]["enable_heartbeat"];

            vector<string> objArr = TC_Common::sepstr<string>(res[i]["servant"], ":");
            for(size_t j = 0; j < objArr.size(); j++)
            {
                vector<string> endpointArr = TC_Common::sepstr<string>(objArr[j], "@");
                if(endpointArr.size() >= 2)
                {
                    routerData.servant[endpointArr[0]] = endpointArr[1];
                }
            }

            routerDataArr.push_back(routerData);
        }
        DB_LOG<<FILE_FUN << "affected rows:" << res.size() << endl;
        string errMsg = "getServers success!";
        ApiRegImp::doGetRouteResponse(current, TSEER_SERVERSUCCESS, routerDataArr, errMsg);
    }
    catch(TC_Mysql_Exception& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    catch(TarsException& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    return 0;
}
int MysqlHandle::addBaseServiceInfo(const string &locatorId, const map<string, string> &mapServantEndpoint)
{
    try
    {
        string sql = "replace into t_registry_info (locator_id, servant, endpoint, last_heartbeat, present_state, tars_version) "
                      "values ";
        map<string, pair<TC_Mysql::FT, string> > m;
        for (map<string, string>::const_iterator iter = mapServantEndpoint.begin(); iter != mapServantEndpoint.end(); iter++)
        {
            sql += (iter == mapServantEndpoint.begin() ? string("") : string(", ")) +
                    "('" + locatorId + "', " +
                    "'" + iter->first + "', '" + iter->second + "', now(), 'active', " +
                    escapeString(TARS_VERSION) + ")";
        }
        executeR(sql);
    }
    catch(TC_Mysql_Exception& ex)
    {
        DB_LOGERROR<<FILE_FUN << locatorId << " exception: " << ex.what() << endl;
        return -1;
    }
    catch(TarsException& ex)
    {
        DB_LOGERROR<<FILE_FUN << locatorId << " exception: " << ex.what() << endl;
        return -1;
    }
    return 0;
}

int MysqlHandle::checkRegistryTimeout()
{
    try
    {
        string sql =
            "update t_registry_info "
            "set present_state='inactive' "
            "where last_heartbeat < date_sub(now(), INTERVAL " + _registryTimeout + " SECOND)";

        size_t affected = executeR(sql);
        DB_LOG<<FILE_FUN << sql <<" affected:"<<affected<<endl;
        return affected;
    }
    catch(TC_Mysql_Exception & ex)
    {
        DB_LOGERROR<<FILE_FUN<<" exception: "<<ex.what()<<endl;
        return -1;
    }
}

int MysqlHandle::updateBaseServiceInfo(const string &locatorId, const map<string, string> &mapServantEndpoint)
{
    try
    {
        string sql = "replace into t_registry_info (locator_id, servant, endpoint, last_heartbeat, present_state, tars_version) "
                      "values ";
        map<string, pair<TC_Mysql::FT, string> > m;
        for (map<string, string>::const_iterator iter = mapServantEndpoint.begin(); iter != mapServantEndpoint.end(); iter++)
        {
            sql += (iter == mapServantEndpoint.begin() ? string("") : string(", ")) +
                    "('" + locatorId + "', " +
                    "'" + iter->first + "', '" + iter->second + "', now(), 'active', " +
                    escapeString(TARS_VERSION) + ")";
        }
        size_t affected = executeR(sql);
        DB_LOG<<FILE_FUN <<sql << "|affected:" << affected << endl;

        //顺便检查其它节点的心跳超时
        checkRegistryTimeout();
    }
    catch (TC_Mysql_Exception& ex)
    {
        DB_LOGERROR<<FILE_FUN << " " << locatorId << " exception: " << ex.what() << endl;
        return -1;
    }
    catch (exception& ex)
    {
        DB_LOGERROR<<FILE_FUN << " " << locatorId << " exception: " << ex.what() << endl;
        return -1;
    }

    return 0;
}
int MysqlHandle::addIdcGroupRule(const string &groupName, const vector<string> &allowIps, TarsCurrentPtr current)
{
    long id = -1;
    try
    {
        map<string, pair<TC_Mysql::FT, string> > m;
        m["group_name"] = make_pair(TC_Mysql::DB_STR, groupName);
        
        string ipStr;
        if(allowIps.size() > 0)
        {
            ipStr = allowIps[0];
        }
        for(size_t i = 1; i < allowIps.size(); i++)
        {
            ipStr += ("," + allowIps[i]);
        }
        m["allow_ip"] = make_pair(TC_Mysql::DB_STR, ipStr);
        size_t affected = insertR(id, "t_idc_rule_info", m);

        DB_LOG<<FILE_FUN << "affected rows:" << affected << endl;
    }
    catch(TC_Mysql_Exception& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    catch(TarsException& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    
    string idStr = TC_Common::tostr<long>(id);
    ApiRegImp::doAddIdcResponse(current, 0, idStr);

    return 0;
}
int MysqlHandle::modifyIdcGroupRule(const string &groupId, const vector<string> &allowIps, TarsCurrentPtr current)
{
    try
    {
        string condition = " where id=" + groupId;
        string selectSql = "select * from t_idc_rule_info" + condition;
        TC_Mysql::MysqlData res = queryR(selectSql);
        if(res.size() == 0)
        {
            DB_LOGERROR<<FILE_FUN << "get idc group info for:" << groupId << " failed" << endl;
            return -2;
        }
        string ipStr;
        if(allowIps.size() > 0)
        {
            ipStr = allowIps[0];
        }
        for(size_t i = 1; i < allowIps.size(); i++)
        {
            ipStr += ("," + allowIps[i]);
        }
        map<string, pair<TC_Mysql::FT, string> > m;
        m["allow_ip"] = make_pair(TC_Mysql::DB_STR, ipStr);
        size_t affected = updateR("t_idc_rule_info", m, condition);
        DB_LOG<<FILE_FUN << "affected rows:" << affected << endl;
    }
    catch(TC_Mysql_Exception& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    catch(TarsException& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    string errMsg = "modify idcGroupRule success!";
    ApiRegImp::doResponse(current, 0, errMsg);
    return 0;
}
int MysqlHandle::delIdcGroupRule(const vector<string> &groupsIds, TarsCurrentPtr current)
{
    try
    {
        for(size_t i = 0; i < groupsIds.size(); i++)
        {
            string groupId = groupsIds[i];
            string condition = "where id=" + groupId;
            size_t affected = deleteR("t_idc_rule_info", condition);
            DB_LOG<<FILE_FUN << "affected rows:" << affected << endl;
        }
    }
    catch(TC_Mysql_Exception& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    catch(TarsException& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    string errMsg = "delete idcGroupRule success!";
    ApiRegImp::doResponse(current, 0, errMsg);
    return 0;
}
int MysqlHandle::getAllIdcGroupRule(vector<IDCGroupInfo> &idcGroupInfoList)
{
    try
    {
        string selectSql = "select * from t_idc_rule_info where 1";
        TC_Mysql::MysqlData res = queryR(selectSql);
        for(size_t i = 0; i < res.size(); i++)
        {
            IDCGroupInfo groupInfo;
            groupInfo.groupId = res[i]["id"];
            groupInfo.groupName = res[i]["group_name"];
            groupInfo.allowIp = TC_Common::sepstr<string>(res[i]["allow_ip"], ",");
            idcGroupInfoList.push_back(groupInfo);
        }
        DB_LOG<<FILE_FUN << "affected rows:" << res.size() << endl;
    }
    catch(TC_Mysql_Exception& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    catch(TarsException& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    return 0;
}

int MysqlHandle::addIdcPriority(int order, const string &priorityName, const vector<string> &groupIdList, TarsCurrentPtr current)
{
    long id = -1;
    try
    {
        map<string, pair<TC_Mysql::FT, string> > m;
        m["priority_name"] = make_pair(TC_Mysql::DB_STR, priorityName);
        string groupIdStr;
        if(groupIdList.size() > 0)
        {
            groupIdStr = groupIdList[0];
        }
        for(size_t i = 1; i < groupIdList.size(); i++)
        {
            groupIdStr += ("," + groupIdList[i]);
        }
        m["group_list"] = make_pair(TC_Mysql::DB_STR, groupIdStr);
        m["order"] = make_pair(TC_Mysql::DB_STR, TC_Common::tostr<int>(order));

        size_t affected = insertR(id, "t_idc_priority_info", m);
        
        DB_LOG<<FILE_FUN << "affected rows:" << affected << endl;
    }
    catch(TC_Mysql_Exception& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -2;
    }
    catch(TarsException& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    string idStr = TC_Common::tostr<long>(id);
    ApiRegImp::doAddIdcResponse(current, 0, idStr);

    return 0;
}
int MysqlHandle::modifyIdcPriority(const string &priorityId, int order, const string &priorityName, const vector<string> &groupIdList, TarsCurrentPtr current)
{
    try
    {
        string condition = " where id=" + priorityId;
        string selectSql = "select * from t_idc_priority_info" + condition;
        TC_Mysql::MysqlData res = queryR(selectSql);
        if(res.size() == 0)
        {
            DB_LOGERROR<<FILE_FUN << "get idc priority info for:" << priorityId << " failed" << endl;
            return -2;
        }
        string groupIdStr;
        if(groupIdList.size() > 0)
        {
            groupIdStr = groupIdList[0];
        }
        for(size_t i = 1; i < groupIdList.size(); i++)
        {
            groupIdStr += ("," + groupIdList[i]);
        }
        map<string, pair<TC_Mysql::FT, string> > m;
        m["group_list"] = make_pair(TC_Mysql::DB_STR, groupIdStr);
        m["order"] = make_pair(TC_Mysql::DB_STR, TC_Common::tostr<int>(order));
        size_t affected = updateR("t_idc_priority_info", m, condition);
        DB_LOG<<FILE_FUN << "affected rows:" << affected << endl;
    }
    catch(TC_Mysql_Exception& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    catch(TarsException& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    string errMsg = "modify idcPriority success!";
    ApiRegImp::doResponse(current, 0, errMsg);
    return 0;
}
int MysqlHandle::delIdcPriority(const vector<string> &priorityIds, TarsCurrentPtr current)
{
    try
    {
        for(size_t i = 0; i < priorityIds.size(); i++)
        {
            string priorityId = priorityIds[i];
            string condition = "where id=" + priorityId;
            size_t affected = deleteR("t_idc_priority_info", condition);
            DB_LOG<<FILE_FUN << "affected rows:" << affected << endl;
        }
    }
    catch(TC_Mysql_Exception& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    catch(TarsException& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    string errMsg = "delete idcPriority success!";
    ApiRegImp::doResponse(current, 0, errMsg);
    return 0;
}
int MysqlHandle::getAllIdcPriority(vector<IDCPriority> &idcPriorityInfo)
{
    try
    {
        string selectSql = "select * from t_idc_priority_info where 1";
        TC_Mysql::MysqlData res = queryR(selectSql);
        for(size_t i = 0; i < res.size(); i++)
        {
            IDCPriority priorityInfo;
            priorityInfo.priorityId = res[i]["id"];
            priorityInfo.priorityName = res[i]["priority_name"];
            priorityInfo.groupList = TC_Common::sepstr<string>(res[i]["group_list"], ",");
            idcPriorityInfo.push_back(priorityInfo);
        }
        DB_LOG<<FILE_FUN << "affected rows:" << res.size() << endl;
    }
    catch(TC_Mysql_Exception& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    catch(TarsException& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    return 0;
}

int MysqlHandle::addServiceGroup(const string& srvGrp,const string& key,const string& userListStr)
{
    try
    {
        map<string, pair<TC_Mysql::FT, string> > m;
        m["service_group"] = make_pair(TC_Mysql::DB_STR, srvGrp);
        m["srvkey"] = make_pair(TC_Mysql::DB_STR, key);
        m["userlist"] = make_pair(TC_Mysql::DB_STR, userListStr);
        size_t affected = insertR("t_servicegroup_info", m);

        DB_LOG<<FILE_FUN << "affected rows:" << affected << endl;
    }
    catch(TC_Mysql_Exception& ex)
    {
        DB_LOGERROR<<FILE_FUN << " " << srvGrp << " exception: " << ex.what() << endl;
        return -1;
    }
    catch(TarsException& ex)
    {
        DB_LOGERROR<<FILE_FUN << " " << srvGrp << " exception: " << ex.what() << endl;
        return -1;
    }
    return 0;
}
int MysqlHandle::updateServiceGroup(const string& srvGrp,const string& key,const string& userListStr)
{
    try
    {
        string condition = "where  service_group=" + escapeString(srvGrp);
        TC_Mysql::RECORD_DATA   rd;
        rd["userlist"] = make_pair(TC_Mysql::DB_STR, userListStr);
        size_t affected = updateR("t_servicegroup_info", rd, condition);

        DB_LOG<<FILE_FUN << " affected:" << affected << endl;
    }
    catch(TC_Mysql_Exception& ex)
    {
        DB_LOGERROR<<FILE_FUN << " " << srvGrp << " exception: " << ex.what() << endl;
        return(2);
    }
    catch(TarsException& ex)
    {
        DB_LOGERROR<<FILE_FUN << " " << srvGrp << " exception: " << ex.what() << endl;
        return(3);
    }
    return(0);
}

int MysqlHandle::getAllServiceGroup(vector<ServiceGroupInfo>& srvGrpInfoList)
{
    try
    {
        string sSelectSql ="select * from t_servicegroup_info";

        TC_Mysql::MysqlData res = queryR(sSelectSql);
        DB_LOG<<FILE_FUN <<sSelectSql << "|affected:" << res.size() << endl;
        if(res.size() >= 0)
        {
            for(size_t i=0;i<res.size();i++)
            {
                ServiceGroupInfo  stInfo;
                stInfo.serviceGrp =  res[i]["service_group"];
                stInfo.key =  res[i]["srvkey"];
                string userListStr = res[i]["userlist"];
                vector<string> vRtxList = TC_Common::sepstr<string>(userListStr,";");
                if(vRtxList.size() >0)
                {
                    stInfo.setUserList.insert(vRtxList.begin(),vRtxList.end());
                }
                srvGrpInfoList.push_back(stInfo);
            }
            DB_LOG<<FILE_FUN <<" srvGrpInfoList.size()="<<srvGrpInfoList.size()<< endl;
        }
        else
        {
            return -1;
        }
    }
    catch(TC_Mysql_Exception& ex)
    {
        DB_LOGERROR<<FILE_FUN << "  exception: " << ex.what() << endl;
        return(2);
    }
    catch(TarsException& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return(3);
    }
    return(0);
}

int MysqlHandle::getServiceGroupKey(const string& srvGrp,string& key)
{
    try
    {
        string sSelectSql ="select srvkey from t_servicegroup_info where service_group=" + escapeString(srvGrp);

        TC_Mysql::MysqlData res = queryR(sSelectSql);
        DB_LOG<<FILE_FUN << sSelectSql << "|affected:" << res.size() << endl;
        if(res.size() > 0)
        {
            key = res[0]["srvkey"];
            DB_LOG<<FILE_FUN << srvGrp << "|srvkey:" << key << endl;
        }
        else
        {
            return -2;
        }
    }
    catch(TC_Mysql_Exception& ex)
    {
        DB_LOGERROR<<FILE_FUN << " " << srvGrp << " exception: " << ex.what() << endl;
        return -1;
    }
    catch(TarsException& ex)
    {
        DB_LOGERROR<<FILE_FUN << " " << srvGrp << " exception: " << ex.what() << endl;
        return -1;
    }
    return 0;
}

int MysqlHandle::getAllServers(vector<map<string, string> > &routerDataMap)
{
    try
    {
        string selectSql ="select * from t_server_conf";
        TC_Mysql::MysqlData res = queryR(selectSql);
        routerDataMap = res.data();
        for(size_t i = 0; i < res.size(); i++)
        {
            vector<string> objArr = TC_Common::sepstr<string>(routerDataMap[i]["servant"], ":");
            for(size_t j = 0; j < objArr.size(); j++)
            {
                vector<string> endpointArr = TC_Common::sepstr<string>(objArr[j], "@");
                if(endpointArr.size() >= 2)
                {
                    routerDataMap[i]["servant"] = endpointArr[0];
                    routerDataMap[i]["endpoint"] = endpointArr[1];
                }
            }
        }
        
        string sqlReg = "select servant, endpoint, enable_group, present_state as setting_state, present_state, tars_version as application, ";
                 sqlReg += "tars_version as service_name, tars_version as node_name,'N' as enable_set,'' as set_name,'' as set_area,'' as set_group,";
                 sqlReg += "'' as ip_group_name, '' as bak_flag, '' as grid_flag, '' as weight,'' as container_name,'' as weight_type,'' as is_gray  from t_registry_info order by endpoint";
        TC_Mysql::MysqlData res2;
        try
        {
            res2 = queryR(sqlReg);
            routerDataMap.insert(routerDataMap.end(),res2.data().begin(),res2.data().end());
        }catch(exception& ex)
        {
            DB_LOGERROR<<FILE_FUN<<ex.what()<<endl;
        }
        
        DB_LOG<<FILE_FUN  << "affected:" << res.size() << endl;
    }
    catch(TC_Mysql_Exception& ex)
    {
        DB_LOGERROR<<FILE_FUN << "  exception: " << ex.what() << endl;
        return -1;
    }
    catch(TarsException& ex)
    {
        DB_LOGERROR<<FILE_FUN << " exception: " << ex.what() << endl;
        return -1;
    }
    return 0;
}


int MysqlHandle::get_agent_baseinfo(const AgentBaseInfoReq& req,vector<AgentBaseInfo>& agentBaseInfos)
{
    __TRY__
        string condition = " where 1=1";
        if(!req.ostype.empty())
        {
            condition += " and ostype=" + escapeString(req.ostype);
        }
        string sqlPatchs = "select ostype,package_type,version from t_server_patchs";
               sqlPatchs += condition;
        TC_Mysql::MysqlData res = queryR(sqlPatchs);
        DB_LOG<<FILE_FUN << sqlPatchs << "|affected:" << res.size() << endl;
        map<string,map<string,string> > packageVersions;
        if(res.size() > 0)
        {
            //获取目前所有操作系统对应版本信息
            for(size_t i = 0; i < res.size();i++)
            {
                string type = res[i]["package_type"];
                if(TSEER_PACKAGE_GRAY_TYPE == type ||
                    TSEER_PACKAGE_FORMAL_TYPE == type)
                {
                    packageVersions[res[i]["ostype"]].insert(make_pair(type,res[i]["version"]));
                }
            }
        }
        condition = " where 1=1";
        if(!req.ostype.empty())
        {
            condition += " and node.ostype=" + escapeString(req.ostype);
        }

        if(!req.ip.empty())
        {
            condition += " and node.node_name=" + escapeString(req.ip);
        }

        if(!req.present_state.empty())
        {
            condition += " and node.present_state=" + escapeString(req.present_state);
        }
        
        string sql  = "select node.node_name,node.ostype,node.present_state,node.locator,node.version,node.last_reg_time,";
               sql += "node.last_heartbeat,publish.grayscale_state,publish.md5,publish.package_name from t_node_info as node left join t_server_publish as publish on node.node_name=publish.node_name";
               sql += condition; 
        TC_Mysql::MysqlData resInfo = queryR(sql);
        DB_LOG<<FILE_FUN << sql << "|affected:" << resInfo.size() << endl;
        for(size_t i = 0; i < resInfo.size();i++)
        {
            AgentBaseInfo info;
            info.ip = resInfo[i]["node_name"];
            info.ostype = resInfo[i]["ostype"];
            info.present_state = resInfo[i]["present_state"];
            info.locator = resInfo[i]["locator"];
            info.version = resInfo[i]["version"];
            info.last_reg_time = resInfo[i]["last_reg_time"];
            info.last_heartbeat_time = resInfo[i]["last_heartbeat"];
            info.grayscale_state = resInfo[i]["grayscale_state"];
            info.md5 = resInfo[i]["md5"];
            info.package_name = resInfo[i]["package_name"];
            info.gray_version = "";
            if(packageVersions.find(info.ostype) != packageVersions.end())
            {
                if(packageVersions.find(info.ostype)->second.find(TSEER_PACKAGE_GRAY_TYPE) != packageVersions.find(info.ostype)->second.end())
                {
                    info.gray_version = packageVersions[info.ostype][TSEER_PACKAGE_GRAY_TYPE];
                }
            }

            info.formal_version = "";
            if(packageVersions.find(info.ostype) != packageVersions.end())
            {
                if(packageVersions.find(info.ostype)->second.find(TSEER_PACKAGE_FORMAL_TYPE) != packageVersions.find(info.ostype)->second.end())
                {
                    info.formal_version = packageVersions[info.ostype][TSEER_PACKAGE_FORMAL_TYPE];
                }
            }

            agentBaseInfos.push_back(info);
        }

        DB_LOG<<FILE_FUN <<"AgentBaseInfos size="<<agentBaseInfos.size()<< endl;
        return 0;
    __CATCH__
    return -1;
}

int MysqlHandle::update_agent_graystate(const UpdateAgentInfoReq& req)
{
    __TRY__
        if(req.grayscale_state == TSEER_PACKAGE_GRAY_TYPE)
        {
            string sql = "select ostype,package_type,version,package_name,md5 from t_server_patchs";
            string condition = " ostype = " +escapeString(req.ostype);
                     condition += " and package_type = " +escapeString(req.grayscale_state); 
            sql += " where " + condition;
            
            TC_Mysql::MysqlData res = queryR(sql);
            DB_LOG<<FILE_FUN << sql << "|affected:" << res.size() << endl;
            string package_name = "";//包名
            string grayscale_version = "";
            string md5 = "";
            if(res.size() > 0)
            {
                grayscale_version = FIRSTRES("version");
                package_name = FIRSTRES("package_name");
                md5 = FIRSTRES("md5");
            }
            else
            {
                DB_LOGERROR<<FILE_FUN << req.ostype <<" has no gray version" << endl;
                return -1;
            }
        
            sql = "replace into t_server_publish (node_name,grayscale_version,grayscale_state,package_name,ostype,md5) values ";
            string value = "(";
                value += escapeString(req.ip);
                value += "," + escapeString(grayscale_version);
                value += "," + escapeString(req.grayscale_state);
                value += "," + escapeString(package_name);
                value += "," + escapeString(req.ostype);
                value += "," + escapeString(md5);
                value +=")";
            sql += value;
            size_t affected = executeR(sql);
            DB_LOG<<FILE_FUN << sql << "|affected:"<<affected<<endl;
        }
        else
        {
            //取消灰度
            string sql = "delete from t_server_publish";
            string condition = "node_name = " +escapeString(req.ip);            
            sql += " where " + condition;
            size_t affected = executeR(sql);
            DB_LOG<<FILE_FUN << sql << "|affected:" << affected<< endl;
        }
        return 0;
    __CATCH__
    return -1;   
}

int MysqlHandle::get_agent_packageinfo(const AgentPackageReq& req,vector<AgentPackageInfo>& agentPackageInfos)
{
    __TRY__
        string condition = " where 1=1";
        if(!req.ostype.empty())
        {
            condition += " and ostype=" + escapeString(req.ostype);
        }

        if(!req.package_name.empty())
        {
            condition += " and package_name=" + escapeString(req.package_name);
        }
        string sql = "select * from t_server_patchs";
               sql += condition;
        TC_Mysql::MysqlData res = queryR(sql);
        DB_LOG<<FILE_FUN << sql << "|affected:" << res.size() << endl;
        if(res.size() > 0)
        {
            for(size_t i = 0; i < res.size();i++)
            {
                AgentPackageInfo info;
                info.ostype = RESEX("ostype");
                info.version = RESEX("version");
                info.package_name =RESEX("package_name");
                info.package_type = RESEX("package_type");
                info.md5 = RESEX("md5");
                agentPackageInfos.push_back(info);
            }
        }
        DB_LOG<<FILE_FUN <<"agentPackageInfos size="<<agentPackageInfos.size()<< endl;
        return 0;
    __CATCH__
    return -1;
}

int MysqlHandle::update_agent_packageinfo(const UpdatePackageInfoReq& req)
{
    __TRY__
    if(req.grayscale_state == TSEER_PACKAGE_GRAY_TYPE || req.grayscale_state == TSEER_PACKAGE_FORMAL_TYPE)
    {
            //保证灰度或者正式版本只有一个
            string     condition = " where  ostype=" + escapeString(req.ostype) + " and package_type=" + TC_Common::tostr(req.grayscale_state);
            TC_Mysql::RECORD_DATA    rd;
            rd["package_type"] = make_pair(TC_Mysql::DB_INT, TC_Common::tostr(TSEER_PACKAGE_NONE_TYPE));
            size_t affected = updateR("t_server_patchs", rd, condition);
            string sql = "update t_server_patchs set package_type=" + TC_Common::tostr(TSEER_PACKAGE_NONE_TYPE) + condition;
            DB_LOG<<FILE_FUN<<sql<< " affected:" << affected<< endl; 
    }
    
        string condition = " where ostype = " +escapeString(req.ostype);
                 condition += " and package_name = " +escapeString(req.package_name); 
        TC_Mysql::RECORD_DATA   rd;
        rd["package_type"] = make_pair(TC_Mysql::DB_INT, req.grayscale_state);
        size_t affected = updateR("t_server_patchs", rd, condition);
        string sql = "update t_server_patchs set package_type=" + TC_Common::tostr(req.grayscale_state) + condition;
        DB_LOG<<FILE_FUN << sql<< "|affected:" << affected << endl;
        return 0;
    __CATCH__
    return -1;
}

int MysqlHandle::delete_agent_package(const AgentPackageReq& req)
{
    __TRY__
        string sql = "delete from t_server_patchs";
        string condition = "ostype = " +escapeString(req.ostype);
                 condition += " and package_name = " +escapeString(req.package_name);
                 
        sql += " where " + condition;
        size_t affected = executeR(sql);
        DB_LOG<<FILE_FUN << sql << "|affected:" << affected << endl;

        sql = "delete from t_server_publish";
        condition = "ostype = " +escapeString(req.ostype);
        condition += " and package_name = " +escapeString(req.package_name); 
        
        sql += " where " + condition;
        affected = executeR(sql);
        DB_LOG<<FILE_FUN << sql << "|affected:" << affected<< endl;

        //TODO同时删除本地文件包?
        return 0;
    __CATCH__
    return -1;
}

int MysqlHandle::add_agent_packageinfo(const AgentPackageInfo & reqInfo)
{
    __TRY__
    if(reqInfo.package_type == TSEER_PACKAGE_GRAY_TYPE || reqInfo.package_type == TSEER_PACKAGE_FORMAL_TYPE)
    {
            //保证灰度或者正式版本只有一个
            string     condition = " where  ostype=" + escapeString(reqInfo.ostype) + " and package_type=" + TC_Common::tostr(reqInfo.package_type);
            TC_Mysql::RECORD_DATA    rd;
            rd["package_type"] = make_pair(TC_Mysql::DB_INT, TC_Common::tostr(TSEER_PACKAGE_NONE_TYPE));
            size_t affected = updateR("t_server_patchs", rd, condition);
            string sql = "update t_server_patchs set package_type=" + TC_Common::tostr(TSEER_PACKAGE_NONE_TYPE) + condition;
            DB_LOG<<FILE_FUN<<sql<< " affected:" << affected<< endl; 
    }
    
    string sql = "replace into t_server_patchs (version,package_name,md5,package_type,ostype,upload_user,upload_time) values ";
    string value = "(";
                value += escapeString(reqInfo.version);
                value += "," + escapeString(reqInfo.package_name);
                value += "," + escapeString(reqInfo.md5);
                value += "," + reqInfo.package_type;
                value += "," + escapeString(reqInfo.ostype);
                value += "," + escapeString(reqInfo.uploadUser);
                value += "," + escapeString(reqInfo.uploadTime);
          value += ")";
    sql += value;

    size_t affected = executeR(sql);
    DB_LOG<<FILE_FUN    << sql << " affected:" << affected<< endl;   
    return 0;
    __CATCH__
    return -1;
}

string MysqlHandle::escapeString(const string& str)
{
    try
    {
        TC_ThreadLock::Lock lock(_mysqlLock);
        return string("'" + _mysqlReg.escapeString(str) + "'");
    }
    catch(...)
    {
        throw;
    }
}

size_t MysqlHandle::updateR(const string &tableName, const map<string, pair<TC_Mysql::FT, string> > &columns, const string &condition)
{
    try
    {
        TC_ThreadLock::Lock lock(_mysqlLock);
        return _mysqlReg.updateRecord(tableName, columns, condition);
    }
    catch(...)
    {
        throw;
    }
}
size_t MysqlHandle::insertR(long &id, const string &tableName, const map<string, pair<TC_Mysql::FT, string> > &columns)
{
    try
    {
        TC_ThreadLock::Lock lock(_mysqlLock);
        size_t affected = _mysqlReg.insertRecord(tableName, columns);
        id = _mysqlReg.lastInsertID();
        return affected;
    }
    catch(...)
    {
        throw;
    }
}
size_t MysqlHandle::insertR(const string &tableName, const map<string, pair<TC_Mysql::FT, string> > &columns)
{
    try
    {
        TC_ThreadLock::Lock lock(_mysqlLock);
        return _mysqlReg.insertRecord(tableName, columns);
    }
    catch(...)
    {
        throw;
    }
}
size_t MysqlHandle::replaceR(const string &tableName, const map<string, pair<TC_Mysql::FT, string> > &columns)
{
    try
    {
        TC_ThreadLock::Lock lock(_mysqlLock);
        return _mysqlReg.replaceRecord(tableName, columns);
    }
    catch(...)
    {
        throw;
    }
}
size_t MysqlHandle::deleteR(const string &tableName, const string &condition)
{
    try
    {
        TC_ThreadLock::Lock lock(_mysqlLock);
        return _mysqlReg.deleteRecord(tableName, condition);
    }
    catch(...)
    {
        throw;
    }
}
size_t MysqlHandle::executeR(const string& sql)
{
    try
    {
        TC_ThreadLock::Lock lock(_mysqlLock);
        _mysqlReg.execute(sql);
        return _mysqlReg.getAffectedRows();
    }
    catch(...)
    {
        throw;
    }
}
TC_Mysql::MysqlData MysqlHandle::queryR(const string& sql)
{
    try
    {
        TC_ThreadLock::Lock lock(_mysqlLock);
        return _mysqlReg.queryRecord(sql);
    }
    catch(...)
    {
        throw;
    }
}
#endif
