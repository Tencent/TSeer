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

#include <iterator>
#include <algorithm>
#include "util/tc_encoder.h"
#include "StoreCache.h"
#include "RouterData.h"
#include "StoreProxy.h"
#include "util.h"

TC_ReadersWriterData<ObjectsCache> StoreCache::_objectsCache;

TC_ReadersWriterData<std::map<int, StoreCache::GroupPriorityEntry> > StoreCache::_mapGroupPriority;

/* key-ip, value-组编号 */
TC_ReadersWriterData<map<string, int> > StoreCache::_groupIdMap;
/* key-group_name, value-组编号 */
TC_ReadersWriterData<map<string, int> > StoreCache::_groupNameMap;

TC_ReadersWriterData<StoreCache::SetDivisionCache> StoreCache::_setDivisionCache;

int StoreCache::init(TC_Config * pconf)
{
    return 0;
}

int StoreCache::getGroupId(const string& ip)
{
    map<string, int>&       groupIdMap  = _groupIdMap.getReaderData();
    map<string, int>::iterator  it      = groupIdMap.find(ip);
    if(it != groupIdMap.end())
    {
        return(it->second);
    }

    uint32_t    uip = stringIpToInt(ip);
    string      ipStar  = Ip2StarStr(uip);
    it = groupIdMap.find(ipStar);
    if(it != groupIdMap.end())
    {
        return(it->second);
    }

    return(-1);
}


int StoreCache::getGroupIdByName(const string&groupName)
{
    int iGroupId = -1;
    try
    {
        if(groupName.empty())
        {
            return(iGroupId);
        }

        map<string, int>&       groupNameMap    = _groupNameMap.getReaderData();
        map<string, int>::iterator  it      = groupNameMap.find(groupName);
        if(it != groupNameMap.end())
        {
            TLOGINFO(FILE_FUN << "|" << groupName << "|" << it->second << endl);
            return(it->second);
        }
    }
    catch(exception&ex)
    {
        TLOGERROR(FILE_FUN << "|exception:" << ex.what() << endl);
    }
    catch(...)
    {
        TLOGERROR(FILE_FUN << "|unknown exception" << endl);
    }

    TLOGINFO(FILE_FUN<< "|" << groupName << "|" << endl);
    return(-1);
}


int StoreCache::loadIPPhysicalGroupInfo()
{
    try
    {
        vector<IDCGroupInfo> idcGroupnInfo;
        int queryRet = STOREPROXY->getAllIdcGroupRule(idcGroupnInfo);
        TSEER_LOG("debug")->debug() << FILE_FUN << "query ret:" << queryRet << endl;
        if (queryRet)
        {
            return -1;
        }

        vector<map<string, string> > groupRules;
        for (size_t i = 0; i < idcGroupnInfo.size(); ++i)
        {
            string allowIps;
            for (size_t j = 0; j < idcGroupnInfo[i].allowIp.size(); ++j)
            {
                allowIps += idcGroupnInfo[i].allowIp[j] + "|";
            }

            map<string, string> tmpRecord;
            tmpRecord["group_id"] = idcGroupnInfo[i].groupId;
            tmpRecord["ip_order"] = "allow_denny";
            tmpRecord["allow_ip_rule"] = allowIps;
            tmpRecord["denny_ip_rule"] = "";
            tmpRecord["group_name"] = idcGroupnInfo[i].groupName;
            groupRules.push_back(tmpRecord);
        }
        load2GroupMap(groupRules);

    }
    catch(exception& ex)
    {
        TLOGERROR(FILE_FUN << ex.what() << endl);
    }
    return(-1);
}


