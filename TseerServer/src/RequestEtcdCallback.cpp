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

#include "RequestEtcdCallback.h"
#include "EtcdHandle.h"
#include "EtcdDataCache.h"
#include "Registry.h"
#include "util.h"
#include "StoreProxy.h"
#include "ApiRegImp.h"

namespace 
{

//在回复时哪些操作需要被唤醒
bool isNeedAwakeOnResponse(EtcdAction etcdAction)
{
    if (etcdAction == ETCD_UPDATE_BATACH || etcdAction == ETCD_UPDATE_NODE_TIMEOUT
        || etcdAction == ETCD_API_ADD_PORTLIST || etcdAction == ETCD_API_UPDATE_BATACH
        || etcdAction == ETCD_API_SET || etcdAction == ETCD_API_DELETE
        || etcdAction == ETCD_UPDATE_SELF_STATE || etcdAction == ETCD_ADD_SELF
        || etcdAction == ETCD_DEL_IDC_RULE || etcdAction == ETCD_DEL_IDC_PRIORITY
        || etcdAction == ETCD_GET_ALL_SERVICE_GROUP || etcdAction == ETCD_GET_SERVICE_GROUP_KEY
        )
    {
        return true;
    }
    else
    {
        return false;
    }
}

//是否是IDC操作相关的
bool isIdcAction(EtcdAction etcdAction)
{
    if (etcdAction == ETCD_ADD_IDC_RULE || etcdAction == ETCD_MODIFY_IDC_RULE
        || etcdAction == ETCD_DEL_IDC_RULE || etcdAction == ETCD_GET_ONE_IDC_RULE
        || etcdAction == ETCD_GET_ALL_IDC_RULE || etcdAction == ETCD_ADD_IDC_PRIORITY
        || etcdAction == ETCD_MODIFY_IDC_PRIORITY || etcdAction == ETCD_DEL_IDC_PRIORITY
        || etcdAction == ETCD_GET_ALL_IDC_PRIORITY)
    {
        return true;
    }
    else
    {
        return false;
    }
}

}


void RequestEtcdCallback::onResponse(bool close, TC_HttpResponse& HttpResponse)
{
    long finishTime    = TNOWMS;
    long timeUsed  = (finishTime - _etcdReqInfo.startTime);

    if(::isNeedAwakeOnResponse(_etcdReqInfo.etcdAction) &&_etcdReqInfo.notifyPtr)
    {
        _etcdReqInfo.notifyPtr->_atomic.inc();
        if(_etcdReqInfo.notifyPtr->_atomic.get() == _etcdReqInfo.notifyPtr->_run_times)
        {
            TC_ThreadLock::Lock lock(_etcdReqInfo.notifyPtr->_monitor);
            _etcdReqInfo.notifyPtr->_monitor.notify();
        }
    }
    
    int ret = Tseer::TSEER_REGISTRY_UNKNOWN;
    do
    {
        __TRY__
        string responeStr = HttpResponse.getContent();
        APIIMP_LOG<<FILE_FUN<< responeStr;
        rapidjson::Document document;
        rapidjson::ParseResult  parseOk = document.Parse(responeStr.c_str());
        if(!parseOk)
        {
            ETCDPROC_LOGERROR << ETCDFILE_FUN<< "|parse json error|" << responeStr <<"|"<< MSTIMEINSTR(finishTime) << "|use=" << timeUsed << "ms"<< endl;
            ret = Tseer::TSEER_REGISTRY_ETCD_PARSE_JSON_FAIL;
            break;  
        }

        Value::ConstMemberIterator it = document.FindMember("errorCode");
        if (it != document.MemberEnd() && it->value.IsNumber())
        {
            string  errorMsg    = document["message"].GetString();
            int errorCode   =it->value.GetInt();

            ETCDPROC_LOGERROR << ETCDFILE_FUN<< "etcd return error: " << errorMsg << ",errorCode:" << errorCode << endl;
            ret = Tseer::TSEER_REGISTRY_ETCD_RET_ERROR;
            if(errorCode == 100)
            {
                ret = Tseer::TSEER_REGISTRY_ETCD_DATA_NOTFOUND;
            }
            break;
        }

        if(_etcdReqInfo.etcdAction==ETCD_API_GET)
        {
            doGetRouteResponse(document);
            return;
        }
        else if (_etcdReqInfo.etcdAction == ETCD_GET_ALL_SEER_AGENTS || 
                  _etcdReqInfo.etcdAction == ETCD_GET_ONE_SEER_AGENT)
        {
            doGetSeerAgentResponse(document);
            return;
        }
        else if (_etcdReqInfo.etcdAction == ETCD_GET_ALL_IDC_RULE)
        {
            doGetIdcRule(document);
            return;
        }
        else if (_etcdReqInfo.etcdAction == ETCD_GET_ONE_IDC_RULE)
        {
            doGetOneIdcRule(document);
            return;
        }
        else if (_etcdReqInfo.etcdAction == ETCD_GET_ALL_IDC_PRIORITY)
        {
            doGetIdcPriority(document);
            return;
        }
        else if(_etcdReqInfo.etcdAction == ETCD_GET_ALL_SERVICE_GROUP)
        {
            doGetServiceGroup(document);
            return;
        }
        else if(_etcdReqInfo.etcdAction == ETCD_GET_SERVICE_GROUP_KEY)
        {
            doGetServiceGroupKey(document);
            return;
        }
        else if(_etcdReqInfo.etcdAction == ETCD_GET_ALL_AGENT_PACKAGE_INFO)
        {
            doGetAgentPackageInfoResponse(document);
            return;
        }
        else if(_etcdReqInfo.etcdAction == ETCD_GET_ALL_GRAYAGENT_INFO)
        {
            doGetGrayAgentInfoResponse(document);
            return;  
        }
        else
        {
            responseClient(Tseer::TSEER_SERVERSUCCESS,timeUsed);
            return;
        }
        __CATCH__
        ret = Tseer::TSEER_REGISTRY_ETCD_UNKNOWN;
    }while(0);


    responseClient(ret,timeUsed);
}


