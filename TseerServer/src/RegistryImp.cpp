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

#include <iostream>
#include "RegistryImp.h"
#include "TSeerServer.h"
#include "StoreProxy.h"
#include "util.h"
#include "UpdateThread.h"
#include "FileWriterFactory.h"

#define DOWNLOAD_CHUNK 5242880

void RegistryImp::initialize()
{
    TLOGDEBUG("RegistryImp init ok."<<endl);
}

int RegistryImp::registerNode(const string & name, const NodeInfo & ni, const LoadInfo & li, tars::TarsCurrentPtr current)
{
    REGIMP_LOG<<FILE_FUN<< current->getIp() <<"|"<<name <<endl;
    NodeServiceInfo nodeInfo = NodeServiceInfo::fromNodeInfo(ni, li);
    nodeInfo.nodeName = name;
    return STOREPROXY->addNodeInfo(nodeInfo);
}

Int32 RegistryImp::registerNodeExt(const NodeInstanceInfo & nodeInfo,tars::TarsCurrentPtr current)
{
    REGIMP_LOG<<FILE_FUN<< current->getIp() <<"|regnode="<<nodeInfo.nodeName <<endl;
    return STOREPROXY->registerNodeExt(nodeInfo);
}


Int32 RegistryImp::keepAliveExt(const NodeInstanceInfo & nodeInfo,tars::TarsCurrentPtr current)
{
    REGIMP_LOG<<FILE_FUN<< current->getIp() <<"|node="<<nodeInfo.nodeName <<endl;
    return STOREPROXY->keepAliveExt(nodeInfo);
}

int RegistryImp::keepAlive(const string & name, const LoadInfo & ni, tars::TarsCurrentPtr current)
{
    REGIMP_LOG<<FILE_FUN<< current->getIp() <<"|node="<<name <<endl;
    return STOREPROXY->keepAlive(name,ni);
}


string RegistryImp::intStateToString(const ServerState &state)
{
    if(state == Inactive)
        return "inactive";
    else if(state == Activating)
        return "activating";
    else if(state == Active)
        return "active";
    else if(state == Deactivating)
        return "deactivating";
    else if(state == Destroying)
        return "destroying";
    else if(state == Destroyed)
        return "destroyed";

    return "";
}

int RegistryImp::updateServer(const ServerStateInfo & stateInfo,tars::TarsCurrentPtr current)
{
    RouterData data;
    data.moduletype = stateInfo.moduleType;
    data.application = stateInfo.application;
    data.service_name = stateInfo.serverName;
    data.node_name = stateInfo.nodeName;
    data.container_name = stateInfo.containerName;
    data.present_state = intStateToString(stateInfo.serverState);
    REGIMP_LOG<<FILE_FUN<< current->getIp() <<"|"<<toStr(data)<<"|"<<intStateToString(stateInfo.serverState)<<"|coming"<<endl;
    
    if(stateInfo.moduleType.empty() || stateInfo.application.empty() || stateInfo.serverName.empty() 
            || stateInfo.nodeName.empty() || stateInfo.containerName.empty() || data.present_state.empty())
    {
        REGIMP_LOGERROR << FILE_FUN<< current->getIp() <<"|"<<toStr(data)<< "|param stateinfo error" << endl;
        return TSEER_REGISTRY_UPDATESERVER_PARAM_ERROR;
    }


    if( data.present_state.empty() )
    {
        REGIMP_LOGERROR<<FILE_FUN << current->getIp() <<"|"<<toStr(data)<< "|param serverState  error!" << endl;
        return TSEER_REGISTRY_UPDATESERVER_PARAM_ERROR;
    }
    
    return STOREPROXY->updateServerState(data,current);
}

