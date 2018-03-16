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

#ifndef __TSEER_API_CACHE_MANAGER_H_
#define __TSEER_API_CACHE_MANAGER_H_

/** 文件定义了缓存管理的对象，它负责把从Tseer服务端拉回来的节点信息写到磁盘上
 *    线程不安全
 */

#include <string>
#include <map>
#include <vector>
#include <set>
#include "EndPoint.h"
#include "tc_file.h"
#include "tc_config.h"

namespace Tseerapi
{

class CacheManager
{
public:
    CacheManager();

    ~CacheManager();

    void init(const std::string &cacheDir);

    void updateAllCache(const std::string &obj, const std::vector<EndPoint> &activeEp, 
        const std::vector<EndPoint> &inactiveEp);

    void updateIDCCache(const std::string &obj, const std::vector<EndPoint> &activeEp, 
        const std::vector<EndPoint> &inactiveEp);

    void updateSetCache(const std::string &obj, const std::string &setInfo, 
        const std::vector<EndPoint> &activeEp, const std::vector<EndPoint> &inactiveEp);

    void updateHashCache(const std::string tableKey, const std::string fileName, const std::map<long, Tseer::RouterNodeInfo> uniRamCache);
    void updateUniCache(const std::string tableKey, const std::string fileName, const std::set<Tseer::RouterNodeInfo> uniRamCache);
    
private:
    void updateCache(const std::string &obj, const std::string &subdomain, const std::vector<EndPoint> &endpoints, 
        const std::vector<EndPoint> &inactiveEp);

    //把节点信息写入磁盘中；成功返回0，失败返回-1
    int writeToDisk(const std::string &obj, Tseerapi::TC_Config *conf);
    int writeUniCache(const std::string fileName, Tseerapi::TC_Config *conf);

private:

    //<服务Obj，节点缓存>
    std::map<std::string, Tseerapi::TC_Config *> _cache;

    //缓存文件目录
    std::string _cacheDir;

    //uni cache的访问时间
    std::map<std::string, long> _unexpreTime;
    //uni cache文件
    std::map<std::string, Tseerapi::TC_Config*> _uniCacheFile;

};

}

#endif // !CACHE_MANAGER_H_