void RequestEtcdCallback::doGetRouteResponse(const rapidjson::Document& jEtcdRootData)
{
    uint64_t    finishTime = TNOWMS;
    int     timeUsed   = (finishTime - _etcdReqInfo.startTime);
    string errMsg = "";
    
    __TRY__
        
        EtcdDataCache::RouterDataCacheT tmpRouterData;

        /* 
        {
            "action":"get",
            "node":{
                "key":"/routertable",
                "dir":true,
                "nodes":[
                    {
                        "key":"/routertable/moduletype=Test",
                        "dir":true,
                        "modifiedIndex":39915952,
                        "createdIndex":39915952
                    },
                    {
                        "key":"/routertable/moduletype=test",
                        "dir":true,
                        "modifiedIndex":42156989,
                        "createdIndex":42156989
                    },

                    ...
                ],
            }
        }
        */
        do
        {
            rapidjson::Value::ConstMemberIterator itr = jEtcdRootData.FindMember("node");
            if (itr != jEtcdRootData.MemberEnd() && itr->value.IsObject())
            {
                const rapidjson::Value& jRoutertable = itr->value;
                if (ISFAILURE(jRoutertable.HasMember("nodes")))
                {
                    ETCDLOAD_LOGERROR << FILE_FUN << "can not find  root nodes from json" << endl;
                    break;
                }

                const rapidjson::Value &jRouterData = jRoutertable["nodes"];
                if (ISFAILURE(jRouterData.IsArray()))
                {
                    ETCDLOAD_LOGERROR << FILE_FUN << "jRouterData nodes is not array" << endl;
                    break;
                }

                ETCDROUTEDATA->parseEtcdRouteData(jRouterData, tmpRouterData);  //路由信息
            }
        } while (0);


        if(tmpRouterData.empty())
        {
            ETCDPROC_LOGERROR << ETCDFILE_FUN<< "|routedatasize="<<tmpRouterData.size() << endl;
            responseClient(TSEER_REGISTRY_ETCD_DATA_NOTFOUND,timeUsed);
            return;
        }
        

        vector<RouterData> routerDataList;
        EtcdDataCache::RouterDataCacheT::iterator it;
        for(it = tmpRouterData.begin(); it != tmpRouterData.end(); ++it)
        {
            routerDataList.push_back(it->second);
        }

        ETCDPROC_LOG << ETCDFILE_FUN <<"routedatasize="<<routerDataList.size()<< endl;
        if(routerDataList.size() == 1)
        {
            //是查询容器状态的在打印具体信息
            ETCDPROC_LOG << ETCDFILE_FUN<<"|"<<display(routerDataList[0])<<endl;
        }
        
        //reportEtcdStat(TSEER_SERVERSUCCESS,StatReport::STAT_SUCC,timeUsed);
        responseClient(Tseer::TSEER_SERVERSUCCESS,timeUsed,routerDataList);
        return;
    
    __CATCHEXT__
    ETCDPROC_LOGERROR << ETCDFILE_FUN<< "parseJson exception," << errMsg << endl;
    //reportEtcdStat(Tseer::TSEER_REGISTRY_ETCD_PARSE_JSON_FAIL,StatReport::STAT_EXCE,timeUsed);

    responseClient(Tseer::TSEER_REGISTRY_ETCD_PARSE_JSON_FAIL,timeUsed);
}

void RequestEtcdCallback::doGetSeerAgentResponse(const rapidjson::Document& jEtcdRootData)
{
    uint64_t finishTime = TNOWMS;
    int timeUsed = (finishTime - _etcdReqInfo.startTime);
    string errMsg = "";
    
    __TRY__
        /* 
        {
            "action":"get",
            "node":{
                "key":"/node_info/node_networkId=",
                "dir":true,
                "nodes":[
                    {
                        "key":"/node_info/node_networkId=/node_name=",
                        "dir":true,
                        ...
                    },
                    {
                        "key":"/node_info/node_networkId=/node_name=",
                        "dir":true,
                        ...
                    },
                    ...
                ],
            }
        }
        */
        do
        {
            rapidjson::Value::ConstMemberIterator itr = jEtcdRootData.FindMember("node");
            if (itr != jEtcdRootData.MemberEnd() && itr->value.IsObject())
            {
                const rapidjson::Value& jRoutertable = itr->value;
                if (ISFAILURE(jRoutertable.HasMember("nodes")))
                {
                    ETCDLOAD_LOGERROR << FILE_FUN << "can not find  root nodes from json" << endl;
                    break;
                }

                const rapidjson::Value &jRouterData = jRoutertable["nodes"];
                if (ISFAILURE(jRouterData.IsArray()))
                {
                    ETCDLOAD_LOGERROR << FILE_FUN << "jRouterData nodes is not array" << endl;
                    break;
                }

                //数据解析到请求线程放进来的结构中
                ETCDROUTEDATA->parseEtcdNodeData(jRouterData, _etcdReqInfo.nodeServiceInfo);
            }

            responseClient(Tseer::TSEER_SERVERSUCCESS, timeUsed);
            return;

        } while (0);


    __CATCHEXT__
    
    ETCDPROC_LOGERROR << ETCDFILE_FUN<< "parseJson exception," << errMsg << endl;
    responseClient(Tseer::TSEER_REGISTRY_ETCD_PARSE_JSON_FAIL, timeUsed);
}