void StoreCache::load2GroupMap(const vector<map<string, string> >& serverGroupRule)
{
    map<string, int>&   groupIdMap  = _groupIdMap.getWriterData();
    map<string, int>&   groupNameMap    = _groupNameMap.getWriterData();
    groupIdMap.clear(); /* 规则改变 清除以前缓存 */
    groupNameMap.clear();
    
    vector<map<string, string> >::const_iterator it = serverGroupRule.begin();
    for(; it != serverGroupRule.end(); it++)
    {
        //group id转换成int类型
        int groupId = TC_Common::strto<int>(it->find("group_id")->second);
        vector<string>  vIp = TC_Common::sepstr<string>(it->find("allow_ip_rule")->second, "|");
        for(size_t j = 0; j < vIp.size(); j++)
        {
            groupIdMap[vIp[j]] = groupId;       //<xx.xx.xx.*, group_id>
        }

        groupNameMap[it->find("group_name")->second] = groupId; //<group_name, group_id>
    }
    
    if(!groupIdMap.empty() && !groupNameMap.empty())
    {
        _groupIdMap.swap();
        _groupNameMap.swap();
    }
}


int StoreCache::loadGroupPriority()
{
    std::map<int, GroupPriorityEntry>& mapPriority = _mapGroupPriority.getWriterData();
    mapPriority.clear();
    try
    {
        vector<IDCPriority> idcPriority;
        int ret = STOREPROXY->getAllIdcPriority(idcPriority);
        TSEER_LOG("debug")->debug() << FILE_FUN << "query ret:" << ret << endl;
        if (ret)
        {
            return ret;
        }
        
        std::sort(idcPriority.begin(), idcPriority.end());  //按优先级排序
        for (size_t i = 0; i < idcPriority.size(); ++i)
        {
            mapPriority[i].grouID = idcPriority[i].priorityId;
            mapPriority[i].station = idcPriority[i].station;
            for (size_t j = 0; j < idcPriority[i].groupList.size(); ++j)
            {
                mapPriority[i].setGroupID.insert(TC_Common::strto<int>(idcPriority[i].groupList[j]));
            }
        }
        
        if (!mapPriority.empty())
        {
            _mapGroupPriority.swap();
        }
    }
    catch(exception& ex)
    {
        TLOGERROR(FILE_FUN<< ex.what() << endl);
        return(-1);
    }
    return 0;
}

