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

#include "cache_manager.h"

#include <time.h>
#include <unistd.h>
#include "tc_common.h"
#include "global.h"
#include "TseerAgentComm.h"
using Tseer::RouterNodeInfo;
using Tseerapi::TC_File;
using Tseerapi::TC_Config;
using Tseerapi::TC_Common;
using std::vector;
using std::string;
using std::map;
using std::set;
using std::endl;

namespace Tseerapi {

CacheManager::CacheManager() {}

CacheManager::~CacheManager()
{
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

void CacheManager::init(const string &cacheDir)
{
    _cacheDir = cacheDir;
    Tseerapi::TC_File::makeDirRecursive(cacheDir);
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

void CacheManager::updateHashCache(const string tableKey, const string fileName, const map<long, Tseer::RouterNodeInfo> hashRamCache)
{
    size_t now = time(NULL);
    //更新缓存文件
    std::map<std::string, long>::iterator exTimeIter = _unexpreTime.find(tableKey);
    if (exTimeIter == _unexpreTime.end())
    {
        //第一次获得数据
        _unexpreTime.insert(make_pair(tableKey, now));
    }
    else
    {    //数据需要写入缓存文件
        if ((int)now > _unexpreTime.find(tableKey)->second)
        {
            //TODO:更新缓存逻辑有问题
            //需要更新缓存文件
            map<string, TC_Config*>::iterator cfIter = _uniCacheFile.find(fileName);
            if (cfIter == _uniCacheFile.end())
            {
                //文件不存在，创建新的缓存文件
                TC_Config *conf = new TC_Config();
                _uniCacheFile.insert(make_pair(fileName, conf));
                conf->insertDomain("/", "cache", true);
            }

            //文件已存在
            TC_Config *oldConf = _uniCacheFile[fileName];

            std::string ips;
            EndPoint ep;
            map<long, RouterNodeInfo>::const_iterator nodeIter = hashRamCache.begin();
            for (nodeIter = hashRamCache.begin(); nodeIter != hashRamCache.end(); nodeIter++)
            {
                EndPoint ep(nodeIter->second.isTcp,
                    "", nodeIter->second.port, 100, nodeIter->second.setname,
                    0);
                ips += TC_Common::tostr(nodeIter->first) + "@";
                ips += ep.toString();
                ips += ":";
            }

            string timeStr = TC_Common::tm2str(now, "%Y-%m-%d %H:%M:%s");
            map<string, string> subdomainValue;
            subdomainValue.insert(std::make_pair("last_update", timeStr));
            subdomainValue.insert(std::make_pair("ip_list", ips));

            map<string, string> cacheDomainValue;
            cacheDomainValue.insert(std::make_pair("last_update", timeStr));
            TC_Config newConf;
            newConf.insertDomainParam("/cache/", subdomainValue, true);
            newConf.insertDomainParam("/cache/", cacheDomainValue, true);

            oldConf->joinConfig(newConf, true);
            writeToDisk(fileName, oldConf);

            _unexpreTime[tableKey] = now + g_cache_expire_interval;

        }
    }
}

void CacheManager::updateUniCache(const string tableKey, const string fileName, const set<Tseer::RouterNodeInfo> uniRamCache)
{
    size_t now = time(NULL);
    //更新缓存文件
    std::map<std::string, long>::iterator exTimeIter = _unexpreTime.find(tableKey);
    if (exTimeIter == _unexpreTime.end())
    {
        //第一次获得数据
        _unexpreTime.insert(make_pair(tableKey, now));
    }
    else
    {    //数据需要写入缓存文件
        if ((int)now > _unexpreTime.find(tableKey)->second)
        {
            //TODO:更新缓存逻辑有问题
            //需要更新缓存文件
            map<string, TC_Config*>::iterator cfIter = _uniCacheFile.find(fileName);
            if (cfIter == _uniCacheFile.end())
            {
                //文件不存在，创建新的缓存文件
                TC_Config *conf = new TC_Config();
                _uniCacheFile.insert(make_pair(fileName, conf));
                conf->insertDomain("/", "cache", true);
            }

            //文件已存在
            TC_Config *oldConf = _uniCacheFile[fileName];

            std::string ips;
            EndPoint ep;
            set<RouterNodeInfo>::const_iterator nodeIter = uniRamCache.begin();
            for (nodeIter = uniRamCache.begin(); nodeIter != uniRamCache.end(); nodeIter++)
            {
                EndPoint ep(nodeIter->isTcp,
                    nodeIter->ip, nodeIter->port, 100, nodeIter->setname,
                    0);
                ips += "0@";
                ips += ep.toString();
                ips += ":";
            }


            string timeStr = TC_Common::tm2str(now, "%Y-%m-%d %H:%M:%s");
            map<string, string> subdomainValue;
            subdomainValue.insert(std::make_pair("last_update", timeStr));
            subdomainValue.insert(std::make_pair("ip_list", ips));

            map<string, string> cacheDomainValue;
            cacheDomainValue.insert(std::make_pair("last_update", timeStr));
            TC_Config newConf;
            newConf.insertDomainParam("/cache/", subdomainValue, true);
            newConf.insertDomainParam("/cache/", cacheDomainValue, true);

            oldConf->joinConfig(newConf, true);
            writeUniCache(fileName, oldConf);

            _unexpreTime[tableKey] = now + g_cache_expire_interval;

        }
    }
}

//TODO:写入Cache失败的处理
void CacheManager::updateCache(const string &obj, const string &subdomain, const vector<EndPoint> &activeEp,
    const vector<EndPoint> &inactiveEp)
{

    //看服务是否已经在缓存当中
    map<string, TC_Config*>::iterator it = _cache.find(obj);
    if (it == _cache.end())
    {
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
    subdomainValue.insert(std::make_pair("last_update", TC_Common::tm2str(time(NULL), "%Y-%m-%d %H:%M:%S")));
    subdomainValue.insert(std::make_pair("active_ip", activeIps));
    subdomainValue.insert(std::make_pair("inative_ip", inactiveIps));
    map<string, string> cacheDomainValue;
    cacheDomainValue.insert(std::make_pair("last_update", TC_Common::tm2str(time(NULL), "%Y-%m-%d %H:%M:%S")));
    TC_Config newConf;
    newConf.insertDomainParam("/cache/" + subdomain, subdomainValue, true);
    newConf.insertDomainParam("/cache/", cacheDomainValue, true);

    oldConf->joinConfig(newConf, true);
    writeToDisk(obj, oldConf);
}


int CacheManager::writeToDisk(const string &obj, Tseerapi::TC_Config *conf)
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
    //TC_File::save2file(filePath, conf->tostr());
    return 0;
}

int CacheManager::writeUniCache(const std::string fileName, Tseerapi::TC_Config *conf)
{
    string content = conf->tostr();
    string filePath = "routersCache/" + fileName;
    int fileFd = open(filePath.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0666);
    if (fileFd != -1)
    {
        write(fileFd, content.c_str(), content.size());
        close(fileFd);
    }
    return 0;
}

}