void RequestEtcdCallback::doGetIdcRule(const rapidjson::Document &jEtcdRootData)
{
    uint64_t finishTime = TNOWMS;
    int timeUsed = (finishTime - _etcdReqInfo.startTime);
    string errMsg = "";
    
    __TRY__
        /* 
        {
            "action":"get",
            "node":{
                "key":"/idc_rule",
                "dir":true,
                "nodes":[
                    {
                        "key":"/idc_rule/group_id=",
                        "value":"group_name|allow_ip|denney_ip|rule_order"
                    },
                    {
                        "key":"/idc_rule/group_id=",
                        "value":"group_name|allow_ip|denney_ip|rule_order"
                    },
                    ...
                ],
            }
        }
        */
        do
        {
            rapidjson::Value::ConstMemberIterator itr = jEtcdRootData.FindMember("node");
            if (itr != jEtcdRootData.MemberEnd() && itr->value.IsObject())
            {
                const rapidjson::Value& jRoutertable = itr->value;
                if (ISFAILURE(jRoutertable.HasMember("nodes")))
                {
                    ETCDLOAD_LOGERROR << FILE_FUN << "can not find  root nodes from json" << endl;
                    break;
                }

                const rapidjson::Value &jRouterData = jRoutertable["nodes"];
                if (ISFAILURE(jRouterData.IsArray()))
                {
                    ETCDLOAD_LOGERROR << FILE_FUN << "jRouterData nodes is not array" << endl;
                    break;
                }

                //解析每个group_id的key和value
                for (rapidjson::SizeType i = 0; i < jRouterData.Size(); ++i)
                {
                    IDCGroupInfo groupInfo;

                    rapidjson::Value::ConstMemberIterator keyIt = jRouterData[i].FindMember("key");
                    if (keyIt == jRouterData[i].MemberEnd())
                    {
                        continue;
                    }
                    string keyStr = keyIt->value.GetString();
                    string groupId = keyStr.substr(keyStr.find_last_of("=") + 1);
                    groupInfo.groupId = groupId;

                    rapidjson::Value::ConstMemberIterator valueIt = jRouterData[i].FindMember("value");
                    if (valueIt == jRouterData[i].MemberEnd())
                    {
                        continue;
                    }
                    const string &value = valueIt->value.GetString();
                    //格式是 group_name|allow_ip|denney_ip|rule_order
                    vector<string> entry = TC_Common::sepstr<string>(value, "|", true);  //withEmpty为真，有些位置是为了兼容而保留的，实际上没有值
                    if (entry.size() < 4)
                    {
                        continue;
                    }
                    groupInfo.groupName = entry[0];
                    groupInfo.ruleOrder = entry[3];

                    vector<string> allowIps = TC_Common::sepstr<string>(entry[1], ",");
                    for (size_t j = 0; j < allowIps.size(); ++j)
                    {
                        groupInfo.allowIp.push_back(allowIps[j]);
                    }

                    vector<string> denneyIps = TC_Common::sepstr<string>(entry[2], ",");
                    for (size_t j = 0; j < denneyIps.size(); ++j)
                    {
                        groupInfo.denneyIp.push_back(denneyIps[j]);
                    }

                    //通过_stReqInfo把结果传给等待的线程
                    _etcdReqInfo.idcGroupInfoList.push_back(groupInfo);
                }
            }

            responseClient(Tseer::TSEER_SERVERSUCCESS, timeUsed);
            return;

        } while (0);


    __CATCHEXT__

    ETCDPROC_LOGERROR << ETCDFILE_FUN << "parseJson exception," << errMsg << endl;
    responseClient(Tseer::TSEER_REGISTRY_ETCD_PARSE_JSON_FAIL, timeUsed);
}

void RequestEtcdCallback::doGetOneIdcRule(const rapidjson::Document &jEtcdRootData)
{
    uint64_t finishTime = TNOWMS;
    int timeUsed = (finishTime - _etcdReqInfo.startTime);
    string errMsg = "";
    
    __TRY__
        /* 
        {
            "action":"get",
            "node":{
                "key": "/idc_rule/group_id=",
                "value":"group_name|allow_ip|denney_ip|rule_order"
            }
        }
        */
        do
        {
            rapidjson::Value::ConstMemberIterator itr = jEtcdRootData.FindMember("node");
            if (itr != jEtcdRootData.MemberEnd() && itr->value.IsObject())
            {
                const rapidjson::Value& jRouterData = itr->value;

                IDCGroupInfo groupInfo;

                rapidjson::Value::ConstMemberIterator keyIt = jRouterData.FindMember("key");
                if (keyIt == jRouterData.MemberEnd())
                {
                    continue;
                }
                string keyStr = keyIt->value.GetString();
                string groupId = keyStr.substr(keyStr.find_last_of("=") + 1);
                groupInfo.groupId = groupId;

                rapidjson::Value::ConstMemberIterator valueIt = jRouterData.FindMember("value");
                if (valueIt == jRouterData.MemberEnd())
                {
                    continue;
                }
                const string &value = valueIt->value.GetString();
                //格式是 group_name|allow_ip|denney_ip|rule_order
                vector<string> entry = TC_Common::sepstr<string>(value, "|", true);  //withEmpty为真，有些位置是为了兼容而保留的，实际上没有值
                if (entry.size() < 4)
                {
                    continue;
                }
                groupInfo.groupName = entry[0];
                groupInfo.ruleOrder = entry[3];

                vector<string> allowIps = TC_Common::sepstr<string>(entry[1], ",");
                for (size_t j = 0; j < allowIps.size(); ++j)
                {
                    groupInfo.allowIp.push_back(allowIps[j]);
                }

                vector<string> denneyIps = TC_Common::sepstr<string>(entry[2], ",");
                for (size_t j = 0; j < denneyIps.size(); ++j)
                {
                    groupInfo.denneyIp.push_back(denneyIps[j]);
                }

                //通过_stReqInfo把结果传给等待的线程
                _etcdReqInfo.idcGroupInfoList.push_back(groupInfo);

            }

            responseClient(Tseer::TSEER_SERVERSUCCESS, timeUsed);
            return;

        } while (0);


    __CATCHEXT__

    ETCDPROC_LOGERROR << ETCDFILE_FUN << "parseJson exception," << errMsg << endl;
    responseClient(Tseer::TSEER_REGISTRY_ETCD_PARSE_JSON_FAIL, timeUsed);
}

