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

#include "CacheManager.h"

#include <time.h>
#include <unistd.h>
#include "util/tc_timeprovider.h"
#include "util/tc_common.h"
#include "RollLogger.h"
#include "TseerAgentComm.h"
#include "RouterManager.h"
#include "util/tc_file.h"
#include "util/tc_common.h"


using namespace tars;
using namespace std;

CacheManager *g_cacheManager = NULL; 

CacheManager::CacheManager() {}

CacheManager::~CacheManager()
{
    TC_ThreadLock::Lock    sync(_lock);
    map<string, TC_Config*>::iterator it = _cache.begin();
    while (it != _cache.end())
    {
        writeToDisk(it->first, it->second);
        TC_Config *conf = it->second;
        _cache.erase(it++);
        if (conf)
        {
            delete conf;
        }
    }
}

void CacheManager::init(int updateInterval, const string &cacheDir)
{
    _updateInterval = updateInterval;
    _cacheDir = cacheDir;
    if (!TC_File::makeDirRecursive(_cacheDir))
    {
        TSEER_LOG(COMMON_LOG)->error() << FILE_FUN << "Cannot make dir: " << _cacheDir << endl;
    }
}

void CacheManager::updateAllCache(const string &obj, const vector<EndPoint> &activeEp, const vector<EndPoint> &inactiveEp)
{
    updateCache(obj, "all", activeEp, inactiveEp);
}

void CacheManager::updateIDCCache(const string &obj, const vector<EndPoint> &activeEp, const vector<EndPoint> &inactiveEp)
{
    updateCache(obj, "idc", activeEp, inactiveEp);
}

void CacheManager::updateSetCache(const string &obj, const string &setInfo, const vector<EndPoint> &activeEp, const vector<EndPoint> &inactiveEp)
{
    updateCache(obj, setInfo, activeEp, inactiveEp);
}

void CacheManager::updateCache(const string &obj, const string &subdomain, const vector<EndPoint> &activeEp, 
    const vector<EndPoint> &inactiveEp)
{
    TC_ThreadLock::Lock sync(_lock);

    //看服务是否已经在缓存当中
    map<string, int>::iterator it = _objLastUpdateTimes.find(obj);
    if (it == _objLastUpdateTimes.end())
    {
        _objLastUpdateTimes[obj] = TNOW;
        TC_Config *conf = new TC_Config();
        _cache[obj] = conf;
        conf->insertDomain("/", "cache", true);
    }

    TC_Config *oldConf = _cache[obj];

    string activeIps;    //活节点
    for (vector<EndPoint>::const_iterator it = activeEp.begin(); it != activeEp.end(); ++it)
    {
        activeIps += it->toString();
        activeIps += ":";
    }
    string inactiveIps;        //死节点
    for (vector<EndPoint>::const_iterator it = inactiveEp.begin(); it != inactiveEp.end(); ++it)
    {
        inactiveIps += it->toString();
        inactiveIps += ":";
    }

    //更新缓存
    map<string, string> subdomainValue;
    subdomainValue.insert(std::make_pair("last_update", TC_Common::tm2str(TNOW, "%Y-%m-%d %H:%M:%S")));
    subdomainValue.insert(std::make_pair("active_ip", activeIps));
    subdomainValue.insert(std::make_pair("inative_ip", inactiveIps));
    map<string, string> cacheDomainValue;
    cacheDomainValue.insert(std::make_pair("last_update", TC_Common::tm2str(TNOW, "%Y-%m-%d %H:%M:%S")));
    TC_Config newConf;
    newConf.insertDomainParam("/cache/" + subdomain, subdomainValue, true);
    newConf.insertDomainParam("/cache/", cacheDomainValue, true);

    oldConf->joinConfig(newConf, true);

    isTimeToUpdate(obj);
}

void CacheManager::isTimeToUpdate(const std::string &obj)
{
    int objLastUpdateTime = _objLastUpdateTimes[obj];
    if (objLastUpdateTime + _updateInterval <= TNOW)
    {
        writeToDisk(obj, _cache[obj]);
        _objLastUpdateTimes[obj] = TNOW;
    }
}

int CacheManager::writeToDisk(const string &obj, tars::TC_Config *conf)
{

    //用linux的系统调用write来执行，保证原子性
    string content = conf->tostr();
    string filePath = _cacheDir + obj;

    int fileFd = open(filePath.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0666);
    if (fileFd != -1)
    {
        write(fileFd, content.c_str(), content.size());
        close(fileFd);
    }
    TSEER_LOG(COMMON_LOG)->debug() << FILE_FUN << "Write " << obj << " to disk" << endl;
    return 0;
}

int CacheManager::getRouterFromDiskCache(const Tseer::AgentRouterRequest & req,Tseer::AgentRouterResponse &rsp,string &errMsg)
{
    //文件名是Obj的名字
    string sFileName = _cacheDir + req.obj;
    if (!TC_File::isFileExist(sFileName))
    {
        ostringstream os;
        os << FILE_FUN << "cannot find cache file :" << sFileName;
        errMsg = os.str();
        return -1;
    }

    try
    {
        TC_Config conf;
        conf.parseFile(sFileName);

        //缓存的目标域根据请求来确定
        std::string targetDomain;
        switch (req.getType)
        {
                case Tseer::GET_ALL:
                    targetDomain = "all";
                    break;
                case Tseer::GET_IDC:
                    targetDomain = "idc";
                    break;
                case Tseer::GET_SET:
                    targetDomain = req.setname;
                    break;
                default:
                {
                    ostringstream os;
                    os << FILE_FUN << "Unknown RouterRequest get type";
                    errMsg = os.str();
                    return -1;
                }
        }

        std::string endpoints = conf.get("/cache/" + targetDomain + "<active_ip>");

        if (endpoints.empty())
        {
            ostringstream os;
            os << FILE_FUN << "not found active ip in cache";
            errMsg = os.str();
            return -1;
        }

        //节点
        vector<Tseer::RouterNodeInfo> nodeInfoList;
        
        const std::string sep = ":";
        std::vector<std::string> endpointList = TC_Common::sepstr<std::string>(endpoints, sep);
        for (std::vector<std::string>::iterator it = endpointList.begin(); it != endpointList.end(); ++it)
        {
                    try
                    {
                    EndPoint ep;
                    std::string parseErrMsg;
                    if (!ep.parse(*it, parseErrMsg))
                    {
                        nodeInfoList.push_back(ep.toRouterNodeInfo());
                    }
                    }catch(...)
                    {
                    }
        }

        //写入出参
        rsp.obj = req.obj;
        rsp.getType = req.getType;
        rsp.setname = req.setname;
        rsp.resultList = nodeInfoList;

        return 0;
    }
    catch (exception &ex)
    {
        ostringstream os;
        os << FILE_FUN << "get cache from " << sFileName << "|exception:" << ex.what();
        errMsg = os.str();
    }
    catch (...)
    {
        ostringstream os;
        os << FILE_FUN << "get cache from " << sFileName << "|unknown exception";
        errMsg = os.str();
    }

    return -1;
}


