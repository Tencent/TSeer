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

#include "route_info_provider.h"
#include <sys/syscall.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>
#include <set>
#include "dns_packer.h"
#include "EndpointF.h"
#include "global.h"
#include "tc_config.h"
#include "tc_common.h"
#include "EndPoint.h"
#include "TseerAgentComm.h"
#include "conn.h"
#include "tup.h"
using namespace Tseer;
using namespace Tseerapi;
using namespace std;


static unsigned int registry_seq = 0;
static unsigned int agent_seq = 0;
static const unsigned int RECV_BUFFER_SIZE = 65535;

namespace
{
    void EndpointF2EndPoint(const vector<EndpointF> &EpfVec, vector<EndPoint> &EpVec)
    {
        for (vector<EndpointF>::const_iterator it = EpfVec.begin(); it != EpfVec.end(); ++it)
        {
            EndPoint ep;
            ep._host = it->host;
            ep._istcp = it->istcp == 1 ? true : false;
            ep._port = it->port;
            ep._weight = it->weight;
            ep._set = it->setId;
            ep._timeout = it->timeout;
            EpVec.push_back(ep);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////

RegistryProvider::RegistryProvider() :_registryEpMgr(NULL) {}

void RegistryProvider::init(const std::string &cacheDir, RegistryEpManager *mgr)
{
    _cacheMgr.init(cacheDir);
    _registryEpMgr = mgr;
    _lastReportApiVersionTime = 0;
}

int RegistryProvider::getRouteInfo(const InnerRouterRequest &req, Tseer::AgentRouterResponse &rsp, std::string &errMsg)
{
    //获取Tseer服务端地址
    if (!_registryEpMgr)
    {
        ostringstream os;
        os << FILE_FUN << "fatal error, null registry ep manager";
        errMsg = os.str();
        return -1;
    }

    std::string subErr;
    std::string registryIp;
    if (_registryEpMgr->getAvaliableIp(registryIp, subErr))
    {
        ostringstream os;
        os << FILE_FUN << "get ip failed";
        errMsg = mergeErrMsg(os.str(), subErr);
        return -1;
    }

    //入参
    vector<Tseerapi::EndpointF> activeEpF;
    vector<Tseerapi::EndpointF> inactiveEpF;
    TarsUniPacket<> packet;
    __asm__ __volatile__ ("lock; incl %0\n\t" : "+m" (registry_seq) :: "memory");
    packet.setRequestId(registry_seq);
    packet.setVersion(3);
    packet.setServantName(g_registry_obj);
    packet.put<string>("id", req.obj);
    switch (req.lbGetType)
    {
    case LB_GET_ALL:
        packet.setFuncName(g_registry_get_all_func);
        break;
    case LB_GET_IDC:
        packet.setFuncName(g_registry_get_idc_func);
        break;
    case LB_GET_SET:
        packet.setFuncName(g_registry_get_set_func);
        packet.put<string>("setId", req.setInfo);
        break;
    }
    
    {
        //每30分钟上报一次自身key、版本给tseerserver
        time_t curr = time(NULL);
        if (curr - _lastReportApiVersionTime > 30 * 60)
        {
            //透传客户端服务key和版本号给tseerserver
            map <std::string, std::string> value;
            value.insert(make_pair(g_TSEERAPI_MASTER_KEY_STR, g_tseer_client_key));
            if (g_reserved_value.empty())
            {
                value.insert(make_pair(g_TSEERAPI_VERSION_STR, g_pure_api_version));
            }
            else
            {
                value.insert(make_pair(g_TSEERAPI_VERSION_STR, g_reserved_value));
            }
            packet.setTarsContext(value);

            _lastReportApiVersionTime = curr;
        }

    }
    
    string buffStr("");
    try
    {
        packet.encode(buffStr);
    }
    catch (exception &ex)
    {
        ostringstream os;
        os << FILE_FUN << "encode failed : " << ex.what();
        errMsg = os.str();
        return -1;
    }
    catch (...)
    {
        ostringstream os;
        os << FILE_FUN << "encode unknown exception";
        errMsg = os.str();
        return -1;
    }

    //发送请求
    char buff[RECV_BUFFER_SIZE];
    unsigned int recvBuffSize = sizeof(buff);
    int ret = Conn::QueryAndRecvRouterFromRegistry(buffStr.c_str(), buffStr.size(), g_agent_timeout, buff, recvBuffSize, registryIp, subErr);
    if (ret < 0)
    {
        ostringstream os;
        os << FILE_FUN << "send and recv data failed";
        errMsg = mergeErrMsg(os.str(), subErr);

        //向RegistryEpManager报道调用失败
        _registryEpMgr->reportResult(registryIp, -1);

        return ret;
    }


    //解码结果
    TarsUniPacket<> response;
    try
    {
        const char *p = buff;
        response.decode(p, recvBuffSize);
        if (response.getTarsResultCode() != 0)
        {
            ostringstream buffer;
            buffer << FILE_FUN << "getRouteFromRegistry error|ret:" << response.getTarsResultCode();
            errMsg = buffer.str();

            //向RegistryEpManager报道调用失败
            _registryEpMgr->reportResult(registryIp, -1);

            return -1;
        }
        else
        {
            //向RegistryEpManager报道调用成功
            _registryEpMgr->reportResult(registryIp, 0);
        }

        activeEpF = response.get<vector<Tseerapi::EndpointF> >("activeEp");
        inactiveEpF = response.get<vector<Tseerapi::EndpointF> >("inactiveEp");
    }
    catch (exception &ex)
    {
        ostringstream os;
        os << FILE_FUN << "decode exception: " << ex.what();
        errMsg = os.str();
        return -1;
    }
    catch (...)
    {
        ostringstream os;
        os << FILE_FUN << "decode unknown exception";
        errMsg = os.str();
        return -1;
    }

    vector<RouterNodeInfo> RouterNodesTmpVec;
    for (size_t i = 0; i < activeEpF.size(); i++)
    {
        RouterNodeInfo nodeInfo;
        nodeInfo.ip = activeEpF[i].host;
        nodeInfo.port = activeEpF[i].port;
        nodeInfo.isTcp = (activeEpF[i].istcp > 0 ? true : false);
        nodeInfo.setname = (activeEpF[i].setId);
        if (activeEpF[i].weight < 0)
        {
            nodeInfo.weight = 100;
        }
        else
        {
            nodeInfo.weight = activeEpF[i].weight;
        }

        RouterNodesTmpVec.push_back(nodeInfo);
    }

    //把结果放入出参
    rsp.obj = req.obj;
    rsp.resultList = RouterNodesTmpVec;

    //把EndPointF转换成EndPoint，并存到磁盘上
    vector<EndPoint> activeEp;
    vector<EndPoint> inactiveEp;
    ::EndpointF2EndPoint(activeEpF, activeEp);
    ::EndpointF2EndPoint(inactiveEpF, inactiveEp);

    switch (req.lbGetType)
    {
    case LB_GET_ALL:
        _cacheMgr.updateAllCache(req.obj, activeEp, inactiveEp);
        break;
    case LB_GET_IDC:
        _cacheMgr.updateIDCCache(req.obj, activeEp, inactiveEp);
        break;
    case LB_GET_SET:
        _cacheMgr.updateSetCache(req.obj, req.setInfo, activeEp, inactiveEp);
        break;
    }

    return 0;
}

bool RegistryProvider::isAvailable() const
{
    return true;
}

bool RegistryProvider::addFailedNumAndCheckAvailable()
{
    return true;
}

void RegistryProvider::setAvailable() {}

////////////////////////////////////////////////////////////////////////////////////////

const int AgentProvider::AGENT_FAIL_THRESHOLD = 3;

AgentProvider::AgentProvider() :_failedNum(0), _revive_time(0)
{
    _tid = syscall(SYS_gettid);
}

void AgentProvider::init(const std::string &cacheDir)
{
    _cacheMgr.init(cacheDir);
}

int AgentProvider::getRouteInfo(const InnerRouterRequest &req, Tseer::AgentRouterResponse &rsp, std::string &errMsg)
{
    //设置请求
    Tseer::AgentRouterRequest request;
    switch (req.lbGetType)
    {
    case LB_GET_ALL:
        request.getType = GET_ALL;
        break;
    case LB_GET_IDC:
        request.getType = GET_IDC;
        break;
    case LB_GET_SET:
        request.getType = GET_SET;
        break;
    }
    request.obj = req.obj;
    if (request.getType == GET_SET)
    {
        request.setname = req.setInfo;
    }
    else
    {
        request.setname = "";
    }
    
    switch (req.type)
    {
    case LB_TYPE_LOOP:
        request.lbType = Tseer::LB_TYPE_LOOP;
        break;
    case LB_TYPE_RANDOM:
        request.lbType = Tseer::LB_TYPE_RANDOM;
        break;
    case LB_TYPE_STATIC_WEIGHT:
        request.lbType = Tseer::LB_TYPE_STATIC_WEIGHT;
        break;
        case LB_TYPE_CST_HASH:
            request.lbType = Tseer::LB_TYPE_CST_HASH;
            break;
        case LB_TYPE_ALL:
            request.lbType = Tseer::LB_TYPE_ALL;
            break;
    }

    request.apiVer = g_agent_api_version;
    request.identifier = TC_Common::tostr(getpid());

    //编码到TUP
    TarsUniPacket<> packet;
    __asm__ __volatile__ ("lock; incl %0\n\t" : "+m" (agent_seq) :: "memory");
    packet.setRequestId(agent_seq);
    packet.setVersion(3);
    packet.setServantName(g_agent_router_obj);
    packet.setFuncName(g_agent_router_func);
    packet.put<Tseer::AgentRouterRequest>("req", request);

    {
        //透传客户端模块KEY给Agent和版本信息
        map <std::string, std::string> value;
        value.insert(make_pair(g_TSEERAPI_MASTER_KEY_STR, g_tseer_client_key));
        if (g_reserved_value.empty())
        {
            value.insert(make_pair(g_TSEERAPI_VERSION_STR, g_agent_api_version));
        }
        else
        {
            value.insert(make_pair(g_TSEERAPI_VERSION_STR, g_reserved_value));
        }
        packet.setTarsContext(value);
    }
    
    string buffStr("");
    try
    {
        packet.encode(buffStr);
    }
    catch (exception &ex)
    {
        ostringstream os;
        os << FILE_FUN << "encode failed: " << ex.what();
        errMsg = os.str();
        return -1;
    }
    catch (...)
    {
        ostringstream os;
        os << FILE_FUN << "encode unknown exception";
        errMsg = os.str();
        return -1;
    }

    //发送请求
    char buff[RECV_BUFFER_SIZE];
    unsigned int recvBuffSize = sizeof(buff);
    int ret = Conn::QueryAndRecvRouterFromAgent(buffStr.c_str(), buffStr.size(), g_agent_timeout, buff, recvBuffSize, errMsg);
    if (ret < 0)
    {
        return ret;
    }

    //解码结果
    TarsUniPacket<> response;
    Tseer::AgentRouterResponse agentRsp;
    try
    {
        const char *p = buff;
        response.decode(p, recvBuffSize);

        if (response.getTarsResultCode() != 0)
        {
            ostringstream buffer;
            buffer << FILE_FUN << "getRouteFromAgent rsp error|ret:" << response.getTarsResultCode();
            errMsg = buffer.str();
            return -1;
        }
        agentRsp = response.get<Tseer::AgentRouterResponse>("rsp");
    }
    catch (exception &ex)
    {
        ostringstream os;
        os << FILE_FUN << "decode exception: " << ex.what();
        errMsg = os.str();
        return -1;
    }
    catch (...)
    {
        ostringstream os;
        os << FILE_FUN << "decode unknown exception";
        errMsg = os.str();
        return -1;
    }
    //写入出参
    rsp = agentRsp;

    vector<EndPoint> activeEp;
    vector<EndPoint> inactiveEp;
    for(size_t i = 0; i < agentRsp.resultList.size(); i++) {
        EndPoint ep(agentRsp.resultList[i].isTcp, agentRsp.resultList[i].ip, agentRsp.resultList[i].port, 0, agentRsp.resultList[i].setname, agentRsp.resultList[i].weight);
        activeEp.push_back(ep);
    }
    if (!req.isAgent)
    {
        //如果使用agent get router方式，则不在这里更新缓存
        switch (req.lbGetType)
        {
        case LB_GET_ALL:
            _cacheMgr.updateAllCache(req.obj, activeEp, inactiveEp);
            break;
        case LB_GET_IDC:
            _cacheMgr.updateIDCCache(req.obj, activeEp, inactiveEp);
            break;
        case LB_GET_SET:
            _cacheMgr.updateSetCache(req.obj, req.setInfo, activeEp, inactiveEp);
            break;
        }
    }

    return 0;
}

bool AgentProvider::isAvailable() const
{
    return time(NULL) >= _revive_time;
}

bool AgentProvider::addFailedNumAndCheckAvailable()
{
    ++_failedNum;
    if (_failedNum >= AGENT_FAIL_THRESHOLD)
    {
        _revive_time = time(NULL) + AGENT_FAIL_THRESHOLD * g_node_normal_expire_interval;
        _failedNum = 0;
        return false;
    }
    return true;
}

void AgentProvider::setAvailable()
{
    _failedNum   = 0;
    _revive_time = 0;
}

////////////////////////////////////////////////////////////////////////////////////////

CacheProvider::CacheProvider() {}

void CacheProvider::setCacheDir(const std::string &cacheDir)
{
    _cacheDir = cacheDir;
}

int CacheProvider::getRouteInfo(const InnerRouterRequest &req, Tseer::AgentRouterResponse &rsp, std::string &errMsg)
{
    //文件名是Obj的名字
    string fileName = _cacheDir + req.obj;
    if (!isFileExist(fileName))
    {
        ostringstream os;
        os << FILE_FUN << "cannot find cache file :" << fileName;
        errMsg = os.str();
        return -1;
    }

    try
    {
        Tseerapi::TC_Config conf;
        conf.parseFile(fileName);

        //缓存的目标域根据请求来确定
        std::string targetDomain;
        switch (req.lbGetType)
        {
        case Tseerapi::LB_GET_ALL:
            targetDomain = "all";
            break;
        case Tseerapi::LB_GET_IDC:
            targetDomain = "idc";
            break;
        case Tseerapi::LB_GET_SET:
            targetDomain = req.setInfo;
            break;
        default:
        {
            ostringstream os;
            os << FILE_FUN << "Unknown RouterRequest get type";
            errMsg = os.str();
            return -1;
        }
        }

        std::string ips = conf.get("/cache/" + targetDomain + "<active_ip>");

        if (ips.empty())
        {
            ostringstream os;
            os << FILE_FUN << "not found active ip in cache";
            errMsg = os.str();
            return -1;
        }

        //节点
        vector<Tseer::RouterNodeInfo> resultVec;
        const std::string sep = ":";
        std::vector<std::string> ip = Tseerapi::TC_Common::sepstr<std::string>(ips, sep);
        for (std::vector<std::string>::iterator it = ip.begin(); it != ip.end(); ++it)
        {
                    try
                    {
                    EndPoint ep;
                    std::string parTseerrMsg;
                    if (!ep.parse(*it, parTseerrMsg))
                    {
                        resultVec.push_back(ep.toRouterNodeInfo());
                    }
                    }catch(...)
                    {
                    }
        }

        //写入出参
        rsp.obj = req.obj;
        rsp.resultList = resultVec;

        return 0;
    }
    catch (exception &ex)
    {
        ostringstream os;
        os << FILE_FUN << "get cache from " << fileName << "|exception:" << ex.what();
        errMsg = os.str();
    }
    catch (...)
    {
        ostringstream os;
        os << FILE_FUN << "get cache from " << fileName << "|unknown exception";
        errMsg = os.str();
    }

    return -1;
}

int CacheProvider::getRouteInfo(const InnerRouterRequest &req, Tseer::AgentRouterResponse &rsp, std::map<long, Tseer::RouterNodeInfo> &routerNodes, std::string &errMsg)
{
    string fileName = _cacheDir + req.obj + req.slaveSet + TC_Common::tostr(req.lbGetType) + TC_Common::tostr(req.type);
    if (!isFileExist(fileName))
    {
        ostringstream os;
        os << FILE_FUN << "cannot find cache file : " << fileName;
        errMsg = os.str();
        return -1;
    }

    try
    {
        Tseerapi::TC_Config conf;
        conf.parseFile(fileName);

        std::string ips = conf.get("/cache/<ip_list>");

        if (ips.empty())
        {
            ostringstream os;
            os << FILE_FUN << "not found ip in cache";
            errMsg = os.str();
            return -1;
        }

        vector<Tseer::RouterNodeInfo> resultVec;
        Tseer::RouterNodeInfo tmpRouterNode;
        const std::string sep = ":";
        std::vector<std::string> ip = Tseerapi::TC_Common::sepstr<std::string>(ips, sep);
        std::vector<std::string> tmpVec;
        for (std::vector<std::string>::iterator it = ip.begin(); it != ip.end(); ++it)
        {
            EndPoint ep;
            std::string parTseerrMsg;
            tmpVec = Tseerapi::TC_Common::sepstr<std::string>(*it, "@");
            if (!ep.parse(tmpVec[1], parTseerrMsg))
            {
                tmpRouterNode = ep.toRouterNodeInfo();
                resultVec.push_back(tmpRouterNode);
                routerNodes.insert(make_pair(TC_Common::strto<long>(tmpVec[0]), tmpRouterNode));
            }
            else
            {
                ostringstream os;
                os << FILE_FUN << "parse endpoint failed" << endl;
                errMsg = os.str();
            }
        }

        rsp.obj = req.obj;
        rsp.resultList = resultVec;

        return 0;
    }
    catch (exception &ex)
    {
        ostringstream os;
        os << FILE_FUN << "get cache from " << fileName << "|exception: " << ex.what();
        errMsg = os.str();
    }
    catch (...)
    {
        ostringstream os;
        os << FILE_FUN << "get cache from " << fileName << "|unknown exception";
        errMsg = os.str();
    }

    return -1;
}

int CacheProvider::getRouteInfo(const InnerRouterRequest &req, Tseer::AgentRouterResponse &rsp, std::set<Tseer::RouterNodeInfo> &routerNodes, std::string &errMsg)
{
    string fileName = _cacheDir + req.obj + req.slaveSet + TC_Common::tostr(req.lbGetType) + TC_Common::tostr(req.type);
    if (!isFileExist(fileName))
    {
        ostringstream os;
        os << FILE_FUN << "cannot find cache file : " << fileName;
        errMsg = os.str();
        return -1;
    }

    try
    {
        Tseerapi::TC_Config conf;
        conf.parseFile(fileName);

        std::string ips = conf.get("/cache/<ip_list>");

        if (ips.empty())
        {
            ostringstream os;
            os << FILE_FUN << "not found ip in cache";
            errMsg = os.str();
            return -1;
        }

        vector<Tseer::RouterNodeInfo> resultVec;
        Tseer::RouterNodeInfo tmpRouterNode;
        const std::string sep = ":";
        std::vector<std::string> ip = Tseerapi::TC_Common::sepstr<std::string>(ips, sep);
        std::vector<std::string> tmpVec;
        for (std::vector<std::string>::iterator it = ip.begin(); it != ip.end(); ++it)
        {
            EndPoint ep;
            std::string parTseerrMsg;
            tmpVec = Tseerapi::TC_Common::sepstr<std::string>(*it, "@");
            if (!ep.parse(tmpVec[1], parTseerrMsg))
            {
                tmpRouterNode = ep.toRouterNodeInfo();
                resultVec.push_back(tmpRouterNode);
                routerNodes.insert(tmpRouterNode);
            }
            else
            {
                ostringstream os;
                os << FILE_FUN << "parse endpoint failed" << endl;
                errMsg = os.str();
                return -1;
            }
        }

        rsp.obj = req.obj;
        rsp.resultList = resultVec;

        return 0;
    }
    catch (exception &ex)
    {
        ostringstream os;
        os << FILE_FUN << "get cache from " << fileName << "|exception: " << ex.what();
        errMsg = os.str();
    }
    catch (...)
    {
        ostringstream os;
        os << FILE_FUN << "get cache from " << fileName << "|unknown exception";
        errMsg = os.str();
    }

    return -1;
}

bool CacheProvider::isAvailable() const
{
    return true;
}

bool CacheProvider::addFailedNumAndCheckAvailable()
{
    return true;
}

void CacheProvider::setAvailable() {}

bool CacheProvider::isFileExist(const std::string sFullFileName)
{
    ifstream ifs(sFullFileName.c_str());
    if (ifs.good())
    {
        return true;
    }
    else
    {
        return false;
    }
}
