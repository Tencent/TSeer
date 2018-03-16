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

#include <sstream>


#include "EtcdHandle.h"
#include "EtcdDataCache.h"
#include "util/tc_md5.h"
#include "util/tc_timeprovider.h"
#include "util/tc_common.h"
#include "EtcdHost.h"
#include "util.h"


int EtcdHandle::_iReqEtcdRetryTime = 3;

EtcdHandle::EtcdHandle():_iTTLTimeout(125){}

EtcdHandle::~EtcdHandle()
{
    _ast.waitForAllDone(5000);
    _ast.terminate();

    _apiAst.waitForAllDone(5000);
    _apiAst.terminate();
}

int EtcdHandle::init(TC_Config * pconf)
{

    extern TC_Config * g_pconf;
    __TRY__

    BaseHandle::init(pconf);
    //初始化etcd集群检测组件
    if(ETCDHOST->init(g_pconf) != 0)
    {
        cerr << "ETCDHOST initialize exception"<< endl;
        exit(-1);
    }
    ETCDHOST->start();

    //初始化etcd缓存
    if(ETCDROUTEDATA->init(g_pconf) != 0)
    {
        cerr << "ETCDROUTEDATA initialize exception"<< endl;
        exit(-1);
    }

    int HttpTimeout  = TC_Common::strto<int>(pconf->get("/tars/etcd<etcdreqtimeout>","2000"));
    HttpTimeout = HttpTimeout < 1000 ?2000:HttpTimeout;
    
    _ast.setTimeout(HttpTimeout);
    _ast.start();

    _apiAst.setTimeout(HttpTimeout);
    _apiAst.start();

    int iTimeout = TC_Common::strto<int>(pconf->get("/tars/etcd<ttltimeout>","125"));
    _iTTLTimeout = iTimeout > _iTTLTimeout ? iTimeout:_iTTLTimeout; 

    _iReqEtcdRetryTime = TC_Common::strto<int>(pconf->get("/tars/etcd<retrymaxtime>","3"));
    return 0;
    __CATCH__
    return -1;
}

bool EtcdHandle::isValidState(const string &state)
{
    if( state.empty() || state == "active" || 
        state == "inactive" || state == "activating" ||
        state == "deactivating" ||state == "destroying" ||
        state == "destroyed" )
        return true;

    return false;
}

bool EtcdHandle::isRouterDataValid(const RouterData &routerDataInfo)
{
    if( routerDataInfo.moduletype.empty() || 
        routerDataInfo.application.empty() || 
        routerDataInfo.service_name.empty() ||
        routerDataInfo.node_name.empty() ||
        routerDataInfo.container_name.empty() )
    {
        ETCDPROC_LOGERROR<< FILE_FUN <<toStr(routerDataInfo) << "|param invalid"<< endl;
        return false;
    }   
    return true;
}

bool EtcdHandle::isBaseServiceInfoValid(const BaseServiceInfo &baseServiceInfo)
{
    if (baseServiceInfo.locatorId.empty() || baseServiceInfo.servant.empty() ||
        baseServiceInfo.endpoint.empty())
    {
        ETCDPROC_LOGERROR << FILE_FUN << baseServiceInfo.toString() << "|param invalid" << endl;
        return false;
    }
    return true;
}

bool EtcdHandle::isNodeServiceInfoValid(const NodeServiceInfo &info)
{
    if (info.nodeName.empty() || info.nodeNetworkId.empty())
    {
        ETCDPROC_LOGERROR << FILE_FUN << info.nodeName << "|" << info.nodeNetworkId << "|param invalid" << endl;
        return false;
    }

    TSEER_LOG("debug")->debug() <<FILE_FUN<< "return true" << endl;
    return true;
}

int EtcdHandle::makeUpdateNodeUrl(const string &networkId, const string &nodeName, string &sEtcdReqUrl, EtcdAction etcdAction)
{
    sEtcdReqUrl = ETCDHOST->getNodeInfoHost();
    MAKEVALUE(sEtcdReqUrl, NODE_NETWORKID, networkId);
    MAKEVALUE(sEtcdReqUrl, NODE_NAME, nodeName);

    if (etcdAction == ETCD_ADD_NODE_INFO)
    {
        sEtcdReqUrl += "/" + OBJNAMEEKEY;
    }
    else if (etcdAction == ETCD_KEEP_NODE_ALIVE)
    {
        sEtcdReqUrl += "/" + PRESENTSTATEKEY;
        //设置心跳超时
        sEtcdReqUrl += "?ttl=" + _nodeTimeout;
    }
    
    return 0;
}

int EtcdHandle::makeNodeValue(const NodeServiceInfo &nodeInfo, string &value)
{
    string sRealValue;
    sRealValue = nodeInfo.nodeObj;
    sRealValue += "|" + nodeInfo.nodeName;
    sRealValue += "|" + nodeInfo.nodeNetworkId;
    sRealValue += "|" + nodeInfo.endpointIp;
    sRealValue += "|" + nodeInfo.endpointPort;
    sRealValue += "|" + nodeInfo.registerTime;
    sRealValue += "|" + nodeInfo.dataDir;
    sRealValue += "|" + nodeInfo.loadAvg1;
    sRealValue += "|" + nodeInfo.loadAvg5;
    sRealValue += "|" + nodeInfo.loadAvg15;
    sRealValue += "|" + nodeInfo.cpuuseAvg1;
    sRealValue += "|" + nodeInfo.cpuNum;
    sRealValue += "|" + nodeInfo.cpuHz;
    sRealValue += "|" + nodeInfo.memorySize;
    sRealValue += "|" + nodeInfo.coreFileSize;
    sRealValue += "|" + nodeInfo.openFileSize;
    sRealValue += "|" + nodeInfo.settingState;
    sRealValue += "|" + nodeInfo.presentState;
    sRealValue += "|" + nodeInfo.version;
    sRealValue += "|" + nodeInfo.ostype;
    sRealValue += "|" + nodeInfo.dockerNodeVersion;
    sRealValue += "|" + nodeInfo.templateName;
    sRealValue += "|" + nodeInfo.groupId;
    sRealValue += "|" + nodeInfo.locator;
    sRealValue += "|" + TC_Common::now2str("%Y-%m-%d %H:%M:%S");

    value = "value=" + sRealValue;
    return 0;
}

int EtcdHandle::makeGroupInfoValue(const IDCGroupInfo &groupInfo, string &value)
{
    //value=group_name|allow_ip|denney_ip|rule_order
    string realValue;
    realValue = groupInfo.groupName;

    string allowIpStr;
    for (size_t i = 0; i < groupInfo.allowIp.size(); ++i)
    {
        allowIpStr += groupInfo.allowIp[i] + ",";   //使用逗号分割
    }
    realValue += "|" + allowIpStr;
    
    string denneyIpStr;
    for (size_t i = 0; i < groupInfo.denneyIp.size(); ++i)
    {
        denneyIpStr += groupInfo.denneyIp[i] + ",";
    }
    realValue += "|" + denneyIpStr;
    realValue += "|" + groupInfo.ruleOrder;

    value = "value=" + realValue;
    return 0;
}

int EtcdHandle::makePriorityInfoValue(const IDCPriority &priorityInfo, string &value)
{
    //value=priority_name|order|group_list|station

    string sRealValue;
    sRealValue = priorityInfo.priorityName;
    sRealValue += "|" + TC_Common::tostr<int>(priorityInfo.order);

    string groupList;
    for (size_t i = 0; i < priorityInfo.groupList.size(); ++i)
    {
        groupList += priorityInfo.groupList[i] + ",";
    }
    sRealValue += "|" + groupList;
    sRealValue += "|" + priorityInfo.station;

    value = "value=" + sRealValue;

    return 0;
}

int EtcdHandle::isNameValidGenNewIdcGroupId(const string &groupName, string &newIdStr)
{
    vector<IDCGroupInfo> existGroupList;
    if (getAllIdcGroupRule(existGroupList))
    {
        return -1;
    }

    //检查名字有没有被用，获取所有的GroupId
    std::set<int> usedId;
    for (size_t i = 0; i < existGroupList.size(); ++i)
    {
        if (groupName == existGroupList[i].groupName)
        {
            return -2;  //名字重复返回-2
        }
        int used = TC_Common::strto<int>(existGroupList[i].groupId);
        usedId.insert(used);
    }

    int newId = 1;
    while (true)
    {
        if (usedId.find(newId) == usedId.end())
        {
            newIdStr = TC_Common::tostr<int>(newId);
            return 0;
        }
        else
        {
            ++newId;

            //预防死循环
            if (newId == INT_MAX)
            {
                return -1;
            }
        }
    }
}

int EtcdHandle::makeRouteValue(const RouterData &routerDataInfo,string &value)
{
    string sRealValue = "";
    if(!routerDataInfo.servant.empty())
    {
        map<string,string>::const_iterator itr = routerDataInfo.servant.begin();
        //格式:value=obj@endpoint:obj@endpoint||||||N||inactive|inactive|0|0|100|||
        for(;itr!=routerDataInfo.servant.end();++itr)
        {
            if(sRealValue.size() > 0)
            {
                sRealValue += ":";
            }
            
            sRealValue += itr->first;
            sRealValue += "@";
            sRealValue += itr->second;
        }
        ETCDPROC_LOG<< FILE_FUN <<toStr(routerDataInfo) << "|endpoint data="<<sRealValue<< endl;

        sRealValue += '|';
        sRealValue +=routerDataInfo.set_name;
        sRealValue += "|";
        sRealValue += routerDataInfo.set_area;
        sRealValue += "|";
        sRealValue += routerDataInfo.set_group;
        sRealValue += "|";
        sRealValue += routerDataInfo.process_id;
        sRealValue += "|";
        sRealValue += routerDataInfo.ip_group_name;
        sRealValue += "|";
        sRealValue += routerDataInfo.enable_set;
        sRealValue += "|";
        sRealValue += routerDataInfo.enable_group;
        sRealValue += "|";
        sRealValue += routerDataInfo.setting_state;
        sRealValue += "|";
        sRealValue += routerDataInfo.present_state;
        sRealValue += "|";
        sRealValue += routerDataInfo.node_port;
        sRealValue += "|";
        sRealValue += routerDataInfo.container_port;
        sRealValue += "|";
        sRealValue += routerDataInfo.weight;
        sRealValue += "|";
        sRealValue += routerDataInfo.bak_flag;
        sRealValue += "|";
        sRealValue += routerDataInfo.grid_flag;
        sRealValue += "|";
        sRealValue += routerDataInfo.weighttype;
        sRealValue += "|";
        sRealValue += routerDataInfo.enable_heartbeat;
    }
    value = "value=" + sRealValue;
    return 0;
}

int EtcdHandle::makeUpdateBaseServiceUrl(const BaseServiceInfo &baseServiceInfo, string &sEtcdReqUrl, EtcdAction etcdAction)
{
    sEtcdReqUrl = ETCDHOST->getBaseServiceHost();
    MAKEVALUE(sEtcdReqUrl, LOCATOR_ID, baseServiceInfo.locatorId);
    MAKEVALUE(sEtcdReqUrl, SERVANT, baseServiceInfo.servant);

    if (etcdAction == ETCD_UPDATE_SELF_STATE)
    {
        sEtcdReqUrl += "/" + PRESENTSTATEKEY;
        //设置心跳ttl
        sEtcdReqUrl += "?ttl=" + _registryTimeout;
    }
    else if (etcdAction == ETCD_ADD_SELF)
    {
        sEtcdReqUrl += "/" + OBJNAMEEKEY;
    }
    return 0;
}

int EtcdHandle::makeBaseServiceValue(const BaseServiceInfo &baseServiceInfo, string &value)
{
    string sRealValue;
    sRealValue += baseServiceInfo.servant + "@" + baseServiceInfo.endpoint;
    sRealValue += "|" + baseServiceInfo.presentState;
    sRealValue += "|" + baseServiceInfo.tarsVersion;
    sRealValue += "|" + baseServiceInfo.enableGroup;
    value = "value=" + sRealValue;

    return 0;
}

int EtcdHandle::makeUpdateRouteUrl(const RouterData &routerDataInfo,string &sEtdReqUrl,EtcdAction etcdAction)
{
    sEtdReqUrl = ETCDHOST->getRouterHost();
    MAKEVALUE(sEtdReqUrl,MODULETYPE,routerDataInfo.moduletype);
    MAKEVALUE(sEtdReqUrl,APPLICATION,routerDataInfo.application);
    MAKEVALUE(sEtdReqUrl,SERVICE_NAME,routerDataInfo.service_name);
    MAKEVALUE(sEtdReqUrl,NODE_NAME,routerDataInfo.node_name);
    MAKEVALUE(sEtdReqUrl,CONTAINER_NAME,routerDataInfo.container_name);
    /*这里先这么实现*/
    if(etcdAction == ETCD_UPDATE ||
        etcdAction == ETCD_UPDATE_BATACH ||
        etcdAction == ETCD_UPDATE_NODE_TIMEOUT ||
        etcdAction == ETCD_API_UPDATE_BATACH)
    {
        sEtdReqUrl += "/" + PRESENTSTATEKEY;
    }
    else if(ETCD_API_SET == etcdAction)
    {
        sEtdReqUrl += "/" + OBJNAMEEKEY +"?prevExist=true";
    }
    else  if(ETCD_API_ADD == etcdAction ||
        ETCD_API_ADD_PORTLIST == etcdAction)
    {
        sEtdReqUrl += "/" + OBJNAMEEKEY;
    }
    return 0;
}

int EtcdHandle::makeDelRouteRequestUrl(const RouterDataRequest &reqInfo,string &sEtdReqUrl)
{
    
    if( reqInfo.moduletype.empty() || reqInfo.application.empty() || 
            reqInfo.service_name.empty() || reqInfo.node_name.empty() || reqInfo.container_name.empty() )
    {
        ETCDPROC_LOGERROR<< FILE_FUN <<toStr(reqInfo) << "|param invalid"<< endl;
        return TSEER_REGISTRY_PARAM_ERROR;
    }

    sEtdReqUrl = ETCDHOST->getRouterHost();
    
    MAKEVALUE(sEtdReqUrl,"moduletype",reqInfo.moduletype);
    MAKEVALUE(sEtdReqUrl,"application",reqInfo.application);
    MAKEVALUE(sEtdReqUrl,"service_name",reqInfo.service_name);
    MAKEVALUE(sEtdReqUrl,"node_name",reqInfo.node_name);
    MAKEVALUE(sEtdReqUrl,"container_name",reqInfo.container_name);
    sEtdReqUrl += "?dir=true&recursive=true";

    return 0;   
}