void RequestEtcdCallback::doGetIdcPriority(const rapidjson::Document &jEtcdRootData)
{
    uint64_t finishTime = TNOWMS;
    int timeUsed = (finishTime - _etcdReqInfo.startTime);
    string errMsg = "";

    __TRY__
        /* 
        {
            "action":"get",
            "node":{
                "key":"/idc_priority",
                "dir":true,
                "nodes":[
                    {
                        "key":"/idc_rule/priority_id=",
                         "value":"priority_name|order|group_list|station"
                    },
                    {
                        "key":"/idc_rule/priority_id=",
                        "value":"priority_name|order|group_list|station"
                    },
                    ...
                ],
            }
        }
        */
        do
        {
            rapidjson::Value::ConstMemberIterator itr = jEtcdRootData.FindMember("node");
            if (itr != jEtcdRootData.MemberEnd() && itr->value.IsObject())
            {
                const rapidjson::Value& jRoutertable = itr->value;
                if (ISFAILURE(jRoutertable.HasMember("nodes")))
                {
                    ETCDLOAD_LOGERROR << FILE_FUN << "can not find  root nodes from json" << endl;
                    break;
                }

                const rapidjson::Value &jRouterData = jRoutertable["nodes"];
                if (ISFAILURE(jRouterData.IsArray()))
                {
                    ETCDLOAD_LOGERROR << FILE_FUN << "jRouterData nodes is not array" << endl;
                    break;
                }

                //解析每个priority_id的key和value
                for (rapidjson::SizeType i = 0; i < jRouterData.Size(); ++i)
                {
                    IDCPriority priorityInfo;

                    rapidjson::Value::ConstMemberIterator keyIt = jRouterData[i].FindMember("key");
                    if (keyIt == jRouterData[i].MemberEnd())
                    {
                        continue;
                    }
                    string keyStr = keyIt->value.GetString();
                    string priorityId = keyStr.substr(keyStr.find_last_of("/") + 1);
                    priorityInfo.priorityId = priorityId;

                    rapidjson::Value::ConstMemberIterator valueIt = jRouterData[i].FindMember("value");
                    if (valueIt == jRouterData[i].MemberEnd())
                    {
                        continue;
                    }
                    const string &value = valueIt->value.GetString();
                    //格式是 priority_name|order|group_list|station
                    vector<string> entry = TC_Common::sepstr<string>(value, "|", true);     //withEmpty为真，有些位置是为了兼容而保留的，实际上没有值
                    if (entry.size() < 4)
                    {
                        continue;
                    }
                    priorityInfo.priorityName = entry[0];
                    priorityInfo.order = TC_Common::strto<int>(entry[1]);
                    priorityInfo.station = TC_Common::strto<int>(entry[3]);

                    vector<string> groupLists = TC_Common::sepstr<string>(entry[2], ",");
                    for (size_t j = 0; j < groupLists.size(); ++j)
                    {
                        priorityInfo.groupList.push_back(groupLists[j]);
                    }

                    //通过_stReqInfo把结果传给等待的线程
                    _etcdReqInfo.idcPriority.push_back(priorityInfo);

                }
            }

            responseClient(Tseer::TSEER_SERVERSUCCESS, timeUsed);
            return;

        } while (0);


    __CATCHEXT__

    ETCDPROC_LOGERROR << ETCDFILE_FUN << "parseJson exception," << errMsg << endl;
    responseClient(Tseer::TSEER_REGISTRY_ETCD_PARSE_JSON_FAIL, timeUsed);
}

void RequestEtcdCallback::doGetServiceGroupKey(const rapidjson::Document &jEtcdRootData)
{
    uint64_t finishTime = TNOWMS;
    int timeUsed = (finishTime - _etcdReqInfo.startTime);
    string errMsg = "";
    
    __TRY__
        /* 
        {
            "action":"get",
            "node":{
                "key": "/group_service/moduletype=",
                "value":
                ...
            }
        }
        */
        do
        {
            rapidjson::Value::ConstMemberIterator itr = jEtcdRootData.FindMember("node");
            if (itr != jEtcdRootData.MemberEnd() && itr->value.IsObject())
            {
                const rapidjson::Value& jRouterData = itr->value;
                ServiceGroupInfo serviceInfo;
                rapidjson::Value::ConstMemberIterator valueIt = jRouterData.FindMember("value");
                if (valueIt == jRouterData.MemberEnd())
                {
                    break;
                }
                const string &value = valueIt->value.GetString();

                vector<string> entry = TC_Common::sepstr<string>(value, "|", true);
                if (entry.size() < 1)
                {
                    break;
                }
                serviceInfo.key = entry[0];
                //通过_stReqInfo把结果传给等待的线程
                _etcdReqInfo.serviceInfoArr.push_back(serviceInfo);

            }

            responseClient(Tseer::TSEER_SERVERSUCCESS, timeUsed);
            return;

        } while (0);


    __CATCHEXT__

    ETCDPROC_LOGERROR << ETCDFILE_FUN << "parseJson exception," << errMsg << endl;
    responseClient(Tseer::TSEER_REGISTRY_ETCD_PARSE_JSON_FAIL, timeUsed);
}



