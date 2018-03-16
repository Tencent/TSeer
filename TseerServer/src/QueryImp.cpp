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

#include "QueryImp.h"
#include "util.h"

void QueryImp::initialize()
{
    TLOGDEBUG("begin QueryImp init"<<endl);

    /* 初始化路由缓存实例*/
    extern TC_Config * g_pconf;
    _storeCache.init(g_pconf);
}

vector<Tseer::EndpointF> QueryImp::findObjectById(const string & id, tars::TarsCurrentPtr current)
{
    vector<Tseer::EndpointF> eps = _storeCache.findObjectById(id);

    ostringstream os;
    doDaylog(FUNID_findObjectById,id,eps,vector<Tseer::EndpointF>(),current,os);

    return eps;
}

tars::Int32 QueryImp::findObjectById4Any(const std::string & id,vector<Tseer::EndpointF> &activeEp,vector<Tseer::EndpointF> &inactiveEp,tars::TarsCurrentPtr current)
{
    int ret = _storeCache.findObjectById4All(id, activeEp, inactiveEp);

    ostringstream os;
    doDaylog(FUNID_findObjectById4Any,id,activeEp,inactiveEp,current,os);

    return ret;
}

tars::Int32 QueryImp::findObjectById4All(const std::string & id, vector<Tseer::EndpointF> &activeEp,vector<Tseer::EndpointF> &inactiveEp,tars::TarsCurrentPtr current)
{
    ostringstream os;

    int ret = _storeCache.findObjectByIdInGroupPriority(id,current->getIp(),activeEp, inactiveEp,os);

    doDaylog(FUNID_findObjectById4All,id,activeEp,inactiveEp,current,os);

    return ret;
}

tars::Int32 QueryImp::findObjectByIdInSameGroup(const std::string & id, vector<Tseer::EndpointF> &activeEp,vector<Tseer::EndpointF> &inactiveEp, tars::TarsCurrentPtr current)
{
    ostringstream os;
    int ret = _storeCache.findObjectByIdInGroupPriority(id, current->getIp(), activeEp, inactiveEp, os);

    doDaylog(FUNID_findObjectByIdInSameGroup,id,activeEp,inactiveEp,current,os);

    return ret;
}

Int32 QueryImp::findObjectByIdInSameStation(const std::string & id, const std::string & station, vector<Tseer::EndpointF> &activeEp, vector<Tseer::EndpointF> &inactiveEp, tars::TarsCurrentPtr current)
{
    ostringstream os;

    int ret = _storeCache.findObjectByIdInSameStation(id, station, activeEp, inactiveEp, os);

    doDaylog(FUNID_findObjectByIdInSameStation,id,activeEp,inactiveEp,current,os);

    return ret;
}

Int32 QueryImp::findObjectByIdInSameSet(const std::string & id,const std::string & setId,vector<Tseer::EndpointF> &activeEp,vector<Tseer::EndpointF> &inactiveEp, tars::TarsCurrentPtr current)
{
    vector<string> setInfo = TC_Common::sepstr<string>(setId,".");

    if (setInfo.size()!=3 ||(setInfo.size()==3&&(setInfo[0]=="*"||setInfo[1]=="*")))
    {
        TLOGERROR("QueryImp::findObjectByIdInSameSet:|set full name error[" << id << "_" << setId <<"]|" << current->getIp() << endl);
        return -1;
    }

    ostringstream os;
    int ret = _storeCache.findObjectByIdInSameSet(id, setInfo, activeEp, inactiveEp, os);
    if (-1 == ret)
    {
        //未启动set，启动ip分组策略
        return findObjectByIdInSameGroup(id, activeEp, inactiveEp, current);
    }
    else if (-2 == ret)
    {
        //启动了set，但未找到任何服务节点
        TLOGERROR("QueryImp::findObjectByIdInSameSet |no one server found for [" << id << "_" << setId <<"]|" << current->getIp() << endl);
        return -1;
    }
    else if (-3 == ret)
    {
        //启动了set，但未找到任何地区set,严格上不应该出现此类情形,配置错误或主调设置错误会引起此类错误
        TLOGERROR("QueryImp::findObjectByIdInSameSet |no set area found [" << id << "_" << setId <<"]|" << current->getIp()  << endl);
        return -1;
    }

    doDaylog(FUNID_findObjectByIdInSameSet,id,activeEp,inactiveEp,current,os,setId);

    return ret;
}

void QueryImp::doDaylog(const FUNID funId,const string& id,const vector<Tseer::EndpointF> &activeEp, const vector<Tseer::EndpointF> &inactiveEp, const tars::TarsCurrentPtr& current,const ostringstream& os,const string& setId)
{
    string epListStr;
    for(size_t i = 0; i < activeEp.size(); i++)
    {
        if(0 != i)
        {
            epListStr += ";";
        }
        epListStr += activeEp[i].host + ":" + TC_Common::tostr(activeEp[i].port);
    }

    epListStr += "|";

    for(size_t i = 0; i < inactiveEp.size(); i++)
    {
        if(0 != i)
        {
            epListStr += ";";
        }
        epListStr += inactiveEp[i].host + ":" + TC_Common::tostr(inactiveEp[i].port);
    }

    switch(funId)
    {
        case FUNID_findObjectById4All:
        case FUNID_findObjectByIdInSameGroup:
        {
            FDLOG("query_idc") << eFunTostr(funId)<<"|"<<current->getIp() << "|"<< current->getPort() << "|" << id << "|" <<setId << "|" << epListStr <<os.str()<< endl;
        }
        break;
        case FUNID_findObjectByIdInSameSet:
        {
            FDLOG("query_set") << eFunTostr(funId)<<"|"<<current->getIp() << "|"<< current->getPort() << "|" << id << "|" <<setId << "|" << epListStr <<os.str()<< endl;
        }
        break;
        case FUNID_findObjectById4Any:
        case FUNID_findObjectById:
        case FUNID_findObjectByIdInSameStation:
        default:
        {
            FDLOG("query") << eFunTostr(funId)<<"|"<<current->getIp() << "|"<< current->getPort() << "|" << id << "|" <<setId << "|" << epListStr <<os.str()<< endl;
        }
        break;
    }
}

string QueryImp::eFunTostr(const FUNID funId)
{
    string fun = "";
    switch(funId)
    {
        case FUNID_findObjectByIdInSameGroup:
        {
            fun = "findObjectByIdInSameGroup";
        }
        break;
        case FUNID_findObjectByIdInSameSet:
        {
            fun = "findObjectByIdInSameSet";
        }
        break;
        case FUNID_findObjectById4Any:
        {
            fun = "findObjectById4All";
        }
        break;
        case FUNID_findObjectById:
        {
            fun = "findObjectById";
        }
        break;
        case FUNID_findObjectById4All:
        {
            fun = "findObjectById4All";
        }
        break;
        case FUNID_findObjectByIdInSameStation:
        {
            fun = "findObjectByIdInSameStation";
        }
        break;
        default:
            fun = "UNKNOWN";
        break;
    }
    return fun;
}