int EtcdHandle::makeGetRouteRequestUrl(const RouterDataRequest & routerDataInfo,string &sEtdReqUrl)
{
    sEtdReqUrl = ETCDHOST->getRouterHost();

    if(routerDataInfo.moduletype.empty())
    {
        ETCDPROC_LOGERROR<< FILE_FUN <<toStr(routerDataInfo) << "|param invalid"<< endl;
        return TSEER_REGISTRY_PARAM_ERROR;
    }
    else
        MAKEVALUE(sEtdReqUrl,"moduletype",routerDataInfo.moduletype);

    if(!routerDataInfo.application.empty())
        MAKEVALUE(sEtdReqUrl,"application",routerDataInfo.application);
    else
    {
        sEtdReqUrl += "?recursive=true";
        return 0;
    }

    if(!routerDataInfo.service_name.empty())
        MAKEVALUE(sEtdReqUrl,"service_name",routerDataInfo.service_name);
    else 
    {
        sEtdReqUrl += "?recursive=true";
        return 0;
    }

    if(!routerDataInfo.node_name.empty())
        MAKEVALUE(sEtdReqUrl,"node_name",routerDataInfo.node_name);
    else
    {
        sEtdReqUrl += "?recursive=true";
        return 0;
    }

    if(!routerDataInfo.container_name.empty())
        MAKEVALUE(sEtdReqUrl,"container_name",routerDataInfo.container_name);
    else
    {
        sEtdReqUrl += "?recursive=true";
        return 0;
    }

    sEtdReqUrl += "?recursive=true";

    return 0;
}


int EtcdHandle::getContainerRouterByIp(const string& ip,vector<RouterData>& routerDataList)
{
    int ret = ETCDROUTEDATA->getRouterDataByIp(ip,routerDataList);    
    ETCDPROC_LOG<<FILE_FUN<<ip << "|ret="<<ret<<"|size="<<routerDataList.size()<<endl;
    return ret;
}

int EtcdHandle::updateNodeServerStates(const string& ip,const string& presentState)
{
    ETCDPROC_LOG<<FILE_FUN<<ip << "|presentState="<<presentState<<"|coming"<<endl;
    vector<RouterData> vRouterList;
    if(getContainerRouterByIp(ip,vRouterList) != 0)
    {
        string info = "get container routerdata from "+ ip + ",failure";
        ETCDPROC_LOGERROR<< FILE_FUN <<"get container routerdata from "<<ip <<",failure"<< endl;
        return TSEER_REGISTRY_ETCD_UNKNOWN;
    }

    vector<RouterData> needUpdateRouterList;
    for(size_t i = 0;i < vRouterList.size();i++)
    {
        //状态和内存态不一直才设置
        if(vRouterList[i].present_state != presentState)
        {
            needUpdateRouterList.push_back(vRouterList[i]);
        }
    }
    
    //原子操作，等待所有更新结束
    NotifyEventPtr notifyEventPtr = new NotifyEvent();
    notifyEventPtr->_run_times = int(needUpdateRouterList.size());
    ETCDPROC_LOG<<FILE_FUN<< ip <<"|need update size="<<notifyEventPtr->_run_times<<endl;
    for(int i = 0;i < notifyEventPtr->_run_times;i++)
    {
        const RouterData &routerDataInfo = needUpdateRouterList[i]; 
        string etcdURL;
        makeUpdateRouteUrl(routerDataInfo,etcdURL,ETCD_UPDATE_NODE_TIMEOUT);

        string value = "value=" + presentState;
        
        tars::TC_HttpRequest HttpReq;
        HttpReq.setHeader("Content-Type","application/x-www-form-urlencoded");
        HttpReq.setPutRequest(etcdURL,value);
        
        EtcdReqestInfo etcdReqInfo;
        setEtcdReqInfo(routerDataInfo,ETCD_UPDATE_NODE_TIMEOUT,NULL,etcdReqInfo);
        etcdReqInfo.notifyPtr = notifyEventPtr;
        HttpReq.getHostPort(etcdReqInfo.etcdHost,etcdReqInfo.etcdPort);
        
        ETCDPROC_LOG<<FILE_FUN<< etcdURL <<"|"<<value<<endl;
        tars::TC_HttpAsync::RequestCallbackPtr callback = new RequestEtcdCallback(etcdReqInfo);

        int ret = _ast.doAsyncRequest(HttpReq, callback);
        if(ret != TC_ClientSocket::EM_SUCCESS)
        {
            notifyEventPtr->_atomic.inc();
            //记录错误个数
            notifyEventPtr->_atomicFault.inc();
            string info = "send req to " + etcdReqInfo.etcdHost + ":" + TC_Common::tostr(etcdReqInfo.etcdPort) + ",failure,ret="+TC_Common::tostr(ret); 
            ETCDPROC_LOGERROR<< FILE_FUN <<EtcdReqStr(etcdReqInfo)<<"|"<<info<<endl;
            ret = TSEER_REGISTRY_ETCD_SEND_REQ_FAIL;
        }
    }

    if (notifyEventPtr->_run_times > 0 && notifyEventPtr->_atomicFault.get() == notifyEventPtr->_run_times)
    {
        string info = "update " + ip + " all containers state to " + presentState + " failure";
        ETCDPROC_LOGERROR << FILE_FUN << ip << "|update all container state failure" << endl;
        return TSEER_REGISTRY_ETCD_SEND_REQ_FAIL;
    }

    ETCDPROC_LOG<<FILE_FUN<< ip <<"end"<<endl;
    return 0;
}

int EtcdHandle::updateServerStates(const vector<RouterData>& needUpdateRouterList, tars::TarsCurrentPtr &current)
{
    size_t iCount = needUpdateRouterList.size();
    ETCDPROC_LOG<<FILE_FUN<<current->getIp() << "|size="<<iCount<<"|coming"<<endl;
    
    //原子操作，等待所有更新结束才返回给客户端
    NotifyEventPtr notifyEventPtr = new NotifyEvent();
    notifyEventPtr->_run_times = int(iCount);
    for(size_t i = 0;i < iCount;i++)
    {
        const RouterData &routerDataInfo = needUpdateRouterList[i]; 

        RouterData  stConStateData;
        int ret = ETCDROUTEDATA->getRouterData(routerDataInfo,stConStateData);
        if(ISSUCC(ret))
        {
            //这个版本都更新,先保证兼容
            if(routerDataInfo.present_state == stConStateData.present_state)
            {
                continue;
            }
        }
        else
        {
            REGIMP_LOGERROR << FILE_FUN<< current->getIp()<<"|etcd cache has no such data|"<<toStr(routerDataInfo)<<endl;
            ETCDPROC_LOGERROR<< FILE_FUN <<current->getIp()<<"|etcd cache has no such data|"<<toStr(routerDataInfo)<<endl;
            continue;
        }
        string etcdURL;
        makeUpdateRouteUrl(routerDataInfo,etcdURL,ETCD_UPDATE_BATACH);
        
        string value = "value=" + routerDataInfo.present_state;
        
        tars::TC_HttpRequest HttpReq;
        HttpReq.setHeader("Content-Type","application/x-www-form-urlencoded");
        HttpReq.setPutRequest(etcdURL,value);
        
        EtcdReqestInfo etcdReqInfo;
        setEtcdReqInfo(routerDataInfo,ETCD_UPDATE_BATACH,current,etcdReqInfo);
        HttpReq.getHostPort(etcdReqInfo.etcdHost,etcdReqInfo.etcdPort); 

        etcdReqInfo.needUpdateRouterList = needUpdateRouterList;
        etcdReqInfo.notifyPtr = notifyEventPtr;
        
        HttpReq.getHostPort(etcdReqInfo.etcdHost,etcdReqInfo.etcdPort);
        
        ETCDPROC_LOG<<FILE_FUN<< etcdURL <<"|"<<value<<endl;
        tars::TC_HttpAsync::RequestCallbackPtr callback = new RequestEtcdCallback(etcdReqInfo);

        ret = _ast.doAsyncRequest(HttpReq, callback);
        if(ret == TC_ClientSocket::EM_SUCCESS)
        {
            current->setResponse(false);
        }
        else
        {
            notifyEventPtr->_atomic.inc();
            
            string info = "send req to " + etcdReqInfo.etcdHost + ":" + TC_Common::tostr(etcdReqInfo.etcdPort) + ",failure,ret="+TC_Common::tostr(ret);
            ETCDPROC_LOGERROR<< FILE_FUN <<EtcdReqStr(etcdReqInfo)<<"|"<<info<<endl;
            ret = TSEER_REGISTRY_ETCD_SEND_REQ_FAIL;
        }
    }

    if(notifyEventPtr->_atomic.get() == notifyEventPtr->_run_times)
    {
        current->setResponse(false);
        Registry::async_response_updateServerBatch(current, TSEER_REGISTRY_ETCD_SEND_REQ_FAIL);
    }
    
    return 0;
}

int EtcdHandle::updateServerState(const RouterData &routerDataInfo,tars::TarsCurrentPtr &current)
{
    string etcdURL;
    makeUpdateRouteUrl(routerDataInfo,etcdURL,ETCD_UPDATE);
    
    string value = "value=" + routerDataInfo.present_state;
    
    tars::TC_HttpRequest HttpReq;
    HttpReq.setHeader("Content-Type","application/x-www-form-urlencoded");
    HttpReq.setPutRequest(etcdURL,value);
    
    EtcdReqestInfo etcdReqInfo;
    setEtcdReqInfo(routerDataInfo,ETCD_UPDATE,current,etcdReqInfo);
    etcdReqInfo.routerDataInfo = routerDataInfo;
    HttpReq.getHostPort(etcdReqInfo.etcdHost,etcdReqInfo.etcdPort); 
    
    ETCDPROC_LOG<<FILE_FUN<< etcdURL <<"|"<<value<<endl;
    tars::TC_HttpAsync::RequestCallbackPtr callback = new RequestEtcdCallback(etcdReqInfo);

    int ret = _ast.doAsyncRequest(HttpReq, callback);
    if(ret == TC_ClientSocket::EM_SUCCESS)
    {
        current->setResponse(false);
    }
    else
    {
        string info = "send req to " + etcdReqInfo.etcdHost + ":" + TC_Common::tostr(etcdReqInfo.etcdPort) + ",failure,ret="+TC_Common::tostr(ret);
        ETCDPROC_LOGERROR<< FILE_FUN <<EtcdReqStr(etcdReqInfo)<<"|"<<info<<endl;
        ret = TSEER_REGISTRY_ETCD_SEND_REQ_FAIL;
    }

    return ret;
}

int EtcdHandle::addServiceGroupDir(const RouterData & routerDataInfo)
{
    //只是添加模块，没有节点和端口信息  
    string etcdURL = ETCDHOST->getRouterHost();
    if(routerDataInfo.moduletype.empty())
    {
        ETCDPROC_LOGERROR <<FILE_FUN<< toStr(routerDataInfo) << "|param error" << endl;
        return TSEER_REGISTRY_SETSERVER_PARAM_ERROR;
    }
    
    MAKEVALUE(etcdURL,MODULETYPE,routerDataInfo.moduletype);
    if(!routerDataInfo.application.empty())
    {
        MAKEVALUE(etcdURL,APPLICATION,routerDataInfo.application);
    }

    if(!routerDataInfo.service_name.empty())
    {
        MAKEVALUE(etcdURL,SERVICE_NAME,routerDataInfo.service_name);
    }
    
    string value ="dir=true";  
    
    TC_HttpRequest HttpReq;
    HttpReq.setHeader("Content-Type","application/x-www-form-urlencoded");
    HttpReq.setPutRequest(etcdURL,value);

    EtcdReqestInfo etcdReqInfo;
    etcdReqInfo.startTime = TNOWMS;
    etcdReqInfo.current = NULL;
    etcdReqInfo.etcdAction = ETCD_API_ADD;
    etcdReqInfo.routerDataInfo = routerDataInfo;
    HttpReq.getHostPort(etcdReqInfo.etcdHost, etcdReqInfo.etcdPort); 
    
    TC_HttpAsync::RequestCallbackPtr callback = new RequestEtcdCallback(etcdReqInfo);

    ETCDPROC_LOG<< FILE_FUN<<etcdURL<<"|"<<value<< endl;
    APIIMP_LOG<<FILE_FUN<< etcdURL <<"|"<<value<<endl;
    int ret =  _apiAst.doAsyncRequest(HttpReq, callback);
    if(ret == TC_ClientSocket::EM_SUCCESS)
    {
        return 0;
    }
    else
    {
        string info = "send req to " + etcdReqInfo.etcdHost + ":" + TC_Common::tostr(etcdReqInfo.etcdPort) + ",failure,ret="+TC_Common::tostr(ret);
        ETCDPROC_LOGERROR<<FILE_FUN << EtcdReqStr(etcdReqInfo) <<"|"<<info<<endl;
        APIIMP_LOGERROR<< FILE_FUN << EtcdReqStr(etcdReqInfo)  <<"|"<<info<< endl;
        ret = TSEER_REGISTRY_ETCD_SEND_REQ_FAIL;
    }
    return ret;
    
}


int EtcdHandle::addServers(const vector<RouterData> & routerDataList,tars::TarsCurrentPtr current)
{
    size_t iCount = routerDataList.size();
    ETCDPROC_LOG<<FILE_FUN<<current->getIp() << "|size="<<iCount<<"|coming"<<endl;
    APIIMP_LOG<<FILE_FUN<<current->getIp() << "|size="<<iCount<<"|coming"<<endl;
    //原子操作，等待所有更新结束才返回给客户端
    NotifyEventPtr notifyEventPtr = new NotifyEvent();
    //因为要同时添加key和present_state,所以要同时发起两倍
    notifyEventPtr->_run_times = int(iCount);
    
    for(size_t i = 0;i < iCount;i++)
    {
        const RouterData &routerDataInfo = routerDataList[i]; 
        
        string etcdURL = "";
        string value = "";

        //有节点信息,同时肯定有端口信息
        makeUpdateRouteUrl(routerDataInfo,etcdURL,ETCD_API_ADD_PORTLIST);
        
        //这里要判断是否是新增加的端口名,不需要了，新增端口则会新增容器名
        makeRouteValue(routerDataInfo,value);
        
        tars::TC_HttpRequest HttpReq;
        HttpReq.setHeader("Content-Type","application/x-www-form-urlencoded");
        HttpReq.setPutRequest(etcdURL,value);
        
        EtcdReqestInfo etcdReqInfo;
        setEtcdReqInfo(routerDataInfo,ETCD_API_ADD_PORTLIST,current,etcdReqInfo);
        HttpReq.getHostPort(etcdReqInfo.etcdHost,etcdReqInfo.etcdPort); 

        etcdReqInfo.needUpdateRouterList = routerDataList;
        etcdReqInfo.notifyPtr = notifyEventPtr;
        
        ETCDPROC_LOG<<FILE_FUN<< etcdURL <<"|"<<value<<endl;
        APIIMP_LOG<<FILE_FUN<< etcdURL <<"|"<<value<<endl;
        tars::TC_HttpAsync::RequestCallbackPtr callback = new RequestEtcdCallback(etcdReqInfo);

        int ret = _apiAst.doAsyncRequest(HttpReq, callback);
        if(ret == TC_ClientSocket::EM_SUCCESS)
        {
            current->setResponse(false);
        }
        else
        {
            notifyEventPtr->_atomic.inc();
            
            string info = "send req to " + etcdReqInfo.etcdHost + ":" + TC_Common::tostr(etcdReqInfo.etcdPort) + ",failure,ret="+TC_Common::tostr(ret);
            ETCDPROC_LOGERROR<< FILE_FUN <<EtcdReqStr(etcdReqInfo)<<"|"<<info<<endl;
            ret = TSEER_REGISTRY_ETCD_SEND_REQ_FAIL;
        }
    }

    if(notifyEventPtr->_atomic.get() == notifyEventPtr->_run_times)
    {
        //都失败了则响应客户端
        return TSEER_REGISTRY_ETCD_SEND_REQ_FAIL;
    }
    
    return 0;
}


