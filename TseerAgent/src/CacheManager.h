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

#ifndef CACHE_MANAGER_H_
#define CACHE_MANAGER_H_

/** 文件定义了缓存管理的对象，它负责把从主控拉回来的节点信息写到磁盘上
 */

#include <string>
#include <map>
#include <vector>
#include "EndPoint.h"
#include "util/tc_file.h"
#include "util/tc_config.h"
#include "util/tc_monitor.h"


class CacheManager
{
public:
    CacheManager();

    ~CacheManager();

    void init(int updateInterval, const std::string &cacheDir);

    void updateAllCache(const std::string &obj, const std::vector<EndPoint> &activeEp, 
        const std::vector<EndPoint> &inactiveEp);

    void updateIDCCache(const std::string &obj, const std::vector<EndPoint> &activeEp, 
        const std::vector<EndPoint> &inactiveEp);

    void updateSetCache(const std::string &obj, const std::string &setInfo, 
        const std::vector<EndPoint> &activeEp, const std::vector<EndPoint> &inactiveEp);

       int getRouterFromDiskCache(const Tseer::AgentRouterRequest & req,Tseer::AgentRouterResponse &rsp,string &errMsg);
private:
    void updateCache(const std::string &obj, const std::string &subdomain, const std::vector<EndPoint> &endpoints, 
        const std::vector<EndPoint> &inactiveEp);
    
    void isTimeToUpdate(const std::string &obj);

    //把节点信息写入磁盘中；成功返回0，失败返回-1
    int writeToDisk(const std::string &obj, tars::TC_Config *conf);

private:
    //锁
    tars::TC_ThreadLock _lock;

    //<服务Obj,上次更新时间>
    std::map<std::string, int> _objLastUpdateTimes;

    //<服务Obj，节点缓存>
    std::map<std::string, tars::TC_Config *> _cache;

    //缓存文件目录
    std::string _cacheDir;

    //更新间隔
    int _updateInterval;
};

extern CacheManager *g_cacheManager;

#endif 