int RegistryImp::updateServerBatch(const std::vector<ServerStateInfo> & vecStateInfo, tars::TarsCurrentPtr current)
{
    REGIMP_LOG<<FILE_FUN<< current->getIp() <<"|size="<<vecStateInfo.size()<<"|coming"<<endl;

    vector<RouterData> needUpdateList;
    
    for(std::vector<ServerStateInfo>::size_type i = 0; i < vecStateInfo.size(); i++)
    {   
        RouterData data;
        data.moduletype = vecStateInfo[i].moduleType;
        data.application = vecStateInfo[i].application;
        data.service_name = vecStateInfo[i].serverName;
        data.node_name = vecStateInfo[i].nodeName;
        data.container_name = vecStateInfo[i].containerName;
        data.present_state = intStateToString(vecStateInfo[i].serverState);
        if(data.present_state.empty())
        {
            REGIMP_LOGERROR << FILE_FUN<< toStr(data)<<"|has bad serverState"<<endl;
            continue;
        }
        needUpdateList.push_back(data);
    }
    
    if(needUpdateList.size() > 0)
    {
        return STOREPROXY->updateServerStates(needUpdateList,current);
    }
    else
    {
        REGIMP_LOG<<FILE_FUN<< current->getIp()<<"|no need to update state"<<endl;
    }
    
    return TSEER_SERVERSUCCESS;
}

int RegistryImp::destroy(const string & name, tars::TarsCurrentPtr current)
{
    REGIMP_LOG<<FILE_FUN<< current->getIp() <<"|"<<name <<endl;
    STOREPROXY->delNodeInfo(name, "dcnode");
    return TSEER_SERVERSUCCESS;
}


void RegistryImp::genFailRsp(const UpdateReq & req, UpdateRsp &rsp)
{
    rsp.offset = req.offset;
    rsp.packageName = req.packageName;
    rsp.finish = false;
    rsp.buff = "";
    rsp.needChange = false;
}

Int32 RegistryImp::checkChange(const PackageInfo & reqInfo, PackageInfo &rspinfo, tars::TarsCurrentPtr current)
{
    __TRY__
    UPDATEPACKAGE_LOG<< FILE_FUN<<current->getIp()<<":"<<current->getPort()<<"|"<<display(reqInfo)<<endl;
    PackageInfo info;
    bool hasData = UPDATETHREAD->getGrayInfo(reqInfo, info);
    if (!hasData)
    {
        bool hasData = UPDATETHREAD->getFormalInfo(reqInfo, info);
        if (hasData)
        {
            rspinfo = info;
        }
        else
        {
            rspinfo = reqInfo;
        }
    }
    else
    {
        rspinfo = info;
    }

    UPDATEPACKAGE_LOG << FILE_FUN <<current->getIp()<<":"<<current->getPort()<<"|req="<<display(reqInfo)<< "|rsp file name=|" << display(rspinfo)<< endl;
    return 0;
    __CATCH__

    UPDATEPACKAGE_LOGERROR<< FILE_FUN <<current->getIp()<<":"<<current->getPort()<<"|req="<<display(reqInfo)<< "|checkChange ex|"<< endl;
    return -3;
}

