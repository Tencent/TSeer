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

#ifndef __STORE_CACHE_H__
#define __STORE_CACHE_H__

#include <set>
#include "util/tc_common.h"
#include "util/tc_config.h"
#include "util/tc_monitor.h"
#include "util/tc_file.h"
#include "jmem/jmem_hashmap.h"
#include "util/tc_readers_writer_data.h"
#include "servant/TarsLogger.h"

#include "RegistryDescriptor.h"
#include "EtcdDataCache.h"

#define GROUPCACHEFILE      "serverGroupCache.dat"
#define GROUPPROICACHEFILE  "GroupPrioCache.dat"

using namespace tars;
using namespace Tseer;

/* <servant, ObjectItem> */
typedef map<string, ObjectItem> ObjectsCache;

/* _mapServantStatus的key */
struct ServantStatusKey 
{
    string  application;
    string  serverName;
    string  nodeName;
};
inline bool operator<(const ServantStatusKey&l, const ServantStatusKey&r)
{
    if(l.application != r.application)
        return(l.application < r.application);
    if(l.serverName != r.serverName)
        return(l.serverName < r.serverName);
    if(l.nodeName != r.nodeName)
        return(l.nodeName < r.nodeName);
    return(false);
}


class StoreCache
{
private:
    //对应t_group_priority表
    struct GroupPriorityEntry
    {
        std::string grouID;           //id
        std::string station;           //station
        std::set<int>   setGroupID;     //group_list
    };

    enum GroupUseSelect
    {
        ENUM_USE_WORK_GROUPID,
        ENUM_USE_REAL_GROUPID
    };

    /* set中服务的信息 */
    struct SetServerInfo
    {
        string      setId;
        string      setArea;
        bool        isActive;
        Tseer::EndpointF   epf;
    };

    /* /<servant,setname,vector> */
    typedef map<string, map<string, vector<StoreCache::SetServerInfo> > > SetDivisionCache;

public:
    static string WEIGHT_SUCC_RATIO;        /* 成功率占比字符串常量 */

    static string WEIGHT_AVGTIME_RATIO;     /* 平均耗时占比字符串常量 */

    static string WEIGHT_CPULOAD_RATIO;     /* cpu负载占比字符串常量 */

    static string WEIGHT_ADJUST_THRESHOLD;  /* 权重调节阀值 */

public:

    /**
     * 构造函数
     */
    StoreCache(){}

    /**
     * 初始化
     * @param pconf 配置文件
     * @return 0-成功 others-失败
     */
    int init(TC_Config *pconf);
    
    /** 根据id获取对象
     * @param id 对象名称
     * @return  返回所有该对象的活动endpoint列表
     */
    vector<Tseer::EndpointF> findObjectById(const string& id);


    /** 根据id获取对象
     *
     * @param id 对象名称
     * @out param activeEp    存活的列表
     * @out param inactiveEp  非存活的列表
     *
     * @return 0-成功 others-失败
     */
    int findObjectById4All(const string& id,
                   vector<Tseer::EndpointF>& activeEp,
                   vector<Tseer::EndpointF>& inactiveEp);


    /** 根据id获取同组对象
     *
     * @param id 对象名称
     * @param ip
     * @out param activeEp    存活的列表
     * @out param inactiveEp  非存活的列表
     * @out param os          打印日志使用
     *
     * @return 0-成功 others-失败
     */
    int findObjectByIdInSameGroup(const string& id,
                      const string& ip,
                      vector<Tseer::EndpointF>& activeEp,
                      vector<Tseer::EndpointF>& inactiveEp,
                      ostringstream&os);


    /** 根据id获取优先级序列中的对象
     *
     * @param id 对象名称
     * @param ip
     * @out param activeEp    存活的列表
     * @out param inactiveEp  非存活的列表
     * @out param os          打印日志使用
     *
     * @return 0-成功 others-失败
     */
    int findObjectByIdInGroupPriority(const std::string&id, const std::string&ip, std::vector<Tseer::EndpointF>& vecActive, std::vector<Tseer::EndpointF>& vecInactive,
                      std::ostringstream& os);