void RequestEtcdCallback::doGetServiceGroup(const rapidjson::Document &jEtcdRootData)
{
    uint64_t finishTime = TNOWMS;
    int timeUsed = (finishTime - _etcdReqInfo.startTime);
    string errMsg = "";

    __TRY__
    /* 
    {
        "action":"get",
        "node":{
            "key":"/service_group",
            "dir":true,
            "nodes":[
                {
                    "key":"/service_group/moduletype=",
                    "value":key|userlist
                    ...
                },
                {
                    "key":"/service_group/moduletype=",
                    "value":key|userlist
                    ...
                },
                ...
            ],
        }
    }
    */
    do
    {
        rapidjson::Value::ConstMemberIterator itr = jEtcdRootData.FindMember("node");
        if (itr != jEtcdRootData.MemberEnd() && itr->value.IsObject())
        {
            const rapidjson::Value& jRoutertable = itr->value;
            if (ISFAILURE(jRoutertable.HasMember("nodes")))
            {
                ETCDLOAD_LOGERROR << FILE_FUN << "can not find  root nodes from json" << endl;
                break;
            }

            const rapidjson::Value &jRouterData = jRoutertable["nodes"];
            if (ISFAILURE(jRouterData.IsArray()))
            {
                ETCDLOAD_LOGERROR << FILE_FUN << "jRouterData nodes is not array" << endl;
                break;
            }

            for (rapidjson::SizeType i = 0; i < jRouterData.Size(); ++i)
            {
                ServiceGroupInfo serviceInfo;

                rapidjson::Value::ConstMemberIterator keyIt = jRouterData[i].FindMember("key");
                if (keyIt == jRouterData[i].MemberEnd())
                {
                    continue;
                }
                
                string keyStr = keyIt->value.GetString();
                serviceInfo.serviceGrp = keyStr.substr(keyStr.find("=") + 1);
                APIIMP_LOG<< FILE_FUN <<serviceInfo.serviceGrp<< endl;
                
                rapidjson::Value::ConstMemberIterator valueIt = jRouterData[i].FindMember("value");
                if (valueIt == jRouterData[i].MemberEnd())
                {
                    continue;
                }
                
                const string &value = valueIt->value.GetString();

                vector<string> keyUserList = TC_Common::sepstr<string>(value, "|", true);
                if (keyUserList.size() < 2)
                {
                    continue;
                }
                
                serviceInfo.key = keyUserList[0];
                APIIMP_LOG<< FILE_FUN <<serviceInfo.key<< endl;
                
                vector<string> userListStr = TC_Common::sepstr<string>(keyUserList[1], ",");
                for(size_t i = 0; i < userListStr.size(); i++)
                {
                    serviceInfo.setUserList.insert(userListStr[i]);
                    APIIMP_LOG<< FILE_FUN <<userListStr[i]<< endl;
                }
                //通过_stReqInfo把结果传给等待的线程
                _etcdReqInfo.serviceInfoArr.push_back(serviceInfo);

            }
        }

        responseClient(Tseer::TSEER_SERVERSUCCESS, timeUsed);
        return;

    } while (0);

    __CATCHEXT__

    ETCDPROC_LOGERROR << ETCDFILE_FUN << "parseJson exception," << errMsg << endl;
    responseClient(Tseer::TSEER_REGISTRY_ETCD_PARSE_JSON_FAIL, timeUsed);
}

void RequestEtcdCallback::doGetAgentPackageInfoResponse(const rapidjson::Document& jEtcdRootData)
{
    uint64_t finishTime = TNOWMS;
    int timeUsed = (finishTime - _etcdReqInfo.startTime);
    /*
    {
        "action":"get",
        "node":{
            "key":"/agent_package_info",
            "dir":true,
            "nodes":[
                {
                    "key":"/agent_package_info/osversion=",
                    "dir": true,
                    "nodes": [
                        {
                            "key": "/agent_package_info/osversion=/packageName=",
                            "value":"ostype|packageType|packageName|md5|version|uploadUser|uploadTime";
                        },
                        {
                            "key": "/agent_package_info/osversion=/packageName=",
                            "value":"ostype|packageType|packageName|md5|version|uploadUser|uploadTime";
                        },
                        ...
                    ]
                },
                ...
            ]
        }
    }
    */
    __TRY__
    do
    {
        rapidjson::Value::ConstMemberIterator itr = jEtcdRootData.FindMember("node");
        if (itr != jEtcdRootData.MemberEnd() && itr->value.IsObject())
        {
            const rapidjson::Value& jAgentPackageInfo = itr->value;
            if (ISFAILURE(jAgentPackageInfo.HasMember("nodes")))
            {
                APIIMP_LOG<<FILE_FUN << "can not find  root nodes from json" << endl;
                break;
            }

            const rapidjson::Value &jPackageInfo = jAgentPackageInfo["nodes"];
            if (ISFAILURE(jPackageInfo.IsArray()))
            {
                APIIMP_LOG<<FILE_FUN << "jAgentInfo nodes is not array" << endl;
                break;
            }
                    ETCDROUTEDATA->parseEtcdAgentPackageInfo(jPackageInfo, _etcdReqInfo.agentPackageInfoList);
         }
            responseClient(Tseer::TSEER_SERVERSUCCESS,timeUsed);
            return;

    } while (0);


    __CATCH__

    APIIMP_LOGERROR<<ETCDFILE_FUN << "parseJson exception," << endl;
    responseClient(Tseer::TSEER_REGISTRY_ETCD_PARSE_JSON_FAIL, timeUsed);
    return;
}