int EtcdHandle::updateServers(const vector<RouterData>& needUpdateRouterList,tars::TarsCurrentPtr &current)
{
    size_t iCount = needUpdateRouterList.size();
    ETCDPROC_LOG<<FILE_FUN<<current->getIp() << "|size="<<iCount<<"|coming"<<endl;
    APIIMP_LOG<<FILE_FUN<<current->getIp() << "|size="<<iCount<<"|coming"<<endl;
    //原子操作，等待所有更新结束才返回给客户端
    NotifyEventPtr notifyEventPtr = new NotifyEvent();
    //因为要同时添加key和present_state,所以要同时发起两倍
    notifyEventPtr->_run_times = int(iCount);
    
    for(size_t i = 0;i < iCount;i++)
    {
        RouterData routerDataInfo = needUpdateRouterList[i]; 
        
        string etcdURL = "";
        //有节点信息,同时肯定有端口信息
        makeUpdateRouteUrl(routerDataInfo,etcdURL,ETCD_API_SET);

        //这里要判断更新了什么属性,属性值为空则表示不更新
        //对比etcd的缓存
        RouterData  stConBufferData;
        int ret = ETCDROUTEDATA->getRouterData(routerDataInfo,stConBufferData);
        if(ISSUCC(ret))
        {
            //没有更新则用缓存里的值
            EMPTYSETVALUE(routerDataInfo.enable_group,stConBufferData.enable_group);
            if(TC_Common::lower(routerDataInfo.enable_group) == "n")
            {
                //清空
                routerDataInfo.ip_group_name = "";
            }
            else
            {
                EMPTYSETVALUE(routerDataInfo.ip_group_name,stConBufferData.ip_group_name);
            }
            
            EMPTYSETVALUE(routerDataInfo.enable_set,stConBufferData.enable_set);
            if(TC_Common::lower(routerDataInfo.enable_set) == "n")
            {
                routerDataInfo.set_name = routerDataInfo.set_area = routerDataInfo.set_group="";
            }
            else
            {
                EMPTYSETVALUE(routerDataInfo.set_name,stConBufferData.set_name);
                EMPTYSETVALUE(routerDataInfo.set_area,stConBufferData.set_area);
                EMPTYSETVALUE(routerDataInfo.set_group,stConBufferData.set_group);
            }
            EMPTYSETVALUE(routerDataInfo.setting_state,stConBufferData.setting_state);
            EMPTYSETVALUE(routerDataInfo.enable_heartbeat, stConBufferData.enable_heartbeat);

            if (TC_Common::lower(routerDataInfo.enable_heartbeat) == "n")
            {
                routerDataInfo.present_state = "active";
            }
            else
            {
                EMPTYSETVALUE(routerDataInfo.present_state, stConBufferData.present_state);
            }

            EMPTYSETVALUE(routerDataInfo.grid_flag,stConBufferData.grid_flag);
            EMPTYSETVALUE(routerDataInfo.weight,stConBufferData.weight);
            EMPTYSETVALUE(routerDataInfo.weighttype,stConBufferData.weighttype);

            if(routerDataInfo.servant != stConBufferData.servant)
            {
                APIIMP_LOGERROR << FILE_FUN<< current->getIp()<<"|etcd cache has no such portname|"<<TC_Common::tostr(routerDataInfo.servant)<<endl;
                return API_NO_PORTNAME;
            }
        }
        else
        {
            APIIMP_LOGERROR << FILE_FUN<< current->getIp()<<"|etcd cache has no such data|"<<toStr(routerDataInfo)<<endl;
            ETCDPROC_LOGERROR<< FILE_FUN <<current->getIp()<<"|etcd cache has no such data|"<<toStr(routerDataInfo)<<endl;
            return API_NO_MODULE;
        }

        string value;
        makeRouteValue(routerDataInfo,value);
        
        tars::TC_HttpRequest HttpReq;
        HttpReq.setHeader("Content-Type","application/x-www-form-urlencoded");
        HttpReq.setPutRequest(etcdURL,value);
        
        EtcdReqestInfo etcdReqInfo;
        setEtcdReqInfo(routerDataInfo,ETCD_API_SET,current,etcdReqInfo);
        HttpReq.getHostPort(etcdReqInfo.etcdHost,etcdReqInfo.etcdPort); 

        etcdReqInfo.needUpdateRouterList = needUpdateRouterList;
        etcdReqInfo.notifyPtr = notifyEventPtr;
        
        HttpReq.getHostPort(etcdReqInfo.etcdHost,etcdReqInfo.etcdPort);
        
        ETCDPROC_LOG<<FILE_FUN<< etcdURL <<"|"<<value<<endl;
        APIIMP_LOG<<FILE_FUN<< etcdURL <<"|"<<value<<endl;
        tars::TC_HttpAsync::RequestCallbackPtr callback = new RequestEtcdCallback(etcdReqInfo);

         ret = _apiAst.doAsyncRequest(HttpReq, callback);
        if(ret == TC_ClientSocket::EM_SUCCESS)
        {
            current->setResponse(false);
        }
        else
        {
            notifyEventPtr->_atomic.inc();
            
            string info = "send req to " + etcdReqInfo.etcdHost + ":" + TC_Common::tostr(etcdReqInfo.etcdPort) + ",failure,ret="+TC_Common::tostr(ret);
            ETCDPROC_LOGERROR<< FILE_FUN <<EtcdReqStr(etcdReqInfo)<<"|"<<info<<endl;
            APIIMP_LOGERROR<< FILE_FUN << EtcdReqStr(etcdReqInfo) <<"|"<<info<< endl;
            ret = TSEER_REGISTRY_ETCD_SEND_REQ_FAIL;
        }
    }
    

    if(notifyEventPtr->_atomic.get() == notifyEventPtr->_run_times)
    {
        //都失败了则响应客户端
        return TSEER_REGISTRY_ETCD_SEND_REQ_FAIL;
    }
    
    return 0;       
}


int EtcdHandle::keepServerAlive(const vector<RouterData>& needUpdateRouterList,tars::TarsCurrentPtr &current)
{
    size_t iCount = needUpdateRouterList.size();
    ETCDPROC_LOG<<FILE_FUN<<current->getIp() << "|size="<<iCount<<"|coming"<<endl;
    APIIMP_LOG<<FILE_FUN<<current->getIp() << "|size="<<iCount<<"|coming"<<endl;
    //原子操作，等待所有更新结束才返回给客户端
    NotifyEventPtr notifyEventPtr = new NotifyEvent();
    //因为要同时添加key和present_state,所以要同时发起两倍
    notifyEventPtr->_run_times = int(iCount);
    
    for(size_t i = 0;i < iCount;i++)
    {
        const RouterData &routerDataInfo = needUpdateRouterList[i]; 
        RouterData  stConBufferData;
        int ret = ETCDROUTEDATA->getRouterData(routerDataInfo, stConBufferData);
        if (ISSUCC(ret))
        {
            if (TC_Common::lower(stConBufferData.enable_heartbeat) == "n")
            {
                continue;
            }
        }
        else
        {
            APIIMP_LOGERROR << FILE_FUN << current->getIp() << "|etcd cache has no such data|" << toStr(routerDataInfo) << endl;
            ETCDPROC_LOGERROR << FILE_FUN << current->getIp() << "|etcd cache has no such data|" << toStr(routerDataInfo) << endl;
            continue;
        }
        
        string etcdURL = "";
        //有节点信息,同时肯定有端口信息
        makeUpdateRouteUrl(routerDataInfo,etcdURL,ETCD_API_UPDATE_BATACH);

        //有调用过一次心跳上报的则加上心跳超时ttl，后续如果不上报，则认为心跳超时
        etcdURL += "?ttl=" + TC_Common::tostr(routerDataInfo.heartbeattimeout);
        
        //这里要判断是否是新增加的端口名,不需要了，新增端口则会新增容器名
        string value = "value=" + routerDataInfo.present_state;
        
        tars::TC_HttpRequest HttpReq;
        HttpReq.setHeader("Content-Type","application/x-www-form-urlencoded");
        HttpReq.setPutRequest(etcdURL,value);
        
        EtcdReqestInfo etcdReqInfo;
        setEtcdReqInfo(routerDataInfo,ETCD_API_UPDATE_BATACH,current,etcdReqInfo);
        HttpReq.getHostPort(etcdReqInfo.etcdHost,etcdReqInfo.etcdPort); 

        etcdReqInfo.needUpdateRouterList = needUpdateRouterList;
        etcdReqInfo.notifyPtr = notifyEventPtr;
        
        HttpReq.getHostPort(etcdReqInfo.etcdHost,etcdReqInfo.etcdPort);
        
        ETCDPROC_LOG<<FILE_FUN<< etcdURL <<"|"<<value<<endl;
        APIIMP_LOG<<FILE_FUN<< etcdURL <<"|"<<value<<endl;
        tars::TC_HttpAsync::RequestCallbackPtr callback = new RequestEtcdCallback(etcdReqInfo);

        ret = _apiAst.doAsyncRequest(HttpReq, callback);
        if(ret == TC_ClientSocket::EM_SUCCESS)
        {
            current->setResponse(false);
        }
        else
        {
            notifyEventPtr->_atomic.inc();
            
            string info = "send req to " + etcdReqInfo.etcdHost + ":" + TC_Common::tostr(etcdReqInfo.etcdPort) + ",failure,ret="+TC_Common::tostr(ret);
            ETCDPROC_LOGERROR<< FILE_FUN <<EtcdReqStr(etcdReqInfo)<<"|"<<info<<endl;
            APIIMP_LOGERROR<< FILE_FUN << EtcdReqStr(etcdReqInfo) <<"|"<<info<< endl;
            ret = TSEER_REGISTRY_ETCD_SEND_REQ_FAIL;
        }
    }
    

    if(notifyEventPtr->_atomic.get() == notifyEventPtr->_run_times)
    {
        //都失败了则响应客户端
        return TSEER_REGISTRY_ETCD_SEND_REQ_FAIL;
    }
    
    return 0;   
}

int EtcdHandle::getServers(const RouterDataRequest & reqInfo,tars::TarsCurrentPtr current)
{
    string etcdURL;
    int ret = makeGetRouteRequestUrl(reqInfo,etcdURL);
    if( ret != 0 )
    {
        ETCDPROC_LOGERROR<<FILE_FUN << toStr(reqInfo)<<"|param invalid|"<<ret<<endl;
        return TSEER_REGISTRY_GETSERVER_PARAM_ERROR;
    }

    TC_HttpRequest HttpReq;

    HttpReq.setGetRequest(etcdURL);

    EtcdReqestInfo etcdReqInfo;
    setEtcdReqInfo(reqInfo,ETCD_API_GET,current,etcdReqInfo);
    etcdReqInfo.routerDataReq = reqInfo;
    HttpReq.getHostPort(etcdReqInfo.etcdHost,etcdReqInfo.etcdPort);

    
    TC_HttpAsync::RequestCallbackPtr callback = new RequestEtcdCallback(etcdReqInfo);

    ETCDPROC_LOG<< FILE_FUN<<etcdURL<< endl;

    ret =  _ast.doAsyncRequest(HttpReq, callback);
    if(ret == TC_ClientSocket::EM_SUCCESS)
    {
        current->setResponse(false);
    }
    else
    {
        string info = "send req to " + etcdReqInfo.etcdHost + ":" + TC_Common::tostr(etcdReqInfo.etcdPort) + ",failure,ret="+TC_Common::tostr(ret);
        ETCDPROC_LOGERROR<<FILE_FUN << EtcdReqStr(etcdReqInfo) <<"|"<<info<<endl;
        ret = TSEER_REGISTRY_ETCD_SEND_REQ_FAIL;
    }
    return ret;    
}


int EtcdHandle::deleteServers(const vector<RouterDataRequest>& delSrvList,tars::TarsCurrentPtr &current)
{
    size_t iCount = delSrvList.size();
    ETCDPROC_LOG<<FILE_FUN<<current->getIp() << "|size="<<iCount<<"|coming"<<endl;
    APIIMP_LOG<<FILE_FUN<<current->getIp() << "|size="<<iCount<<"|coming"<<endl;
    //原子操作，等待所有更新结束才返回给客户端
    NotifyEventPtr notifyEventPtr = new NotifyEvent();
    //因为要同时添加key和present_state,所以要同时发起两倍
    notifyEventPtr->_run_times = int(iCount);
    
    for(size_t i = 0;i < iCount;i++)
    {
        const RouterDataRequest &reqInfo = delSrvList[i]; 

        string etcdURL = ETCDHOST->getRouterHost();
        
        MAKEVALUE(etcdURL,"moduletype",reqInfo.moduletype);
        MAKEVALUE(etcdURL,"application",reqInfo.application);
        MAKEVALUE(etcdURL,"service_name",reqInfo.service_name);
        if(!reqInfo.node_name.empty())
        {
            MAKEVALUE(etcdURL,"node_name",reqInfo.node_name);
            if(!reqInfo.container_name.empty())
            {
                MAKEVALUE(etcdURL,"container_name",reqInfo.container_name);
            }
        }
        
        etcdURL += "?dir=true&recursive=true";

        TC_HttpRequest HttpReq;

        HttpReq.setHeader("Content-Type","application/x-www-form-urlencoded");
        HttpReq.setDeleteRequest(etcdURL,"");
        
        EtcdReqestInfo etcdReqInfo;
        setEtcdReqInfo(reqInfo,ETCD_API_DELETE,current,etcdReqInfo);
        etcdReqInfo.reqRouteList = delSrvList;
        etcdReqInfo.notifyPtr = notifyEventPtr;
        
        HttpReq.getHostPort(etcdReqInfo.etcdHost,etcdReqInfo.etcdPort);
        
        ETCDPROC_LOG<<FILE_FUN<< etcdURL<<endl;
        APIIMP_LOG<<FILE_FUN<< etcdURL<<endl;
        tars::TC_HttpAsync::RequestCallbackPtr callback = new RequestEtcdCallback(etcdReqInfo);

        int ret = _apiAst.doAsyncRequest(HttpReq, callback);
        if(ret == TC_ClientSocket::EM_SUCCESS)
        {
            current->setResponse(false);
        }
        else
        {
            notifyEventPtr->_atomic.inc();
            
            string info = "send req to " + etcdReqInfo.etcdHost + ":" + TC_Common::tostr(etcdReqInfo.etcdPort) + ",failure,ret="+TC_Common::tostr(ret); 
 
            ETCDPROC_LOGERROR<< FILE_FUN <<EtcdReqStr(etcdReqInfo)<<"|"<<info<<endl;
            APIIMP_LOGERROR<< FILE_FUN << EtcdReqStr(etcdReqInfo) <<"|"<<info<< endl;
            ret = TSEER_REGISTRY_ETCD_SEND_REQ_FAIL;
        }
    }
    

    if(notifyEventPtr->_atomic.get() == notifyEventPtr->_run_times)
    {
        //都失败了则响应客户端
        return TSEER_REGISTRY_ETCD_SEND_REQ_FAIL;
    }
    
    return 0;       
}

