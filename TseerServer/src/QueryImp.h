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

#ifndef __QUERY_IMP_H__
#define __QUERY_IMP_H__

#include "servant/TarsCurrent.h"
#include "EndpointF.h"
#include "StoreCache.h"

using namespace Tseer;

/**
 * 对象查询接口类
 */
enum FUNID
{
    FUNID_findObjectById              = 0,
    FUNID_findObjectById4Any          = 1,
    FUNID_findObjectById4All          = 2,
    FUNID_findObjectByIdInSameGroup   = 3,
    FUNID_findObjectByIdInSameStation = 4,
    FUNID_findObjectByIdInSameSet     = 5
};
class QueryImp : public Tseer::QueryF
{
public:


    /**
     * 构造函数
     */
    QueryImp()
    {
    };


    /**
     * 初始化
     */
    virtual void initialize();


    /**
    ** 退出
    */
    virtual void destroy()
    {
    };

    /** 根据id获取对象
     *
     * @param id 对象名称
     *
     * @return  返回所有该对象的活动endpoint列表
     */
    virtual vector<Tseer::EndpointF> findObjectById(const string& id, tars::TarsCurrentPtr current);


    /**根据id获取所有对象,包括活动和非活动对象
     *
     * @param id         对象名称
     * @param activeEp   存活endpoint列表
     * @param inactiveEp 非存活endpoint列表
     * @return:  0-成功  others-失败
     */
    virtual tars::Int32 findObjectById4Any(const std::string& id, vector<Tseer::EndpointF>&activeEp, vector<Tseer::EndpointF>&inactiveEp, tars::TarsCurrentPtr current);


    /** 根据id获取对象所有endpoint列表
     *
     * @param id         对象名称
     * @param activeEp   存活endpoint列表
     * @param inactiveEp 非存活endpoint列表
     * @return:  0-成功  others-失败
     */
    virtual tars::Int32 findObjectById4All(const std::string& id,
                 vector<Tseer::EndpointF>&activeEp, vector<Tseer::EndpointF>&inactiveEp, tars::TarsCurrentPtr current);


    /** 根据id获取对象同组endpoint列表
     *
     * @param id         对象名称
     * @param activeEp   存活endpoint列表
     * @param inactiveEp 非存活endpoint列表
     * @return:  0-成功  others-失败
     */
    virtual tars::Int32 findObjectByIdInSameGroup(const std::string& id,
                    vector<Tseer::EndpointF>&activeEp, vector<Tseer::EndpointF>&inactiveEp, tars::TarsCurrentPtr current);


    /** 根据id获取对象指定归属地的endpoint列表
     *
     * @param id         对象名称
     * @param activeEp   存活endpoint列表
     * @param inactiveEp 非存活endpoint列表
     * @return:  0-成功  others-失败
     */
    virtual tars::Int32 findObjectByIdInSameStation(const std::string& id, const std::string& station, vector<Tseer::EndpointF>&activeEp, vector<Tseer::EndpointF>&inactiveEp, tars::TarsCurrentPtr current);


    /** 根据id获取对象同set endpoint列表
     *
     * @param id         对象名称
     * @param setId      set全称,格式为setname.setarea.setgroup
     * @param activeEp   存活endpoint列表
     * @param inactiveEp 非存活endpoint列表
     * @return:  0-成功  others-失败
     */
    virtual tars::Int32 findObjectByIdInSameSet(const std::string& id, const std::string& setId, vector<Tseer::EndpointF>&activeEp, vector<Tseer::EndpointF>&inactiveEp, tars::TarsCurrentPtr current);

private:
    void doDaylog(const FUNID eFnId, const string& id, const vector<Tseer::EndpointF>&activeEp, const vector<Tseer::EndpointF>&inactiveEp, 
                         const tars::TarsCurrentPtr& current, const std::ostringstream& os,const string& setId = "");

    string eFunTostr(const FUNID fnId);
    tars::Int32 doFindObjectInSameGroup(const std::string& id, const std::string&setId, vector<Tseer::EndpointF>&activeEp, vector<Tseer::EndpointF>&inactiveEp, tars::TarsCurrentPtr current);

protected:  
    /* 缓存操作 */
    StoreCache   _storeCache;
};

#endif