int StoreCache::loadObjectIdCache(bool recoverProtect, int recoverProtectRate)
{
    ObjectsCache                objectsCache;
    SetDivisionCache            setDivisionCache;
    try
    {
        int64_t iStart = TNOWMS;
        STARTTIME
            
        //IDC分组
        loadIPPhysicalGroupInfo();

        //IDC优先级
        loadGroupPriority();

        vector<map<string, string> > res;
        STOREPROXY->getAllServers(res);
        
        TLOGDEBUG(FILE_FUN << "load all objects from storage,records affected:" << res.size()<< "|cost:" << (TNOWMS - iStart)<<"ms"<< endl);
        iStart  = TNOWMS;
        for(unsigned i = 0; i < res.size(); i++)
        {
            try
            {
                if(res[i]["servant"].empty() && res[i]["endpoint"].empty())
                {
                    TLOGDEBUG(FILE_FUN <<res[i]["application"] << "-" << res[i]["service_name"] << "-" << res[i]["node_name"] << " NULL" << endl);
                    continue;
                }

                if(res[i]["grid_flag"] == "NO_FLOW")
                {
                    //设置成无流量就不要返回去了
                    continue;
                }

                tars::TC_Endpoint ep;
                try
                {
                    ep.parse(res[i]["endpoint"]);
                }
                catch (exception& ex)
                {
                    TLOGERROR(FILE_FUN << ex.what() << endl);
                    continue;
                }
                
                Tseer::EndpointF epf;
                epf.host    = ep.getHost();
                epf.port    = ep.getPort();
                epf.timeout = ep.getTimeout();
                epf.istcp   = ep.isTcp();
                epf.grid    = ep.getGrid();
                
                string ip_group_name = res[i]["ip_group_name"];
                epf.grouprealid = ip_group_name.empty() ? getGroupId(epf.host) : getGroupIdByName(ip_group_name);
                epf.groupworkid = TC_Common::lower(res[i]["enable_group"]) == "y" ? epf.grouprealid : -1;
                if(TC_Common::lower(res[i]["enable_group"]) == "y" && epf.grouprealid == -1)
                {
                    /* 记录查不到分组的组名和ip */
                    TSEER_LOG("group_id")->debug() << FILE_FUN <<ip_group_name << "|" << epf.host << endl;
                }

                epf.setId   = "";
                epf.qos     = ep.getQos();
                epf.bakFlag = TC_Common::strto<int>(res[i]["bak_flag"]);

                int     weightType = TC_Common::strto<int>(res[i]["weight_type"]);
                if(weightType != 0)
                {
                    if(res[i]["weight"] == "")
                    {
                        epf.weight = 100;
                    }
                    else{
                        if(weightType == 1)
                        {
                            int weight = TC_Common::strto<int>(res[i]["weight"]);
                            if(weight < 0)
                            {
                                weight = 100;
                            }

                            if(weight > 100)
                            {
                                weight = 100;
                            }
                            epf.weight = weight;
                        }
                        else{
                            epf.weight = 100;/* 动态默认权重值都是一样的 */
                        }
                    }
                }
                else{
                    epf.weight = -1;
                }
                epf.weightType = weightType;
                
                bool bSet = TC_Common::lower(res[i]["enable_set"]) == "y";
                if(bSet)
                {
                    epf.setId = res[i]["set_name"] + "." + res[i]["set_area"] + "." + res[i]["set_group"];
                }

                TLOGDEBUG(FILE_FUN<< res[i]["servant"] << "|" << epf.host << "|" << epf.grouprealid << "|" << epf.groupworkid << "|" << res[i]["setting_state"] << "|" << res[i]["present_state"] << endl);
                bool            isActive     = true;
                if(res[i]["setting_state"] == "active" && res[i]["present_state"] == "active")
                {
                    /* 存活列表 */
                    objectsCache[res[i]["servant"]].vActiveEndpoints.push_back(epf);
                }
                else{
                    /* 非存活列表 */
                    objectsCache[res[i]["servant"]].vInactiveEndpoints.push_back(epf);
                    isActive         = false;
                }

                if(bSet)
                {
                    if(res[i]["set_name"].empty() || res[i]["set_area"].empty() || res[i]["set_group"].empty()
                       || res[i]["set_name"] == "*" || res[i]["set_area"] == "*")
                    {
                        TLOGERROR(FILE_FUN<< res[i]["servant"] << "." << epf.host << "|set division invalid[" << res[i]["set_name"] << "." << res[i]["set_area"] << "." << res[i]["set_group"] << "]" << endl);
                        bSet = false;
                    }
                }

                /* set划分信息 */
                if(bSet)
                {
                    /* set区域 */
                    string setArea = res[i]["set_name"] + "." + res[i]["set_area"];
                    /* set全称 */
                    string setId = res[i]["set_name"] + "." + res[i]["set_area"] + "." + res[i]["set_group"];

                    SetServerInfo setServerInfo;
                    setServerInfo.isActive   = isActive;
                    setServerInfo.epf   = epf;

                    setServerInfo.setId    = setId;
                    setServerInfo.setArea  = setArea;

                    setDivisionCache[res[i]["servant"]][res[i]["set_name"]].push_back(setServerInfo);
                    TLOGINFO(FILE_FUN<< res[i]["servant"] << "." << epf.host << "|" << setId << "|" << setServerInfo.isActive << endl);
                }
            }
            catch(exception& ex)
            {
                TLOGERROR(FILE_FUN <<res[i]["servant"] << "|" << ex.what() << endl);
            }
        }

        updateObjectsCache(objectsCache);
        updateDivisionCache(setDivisionCache);

        TLOGDEBUG(FILE_FUN << "loaded objects to cache  size:" << objectsCache.size() << endl);
        TLOGDEBUG(FILE_FUN << "loaded set server to cache size:" << setDivisionCache.size() << endl);
        TLOGDEBUG(FILE_FUN << "parse all objects finish"<<USETIME << endl);

        //测试的时候用，输出路由数据
        printCacheData();
    }
    catch(exception& ex)
    {
        TLOGERROR(FILE_FUN << ex.what() << endl);
        return(-1);
    }

    return(0);
}