template<typename T>
void EtcdHandle::setEtcdReqInfo(const T& t,const EtcdAction& etcdAction,const tars::TarsCurrentPtr& current,EtcdReqestInfo& reqInfo)
{
    reqInfo.startTime = TNOWMS;
    reqInfo.current = current;
    reqInfo.etcdAction = etcdAction;
    reqInfo.moduletype = t.moduletype;
    reqInfo.application = t.application;
    reqInfo.service_name = t.service_name;
    reqInfo.node_name = t.node_name;
    reqInfo.container_name = t.container_name;    
}


int EtcdHandle::maxRetryTime()
{
    return _iReqEtcdRetryTime;
}

int EtcdHandle::addBaseServiceInfo(const string &locatorId, const map<string, string> &mapServantEndpoint)
{
    BaseServiceInfo baseServiceInfo;
    baseServiceInfo.locatorId = locatorId;
    baseServiceInfo.tarsVersion = TARS_VERSION;
    baseServiceInfo.enableGroup = "N";
    baseServiceInfo.presentState = "active";

    //用异步方式实现同步操作，逐一添加
    NotifyEventPtr notifyEventPtr = new NotifyEvent();
    notifyEventPtr->_run_times = mapServantEndpoint.size();

    ETCDPROC_LOG << FILE_FUN << "register registry obj size=" << notifyEventPtr->_run_times << endl;
    for (map<string, string>::const_iterator it = mapServantEndpoint.begin(); it != mapServantEndpoint.end(); ++it)
    {
        baseServiceInfo.servant = it->first;
        baseServiceInfo.endpoint = it->second;
        string etcdURL;
        makeUpdateBaseServiceUrl(baseServiceInfo, etcdURL, ETCD_ADD_SELF);

        string value;
        makeBaseServiceValue(baseServiceInfo, value);

        TC_HttpRequest HttpReq;
        HttpReq.setHeader("Content-Type", "application/x-www-form-urlencoded");
        HttpReq.setPutRequest(etcdURL, value);

        //回调需要的信息，里面的路由数据不用填
        EtcdReqestInfo etcdReqInfo;
        etcdReqInfo.startTime = TNOWMS;
        etcdReqInfo.etcdAction = ETCD_ADD_SELF;
        etcdReqInfo.current = NULL;
        HttpReq.getHostPort(etcdReqInfo.etcdHost, etcdReqInfo.etcdPort);
        etcdReqInfo.notifyPtr = notifyEventPtr;
        ETCDPROC_LOG << FILE_FUN << etcdURL << "|" << value << endl;

        tars::TC_HttpAsync::RequestCallbackPtr callback = new RequestEtcdCallback(etcdReqInfo);

        int ret = _ast.doAsyncRequest(HttpReq, callback);
        if (ret != TC_ClientSocket::EM_SUCCESS)
        {
            notifyEventPtr->_atomic.inc();
            //记录错误个数
            notifyEventPtr->_atomicFault.inc();
            string info = "send req to " + etcdReqInfo.etcdHost + ":" + TC_Common::tostr(etcdReqInfo.etcdPort) + ",failure,ret=" + TC_Common::tostr(ret);

            ETCDPROC_LOGERROR << FILE_FUN << EtcdReqStr(etcdReqInfo) << "|" << info << endl;
            return TSEER_REGISTRY_ETCD_SEND_REQ_FAIL;          //因网络原因添加一个失败直接返回错误
        }
    }

    if (notifyEventPtr->_run_times > 0 && 
        notifyEventPtr->_atomicFault.get() == notifyEventPtr->_run_times)
    {
        string info = "add self objs to base_service_info fail";
        ETCDPROC_LOGERROR << FILE_FUN << info << endl;
        return TSEER_REGISTRY_ETCD_SEND_REQ_FAIL;
    }

    ETCDPROC_LOG << FILE_FUN << "end" << endl;
    return 0;
}

int EtcdHandle::updateBaseServiceInfo(const string &locatorId, const map<string, string> &mapServantEndpoint)
{
    BaseServiceInfo baseServiceInfo;
    baseServiceInfo.locatorId = locatorId;

    //用异步方式实现同步操作，逐一上报（为了兼容老代码）
    NotifyEventPtr notifyEventPtr = new NotifyEvent();
    notifyEventPtr->_run_times = mapServantEndpoint.size();
    ETCDPROC_LOG << FILE_FUN << "need update self state size=" << notifyEventPtr->_run_times << endl;

    for (map<string, string>::const_iterator it = mapServantEndpoint.begin(); it != mapServantEndpoint.end(); ++it)
    {
        baseServiceInfo.servant = it->first;
        baseServiceInfo.endpoint = it->second;
        string etcdURL;
        makeUpdateBaseServiceUrl(baseServiceInfo, etcdURL, ETCD_UPDATE_SELF_STATE);

        string value = "value=active";

        tars::TC_HttpRequest HttpReq;
        HttpReq.setHeader("Content-Type", "application/x-www-form-urlencoded");
        HttpReq.setPutRequest(etcdURL, value);

        //回调需要的信息，里面的路由数据不用填
        EtcdReqestInfo etcdReqInfo;
        etcdReqInfo.startTime = TNOWMS;
        etcdReqInfo.etcdAction = ETCD_UPDATE_SELF_STATE;
        etcdReqInfo.current = NULL;
        HttpReq.getHostPort(etcdReqInfo.etcdHost, etcdReqInfo.etcdPort);
        etcdReqInfo.notifyPtr = notifyEventPtr;

        ETCDPROC_LOG << FILE_FUN << etcdURL << "|" << value << endl;
        tars::TC_HttpAsync::RequestCallbackPtr callback = new RequestEtcdCallback(etcdReqInfo);

        int ret = _ast.doAsyncRequest(HttpReq, callback);
        if (ret != TC_ClientSocket::EM_SUCCESS)
        {
            notifyEventPtr->_atomic.inc();
            //记录错误个数
            notifyEventPtr->_atomicFault.inc();
            string info = "send req to " + etcdReqInfo.etcdHost + ":" + TC_Common::tostr(etcdReqInfo.etcdPort) + ",failure,ret=" + TC_Common::tostr(ret);

            ETCDPROC_LOGERROR << FILE_FUN << EtcdReqStr(etcdReqInfo) << "|" << info << endl;
            ret = TSEER_REGISTRY_ETCD_SEND_REQ_FAIL;
        }
    }

    if (notifyEventPtr->_run_times > 0 && notifyEventPtr->_atomicFault.get() == notifyEventPtr->_run_times)
    {
        string info = "update all self objs state to active fail";
        ETCDPROC_LOGERROR << FILE_FUN << "update all self objs state to active fail" << endl;
        return TSEER_REGISTRY_ETCD_SEND_REQ_FAIL;
    }

    ETCDPROC_LOG << FILE_FUN << "end" << endl;
    return 0;
}

int EtcdHandle::addNodeInfo(const NodeServiceInfo &nodeInfo)
{
    if (!isNodeServiceInfoValid(nodeInfo))
    {
        ETCDPROC_LOGERROR << FILE_FUN << nodeInfo.nodeName << "|" << nodeInfo.nodeNetworkId << "|" 
            << nodeInfo.nodeObj << "|pram error" << endl;
        return TSEER_REGISTRY_PARAM_ERROR;
    }

    string etcdURL;
    makeUpdateNodeUrl(nodeInfo.nodeNetworkId, nodeInfo.nodeName, etcdURL, ETCD_ADD_NODE_INFO);
    string value;
    makeNodeValue(nodeInfo, value);

    TC_HttpRequest HttpReq;
    HttpReq.setHeader("Content-Type", "application/x-www-form-urlencoded");
    HttpReq.setPutRequest(etcdURL, value);

    EtcdReqestInfo etcdReqInfo;
    etcdReqInfo.startTime = TNOWMS;
    etcdReqInfo.etcdAction = ETCD_ADD_NODE_INFO;
    etcdReqInfo.current = NULL;
    HttpReq.getHostPort(etcdReqInfo.etcdHost, etcdReqInfo.etcdPort);
    
    TC_HttpAsync::RequestCallbackPtr callback = new RequestEtcdCallback(etcdReqInfo);
    
    ETCDPROC_LOG << FILE_FUN << etcdURL << "|" << value << endl;

    int ret = _ast.doAsyncRequest(HttpReq, callback);
    if (ret == TC_ClientSocket::EM_SUCCESS)
    {
        return 0;
    }
    else
    {
        string info = "send req to " + etcdReqInfo.etcdHost + ":" + TC_Common::tostr(etcdReqInfo.etcdPort) + ",failure,ret=" + TC_Common::tostr(ret);
        ETCDPROC_LOGERROR << FILE_FUN << EtcdReqStr(etcdReqInfo) << "|" << info << endl;
        ret = TSEER_REGISTRY_ETCD_SEND_REQ_FAIL;
    }
    return ret;
}

int EtcdHandle::delNodeInfo(const string &nodeName, const string &networkId)
{
    string etcdURL = ETCDHOST->getNodeInfoHost();
    MAKEVALUE(etcdURL, NODE_NETWORKID, networkId);
    MAKEVALUE(etcdURL, NODE_NAME, nodeName);
    etcdURL += "?dir=true&recursive=true";

    ETCDPROC_LOG << FILE_FUN << etcdURL << endl;
    APIIMP_LOG << FILE_FUN << etcdURL << endl;

    TC_HttpRequest HttpReq;
    HttpReq.setHeader("Content-Type", "application/x-www-form-urlencoded");
    HttpReq.setDeleteRequest(etcdURL, "");

    EtcdReqestInfo etcdReqInfo;
    etcdReqInfo.startTime = TNOWMS;
    etcdReqInfo.etcdAction = ETCD_DEL_NODE_INFO;
    etcdReqInfo.current = NULL;
    HttpReq.getHostPort(etcdReqInfo.etcdHost, etcdReqInfo.etcdPort);
    tars::TC_HttpAsync::RequestCallbackPtr callback = new RequestEtcdCallback(etcdReqInfo);

    int ret = _ast.doAsyncRequest(HttpReq, callback);
    if (ret == TC_ClientSocket::EM_SUCCESS)
    {
        return 0;
    }
    else
    {
        string info = "send req to " + etcdReqInfo.etcdHost + ":" + TC_Common::tostr(etcdReqInfo.etcdPort) + ",failure,ret=" + TC_Common::tostr(ret);
        ETCDPROC_LOGERROR << FILE_FUN << EtcdReqStr(etcdReqInfo) << "|" << info << endl;
        ret = TSEER_REGISTRY_ETCD_SEND_REQ_FAIL;
    }
    return ret;
}

int EtcdHandle::keepAlive(const string& name, const LoadInfo& li)
{
    return -1;
}

int EtcdHandle::getAllNodes(vector<NodeServiceInfo> &nodeInfo, const string &networkId)
{
    string etcdURL = ETCDHOST->getNodeInfoHost();
    etcdURL += string("/") + NODE_NETWORKID + "=" + networkId + "?recursive=true";

    TC_HttpRequest HttpReq;
    HttpReq.setGetRequest(etcdURL);

    EtcdReqestInfo getEtcdReqInfo;
    getEtcdReqInfo.current = NULL;
    getEtcdReqInfo.etcdAction = ETCD_GET_ALL_SEER_AGENTS;
    getEtcdReqInfo.startTime = TNOWMS;
    HttpReq.getHostPort(getEtcdReqInfo.etcdHost, getEtcdReqInfo.etcdPort);
    NotifyEventPtr notifyEventPtr = new NotifyEvent();      //解析好了数据才返回
    notifyEventPtr->_run_times = 1;
    getEtcdReqInfo.notifyPtr = notifyEventPtr;

    TC_HttpAsync::RequestCallbackPtr callback = new RequestEtcdCallback(getEtcdReqInfo);
    ETCDPROC_LOG << FILE_FUN << etcdURL << endl;

    int ret = _ast.doAsyncRequest(HttpReq, callback);
    if (ret == TC_ClientSocket::EM_SUCCESS)
    {
        //等待结果
        {
            TC_ThreadLock::Lock lock(notifyEventPtr->_monitor);
            while (!notifyEventPtr->_canWake)
            {
                notifyEventPtr->_monitor.wait();
            }
        }
        
        RequestEtcdCallbackPtr etcdCallBackPtr = TC_AutoPtr<RequestEtcdCallback>::dynamicCast(callback);
        const vector<NodeServiceInfo> &result = etcdCallBackPtr->_etcdReqInfo.nodeServiceInfo;
        for (size_t i = 0; i < result.size(); ++i)
        {
            nodeInfo.push_back(result[i]);
        }

        return 0;
    }
    else
    {
        string info = "send req to " + getEtcdReqInfo.etcdHost + ":" + TC_Common::tostr(getEtcdReqInfo.etcdPort) + ",failure,ret=" + TC_Common::tostr(ret);

        ETCDPROC_LOGERROR << FILE_FUN << EtcdReqStr(getEtcdReqInfo) << "|" << info << endl;
        return TSEER_REGISTRY_ETCD_SEND_REQ_FAIL;
    }
}

int EtcdHandle::getAllTimeOutNodes(vector<NodeServiceInfo> &nodeList, const string &networkId, const unsigned int interval)
{
    return 0;
}

int EtcdHandle::getOneSeerAgent(const string &nodeName, NodeServiceInfo &nodeServiceInfo)
{
    string etcdURL = ETCDHOST->getNodeInfoHost();
    MAKEVALUE(etcdURL, NODE_NETWORKID, TSEER_AGENT_NET_WORK_ID);
    MAKEVALUE(etcdURL, NODE_NAME, nodeName);

    TC_HttpRequest HttpReq;
    HttpReq.setHeader("Content-Type", "application/x-www-form-urlencoded");
    HttpReq.setGetRequest(etcdURL);

    EtcdReqestInfo getEtcdReqInfo;
    getEtcdReqInfo.current = NULL;
    getEtcdReqInfo.etcdAction = ETCD_GET_ONE_SEER_AGENT;
    getEtcdReqInfo.startTime = TNOWMS;
    HttpReq.getHostPort(getEtcdReqInfo.etcdHost, getEtcdReqInfo.etcdPort);
    NotifyEventPtr notifyEventPtr = new NotifyEvent();      //解析好了数据才返回
    notifyEventPtr->_run_times = 1;
    getEtcdReqInfo.notifyPtr = notifyEventPtr;
    TC_HttpAsync::RequestCallbackPtr callback = new RequestEtcdCallback(getEtcdReqInfo);

    ETCDPROC_LOG << FILE_FUN << etcdURL << endl;
    int ret = _ast.doAsyncRequest(HttpReq, callback);
    if (ret == TC_ClientSocket::EM_SUCCESS)
    {
        //等待结果
        {
            TC_ThreadLock::Lock lock(notifyEventPtr->_monitor);
            while (!notifyEventPtr->_canWake)
            {
                notifyEventPtr->_monitor.wait();
            }
        }

        RequestEtcdCallbackPtr etcdCallBackPtr = TC_AutoPtr<RequestEtcdCallback>::dynamicCast(callback);
        const vector<NodeServiceInfo> &result = etcdCallBackPtr->_etcdReqInfo.nodeServiceInfo;
        if (result.empty())
        {
            ETCDPROC_LOG << FILE_FUN << EtcdReqStr(getEtcdReqInfo)
                << "|can't find seer agent in:" << nodeName << endl;
            return TSEER_REGISTRY_ETCD_DATA_NOTFOUND;
        }
        else
        {
            nodeServiceInfo = result[0];
            return TSEER_SERVERSUCCESS;
        }
    }
    else
    {
        string info = "send req to " + getEtcdReqInfo.etcdHost + ":" + TC_Common::tostr(getEtcdReqInfo.etcdPort) + ",failure,ret=" + TC_Common::tostr(ret);

        ETCDPROC_LOGERROR << FILE_FUN << EtcdReqStr(getEtcdReqInfo) << "|" << info << endl;

        return TSEER_REGISTRY_ETCD_SEND_REQ_FAIL;
    }
}