void RequestEtcdCallback::doGetGrayAgentInfoResponse(const rapidjson::Document& jEtcdRootData)
{
    /*
    {
        "action":"get",
        "node":{
            "key":"/agent_gray_info",
            "dir":true,
            "nodes":[
                {
                    "key":"/agent_gray_info/nodeip=",
                    "value":"node_ip|grayscale_version|grayscale_state|package_name|ostype"
                },
                {
                    "key":"/agent_gray_info/nodeip=",
                    "value":"node_ip|grayscale_version|grayscale_state|package_name|ostype"
                },
                ...
            ],
        }
    }
    */
    uint64_t finishTime = TNOWMS;
    int timeUsed = (finishTime - _etcdReqInfo.startTime);
    __TRY__
    do
    {
        rapidjson::Value::ConstMemberIterator itr = jEtcdRootData.FindMember("node");
        if (itr != jEtcdRootData.MemberEnd() && itr->value.IsObject())
        {
            const rapidjson::Value& jAgentGrayInfo = itr->value;
            if (ISFAILURE(jAgentGrayInfo.HasMember("nodes")))
            {
                APIIMP_LOG<<FILE_FUN << "can not find  root nodes from json" << endl;
                break;
            }

            const rapidjson::Value &jGrayInfo = jAgentGrayInfo["nodes"];
            if (ISFAILURE(jGrayInfo.IsArray()))
            {
                APIIMP_LOG<<FILE_FUN << "jRouterData nodes is not array" << endl;
                break;
            }
                    ETCDROUTEDATA->parseEtcdGrayAgentPackageInfo(jGrayInfo, _etcdReqInfo.grayAgentInfoList);
        }
              responseClient(Tseer::TSEER_SERVERSUCCESS,timeUsed);
        return;

    } while (0);

    __CATCH__

    APIIMP_LOGERROR<<ETCDFILE_FUN << "parseJson exception," << endl;
    responseClient(Tseer::TSEER_REGISTRY_ETCD_PARSE_JSON_FAIL, timeUsed);
    return;
}


void RequestEtcdCallback::onException(const string&ex)
{
    uint64_t    finishTime = TNOWMS;
    int     timeUsed   = (finishTime - _etcdReqInfo.startTime);

    TSEER_LOG("debug")->debug() << FILE_FUN << endl;

    if (_etcdReqInfo.notifyPtr)
    {
        _etcdReqInfo.notifyPtr->_atomic.inc();
        if(_etcdReqInfo.notifyPtr->_atomic.get() == _etcdReqInfo.notifyPtr->_run_times)
        {
            TC_ThreadLock::Lock lock(_etcdReqInfo.notifyPtr->_monitor);
            _etcdReqInfo.notifyPtr->_canWake = true;
            _etcdReqInfo.notifyPtr->_monitor.notify();
        }
    }
    
    ETCDPROC_LOGERROR << ETCDFILE_FUN << MSTIMEINSTR(finishTime) << "|use" << timeUsed << "ms|exception=" << ex << endl;
    
    responseClient(Tseer::TSEER_REGISTRY_ETCD_RET_ERROR,timeUsed);
}


void RequestEtcdCallback::onTimeout()
{
    uint64_t    finishTime = TNOWMS;
    int     timeUsed   = (finishTime - _etcdReqInfo.startTime);

    TSEER_LOG("debug")->debug() << FILE_FUN << endl;

    if (_etcdReqInfo.notifyPtr)
    {
        _etcdReqInfo.notifyPtr->_atomic.inc();
        if(_etcdReqInfo.notifyPtr->_atomic.get() == _etcdReqInfo.notifyPtr->_run_times)
        {
            TC_ThreadLock::Lock lock(_etcdReqInfo.notifyPtr->_monitor);
            _etcdReqInfo.notifyPtr->_canWake = true;
            _etcdReqInfo.notifyPtr->_monitor.notify();
        }
    }
    
    ETCDPROC_LOGERROR << ETCDFILE_FUN << MSTIMEINSTR(finishTime) << "|use=" << timeUsed << "ms" << endl;

    responseClient(Tseer::TSEER_REGISTRY_ETCD_TIMEOUT,timeUsed);
}

void RequestEtcdCallback::onClose()
{
}

void RequestEtcdCallback::reportEtcdStat(int ret,StatReport::StatResult statResult,long timeUse)
{
    string interfaceName = "unknown";
    switch(_etcdReqInfo.etcdAction)
    {
        case ETCD_API_ADD:
        case ETCD_API_ADD_PORTLIST:
        case ETCD_ADD_SELF:
        case ETCD_ADD_NODE_INFO:
        case ETCD_ADD_IDC_RULE:
        case ETCD_ADD_IDC_PRIORITY:
        case ETCD_ADD_SERVICE_GROUP:
        case ETCD_ADD_AGENT_GRAY_STATE:
        {
            interfaceName="add";
            break;
        }
        case ETCD_API_GET:
        case ETCD_GET_ALL_SEER_AGENTS:
        case ETCD_GET_ONE_SEER_AGENT:
        case ETCD_GET_ALL_IDC_RULE:
        case ETCD_GET_ONE_IDC_RULE:
        case ETCD_GET_ALL_AGENT_PACKAGE_INFO:
        case ETCD_GET_ALL_GRAYAGENT_INFO:
        case ETCD_GET_ALL_SERVICE_GROUP:
        case ETCD_GET_SERVICE_GROUP_KEY:
        {
            interfaceName="get";
            break;
        }
        case ETCD_API_DELETE:
        case ETCD_DEL_NODE_INFO:
        case ETCD_DEL_IDC_RULE:
        case ETCD_DEL_IDC_PRIORITY:
        case ETCD_DEL_AGENTPACKAGEINFO:
        case ETCD_DEL_AGENT_GRAY_STATE:
        {
            interfaceName="delete";
            break;  
        }
        case ETCD_UPDATE:
        case ETCD_UPDATE_BATACH:
        case ETCD_UPDATE_NODE_TIMEOUT:
        case ETCD_API_UPDATE_BATACH:
        case ETCD_API_SET:
        case ETCD_UPDATE_SELF_STATE:
        case ETCD_KEEP_NODE_ALIVE:
        case ETCD_MODIFY_IDC_RULE:
        case ETCD_MODIFY_IDC_PRIORITY:
        case ETCD_UPDATE_AGENTPACKAGEINFO:
        case ETCD_UPDATE_SERVICE_GROUP:
        {
            interfaceName="set";
            break;
        }
        default: break;
    }
    
    StatReport  *stat = Application::getCommunicator()->getStatReport();
    if(stat)
    {
        stat->report(ClientConfig::ModuleName, ServerConfig::LocalIp, "etcd", _etcdReqInfo.etcdHost, _etcdReqInfo.etcdPort, interfaceName, statResult, timeUse,ret);
    }
    else{
        ETCDPROC_LOGERROR << ETCDFILE_FUN<< "StatReport has not been initil" << endl;
    }   
}