void StoreCache::printCacheData()
{
    ObjectsCache readerData;
    readerData = _objectsCache.getReaderData();

    TSEER_LOG("debug")->debug() << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>begin" << endl;

    TSEER_LOG("debug")->debug() << "-----------------------------OBJ" << endl;

    ObjectsCache::iterator itr = readerData.begin();
    while (itr != readerData.end())
    {
        vector<Tseer::EndpointF> activeVec   = itr->second.vActiveEndpoints;
        vector<Tseer::EndpointF> inActiveVec = itr->second.vInactiveEndpoints;
        for(uint32_t i = 0; i < activeVec.size(); ++i)
        {
            TSEER_LOG("debug")->debug() << "active|" << itr->first << "|" << activeVec[i].host << ":" << activeVec[i].port <<"|"<<activeVec[i].istcp<< "|" << activeVec[i].groupworkid << endl;
        }
        for(uint32_t i = 0; i < inActiveVec.size(); ++i)
        {
            TSEER_LOG("debug")->debug() << "inactive|" << itr->first << "|" << inActiveVec[i].host << ":" << inActiveVec[i].port <<"|"<<inActiveVec[i].istcp<< "|" << inActiveVec[i].groupworkid << endl;
        }
        ++itr;
    }

    TSEER_LOG("debug")->debug() << "-----------------------------NODE" << endl;

    vector<NodeServiceInfo> nodeInfo;
    STOREPROXY->getAllNodes(nodeInfo, TSEER_AGENT_NET_WORK_ID);
    TSEER_LOG("debug")->debug() << "All seer agents, size:" << nodeInfo.size() << endl;
    for (vector<NodeServiceInfo>::iterator it = nodeInfo.begin(); it != nodeInfo.end(); ++it)
    {
        TSEER_LOG("debug")->debug() << it->nodeNetworkId << "|" << it->nodeName << "|" << it->presentState 
            << "|" << it->registerTime << endl;
    }
    
    std::set<string> aliveNodes;
    STOREPROXY->getAllAliveSeerAgents(aliveNodes);
    TSEER_LOG("debug")->debug() << "All [alive] agents, size:" << aliveNodes.size() << endl;
    for (std::set<string>::iterator it = aliveNodes.begin(); it != aliveNodes.end(); ++it)
    {
        TSEER_LOG("debug")->debug() << (*it) << endl;
    }

    TSEER_LOG("debug")->debug() << "-----------------------------IDC GROUP" << endl;
    vector<IDCGroupInfo> idcGroupInfoList;
    STOREPROXY->getAllIdcGroupRule(idcGroupInfoList);
    TSEER_LOG("debug")->debug() << "All rule, size:" << idcGroupInfoList.size() << endl;
    for (size_t i = 0; i < idcGroupInfoList.size(); ++i)
    {
        TSEER_LOG("debug")->debug() << idcGroupInfoList[i].toString() << endl;
    }

    TSEER_LOG("debug")->debug() << "-----------------------------PRIORITY" << endl;
    vector<IDCPriority> idcPriorityInfo;
    STOREPROXY->getAllIdcPriority(idcPriorityInfo);
    TSEER_LOG("debug")->debug() << "All priority, size:" << idcPriorityInfo.size() << endl;
    for (size_t i = 0; i < idcPriorityInfo.size(); ++i)
    {
        TSEER_LOG("debug")->debug() << idcPriorityInfo[i].toString() << endl;
    }

    TSEER_LOG("debug")->debug() << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<end" << endl << endl;

}