int EtcdHandle::getAllAliveSeerAgents(std::set<string> &aliveNodes)
{
    vector<NodeServiceInfo> nodeInfo;
    if (getAllNodes(nodeInfo, TSEER_AGENT_NET_WORK_ID))
    {
        TSEER_LOG("debug")->debug() << FILE_FUN << endl;
        return -1;
    }

    for (vector<NodeServiceInfo>::iterator it = nodeInfo.begin(); it != nodeInfo.end(); ++it)
    {
        if (it->presentState == "active")
        {
            aliveNodes.insert(it->nodeName);
        }
    }

    return 0;
}

int EtcdHandle::keepNodeAlive(const NodeInstanceInfo& stNodeInstanceInfo, const string &networkId)
{
    //先更新心跳时间key
    NodeServiceInfo nodeServiceInfo;
    if(getOneSeerAgent(stNodeInstanceInfo.nodeName,nodeServiceInfo))
    {
        ETCDPROC_LOG << FILE_FUN << "getOneSeerAgent failure" << endl;
        return TSEER_REGISTRY_ETCD_DATA_NOTFOUND;
    }

    nodeServiceInfo.version = stNodeInstanceInfo.version;
    nodeServiceInfo.ostype = stNodeInstanceInfo.osversion;
    nodeServiceInfo.locator = stNodeInstanceInfo.locator;
    if(addNodeInfo(nodeServiceInfo))
    {
        ETCDPROC_LOG << FILE_FUN << "addNodeInfo failure" << endl;
    }
    
    //在更新状态key
    string etcdURL = ETCDHOST->getNodeInfoHost();
    makeUpdateNodeUrl(networkId, stNodeInstanceInfo.nodeName, etcdURL, ETCD_KEEP_NODE_ALIVE);
    string value = "value=active";

    TC_HttpRequest HttpReq;
    HttpReq.setHeader("Content-Type", "application/x-www-form-urlencoded");
    HttpReq.setPutRequest(etcdURL, value);

    EtcdReqestInfo getEtcdReqInfo;
    getEtcdReqInfo.current = NULL;
    getEtcdReqInfo.etcdAction = ETCD_KEEP_NODE_ALIVE;
    getEtcdReqInfo.startTime = TNOWMS;
    HttpReq.getHostPort(getEtcdReqInfo.etcdHost, getEtcdReqInfo.etcdPort);
    getEtcdReqInfo.notifyPtr = NULL;          //无需等待结果
    TC_HttpAsync::RequestCallbackPtr callback = new RequestEtcdCallback(getEtcdReqInfo);

    ETCDPROC_LOG << FILE_FUN << etcdURL << endl;
    int ret = _ast.doAsyncRequest(HttpReq, callback);
    if (ret == TC_ClientSocket::EM_SUCCESS)
    {
        return 0;
    }
    else
    {
        string info = "send req to " + getEtcdReqInfo.etcdHost + ":" + TC_Common::tostr(getEtcdReqInfo.etcdPort) + ",failure,ret=" + TC_Common::tostr(ret);

        ETCDPROC_LOGERROR << FILE_FUN << EtcdReqStr(getEtcdReqInfo) << "|" << info << endl;

        return TSEER_REGISTRY_ETCD_SEND_REQ_FAIL;
    }
}

int EtcdHandle::updateNodeStateBatch(vector<NodeServiceInfo> &nodeList)
{
    return 0;
}

int EtcdHandle::addIdcGroupRule(const string &groupName, const vector<string> &allowIps, TarsCurrentPtr current)
{
    string etcdURL = ETCDHOST->getIdcRuleHost();

    //生成新的分组ID
    string key = "";
    int ret = isNameValidGenNewIdcGroupId(groupName, key);
    if (ret)
    {
        ETCDPROC_LOGERROR << FILE_FUN << "generate new group id error" << endl;
        return ret;
    }
    
    ETCDPROC_LOG  << FILE_FUN << "new group id:" << key << endl;
    MAKEVALUE(etcdURL, GROUP_ID, key);

    IDCGroupInfo groupInfo;
    groupInfo.groupName = groupName;
    groupInfo.allowIp = allowIps;
    
    string value;
    makeGroupInfoValue(groupInfo, value);

    TC_HttpRequest HttpReq;
    HttpReq.setHeader("Content-Type", "application/x-www-form-urlencoded");
    HttpReq.setPutRequest(etcdURL, value);

    EtcdReqestInfo etcdReqInfo;
    etcdReqInfo.etcdAction = ETCD_ADD_IDC_RULE;
    etcdReqInfo.startTime = TNOWMS;
    etcdReqInfo.current = current;
    HttpReq.getHostPort(etcdReqInfo.etcdHost, etcdReqInfo.etcdPort);
    etcdReqInfo.groupName = groupName;
    etcdReqInfo.allowIps = allowIps;
    etcdReqInfo.newGroupId = key;        //分组id要放入其中供回包使用

    TC_HttpAsync::RequestCallbackPtr callback = new RequestEtcdCallback(etcdReqInfo);
    ETCDPROC_LOG << FILE_FUN << etcdURL << "|" << value << endl;

    ret = _ast.doAsyncRequest(HttpReq, callback);
    if (ret == TC_ClientSocket::EM_SUCCESS)
    {
        current->setResponse(false);
    }
    else
    {
        string info = "send req to " + etcdReqInfo.etcdHost + ":" + TC_Common::tostr(etcdReqInfo.etcdPort) + ",failure,ret=" + TC_Common::tostr(ret);

        ETCDPROC_LOGERROR << FILE_FUN << EtcdReqStr(etcdReqInfo) << "|" << info << endl;

        ret = TSEER_REGISTRY_ETCD_SEND_REQ_FAIL;
    }
    return ret;
}

int EtcdHandle::modifyIdcGroupRule(const string &groupId, const vector<string> &allowIps, TarsCurrentPtr current)
{
    //先取出该id的信息，如果分组不存在则返回错误
    IDCGroupInfo oldIdcGroupInfo;
    int ret = getOneIdcGroupRule(groupId, oldIdcGroupInfo);
    if (ret == -2)
    {
        return -2;
    }
    else if (ret != 0)
    {
        TSEER_LOG("debug")->debug() << FILE_FUN << "get idc group info for:" << groupId << " failed" << endl;
        return -1;
    }

    string etcdURL = ETCDHOST->getIdcRuleHost();
    string value;
    MAKEVALUE(etcdURL, GROUP_ID, groupId);

    IDCGroupInfo groupInfo;
    groupInfo.groupName = oldIdcGroupInfo.groupName;
    groupInfo.allowIp = allowIps;
    makeGroupInfoValue(groupInfo, value);

    TC_HttpRequest HttpReq;
    HttpReq.setHeader("Content-Type", "application/x-www-form-urlencoded");
    HttpReq.setPutRequest(etcdURL, value);

    EtcdReqestInfo etcdReqInfo;
    etcdReqInfo.etcdAction = ETCD_MODIFY_IDC_RULE;
    etcdReqInfo.startTime = TNOWMS;
    etcdReqInfo.current = current;
    HttpReq.getHostPort(etcdReqInfo.etcdHost, etcdReqInfo.etcdPort);
    etcdReqInfo.groupId = groupId;        //要带上group id，如果失败可以重试
    etcdReqInfo.groupName = oldIdcGroupInfo.groupName;;
    etcdReqInfo.allowIps = allowIps;

    TC_HttpAsync::RequestCallbackPtr callback = new RequestEtcdCallback(etcdReqInfo);
    ETCDPROC_LOG << FILE_FUN << etcdURL << "|" << value << endl;

    ret = _ast.doAsyncRequest(HttpReq, callback);
    if (ret == TC_ClientSocket::EM_SUCCESS)
    {
        current->setResponse(false);
    }
    else
    {
        string info = "send req to " + etcdReqInfo.etcdHost + ":" + TC_Common::tostr(etcdReqInfo.etcdPort) + ",failure,ret=" + TC_Common::tostr(ret);

        ETCDPROC_LOGERROR << FILE_FUN << EtcdReqStr(etcdReqInfo) << "|" << info << endl;
        ret = TSEER_REGISTRY_ETCD_SEND_REQ_FAIL;
    }
    return ret;
}

int EtcdHandle::delIdcGroupRule(const vector<string> &groupsIds, TarsCurrentPtr current)
{
    NotifyEventPtr notifyEventPtr = new NotifyEvent();
    notifyEventPtr->_run_times = groupsIds.size();

    for (size_t i = 0; i < groupsIds.size(); ++i)
    {
        string etcdURL = ETCDHOST->getIdcRuleHost();
        MAKEVALUE(etcdURL, GROUP_ID, groupsIds[i]);
        etcdURL += "?dir=true&recursive=true";

        TC_HttpRequest HttpReq;
        HttpReq.setHeader("Content-Type", "application/x-www-form-urlencoded");
        HttpReq.setDeleteRequest(etcdURL, "");

        EtcdReqestInfo etcdReqInfo;
        etcdReqInfo.current = current;
        etcdReqInfo.etcdAction = ETCD_DEL_IDC_RULE;
        etcdReqInfo.startTime = TNOWMS;
        HttpReq.getHostPort(etcdReqInfo.etcdHost, etcdReqInfo.etcdPort);
        etcdReqInfo.notifyPtr = notifyEventPtr;
        etcdReqInfo.delGroupId = groupsIds;

        tars::TC_HttpAsync::RequestCallbackPtr callback = new RequestEtcdCallback(etcdReqInfo);
        ETCDPROC_LOG << FILE_FUN << etcdURL << endl;
    
        int ret = _ast.doAsyncRequest(HttpReq, callback);
        if (ret == TC_ClientSocket::EM_SUCCESS)
        {
            current->setResponse(false);
        }
        else
        {
            notifyEventPtr->_atomic.inc();
            notifyEventPtr->_atomicFault.inc();
            string info = "send req to " + etcdReqInfo.etcdHost + ":" + TC_Common::tostr(etcdReqInfo.etcdPort) + ",failure,ret=" + TC_Common::tostr(ret);

            ETCDPROC_LOGERROR << FILE_FUN << EtcdReqStr(etcdReqInfo) << "|" << info << endl;
        }
    }

    if (notifyEventPtr->_atomicFault.get() == notifyEventPtr->_run_times)
    {
        //全部出错，返回错误让上一层调用回包
        return TSEER_REGISTRY_ETCD_SEND_REQ_FAIL;
    }
    else
    {
        return 0;
    }
}

int EtcdHandle::getOneIdcGroupRule(const string &groupId, IDCGroupInfo &idcGroupInfoList)
{
    string etcdURL = ETCDHOST->getIdcRuleHost();
    etcdURL += "/group_id=" + groupId;

    TC_HttpRequest HttpReq;
    HttpReq.setHeader("Content-Type", "application/x-www-form-urlencoded");
    HttpReq.setGetRequest(etcdURL);

    NotifyEventPtr notifyEventPtr = new NotifyEvent();
    notifyEventPtr->_run_times = 1;
    
    EtcdReqestInfo etcdReqInfo;
    etcdReqInfo.startTime = TNOWMS;
    etcdReqInfo.etcdAction = ETCD_GET_ONE_IDC_RULE;
    etcdReqInfo.current = NULL;
    etcdReqInfo.notifyPtr = notifyEventPtr;
    HttpReq.getHostPort(etcdReqInfo.etcdHost, etcdReqInfo.etcdPort);

    TC_HttpAsync::RequestCallbackPtr callback = new RequestEtcdCallback(etcdReqInfo);
    ETCDPROC_LOG << FILE_FUN << etcdURL << endl;

    int ret = _ast.doAsyncRequest(HttpReq, callback);
    if (ret == TC_ClientSocket::EM_SUCCESS)
    {
        {
            TC_ThreadLock::Lock sync(notifyEventPtr->_monitor);
            //等待结果返回
            while (!notifyEventPtr->_canWake)
            {
                notifyEventPtr->_monitor.wait();
            }
        }

        RequestEtcdCallbackPtr etcdCallBackPtr = TC_AutoPtr<RequestEtcdCallback>::dynamicCast(callback);
        const EtcdReqestInfo &backEtcdReqInfo = etcdCallBackPtr->_etcdReqInfo;
        if (backEtcdReqInfo.idcGroupInfoList.empty())
        {
            ETCDPROC_LOG << FILE_FUN << "Not found IDC group info for :" << groupId << endl;
            return -2;      //如果找不到则返回-2
        }
        else
        {
            idcGroupInfoList = backEtcdReqInfo.idcGroupInfoList[0];
            return 0;
        }
    }
    else
    {
        string info = "send req to " + etcdReqInfo.etcdHost + ":" + TC_Common::tostr(etcdReqInfo.etcdPort) + ",failure,ret=" + TC_Common::tostr(ret);

        ETCDPROC_LOGERROR << FILE_FUN << EtcdReqStr(etcdReqInfo) << "|" << info << endl;
        ret = TSEER_REGISTRY_ETCD_SEND_REQ_FAIL;
    }

    return ret;
}