void RequestEtcdCallback::responseClient(int ret,long timeUse, const vector<RouterData>& routerDataList)
{
    __TRY__
    const string retryFlag= "DOCKER_ETCD_RETRY_TIME";

    int retryTime = 1;
    if (_etcdReqInfo.current)
    {
        const map<string, string>&  context    = _etcdReqInfo.current->getResponseContext();
        if(context.find(retryFlag) != context.end())
        {
            retryTime = TC_Common::strto<int>(context.find(retryFlag)->second);
        }
    }

    int apiRet = ret;
    string errMsg = "";
    if(TSEER_REGISTRY_ETCD_DATA_NOTFOUND == ret)
    {
        if (::isIdcAction(_etcdReqInfo.etcdAction))
        {
            apiRet = API_ALL_OR_PARTIAL_DATA_NOT_FOUND;
            errMsg = "all or some data not found";
        }
        else
        {
            apiRet = API_NO_MODULE;
            errMsg = "has no module exist";
        }
    }
    
    if(ret != TSEER_SERVERSUCCESS  &&
        ret != TSEER_REGISTRY_ETCD_DATA_NOTFOUND)
    {
        apiRet = API_INTERNAL_ERROR;
        errMsg = "invalid data";
    }
    
    
    bool needRetry = false;
    if(ret == TSEER_REGISTRY_ETCD_TIMEOUT ||
        ret ==TSEER_REGISTRY_ETCD_RET_ERROR)
    {
        //check need to retry or not
        if(retryTime < EtcdHandle::maxRetryTime())
        {
            retryTime += 1;
            
            map<string, string> context;
            context.insert(make_pair(retryFlag,TC_Common::tostr(retryTime)));
            _etcdReqInfo.current->setResponseContext(context);
            needRetry = true;
        }
    }
    
    switch(_etcdReqInfo.etcdAction)
    {
        case ETCD_UPDATE: 
        {
            if(needRetry)
            {
                STOREPROXY->updateServerState(_etcdReqInfo.routerDataInfo,_etcdReqInfo.current);
            }
            else
            {
                Registry::async_response_updateServer(_etcdReqInfo.current, ret);
            }
            break;
        }
        case ETCD_UPDATE_BATACH:
        {
            //全部设置完成后才响应客户端
            if(_etcdReqInfo.notifyPtr->_atomic.get() == _etcdReqInfo.notifyPtr->_run_times)
            {
                if(needRetry)
                {
                    //这里可能有部分已经设置成功了，再设置一次也没关系
                    STOREPROXY->updateServerStates(_etcdReqInfo.needUpdateRouterList,_etcdReqInfo.current);
                }
                else
                {
                    Registry::async_response_updateServerBatch(_etcdReqInfo.current, ret);
                }
            }
            break;  
        }
        case ETCD_API_ADD_PORTLIST:
        {
            //全部设置完成后才响应客户端
            if(_etcdReqInfo.notifyPtr->_atomic.get() == _etcdReqInfo.notifyPtr->_run_times)
            {
                if(needRetry)
                {
                    STOREPROXY->addServers(_etcdReqInfo.needUpdateRouterList,_etcdReqInfo.current);
                }
                else
                {
                    ApiRegImp::doResponse(_etcdReqInfo.current,apiRet,errMsg);
                }
            }
            break;  
        }
        case ETCD_API_UPDATE_BATACH:
        {
            //全部设置完成后才响应客户端
            if(_etcdReqInfo.notifyPtr->_atomic.get() == _etcdReqInfo.notifyPtr->_run_times)
            {
                if(needRetry)
                {
                    STOREPROXY->keepServerAlive(_etcdReqInfo.needUpdateRouterList,_etcdReqInfo.current);
                }
                else
                {
                    ApiRegImp::doResponse(_etcdReqInfo.current,apiRet,errMsg);
                }
            }
            break;  
        }
        case ETCD_API_SET:
        {
            //全部设置完成后才响应客户端
            if(_etcdReqInfo.notifyPtr->_atomic.get() == _etcdReqInfo.notifyPtr->_run_times)
            {
                if(needRetry)
                {
                    STOREPROXY->updateServers(_etcdReqInfo.needUpdateRouterList,_etcdReqInfo.current);
                }
                else
                {
                    ApiRegImp::doResponse(_etcdReqInfo.current,apiRet,errMsg);
                }
            }
            break;  
        }
        case ETCD_API_DELETE:
        {
            //全部设置完成后才响应客户端
            if(_etcdReqInfo.notifyPtr->_atomic.get() == _etcdReqInfo.notifyPtr->_run_times)
            {
                if(needRetry)
                {
                    STOREPROXY->deleteServers(_etcdReqInfo.reqRouteList,_etcdReqInfo.current);
                }
                else
                {
                    ApiRegImp::doResponse(_etcdReqInfo.current,apiRet,errMsg);
                }
            }
            break;      
        }
        case ETCD_API_GET:
        {
            if(needRetry)
            {
                STOREPROXY->getServers(_etcdReqInfo.routerDataReq,_etcdReqInfo.current);
            }
            else
            {
                ApiRegImp::doGetRouteResponse(_etcdReqInfo.current, apiRet, routerDataList, errMsg);
            }
            break;
        }
        case ETCD_UPDATE_NODE_TIMEOUT:
        case ETCD_UPDATE_SELF_STATE:
        case ETCD_ADD_SELF:
        case ETCD_API_ADD:
        case ETCD_ADD_NODE_INFO:
        case ETCD_DEL_NODE_INFO:
        case ETCD_KEEP_NODE_ALIVE:
        {
            //自身发起的请求，不用回传数据，也不需要回包
            break;
        }
        case ETCD_GET_ALL_SEER_AGENTS:
        case ETCD_GET_ONE_SEER_AGENT:
        case ETCD_GET_ALL_IDC_RULE:
        case ETCD_GET_ONE_IDC_RULE:
        case ETCD_GET_ALL_IDC_PRIORITY:
        case ETCD_GET_ALL_SERVICE_GROUP:
        case ETCD_GET_SERVICE_GROUP_KEY:
        case ETCD_GET_ALL_AGENT_PACKAGE_INFO:
        case ETCD_GET_ALL_GRAYAGENT_INFO:
        {
            //唤醒请求的线程
            {
                TC_ThreadLock::Lock lock(_etcdReqInfo.notifyPtr->_monitor);
                _etcdReqInfo.notifyPtr->_canWake = true;
                _etcdReqInfo.notifyPtr->_monitor.notify();
            }
            break;
        }
        case ETCD_ADD_IDC_RULE:
        {
            if (needRetry)
            {
                STOREPROXY->addIdcGroupRule(_etcdReqInfo.groupName, _etcdReqInfo.allowIps, _etcdReqInfo.current);
            }
            else
            {
                ApiRegImp::doAddIdcResponse(_etcdReqInfo.current, apiRet, _etcdReqInfo.newGroupId);
            }
            break;
        }
        case ETCD_MODIFY_IDC_RULE:
        {
            if (needRetry)
            {
                STOREPROXY->modifyIdcGroupRule(_etcdReqInfo.groupId, _etcdReqInfo.allowIps, _etcdReqInfo.current);
            }
            else
            {
                ApiRegImp::doResponse(_etcdReqInfo.current, apiRet, errMsg);
            }
            break;
        }
        case ETCD_DEL_IDC_RULE:
        {
            //全部删除完成后才响应客户端
            if (_etcdReqInfo.notifyPtr->_atomic.get() == _etcdReqInfo.notifyPtr->_run_times)
            {
                if (needRetry)
                {
                    STOREPROXY->delIdcGroupRule(_etcdReqInfo.delGroupId, _etcdReqInfo.current);
                }
                else
                {
                    ApiRegImp::doResponse(_etcdReqInfo.current, apiRet, errMsg);
                }
            }
            break;
        }
        case ETCD_ADD_IDC_PRIORITY:
        {
            if (needRetry)
            {
                STOREPROXY->addIdcPriority(_etcdReqInfo.order, _etcdReqInfo.priorityName, _etcdReqInfo.groupIdList, _etcdReqInfo.current);
            }
            else
            {
                ApiRegImp::doAddIdcResponse(_etcdReqInfo.current, apiRet, _etcdReqInfo.newPriorityId);
            }
            break;
        }
        case ETCD_MODIFY_IDC_PRIORITY:
        {
            if (needRetry)
            {
                STOREPROXY->modifyIdcPriority(_etcdReqInfo.priorityId, _etcdReqInfo.order, _etcdReqInfo.priorityName, _etcdReqInfo.groupIdList, _etcdReqInfo.current);
            }
            else
            {
                ApiRegImp::doResponse(_etcdReqInfo.current, apiRet, errMsg);
            }
            break;
        }
        case ETCD_DEL_IDC_PRIORITY:
        {
            //全部删除完成后才响应客户端
            if (_etcdReqInfo.notifyPtr->_atomic.get() == _etcdReqInfo.notifyPtr->_run_times)
            {
                if (needRetry)
                {
                    STOREPROXY->delIdcPriority(_etcdReqInfo.delPriorityIds, _etcdReqInfo.current);
                }
                else
                {
                    ApiRegImp::doResponse(_etcdReqInfo.current, apiRet, errMsg);
                }
            }
            break;
        }

        default: break;
    }

    StatReport::StatResult statResult = StatReport::STAT_SUCC;
    if(ret == TSEER_REGISTRY_ETCD_TIMEOUT)
    {
        statResult = StatReport::STAT_TIMEOUT;
    }

    if(ret != TSEER_REGISTRY_ETCD_TIMEOUT  &&
        ret != TSEER_SERVERSUCCESS  &&
        ret != TSEER_REGISTRY_ETCD_DATA_NOTFOUND)
    {
        statResult = StatReport::STAT_EXCE;
    }
    
    reportEtcdStat(ret,statResult,timeUse);
    ETCDPROC_LOG << ETCDFILE_FUN << "|response,ret= " << ret << "|retryTime= " << retryTime << endl;
    __CATCH__
}