vector<Tseer::EndpointF> StoreCache::findObjectById(const string& id)
{
    ObjectsCache::iterator  it;
    ObjectsCache&       usingCache = _objectsCache.getReaderData();

    if((it = usingCache.find(id)) != usingCache.end())
    {
        return(it->second.vActiveEndpoints);
    }
    else{
        vector<Tseer::EndpointF> activeEp;
        return(activeEp);
    }
}


int StoreCache::findObjectById4All(const string& id, vector<Tseer::EndpointF>& activeEp, vector<Tseer::EndpointF>& inactiveEp)
{
    ObjectsCache::iterator  it;
    ObjectsCache&       usingCache = _objectsCache.getReaderData();

    if((it = usingCache.find(id)) != usingCache.end())
    {
        activeEp    = it->second.vActiveEndpoints;
        inactiveEp  = it->second.vInactiveEndpoints;
    }
    else{
        activeEp.clear();
        inactiveEp.clear();
    }

    return(0);
}


vector<Tseer::EndpointF> StoreCache::getEpsByGroupId(const vector<Tseer::EndpointF>& vecEps, const GroupUseSelect GroupSelect, int iGroupId, ostringstream& os)
{
    os << "|";
    vector<Tseer::EndpointF> vResult;

    for(unsigned i = 0; i < vecEps.size(); i++)
    {
        os << vecEps[i].host << ":" << vecEps[i].port << "(" << vecEps[i].groupworkid << ");";
        if(GroupSelect == ENUM_USE_WORK_GROUPID && vecEps[i].groupworkid == iGroupId)
        {
            vResult.push_back(vecEps[i]);
        }
        if(GroupSelect == ENUM_USE_REAL_GROUPID && vecEps[i].grouprealid == iGroupId)
        {
            vResult.push_back(vecEps[i]);
        }
    }

    return(vResult);
}


vector<Tseer::EndpointF> StoreCache::getEpsByGroupId(const vector<Tseer::EndpointF>& vecEps, const GroupUseSelect GroupSelect, const set<int>& setGroupID, ostringstream& os)
{
    os << "|";
    std::vector<Tseer::EndpointF> vecResult;

    for(std::vector<Tseer::EndpointF>::size_type i = 0; i < vecEps.size(); i++)
    {
        os << vecEps[i].host << ":" << vecEps[i].port << "(" << vecEps[i].groupworkid << ")";
        if(GroupSelect == ENUM_USE_WORK_GROUPID && setGroupID.count(vecEps[i].groupworkid) == 1)
        {
            vecResult.push_back(vecEps[i]);
        }
        if(GroupSelect == ENUM_USE_REAL_GROUPID && setGroupID.count(vecEps[i].grouprealid) == 1)
        {
            vecResult.push_back(vecEps[i]);
        }
    }

    return(vecResult);
}


int StoreCache::findObjectByIdInSameGroup(const string& id, const string& ip,
    vector<Tseer::EndpointF>& activeEp, vector<Tseer::EndpointF>& inactiveEp, ostringstream&os)
{
    activeEp.clear();
    inactiveEp.clear();

    int iClientGroupId = getGroupId(ip);

    os << "|(" << iClientGroupId << ")";

    if(iClientGroupId == -1)
    {
        return(findObjectById4All(id, activeEp, inactiveEp));
    }
 
    ObjectsCache::iterator  it;
    ObjectsCache&       usingCache = _objectsCache.getReaderData();

    if((it = usingCache.find(id)) != usingCache.end())
    {
        activeEp    = getEpsByGroupId(it->second.vActiveEndpoints, ENUM_USE_WORK_GROUPID, iClientGroupId, os);
        inactiveEp  = getEpsByGroupId(it->second.vInactiveEndpoints, ENUM_USE_WORK_GROUPID, iClientGroupId, os);

        if(activeEp.size() == 0)        /* 没有同组的endpoit,匹配未启用分组的服务 */
        {
            activeEp    = getEpsByGroupId(it->second.vActiveEndpoints, ENUM_USE_WORK_GROUPID, -1, os);
            inactiveEp  = getEpsByGroupId(it->second.vInactiveEndpoints, ENUM_USE_WORK_GROUPID, -1, os);
        }
        if(activeEp.size() == 0)        /* 没有同组的endpoit */
        {
            activeEp    = it->second.vActiveEndpoints;
            inactiveEp  = it->second.vInactiveEndpoints;
        }
    }

    return(0);
}