int EtcdHandle::getAllIdcGroupRule(vector<IDCGroupInfo> &idcGroupInfoList)
{
    string etcdURL = ETCDHOST->getIdcRuleHost();
    etcdURL += "?recursive=true";

    TC_HttpRequest HttpReq;
    HttpReq.setHeader("Content-Type", "application/x-www-form-urlencoded");
    HttpReq.setGetRequest(etcdURL);

    NotifyEventPtr notifyEventPtr = new NotifyEvent();
    notifyEventPtr->_run_times = 1;
    
    EtcdReqestInfo etcdReqInfo;
    etcdReqInfo.startTime = TNOWMS;
    etcdReqInfo.etcdAction = ETCD_GET_ALL_IDC_RULE;
    etcdReqInfo.current = NULL;
    etcdReqInfo.notifyPtr = notifyEventPtr;
    HttpReq.getHostPort(etcdReqInfo.etcdHost, etcdReqInfo.etcdPort);

    TC_HttpAsync::RequestCallbackPtr callback = new RequestEtcdCallback(etcdReqInfo);
    ETCDPROC_LOG << FILE_FUN << etcdURL << endl;

    int ret = _ast.doAsyncRequest(HttpReq, callback);
    if (ret == TC_ClientSocket::EM_SUCCESS)
    {
        {
            TC_ThreadLock::Lock sync(notifyEventPtr->_monitor);
            while (!notifyEventPtr->_canWake)
            {
                notifyEventPtr->_monitor.wait();
            }
        }

        RequestEtcdCallbackPtr etcdCallBackPtr = TC_AutoPtr<RequestEtcdCallback>::dynamicCast(callback);
        const EtcdReqestInfo &backEtcdReqInfo = etcdCallBackPtr->_etcdReqInfo;
        idcGroupInfoList.clear();
        for (size_t i = 0; i < backEtcdReqInfo.idcGroupInfoList.size(); ++i)
        {
            idcGroupInfoList.push_back(backEtcdReqInfo.idcGroupInfoList[i]);
        }

        return 0;
    }
    else
    {
        string info = "send req to " + etcdReqInfo.etcdHost + ":" + TC_Common::tostr(etcdReqInfo.etcdPort) + ",failure,ret=" + TC_Common::tostr(ret);

        ETCDPROC_LOGERROR << FILE_FUN << EtcdReqStr(etcdReqInfo) << "|" << info << endl;
        ret = TSEER_REGISTRY_ETCD_SEND_REQ_FAIL;
    }

    return ret;
}

int EtcdHandle::addIdcPriority(int order, const string &priorityName, const vector<string> &groupIdList, TarsCurrentPtr current)
{
    string etcdURL = ETCDHOST->getIdcPriorityHost();
    string value;

    //根据priorityName和此刻时间拼成的字符串的MD5值组成id
    string nowStr = TC_Common::tostr<int64_t>(TC_Common::now2ms());
    string key = TC_MD5::md5str(priorityName + nowStr);
    MAKEVALUE(etcdURL, PRIORITY_ID, key);

    IDCPriority priorityInfo;
    priorityInfo.priorityName = priorityName;
    priorityInfo.groupList = groupIdList;
    priorityInfo.order = order;
    makePriorityInfoValue(priorityInfo, value);

    TC_HttpRequest HttpReq;
    HttpReq.setHeader("Content-Type", "application/x-www-form-urlencoded");
    HttpReq.setPutRequest(etcdURL, value);

    EtcdReqestInfo etcdReqInfo;
    etcdReqInfo.etcdAction = ETCD_ADD_IDC_PRIORITY;
    etcdReqInfo.startTime = TNOWMS;
    etcdReqInfo.current = current;
    HttpReq.getHostPort(etcdReqInfo.etcdHost, etcdReqInfo.etcdPort);
    etcdReqInfo.order = order;
    etcdReqInfo.priorityName = priorityName;
    etcdReqInfo.groupIdList = groupIdList;
    etcdReqInfo.newPriorityId = key;     //优先级ID，供回包使用

    TC_HttpAsync::RequestCallbackPtr callback = new RequestEtcdCallback(etcdReqInfo);
    ETCDPROC_LOG << FILE_FUN << etcdURL << "|" << value << endl;

    int ret = _ast.doAsyncRequest(HttpReq, callback);
    if (ret == TC_ClientSocket::EM_SUCCESS)
    {
        current->setResponse(false);
    }
    else
    {
        string info = "send req to " + etcdReqInfo.etcdHost + ":" + TC_Common::tostr(etcdReqInfo.etcdPort) + ",failure,ret=" + TC_Common::tostr(ret);

        ETCDPROC_LOGERROR << FILE_FUN << EtcdReqStr(etcdReqInfo) << "|" << info << endl;
        ret = TSEER_REGISTRY_ETCD_SEND_REQ_FAIL;
    }
    return ret;
}

int EtcdHandle::modifyIdcPriority(const string &priorityId, int order, const string &priorityName, const vector<string> &groupIdList, TarsCurrentPtr current)
{
    string etcdURL = ETCDHOST->getIdcPriorityHost();
    MAKEVALUE(etcdURL, PRIORITY_ID, priorityId);
    etcdURL += "?prevExist=true";
    
    string value;
    IDCPriority priorityInfo;
    priorityInfo.priorityName = priorityName;
    priorityInfo.order = order;
    priorityInfo.groupList = groupIdList;
    makePriorityInfoValue(priorityInfo, value);

    TC_HttpRequest HttpReq;
    HttpReq.setHeader("Content-Type", "application/x-www-form-urlencoded");
    HttpReq.setPutRequest(etcdURL, value);

    EtcdReqestInfo etcdReqInfo;
    etcdReqInfo.etcdAction = ETCD_MODIFY_IDC_PRIORITY;
    etcdReqInfo.startTime = TNOWMS;
    etcdReqInfo.current = current;
    HttpReq.getHostPort(etcdReqInfo.etcdHost, etcdReqInfo.etcdPort);
    etcdReqInfo.priorityId = priorityId;
    etcdReqInfo.order = order;
    etcdReqInfo.priorityName = priorityName;
    etcdReqInfo.groupIdList = groupIdList;

    TC_HttpAsync::RequestCallbackPtr callback = new RequestEtcdCallback(etcdReqInfo);
    ETCDPROC_LOG << FILE_FUN << etcdURL << "|" << value << endl;

    int ret = _ast.doAsyncRequest(HttpReq, callback);
    if (ret == TC_ClientSocket::EM_SUCCESS)
    {
        current->setResponse(false);
    }
    else
    {
        string info = "send req to " + etcdReqInfo.etcdHost + ":" + TC_Common::tostr(etcdReqInfo.etcdPort) + ",failure,ret=" + TC_Common::tostr(ret);
        ETCDPROC_LOGERROR << FILE_FUN << EtcdReqStr(etcdReqInfo) << "|" << info << endl;
        ret = TSEER_REGISTRY_ETCD_SEND_REQ_FAIL;
    }
    return ret;
}

int EtcdHandle::delIdcPriority(const vector<string> &priorityIds, TarsCurrentPtr current)
{
    NotifyEventPtr notifyEventPtr = new NotifyEvent();
    notifyEventPtr->_run_times = priorityIds.size();

    for (size_t i = 0; i < priorityIds.size(); ++i)
    {
        string etcdURL = ETCDHOST->getIdcPriorityHost();
        MAKEVALUE(etcdURL, PRIORITY_ID, priorityIds[i]);
        etcdURL += "?dir=true&recursive=true";

        TC_HttpRequest HttpReq;
        HttpReq.setHeader("Content-Type", "application/x-www-form-urlencoded");
        HttpReq.setDeleteRequest(etcdURL, "");

        EtcdReqestInfo etcdReqInfo;
        etcdReqInfo.current = current;
        etcdReqInfo.etcdAction = ETCD_DEL_IDC_PRIORITY;
        etcdReqInfo.startTime = TNOWMS;
        HttpReq.getHostPort(etcdReqInfo.etcdHost, etcdReqInfo.etcdPort);
        etcdReqInfo.notifyPtr = notifyEventPtr;
        etcdReqInfo.delPriorityIds = priorityIds;

        tars::TC_HttpAsync::RequestCallbackPtr callback = new RequestEtcdCallback(etcdReqInfo);
        ETCDPROC_LOG << FILE_FUN << etcdURL << endl;

        int ret = _ast.doAsyncRequest(HttpReq, callback);
        if (ret == TC_ClientSocket::EM_SUCCESS)
        {
            current->setResponse(false);
        }
        else
        {
            notifyEventPtr->_atomic.inc();
            notifyEventPtr->_atomicFault.inc();
            string info = "send req to " + etcdReqInfo.etcdHost + ":" + TC_Common::tostr(etcdReqInfo.etcdPort) + ",failure,ret=" + TC_Common::tostr(ret);
            ETCDPROC_LOGERROR << FILE_FUN << EtcdReqStr(etcdReqInfo) << "|" << info << endl;
        }
    }

    if (notifyEventPtr->_atomicFault.get() == notifyEventPtr->_run_times)
    {
        //全部出错，返回错误让上一层调用回包
        return TSEER_REGISTRY_ETCD_SEND_REQ_FAIL;
    }
    else
    {
        return 0;
    }
}

int EtcdHandle::getAllIdcPriority(vector<IDCPriority> &idcPriorityInfo)
{
    string etcdURL = ETCDHOST->getIdcPriorityHost();
    etcdURL += "?recursive=true";

    TC_HttpRequest HttpReq;
    HttpReq.setHeader("Content-Type", "application/x-www-form-urlencoded");
    HttpReq.setGetRequest(etcdURL);

    NotifyEventPtr notifyEventPtr = new NotifyEvent();
    notifyEventPtr->_run_times = 1;
    EtcdReqestInfo etcdReqInfo;
    etcdReqInfo.startTime = TNOWMS;
    etcdReqInfo.etcdAction = ETCD_GET_ALL_IDC_PRIORITY;
    etcdReqInfo.current = NULL;
    etcdReqInfo.notifyPtr = notifyEventPtr;
    HttpReq.getHostPort(etcdReqInfo.etcdHost, etcdReqInfo.etcdPort);

    TC_HttpAsync::RequestCallbackPtr callback = new RequestEtcdCallback(etcdReqInfo);
    ETCDPROC_LOG << FILE_FUN << etcdURL << endl;

    int ret = _ast.doAsyncRequest(HttpReq, callback);
    if (ret == TC_ClientSocket::EM_SUCCESS)
    {
        {
            TC_ThreadLock::Lock sync(notifyEventPtr->_monitor);
            while (!notifyEventPtr->_canWake)
            {
                notifyEventPtr->_monitor.wait();
            }
        }

        RequestEtcdCallbackPtr etcdCallBackPtr = TC_AutoPtr<RequestEtcdCallback>::dynamicCast(callback);
        const EtcdReqestInfo &backEtcdReqInfo = etcdCallBackPtr->_etcdReqInfo;
        idcPriorityInfo.clear();
        for (size_t i = 0; i < backEtcdReqInfo.idcPriority.size(); ++i)
        {
            idcPriorityInfo.push_back(backEtcdReqInfo.idcPriority[i]);
        }

        return 0;
    }
    else
    {
        string info = "send req to " + etcdReqInfo.etcdHost + ":" + TC_Common::tostr(etcdReqInfo.etcdPort) + ",failure,ret=" + TC_Common::tostr(ret);
        ETCDPROC_LOGERROR << FILE_FUN << EtcdReqStr(etcdReqInfo) << "|" << info << endl;
        ret = TSEER_REGISTRY_ETCD_SEND_REQ_FAIL;
    }

    return ret;
}

int EtcdHandle::addServiceGroup(const string& srvGrp, const string& key, const string& userListStr)
{
    string etcdURL = ETCDHOST->getServiceGroupHost();
    if(srvGrp.empty())
    {
        ETCDPROC_LOGERROR <<FILE_FUN<< "param error" << endl;
        return TSEER_REGISTRY_SETSERVER_PARAM_ERROR;
    }

    
    RouterData routerData;
    routerData.moduletype = srvGrp;
    if(addServiceGroupDir(routerData) != 0)
    {
        string errMsg = srvGrp + " record failure";
        APIIMP_LOGERROR << FILE_FUN << display(routerData) << "|"<< errMsg << API_INTERNAL_ERROR << endl;
        return TSEER_REGISTRY_ETCD_SEND_REQ_FAIL;
    }
    
    MAKEVALUE(etcdURL, MODULETYPE, srvGrp);

    string value = "value=" + key + "|" + userListStr;  
    
    TC_HttpRequest HttpReq;
    HttpReq.setHeader("Content-Type","application/x-www-form-urlencoded");
    HttpReq.setPutRequest(etcdURL,value);
    
    EtcdReqestInfo etcdReqInfo;
    etcdReqInfo.startTime = TNOWMS;
    etcdReqInfo.etcdAction = ETCD_ADD_SERVICE_GROUP;
    HttpReq.getHostPort(etcdReqInfo.etcdHost,etcdReqInfo.etcdPort);
    
    TC_HttpAsync::RequestCallbackPtr callback = new RequestEtcdCallback(etcdReqInfo);

    ETCDPROC_LOG<< FILE_FUN<<etcdURL<<"|"<<value<< endl;
    APIIMP_LOG<<FILE_FUN<< etcdURL <<"|"<<value<<endl;
    int ret =  _apiAst.doAsyncRequest(HttpReq, callback);
    if(ret == TC_ClientSocket::EM_SUCCESS)
    {
        return 0;
    }
    else
    {
        string info = "send req to " + etcdReqInfo.etcdHost + ":" + TC_Common::tostr(etcdReqInfo.etcdPort) + ",failure,ret="+TC_Common::tostr(ret);
        ETCDPROC_LOGERROR<<FILE_FUN << EtcdReqStr(etcdReqInfo) <<"|"<<info<<endl;
        APIIMP_LOGERROR<< FILE_FUN << EtcdReqStr(etcdReqInfo)  <<"|"<<info<< endl;
        ret = TSEER_REGISTRY_ETCD_SEND_REQ_FAIL;
    }
    return ret;
}

int EtcdHandle::updateServiceGroup(const string& srvGrp, const string& key,const string& userListStr)
{
    string etcdURL = ETCDHOST->getServiceGroupHost();
    if(srvGrp.empty())
    {
        ETCDPROC_LOGERROR <<FILE_FUN<< "param error" << endl;
        return TSEER_REGISTRY_SETSERVER_PARAM_ERROR;
    }

    MAKEVALUE(etcdURL, MODULETYPE, srvGrp);

    string value = "value=" + key + "|" + userListStr;  
    
    TC_HttpRequest HttpReq;
    HttpReq.setHeader("Content-Type","application/x-www-form-urlencoded");
    HttpReq.setPutRequest(etcdURL,value);
    
    EtcdReqestInfo etcdReqInfo;
    etcdReqInfo.startTime = TNOWMS;
    etcdReqInfo.etcdAction = ETCD_UPDATE_SERVICE_GROUP;
    HttpReq.getHostPort(etcdReqInfo.etcdHost,etcdReqInfo.etcdPort);
    
    TC_HttpAsync::RequestCallbackPtr callback = new RequestEtcdCallback(etcdReqInfo);

    ETCDPROC_LOG<< FILE_FUN<<etcdURL<<"|"<<value<< endl;
    APIIMP_LOG<<FILE_FUN<< etcdURL <<"|"<<value<<endl;
    int ret =  _apiAst.doAsyncRequest(HttpReq, callback);
    if(ret == TC_ClientSocket::EM_SUCCESS)
    {
        return 0;
    }
    else
    {
        string info = "send req to " + etcdReqInfo.etcdHost + ":" + TC_Common::tostr(etcdReqInfo.etcdPort) + ",failure,ret="+TC_Common::tostr(ret);
        ETCDPROC_LOGERROR<<FILE_FUN << EtcdReqStr(etcdReqInfo) <<"|"<<info<<endl;
        APIIMP_LOGERROR<< FILE_FUN << EtcdReqStr(etcdReqInfo)  <<"|"<<info<< endl;
        ret = TSEER_REGISTRY_ETCD_SEND_REQ_FAIL;
    }
    return ret;    
}