    /** 根据id和归属地获取全部对象
     *
     * @param id 对象名称
     * @param stattion 归属地
     * @out param activeEp    存活的列表
     * @out param inactiveEp  非存活的列表
     * @out param os          打印日志使用
     *
     * @return 0-成功 others-失败
     */
    int findObjectByIdInSameStation(const std::string&id, const std::string& stattion, std::vector<Tseer::EndpointF>& vecActive, std::vector<Tseer::EndpointF>& vecInactive,
                    std::ostringstream& os);

    /** 根据id获取对象同set endpoint列表
     *
     * @param id         对象名称
     * @param setInfo  set全称,格式为setname.setarea.setgroup
     * @param activeEp   存活endpoint列表
     * @param inactiveEp 非存活endpoint列表
     * @return:  0-成功  others-失败
     */
    int findObjectByIdInSameSet(const string&id, const vector<string>&setInfo, std::vector<Tseer::EndpointF>& vecActive, std::vector<Tseer::EndpointF>& vecInactive,
                    std::ostringstream& os);


    int findObjectByIdInSameSet(const string&setId, const vector<SetServerInfo>& vSetServerInfo, std::vector<Tseer::EndpointF>& vecActive, std::vector<Tseer::EndpointF>& vecInactive, 
                    std::ostringstream& os);    
    
    /**
     * 加载对象列表到内存
     * @param recoverProtect
     * @param recoverProtectRate
     * @return
     */
    int loadObjectIdCache(bool recoverProtect, int recoverProtectRate);


    /**
     * 加载组优先级到内存
     */
    int loadGroupPriority();

    /**
     * 根据ip获取组id
     * @return int <0 失败 其它正常
     */
    int getGroupId(const string& ip);


    /**
     *根据组名获取组id
     * @return int <0 失败 其它正常
     */
    int getGroupIdByName(const string& groupName);


    /**
     * 加载IP物理分组信息
     */
    int loadIPPhysicalGroupInfo();

private:

    /**
     * 根据group id获取Endpoint
     */
    vector<Tseer::EndpointF> getEpsByGroupId(const vector<Tseer::EndpointF>& vecEps, const GroupUseSelect GroupSelect, int iGroupId, ostringstream&os);


    vector<Tseer::EndpointF> getEpsByGroupId(const vector<Tseer::EndpointF>& vecEps, const GroupUseSelect GroupSelect, const set<int>& setGroupID, ostringstream& os);


    void loadSetInfofromObjectItem(const string& objId, const ObjectItem& objItem, SetDivisionCache& setDivisionCache);

    /**
     * 更新缓存中的服务信息
     *
     * @param objCache
     * @param updateAll 是否全部更新
     * @param bFirstLoad  是否是第一次全量加载
     */
    void updateObjectsCache(ObjectsCache& objCache, bool updateAll = true);


    /**
     * 更新缓存中的set信息
     *
     * @param objCache
     * @param updateAll 是否全部更新
     * @param bFirstLoad  是否是第一次全量加载
     */
    void updateDivisionCache(SetDivisionCache& setDivisionCache, bool updateAll = true);

    /**
     * 更新idc分组信息
     */
    void load2GroupMap(const vector<map<string, string> >& serverGroupRule);

    /**
     * 输出所有存储数据
     */
    void printCacheData();
protected:

    /* 对象列表缓存 */
    static TC_ReadersWriterData<ObjectsCache> _objectsCache;

    /* set划分缓存 */
    static TC_ReadersWriterData<SetDivisionCache> _setDivisionCache;

    /* 优先级的序列 */
    static TC_ReadersWriterData<std::map<int, GroupPriorityEntry> > _mapGroupPriority;

    /* 分组信息 */
    static TC_ReadersWriterData<map<string, int> >  _groupIdMap;
    static TC_ReadersWriterData<map<string, int> >  _groupNameMap;
};

#endif