int StoreCache::findObjectByIdInGroupPriority(const std::string&sID, const std::string&sIP, std::vector<Tseer::EndpointF>& vecActive, std::vector<Tseer::EndpointF>& vecInactive,
                                                  std::ostringstream& os)
{
    vecActive.clear();
    vecInactive.clear();
    int iClientGroupID = getGroupId(sIP);

    os << "|(" << iClientGroupID << ")";
    if(iClientGroupID == -1)
    {
        return(findObjectById4All(sID, vecActive, vecInactive));
    }


    ObjectsCache&       usingCache  = _objectsCache.getReaderData();
    ObjectsCache::iterator  itObject    = usingCache.find(sID);
    if(itObject == usingCache.end())
    {
        return(0);
    }


    /* 首先在同组中查找 */
    {
        vecActive   = getEpsByGroupId(itObject->second.vActiveEndpoints, ENUM_USE_WORK_GROUPID, iClientGroupID, os);
        vecInactive = getEpsByGroupId(itObject->second.vInactiveEndpoints, ENUM_USE_WORK_GROUPID, iClientGroupID, os);
        os << "|(In Same Group: " << iClientGroupID << " Active=" << vecActive.size() << " Inactive=" << vecInactive.size() << ")";
    }

    /* 启用分组，但同组中没有找到，在优先级序列中查找 */
    std::map<int, GroupPriorityEntry>& mapPriority = _mapGroupPriority.getReaderData();
    for(std::map<int, GroupPriorityEntry>::iterator it = mapPriority.begin(); it != mapPriority.end() && vecActive.empty(); it++)
    {
        if(it->second.setGroupID.count(iClientGroupID) == 0)
        {
            os << "|(Not In Priority " << it->second.grouID << ")";
            continue;
        }
        vecActive   = getEpsByGroupId(itObject->second.vActiveEndpoints, ENUM_USE_WORK_GROUPID, it->second.setGroupID, os);
        vecInactive = getEpsByGroupId(itObject->second.vInactiveEndpoints, ENUM_USE_WORK_GROUPID, it->second.setGroupID, os);
        os << "|(In Priority: " << it->second.grouID << " Active=" << vecActive.size() << " Inactive=" << vecInactive.size() << ")";
    }

    /* 没有同组的endpoit,匹配未启用分组的服务 */
    if(vecActive.empty())
    {
        vecActive   = getEpsByGroupId(itObject->second.vActiveEndpoints, ENUM_USE_WORK_GROUPID, -1, os);
        vecInactive = getEpsByGroupId(itObject->second.vInactiveEndpoints, ENUM_USE_WORK_GROUPID, -1, os);
        os << "|(In No Grouop: Active=" << vecActive.size() << " Inactive=" << vecInactive.size() << ")";
    }

    /* 在未分组的情况下也没有找到，返回全部地址(此时基本上所有的服务都已挂掉) */
    if(vecActive.empty())
    {
        vecActive   = itObject->second.vActiveEndpoints;
        vecInactive = itObject->second.vInactiveEndpoints;
        os << "|(In All: Active=" << vecActive.size() << " Inactive=" << vecInactive.size() << ")";
    }

    return(0);
}