int EtcdHandle::getAllServiceGroup(vector<ServiceGroupInfo>& srvGrpInfoList)
{
    string etcdURL = ETCDHOST->getServiceGroupHost();

    TC_HttpRequest HttpReq;
    HttpReq.setHeader("Content-Type", "application/x-www-form-urlencoded");
    HttpReq.setGetRequest(etcdURL);

    NotifyEventPtr notifyEventPtr = new NotifyEvent();
    notifyEventPtr->_run_times = 1;
    
    EtcdReqestInfo etcdReqInfo;
    etcdReqInfo.startTime = TNOWMS;
    etcdReqInfo.etcdAction = ETCD_GET_ALL_SERVICE_GROUP;
    etcdReqInfo.current = NULL;
    etcdReqInfo.notifyPtr = notifyEventPtr;
    HttpReq.getHostPort(etcdReqInfo.etcdHost, etcdReqInfo.etcdPort);

    TC_HttpAsync::RequestCallbackPtr callback = new RequestEtcdCallback(etcdReqInfo);
    ETCDPROC_LOG << FILE_FUN << etcdURL << endl;
    APIIMP_LOG<<FILE_FUN<< etcdURL<<endl;
    int ret = _ast.doAsyncRequest(HttpReq, callback);
    if (ret == TC_ClientSocket::EM_SUCCESS)
    {
        {
            TC_ThreadLock::Lock sync(notifyEventPtr->_monitor);
            //等待结果返回
            while (!notifyEventPtr->_canWake)
            {
                notifyEventPtr->_monitor.wait();
            }
        }
        RequestEtcdCallbackPtr etcdCallBackPtr = TC_AutoPtr<RequestEtcdCallback>::dynamicCast(callback);
        const EtcdReqestInfo &backEtcdReqInfo = etcdCallBackPtr->_etcdReqInfo;
        srvGrpInfoList = backEtcdReqInfo.serviceInfoArr;
        return 0;
    }
    else
    {
        string info = "send req to " + etcdReqInfo.etcdHost + ":" + TC_Common::tostr(etcdReqInfo.etcdPort) + ",failure,ret=" + TC_Common::tostr(ret);
        ETCDPROC_LOGERROR << FILE_FUN << EtcdReqStr(etcdReqInfo) << "|" << info << endl;
        ret = TSEER_REGISTRY_ETCD_SEND_REQ_FAIL;
    }

    return ret;
}

int EtcdHandle::getServiceGroupKey(const string& srvGrp,string& key)
{
    string etcdURL = ETCDHOST->getServiceGroupHost();

    TC_HttpRequest HttpReq;
    HttpReq.setHeader("Content-Type", "application/x-www-form-urlencoded");
    MAKEVALUE(etcdURL, MODULETYPE, srvGrp);
    HttpReq.setGetRequest(etcdURL);

    NotifyEventPtr notifyEventPtr = new NotifyEvent();
    notifyEventPtr->_run_times = 1;
    EtcdReqestInfo etcdReqInfo;
    etcdReqInfo.startTime = TNOWMS;
    etcdReqInfo.etcdAction = ETCD_GET_SERVICE_GROUP_KEY;
    etcdReqInfo.current = NULL;
    etcdReqInfo.notifyPtr = notifyEventPtr;
    HttpReq.getHostPort(etcdReqInfo.etcdHost, etcdReqInfo.etcdPort);

    TC_HttpAsync::RequestCallbackPtr callback = new RequestEtcdCallback(etcdReqInfo);
    ETCDPROC_LOG << FILE_FUN << etcdURL << endl;
    APIIMP_LOG<<FILE_FUN<< etcdURL<<endl;
    int ret = _ast.doAsyncRequest(HttpReq, callback);
    if (ret == TC_ClientSocket::EM_SUCCESS)
    {
        {
            TC_ThreadLock::Lock sync(notifyEventPtr->_monitor);
            //等待结果返回
            while (!notifyEventPtr->_canWake)
            {
                notifyEventPtr->_monitor.wait();
            }
        }
        RequestEtcdCallbackPtr etcdCallBackPtr = TC_AutoPtr<RequestEtcdCallback>::dynamicCast(callback);
        const EtcdReqestInfo &backEtcdReqInfo = etcdCallBackPtr->_etcdReqInfo;
        if (backEtcdReqInfo.serviceInfoArr.empty())
        {
            ETCDPROC_LOG << FILE_FUN << "Not found service group key for :" << srvGrp << endl;
            return -2;      //如果找不到则返回-2
        }
        else
        {
            key = backEtcdReqInfo.serviceInfoArr[0].key;
            return 0;
        }
    }
    else
    {
        string info = "send req to " + etcdReqInfo.etcdHost + ":" + TC_Common::tostr(etcdReqInfo.etcdPort) + ",failure,ret=" + TC_Common::tostr(ret);
        ETCDPROC_LOGERROR << FILE_FUN << EtcdReqStr(etcdReqInfo) << "|" << info << endl;
        ret = TSEER_REGISTRY_ETCD_SEND_REQ_FAIL;
    }

    return ret;
}

int EtcdHandle::getAllServers(vector<map<string, string> > &routerDataMap)
{
    return ETCDROUTEDATA->loadAllEtcdData(routerDataMap);
}

int EtcdHandle::getAllGrayAgentInfo(map<string,GrayAgentInfo>& grayAgentInfoList)
{
    string etcdURL = ETCDHOST->getGrayAgentInfoHost();

    etcdURL += "?dir=true&recursive=true";

    TC_HttpRequest HttpReq;
    HttpReq.setGetRequest(etcdURL);

    EtcdReqestInfo getEtcdReqInfo;
    getEtcdReqInfo.current = NULL;
    getEtcdReqInfo.etcdAction = ETCD_GET_ALL_GRAYAGENT_INFO;
    getEtcdReqInfo.startTime = TNOWMS;
    HttpReq.getHostPort(getEtcdReqInfo.etcdHost, getEtcdReqInfo.etcdPort);
    NotifyEventPtr notifyEventPtr = new NotifyEvent();        //解析好了数据才返回
    notifyEventPtr->_run_times = 1;
    getEtcdReqInfo.notifyPtr = notifyEventPtr;

    TC_HttpAsync::RequestCallbackPtr callback = new RequestEtcdCallback(getEtcdReqInfo);
    ETCDPROC_LOG << FILE_FUN << etcdURL << endl;
    APIIMP_LOG<<FILE_FUN<< etcdURL<<endl;


    int ret = _ast.doAsyncRequest(HttpReq, callback);
    if (ret == TC_ClientSocket::EM_SUCCESS)
    {
        {
            TC_ThreadLock::Lock lock(notifyEventPtr->_monitor);
            while (!notifyEventPtr->_canWake)
            {
                notifyEventPtr->_monitor.wait();
            }
        }

        RequestEtcdCallbackPtr etcdCallBackPtr = TC_AutoPtr<RequestEtcdCallback>::dynamicCast(callback);
        const EtcdReqestInfo &backEtcdReqInfo = etcdCallBackPtr->_etcdReqInfo;
        grayAgentInfoList = backEtcdReqInfo.grayAgentInfoList;
        return 0;
    }
    else
    {
        string info = "send req to " + getEtcdReqInfo.etcdHost + ":" + TC_Common::tostr(getEtcdReqInfo.etcdPort) + ",failure,ret="+TC_Common::tostr(ret);
        ETCDPROC_LOGERROR << FILE_FUN << EtcdReqStr(getEtcdReqInfo) << "|" << info << endl;
        return -1;
    }
}

int EtcdHandle::get_agent_baseinfo(const AgentBaseInfoReq& req,vector<AgentBaseInfo>& agentBaseInfos)
{
    AgentPackageReq packageReq;
    packageReq.ostype = req.ostype;

    vector<AgentPackageInfo> agentPackageInfoList;
    int errCode = get_agent_packageinfo(packageReq,agentPackageInfoList);
    if (errCode != TSEER_SERVERSUCCESS && errCode != TSEER_REGISTRY_ETCD_DATA_NOTFOUND)
    {
        ETCDPROC_LOGERROR << FILE_FUN <<"get packageinfo failure"<< endl;
        //输出错误日志就可以了，查询失败不影响展示
    }

    map<string,map<string,AgentPackageInfo> > packageVersions;
    //获取目前所有操作系统对应版本信息
    for(size_t i = 0; i < agentPackageInfoList.size();i++)
    {
        string type = agentPackageInfoList[i].package_type;
        if(TSEER_PACKAGE_GRAY_TYPE == type ||
            TSEER_PACKAGE_FORMAL_TYPE == type)
        {
            packageVersions[agentPackageInfoList[i].ostype].insert(make_pair(type,agentPackageInfoList[i]));
        }
    }

    //获取所有agent灰度记录
    map<string,GrayAgentInfo> grayAgentInfoList;
    errCode = getAllGrayAgentInfo(grayAgentInfoList);
    if(errCode != 0)
    {
        ETCDPROC_LOGERROR << FILE_FUN <<"get packageinfo failure"<< endl;
    }
    
    //agent info url
    string etcdURL = ETCDHOST->getNodeInfoHost();

    EtcdAction  action = ETCD_GET_ALL_SEER_AGENTS;
    if(!req.ip.empty())
    {
        MAKEVALUE(etcdURL, NODE_NETWORKID, TSEER_AGENT_NET_WORK_ID);
        MAKEVALUE(etcdURL, NODE_NAME, req.ip);
        action = ETCD_GET_ONE_SEER_AGENT;
    }
    else
    {
        etcdURL += string("/") + NODE_NETWORKID + "=" + TSEER_AGENT_NET_WORK_ID + "?recursive=true"; 
    }
    
    TC_HttpRequest HttpReq;
    HttpReq.setGetRequest(etcdURL);

    EtcdReqestInfo getEtcdReqInfo;
    getEtcdReqInfo.current = NULL;
    getEtcdReqInfo.etcdAction = action;
    getEtcdReqInfo.startTime = TNOWMS;
    HttpReq.getHostPort(getEtcdReqInfo.etcdHost, getEtcdReqInfo.etcdPort);
    
    NotifyEventPtr notifyEventPtr = new NotifyEvent();      //解析好了数据才返回
    notifyEventPtr->_run_times = 1;
    getEtcdReqInfo.notifyPtr = notifyEventPtr;

    TC_HttpAsync::RequestCallbackPtr callback = new RequestEtcdCallback(getEtcdReqInfo);
    ETCDPROC_LOG << FILE_FUN << etcdURL << endl;
    APIIMP_LOG<<FILE_FUN<< etcdURL<<endl;


    int ret = _ast.doAsyncRequest(HttpReq, callback);
    if (ret == TC_ClientSocket::EM_SUCCESS)
    {
        //等待结果
        {
            TC_ThreadLock::Lock lock(notifyEventPtr->_monitor);
            while (!notifyEventPtr->_canWake)
            {
                notifyEventPtr->_monitor.wait();
            }
        }
        
        RequestEtcdCallbackPtr etcdCallBackPtr = TC_AutoPtr<RequestEtcdCallback>::dynamicCast(callback);
        const vector<NodeServiceInfo> &result = etcdCallBackPtr->_etcdReqInfo.nodeServiceInfo;
        for (size_t i = 0; i < result.size(); ++i)
        {
            AgentBaseInfo info;
            if(!req.ostype.empty() && 
                req.ostype != result[i].ostype)
                continue;

            if(!req.present_state.empty() &&
                req.present_state != result[i].presentState)
                continue;

            info.ip = result[i].nodeName;
            info.ostype = result[i].ostype;
            info.present_state = result[i].presentState;
            info.locator = result[i].locator;
            info.version = result[i].version;
            info.last_reg_time = result[i].registerTime;
            info.last_heartbeat_time = result[i].last_hearttime;
            info.grayscale_state = "";
            if(grayAgentInfoList.find(info.ip) != grayAgentInfoList.end())
            {
                info.grayscale_state = grayAgentInfoList.find(info.ip)->second.grayscale_state;
            }
            
            info.gray_version = "";
            if(packageVersions.find(info.ostype) != packageVersions.end())
            {
                if(packageVersions.find(info.ostype)->second.find(TSEER_PACKAGE_GRAY_TYPE) != packageVersions.find(info.ostype)->second.end())
                {
                    info.gray_version = packageVersions[info.ostype][TSEER_PACKAGE_GRAY_TYPE].version;
                    //灰度逻辑需要包名
                    info.package_name = packageVersions[info.ostype][TSEER_PACKAGE_GRAY_TYPE].package_name;
                    info.md5 = packageVersions[info.ostype][TSEER_PACKAGE_GRAY_TYPE].md5;
                }
            }

            info.formal_version = "";
            if(packageVersions.find(info.ostype) != packageVersions.end())
            {
                if(packageVersions.find(info.ostype)->second.find(TSEER_PACKAGE_FORMAL_TYPE) != packageVersions.find(info.ostype)->second.end())
                {
                    info.formal_version = packageVersions[info.ostype][TSEER_PACKAGE_FORMAL_TYPE].version;
                }
            }

            agentBaseInfos.push_back(info);
        }

        return TSEER_SERVERSUCCESS;
    }
    else
    {
        string info = "send req to " + getEtcdReqInfo.etcdHost + ":" + TC_Common::tostr(getEtcdReqInfo.etcdPort) + ",failure,ret=" + TC_Common::tostr(ret);

        ETCDPROC_LOGERROR << FILE_FUN << EtcdReqStr(getEtcdReqInfo) << "|" << info << endl;
        return TSEER_REGISTRY_ETCD_SEND_REQ_FAIL;
    }  
}