Int32 RegistryImp::updatePacket(const UpdateReq & req, UpdateRsp &rsp, tars::TarsCurrentPtr current)
{
    __TRY__
    UPDATEPACKAGE_LOG << FILE_FUN <<current->getIp()<<":"<<current->getPort()<<"|req="<<display(req)<<endl;
    //分片拉取数据
    PackageData data;
    bool hasData(false);

    if (req.gray)
    {
        hasData = UPDATETHREAD->getGrayPackageData(req.ostype, data);
    }
    else
    {
        hasData = UPDATETHREAD->getFormalPackageData(req.ostype, data);
    }

    if (hasData)
    {
        UPDATEPACKAGE_LOG << FILE_FUN << "|get data statuc" << hasData << "|os ver|" << data.ostype
            << "|ser ver|" << data.version << "|buff size|" << data.dataBuff.size() << "|gray|" << req.gray << endl;
        if (needUpdate(req.version,data.version,std::less<int>()))
        {
            UPDATEPACKAGE_LOGWARN<< FILE_FUN << "|client ver|" << req.version
                << "|is little ser ver|" << data.version <<"|inner ip|"<<req.ip<<"|req ip|"<<current->getIp()<< " client need recheck!" << endl;

            genFailRsp(req, rsp);
            return -1;
        }
        else if (needUpdate(req.version,data.version,std::greater<int>()))
        {
            UPDATEPACKAGE_LOGWARN<< FILE_FUN << "|client ver|" << req.version
                << "|is greate ser ver|" << data.version <<"|inner ip|"<<req.ip<<"|req ip|"<<current->getIp()<<" client need retry!" << endl;

            genFailRsp(req, rsp);
            return -2;
        }
    }
    else
    {
        //客户端换节点重试
        UPDATEPACKAGE_LOGERROR << FILE_FUN << "|req os ver|" << req.ostype << "|file name|"
            << req.packageName<<"|ser ver|"<<req.version <<"|sInner ip|"<<req.ip<<"|req ip|"<<current->getIp()
            <<"|data is not exist in this node, need retry!" << endl;

        genFailRsp(req, rsp);
        return -2;
    }

    if (data.dataBuff.empty())
    {
        UPDATEPACKAGE_LOGERROR << FILE_FUN << "|file name|" << req.packageName << "|os ver|"<<req.ostype
            << "|ver|" << req.version << "|" << req.offset <<  "|sInner ip|" << req.ip
            << "|req ip|" << current->getIp()<<"|data buff is null, need retry!" << endl;

        genFailRsp(req, rsp);
        return -2;
    }


    int iLen(DOWNLOAD_CHUNK);
    rsp.offset = req.offset + DOWNLOAD_CHUNK;
    rsp.finish = false;
    rsp.packageName = req.packageName;
    if (size_t(rsp.offset) >= data.dataBuff.size())
    {
        rsp.finish = true;
        iLen = data.dataBuff.size() - req.offset;
    }

    rsp.buff = data.dataBuff.substr(req.offset, iLen);
    UPDATEPACKAGE_LOG << FILE_FUN << "|get proxy data succ, buff size|" << data.dataBuff.size() << "|rsp offset|" << rsp.offset
        << "|finish|" << rsp.finish << "|iLen|" << iLen << "|file name|" << rsp.packageName
        <<"|gray|"<<req.gray<< "|inner ip|"<<req.ip<<"|req ip|"<<current->getIp()<< endl;
        return 0;
    __CATCH__
    genFailRsp(req, rsp);
    return -3;
}


Int32 RegistryImp::pushPackage(const Tseer::PushPackageInfo & pushPkInfo,std::string &result,tars::TarsCurrentPtr current)
{
    if(pushPkInfo.packageName.empty())
    {
        result = "packageName should not be empty";
        return -1;
    }

    string dirPath = TC_File::simplifyDirectory(ServerConfig::DataPath + "/" + pushPkInfo.packageName);
    UPDATEPACKAGE_LOG<< FILE_FUN<<current->getIp()<<":"<<current->getPort()
                  <<"|"<<current->getRequestId()
                  <<"|"<<pushPkInfo.packageName
                  <<"|"<<pushPkInfo.md5
                  <<"|"<<pushPkInfo.size
                  <<"|"<<pushPkInfo.pos
                  <<"|"<<pushPkInfo.version
                  <<"|"<<pushPkInfo.ostype
                  <<"|"<<pushPkInfo.package_type
                  <<"|dirPath=" << dirPath << endl;

    int ret =  FileWriterFactory::getInstance()->write(pushPkInfo,result);
    UPDATEPACKAGE_LOG<< FILE_FUN<<current->getIp()<<":"<<current->getPort()
                  <<"|"<<current->getRequestId()
                    <<"|"<<pushPkInfo.packageName
                  <<"|"<<pushPkInfo.md5
                  <<"|"<<pushPkInfo.size
                  <<"|"<<pushPkInfo.pos
                  <<"|"<<pushPkInfo.version
                  <<"|"<<pushPkInfo.ostype
                  <<"|"<<pushPkInfo.package_type
                  <<"|result="<<result
                  <<"|ret=" << ret << endl;
    return ret;
}