int StoreCache::findObjectByIdInSameStation(const std::string&sID, const std::string& station, 
                                                std::vector<Tseer::EndpointF>& vecActive, 
                                                std::vector<Tseer::EndpointF>& vecInactive,
                                                std::ostringstream& os)
{
    vecActive.clear();
    vecInactive.clear();

    /* 获得station所有组 */
    std::map<int, GroupPriorityEntry>&      mapPriority = _mapGroupPriority.getReaderData();
    std::map<int, GroupPriorityEntry>::iterator     itGroup     = mapPriority.end();
    for(itGroup = mapPriority.begin(); itGroup != mapPriority.end(); itGroup++)
    {
        if(itGroup->second.station != station)
            continue;

        break;
    }

    if(itGroup == mapPriority.end())
    {
        os << "|not found station:" << station;
        return(-1);
    }

    ObjectsCache&       usingCache  = _objectsCache.getReaderData();
    ObjectsCache::iterator  itObject    = usingCache.find(sID);
    if(itObject == usingCache.end())
    {
        return(0);
    }

    /* 查找对应所有组下的IP地址 */
    vecActive   = getEpsByGroupId(itObject->second.vActiveEndpoints, ENUM_USE_REAL_GROUPID, itGroup->second.setGroupID, os);
    vecInactive = getEpsByGroupId(itObject->second.vInactiveEndpoints, ENUM_USE_REAL_GROUPID, itGroup->second.setGroupID, os);
    return(0);
}


int StoreCache::findObjectByIdInSameSet(const string&sID, const vector<string>&setInfo, 
                                            std::vector<Tseer::EndpointF>& vecActive, std::vector<Tseer::EndpointF>& vecInactive, 
                                            std::ostringstream& os)
{
    string  sSetName    = setInfo[0];
    string  setArea    = setInfo[0] + "." + setInfo[1];
    string  setId      = setInfo[0] + "." + setInfo[1] + "." + setInfo[2];

    SetDivisionCache&       usingSetDivisionCache   = _setDivisionCache.getReaderData();
    SetDivisionCache::iterator  it          = usingSetDivisionCache.find(sID);
    if(it == usingSetDivisionCache.end())
    {
        /* 此情况下没启动set */
        TLOGINFO(FILE_FUN << sID << " haven't start set|" << setId << endl);
        return(-1);
    }

    map<string, vector<SetServerInfo> >::iterator setNameIt = it->second.find(sSetName);
    if(setNameIt == (it->second).end())
    {
        /* 此情况下没启动set */
        TLOGINFO(FILE_FUN << sID << " haven't start set|" << setId << endl);
        return(-1);
    }

    if(setInfo[2] == "*")
    {
        /* 检索通配组和set组中的所有服务 */
        vector<SetServerInfo> vServerInfo = setNameIt->second;
        for(size_t i = 0; i < vServerInfo.size(); i++)
        {
            if(vServerInfo[i].setArea == setArea)
            {
                if(vServerInfo[i].isActive)
                {
                    vecActive.push_back(vServerInfo[i].epf);
                }
                else{
                    vecInactive.push_back(vServerInfo[i].epf);
                }
            }
        }
        return((vecActive.empty() && vecInactive.empty()) ? -2 : 0);
    }
    else{
        /* 1.从指定set组中查找 */
        int ret = findObjectByIdInSameSet(setId, setNameIt->second, vecActive, vecInactive, os);
        if(ret != 0 && setInfo[2] != "*")
        {
            /* 2. 步骤1中没找到，在通配组里找 */
            string sWildSetId = setInfo[0] + "." + setInfo[1] + ".*";
            ret = findObjectByIdInSameSet(sWildSetId, setNameIt->second, vecActive, vecInactive, os);
        }
        return(ret);
    }
}