int EtcdHandle::update_agent_graystate(const UpdateAgentInfoReq& req)
{
    if(req.ostype.empty() ||
        req.ip.empty())
    {
        ETCDPROC_LOGERROR << FILE_FUN <<"req.ostype or  req.ip should not be empty()"<< endl;
        return TSEER_REGISTRY_PARAM_ERROR;
    }

     if(req.grayscale_state == TSEER_PACKAGE_GRAY_TYPE)
     {
            AgentPackageReq packageReq;
            packageReq.ostype = req.ostype;

            vector<AgentPackageInfo> agentPackageInfoList;
            int errCode = get_agent_packageinfo(packageReq,agentPackageInfoList);
            if (errCode != TSEER_SERVERSUCCESS && errCode != TSEER_REGISTRY_ETCD_DATA_NOTFOUND)
            {
                ETCDPROC_LOGERROR << FILE_FUN <<"get packageinfo failure"<< endl;
                return errCode;
            }
            
            string package_name = "";//包名
            string grayscale_version = "";
            for (size_t i = 0; i < agentPackageInfoList.size(); i++)
            {
                AgentPackageInfo info = agentPackageInfoList[i];
                if(info.package_type == req.grayscale_state)
                {
                    package_name =info.package_name;
                    grayscale_version = info.version;
                    break;
                }
            }

            if(grayscale_version.empty() || package_name.empty())
            {
                ETCDPROC_LOGERROR << FILE_FUN <<req.ostype<<"|"<<req.ip<<" has no gray version"<< endl;
                return TSEER_REGISTRY_PARAM_ERROR;   
            }
            
            string etcdURL = ETCDHOST->getGrayAgentInfoHost();
            string key = req.ip;
            MAKEVALUE(etcdURL, NODE_NAME, key);
            //"value":"node_ip|grayscale_version|grayscale_state|package_name|ostype";
            string value = "value=" + req.ip \
                            + "|" + grayscale_version \
                            + "|" + req.grayscale_state\
                            + "|" + package_name \
                            + "|" + req.ostype;

            TC_HttpRequest HttpReq;
            HttpReq.setHeader("Content-Type", "application/x-www-form-urlencoded");
            HttpReq.setPutRequest(etcdURL,value);

            EtcdReqestInfo etcdReqInfo;
            etcdReqInfo.startTime = TNOWMS;
            etcdReqInfo.etcdAction = ETCD_ADD_AGENT_GRAY_STATE;
            HttpReq.getHostPort(etcdReqInfo.etcdHost,etcdReqInfo.etcdPort);

            TC_HttpAsync::RequestCallbackPtr callback = new RequestEtcdCallback(etcdReqInfo);

            ETCDPROC_LOG << FILE_FUN <<etcdURL<<"|"<<value<< endl;

            int ret =  _ast.doAsyncRequest(HttpReq, callback);
            if(ret == TC_ClientSocket::EM_SUCCESS)
            {
                errCode = TSEER_SERVERSUCCESS;
            }
            else
            {
                string info = "send req to " + etcdReqInfo.etcdHost + ":" + TC_Common::tostr(etcdReqInfo.etcdPort) + ",failure,ret="+TC_Common::tostr(ret);
                ETCDPROC_LOGERROR << FILE_FUN << EtcdReqStr(etcdReqInfo) << "|" << info << endl;
                errCode = TSEER_REGISTRY_ETCD_SEND_REQ_FAIL;
            }
             return errCode;   
     }
     else
     {
            //取消灰度
            string etcdURL = ETCDHOST->getGrayAgentInfoHost();
            MAKEVALUE(etcdURL, NODE_NAME, req.ip);
            etcdURL += "?dir=true&recursive=true";

            TC_HttpRequest HttpReq;
            HttpReq.setHeader("Content-Type", "application/x-www-form-urlencoded");
            HttpReq.setDeleteRequest(etcdURL, "");

            EtcdReqestInfo etcdReqInfo;
            etcdReqInfo.startTime = TNOWMS;
            etcdReqInfo.etcdAction = ETCD_DEL_AGENT_GRAY_STATE;
            etcdReqInfo.current = NULL;
            HttpReq.getHostPort(etcdReqInfo.etcdHost, etcdReqInfo.etcdPort);

            ETCDPROC_LOG << FILE_FUN << EtcdReqStr(etcdReqInfo) << endl;
            tars::TC_HttpAsync::RequestCallbackPtr callback = new RequestEtcdCallback(etcdReqInfo);

            int ret = _ast.doAsyncRequest(HttpReq, callback);
            if (ret == TC_ClientSocket::EM_SUCCESS)
            {
                return 0;
            }
            else
            {
                string info = "send req to " + etcdReqInfo.etcdHost + ":" + TC_Common::tostr(etcdReqInfo.etcdPort) + ",failure,ret=" + TC_Common::tostr(ret);
                ETCDPROC_LOGERROR << FILE_FUN << EtcdReqStr(etcdReqInfo) << "|" << info << endl;
                ret = TSEER_REGISTRY_ETCD_SEND_REQ_FAIL;
            }
            return ret;
     }
     return TSEER_REGISTRY_PARAM_ERROR;
}

int EtcdHandle::get_agent_packageinfo(const AgentPackageReq& req,vector<AgentPackageInfo>& agentPackageInfoList)
{
    string etcdURL = ETCDHOST->getAgentPackageInfoHost();
    if(!req.ostype.empty())
    {
        MAKEVALUE(etcdURL, OSVERSION, req.ostype);
        if(!req.package_name.empty())
        {
            MAKEVALUE(etcdURL, PACKAGENAME, req.package_name);
        }
    }
    etcdURL += "?recursive=true";

    TC_HttpRequest HttpReq;
    HttpReq.setGetRequest(etcdURL);

    EtcdReqestInfo getEtcdReqInfo;
    getEtcdReqInfo.current = NULL;
    getEtcdReqInfo.etcdAction = ETCD_GET_ALL_AGENT_PACKAGE_INFO;
    getEtcdReqInfo.startTime = TNOWMS;
    HttpReq.getHostPort(getEtcdReqInfo.etcdHost, getEtcdReqInfo.etcdPort);
    NotifyEventPtr notifyEventPtr = new NotifyEvent();        //解析好了数据才返回
    notifyEventPtr->_run_times = 1;
    getEtcdReqInfo.notifyPtr = notifyEventPtr;

    TC_HttpAsync::RequestCallbackPtr callback = new RequestEtcdCallback(getEtcdReqInfo);
    ETCDPROC_LOG << FILE_FUN << etcdURL << endl;

    int errCode = TSEER_REGISTRY_ETCD_SEND_REQ_FAIL;
    int ret = _ast.doAsyncRequest(HttpReq, callback);
    if (ret == TC_ClientSocket::EM_SUCCESS)
    {
        //等待结果
        {
            TC_ThreadLock::Lock lock(notifyEventPtr->_monitor);
            while (!notifyEventPtr->_canWake)
            {
                notifyEventPtr->_monitor.wait();
            }
        }

        RequestEtcdCallbackPtr etcdCallBackPtr = TC_AutoPtr<RequestEtcdCallback>::dynamicCast(callback);
        const EtcdReqestInfo &backEtcdReqInfo = etcdCallBackPtr->_etcdReqInfo;
        
       agentPackageInfoList.clear();
        for (size_t i = 0; i < backEtcdReqInfo.agentPackageInfoList.size(); ++i)
        {
            if(!req.ostype.empty() &&
                req.ostype != backEtcdReqInfo.agentPackageInfoList[i].ostype)
                continue;

            if(!req.package_name.empty() &&
                req.package_name != backEtcdReqInfo.agentPackageInfoList[i].package_name)
                continue;
            
            agentPackageInfoList.push_back(backEtcdReqInfo.agentPackageInfoList[i]);
        }
        errCode = TSEER_SERVERSUCCESS;
    }
    else
    {
        string info = "send req to " + getEtcdReqInfo.etcdHost + ":" + TC_Common::tostr(getEtcdReqInfo.etcdPort) + ",failure,ret="+TC_Common::tostr(ret);
        //记录日志
        ETCDPROC_LOGERROR << FILE_FUN << EtcdReqStr(getEtcdReqInfo) << "|" << info << endl;
        errCode = TSEER_REGISTRY_ETCD_SEND_REQ_FAIL;
    }

    return errCode;    
}

int EtcdHandle::updateOnePackageInfo(const AgentPackageInfo& agentPackageInfo)
{
    if(agentPackageInfo.ostype.empty() || agentPackageInfo.package_name.empty() || 
            agentPackageInfo.package_type.empty() || agentPackageInfo.md5.empty() || agentPackageInfo.version.empty())
    {
        return TSEER_REGISTRY_PARAM_ERROR;
    }

    string etcdURL = ETCDHOST->getAgentPackageInfoHost();

    MAKEVALUE(etcdURL, OSVERSION, agentPackageInfo.ostype);
    MAKEVALUE(etcdURL, PACKAGENAME, agentPackageInfo.package_name);

    string value = "value=" + agentPackageInfo.ostype+ \
                    "|" + agentPackageInfo.package_type + \
                    "|" + agentPackageInfo.package_name+ \
                    "|" + agentPackageInfo.md5 + \
                    "|" + agentPackageInfo.version + \
                    "|" + agentPackageInfo.uploadUser + \
                    "|" + agentPackageInfo.uploadTime;

    TC_HttpRequest HttpReq;
    HttpReq.setHeader("Content-Type","application/x-www-form-urlencoded");
    HttpReq.setPutRequest(etcdURL,value);
    
    EtcdReqestInfo etcdReqInfo;
    etcdReqInfo.startTime = TNOWMS;
    etcdReqInfo.etcdAction = ETCD_UPDATE_AGENTPACKAGEINFO;
    HttpReq.getHostPort(etcdReqInfo.etcdHost,etcdReqInfo.etcdPort);

    TC_HttpAsync::RequestCallbackPtr callback = new RequestEtcdCallback(etcdReqInfo);

    ETCDPROC_LOG << FILE_FUN <<etcdURL<<"|"<<value<< endl;

    int ret =  _ast.doAsyncRequest(HttpReq, callback);
    int errCode;
    if(ret == TC_ClientSocket::EM_SUCCESS)
    {
        errCode = TSEER_SERVERSUCCESS;
    }
    else
    {
        string info = "send req to " + etcdReqInfo.etcdHost + ":" + TC_Common::tostr(etcdReqInfo.etcdPort) + ",failure,ret="+TC_Common::tostr(ret);
        //记录日志
        errCode = TSEER_REGISTRY_ETCD_SEND_REQ_FAIL;
        ETCDPROC_LOGERROR<<FILE_FUN << info <<"|"<<errCode<< endl;
    }

    return errCode;
}

int EtcdHandle::update_agent_packageinfo(const UpdatePackageInfoReq& req)
{
    if(req.ostype.empty() ||
        req.package_name.empty())
    {
        ETCDPROC_LOGERROR << FILE_FUN <<"req.ostype or  req.package_name should not be empty()"<< endl;
        return TSEER_REGISTRY_PARAM_ERROR;
    }
    
    AgentPackageReq packageReq;
    packageReq.ostype = req.ostype;

    vector<AgentPackageInfo> agentPackageInfoList;
    int errCode = get_agent_packageinfo(packageReq,agentPackageInfoList);
    if (errCode != TSEER_SERVERSUCCESS && errCode != TSEER_REGISTRY_ETCD_DATA_NOTFOUND)
    {
        ETCDPROC_LOGERROR << FILE_FUN <<"get packageinfo failure"<< endl;
        return errCode;
    }
        
    AgentPackageInfo updateInfo;
    
    //保证灰度或者正式版本只有一个
    if (req.grayscale_state == TSEER_PACKAGE_GRAY_TYPE || req.grayscale_state == TSEER_PACKAGE_FORMAL_TYPE)
    {
        for (size_t i = 0; i < agentPackageInfoList.size(); i++)
        {
            AgentPackageInfo info = agentPackageInfoList[i];
            if(info.package_type == req.grayscale_state &&
                info.package_type != TSEER_PACKAGE_NONE_TYPE &&
                info.package_name != req.package_name)
            {
                //把其它以前是灰度或者正式版本的包设置为无状态
                info.package_type = TSEER_PACKAGE_NONE_TYPE;
                errCode = updateOnePackageInfo(info);
                if (errCode != TSEER_SERVERSUCCESS)
                {
                    break;
                }
            }

            if(info.package_name == req.package_name)
            {
                updateInfo = agentPackageInfoList[i];
                updateInfo.package_type = req.grayscale_state;
            }
        }
        
        if (errCode != TSEER_SERVERSUCCESS)
        {
            return errCode;
        }
    }

    errCode = updateOnePackageInfo(updateInfo);
    return errCode;
}

int EtcdHandle::delete_agent_package(const AgentPackageReq& req)
{
    if(req.ostype.empty() ||
        req.package_name.empty())
    {
        ETCDPROC_LOGERROR << FILE_FUN <<"req.ostype or  req.package_name should not be empty()"<< endl;
        return TSEER_REGISTRY_PARAM_ERROR;
    }
    string etcdURL = ETCDHOST->getAgentPackageInfoHost();
    MAKEVALUE(etcdURL, OSVERSION, req.ostype);
    MAKEVALUE(etcdURL, PACKAGENAME, req.package_name);
    etcdURL += "?dir=true&recursive=true";

    ETCDPROC_LOG << FILE_FUN << etcdURL << endl;
    APIIMP_LOG << FILE_FUN << etcdURL << endl;

    TC_HttpRequest HttpReq;
    HttpReq.setHeader("Content-Type", "application/x-www-form-urlencoded");
    HttpReq.setDeleteRequest(etcdURL, "");

    EtcdReqestInfo etcdReqInfo;
    etcdReqInfo.startTime = TNOWMS;
    etcdReqInfo.etcdAction = ETCD_DEL_AGENTPACKAGEINFO;
    etcdReqInfo.current = NULL;
    HttpReq.getHostPort(etcdReqInfo.etcdHost, etcdReqInfo.etcdPort);
    tars::TC_HttpAsync::RequestCallbackPtr callback = new RequestEtcdCallback(etcdReqInfo);

    int ret = _ast.doAsyncRequest(HttpReq, callback);
    if (ret == TC_ClientSocket::EM_SUCCESS)
    {
        return 0;
    }
    else
    {
        string info = "send req to " + etcdReqInfo.etcdHost + ":" + TC_Common::tostr(etcdReqInfo.etcdPort) + ",failure,ret=" + TC_Common::tostr(ret);
        ETCDPROC_LOGERROR << FILE_FUN << EtcdReqStr(etcdReqInfo) << "|" << info << endl;
        ret = TSEER_REGISTRY_ETCD_SEND_REQ_FAIL;
    }
    return ret;    
}

int EtcdHandle::add_agent_packageinfo(const AgentPackageInfo & reqInfo)
{
    //保证灰度或者正式版本只有一个
    if(reqInfo.package_type == TSEER_PACKAGE_GRAY_TYPE || reqInfo.package_type == TSEER_PACKAGE_FORMAL_TYPE)
    {
        AgentPackageReq packageReq;
        packageReq.ostype = reqInfo.ostype;

        vector<AgentPackageInfo> agentPackageInfoList;
        int errCode = get_agent_packageinfo(packageReq,agentPackageInfoList);
        if (errCode != TSEER_SERVERSUCCESS && errCode != TSEER_REGISTRY_ETCD_DATA_NOTFOUND)
        {
            ETCDPROC_LOGERROR << FILE_FUN <<"get packageinfo failure"<< endl;
            return errCode;
        }
        
        for (size_t i = 0; i < agentPackageInfoList.size(); i++)
        {
            AgentPackageInfo info = agentPackageInfoList[i];
            if(info.package_type == reqInfo.package_type &&
                info.package_type != TSEER_PACKAGE_NONE_TYPE &&
                info.package_name != reqInfo.package_name)
            {
                //把其它以前是灰度或者正式版本的包设置为无状态
                info.package_type = TSEER_PACKAGE_NONE_TYPE;
                errCode = updateOnePackageInfo(info);
                if (errCode != TSEER_SERVERSUCCESS)
                {
                    ETCDPROC_LOGERROR << FILE_FUN <<"updateOnePackageInfo failure|"<<info.package_name<< endl;
                    break;
                }
            }
        }        
    }
     //再插入新数据
    return updateOnePackageInfo(reqInfo);
}