int StoreCache::findObjectByIdInSameSet(const string&setId, const vector<SetServerInfo>& vSetServerInfo, std::vector<Tseer::EndpointF>& vecActive, std::vector<Tseer::EndpointF>& vecInactive, std::ostringstream& os)
{
    for(size_t i = 0; i < vSetServerInfo.size(); ++i)
    {
        if(vSetServerInfo[i].setId == setId)
        {
            if(vSetServerInfo[i].isActive)
            {
                vecActive.push_back(vSetServerInfo[i].epf);
            }
            else{
                vecInactive.push_back(vSetServerInfo[i].epf);
            }
        }
    }

    int ret = (vecActive.empty() && vecInactive.empty()) ? -2 : 0;
    return(ret);
}

void StoreCache::loadSetInfofromObjectItem(const string& objId, const ObjectItem& objItem, SetDivisionCache& setDivisionCache)
{
    for(size_t i = 0; i < objItem.vActiveEndpoints.size(); i++)
    {
        string setId = objItem.vActiveEndpoints[i].setId;
        if(!setId.empty())
        {
            vector<string> setInfo = TC_Common::sepstr<string>(setId, ".");
            if(setInfo.size() != 3 || (setInfo.size() == 3 && (setInfo[0] == "*" || setInfo[1] == "*")))
            {
                continue;
            }

            SetServerInfo setServerInfo;
            setServerInfo.isActive   = true;
            setServerInfo.epf   = objItem.vActiveEndpoints[i];

            setServerInfo.setId    = setId;
            setServerInfo.setArea  = setInfo[0] + "." + setInfo[1];

            setDivisionCache[objId][setInfo[0]].push_back(setServerInfo);
        }
    }

    for(size_t j = 0; j < objItem.vInactiveEndpoints.size(); j++)
    {
        string setId = objItem.vInactiveEndpoints[j].setId;
        if(!setId.empty())
        {
            vector<string> setInfo = TC_Common::sepstr<string>(setId, ".");
            if(setInfo.size() != 3 || (setInfo.size() == 3 && (setInfo[0] == "*" || setInfo[1] == "*")))
            {
                continue;
            }

            SetServerInfo setServerInfo;
            setServerInfo.isActive   = false;
            setServerInfo.epf   = objItem.vInactiveEndpoints[j];

            setServerInfo.setId    = setId;
            setServerInfo.setArea  = setInfo[0] + "." + setInfo[1];

            setDivisionCache[objId][setInfo[0]].push_back(setServerInfo);
        }
    }
}

void StoreCache::updateObjectsCache(ObjectsCache& objCache, bool updateAll)
{
    /* 全量更新 */
    if(updateAll)
    {
        ObjectsCache::iterator it = objCache.begin();
        _objectsCache.getWriterData() = objCache;
        _objectsCache.swap();
    }
    else{
        /* 用查询数据覆盖一下 */
        _objectsCache.getWriterData() = _objectsCache.getReaderData();
        ObjectsCache& tmpObjCache = _objectsCache.getWriterData();

        ObjectsCache::const_iterator it = objCache.begin();
        for(; it != objCache.end(); it++)
        {
            /* 增量的时候加载的是服务的所有节点，因此这里直接替换 */
            tmpObjCache[it->first] = it->second;
        }
        _objectsCache.swap();
    }
}


void StoreCache::updateDivisionCache(SetDivisionCache& setDivisionCache, bool updateAll)
{
    /* 全量更新 */
    if(updateAll)
    {
        _setDivisionCache.getWriterData() = setDivisionCache;
        _setDivisionCache.swap();
    }
    else{
        _setDivisionCache.getWriterData() = _setDivisionCache.getReaderData();
        SetDivisionCache&           tmpsetCache = _setDivisionCache.getWriterData();
        SetDivisionCache::const_iterator    it      = setDivisionCache.begin();
        for(; it != setDivisionCache.end(); it++)
        {
            /* 有set信息才更新 */
            if(it->second.size() > 0)
            {
                tmpsetCache[it->first] = it->second;
            }
            else if(tmpsetCache.count(it->first))
            {
                /* 这个服务的所有节点都没有启用set，删除缓存中的set信息 */
                tmpsetCache.erase(it->first);
            }
        }
        _setDivisionCache.swap();
    }
}

