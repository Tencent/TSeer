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

#ifndef _APIREGIMP_H_
#define _APIREGIMP_H_

#include "servant/Application.h"
#include "servant/Servant.h"
#include "util/tc_http.h"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "RouterData.h"
#include "EtcdCommon.h"
#include "BaseHandle.h"
#include "TseerAgentUpdate.h"

class NotifyUpdateEvent : public TC_HandleBase
{
public:
    //事件总数
    int                 _run_times;
    //已经完成的事件计数
    tars::TC_Atomic         _atomic;
    tars::TC_ThreadLock     _monitor;
    tars::TC_ThreadLock     _metux;
    //ip=result
    map<string,string>      _succNum;
    map<string,string>      _failureNum;

    NotifyUpdateEvent()
        : _run_times(0)
        , _atomic(0)
    {
    }

    void addSuc(const string& ip);
    void addFail(const string& ip,const string& errMsg);
};

typedef tars::TC_AutoPtr<NotifyUpdateEvent> NotifyUpdateEventPtr;

class UpdateAgentCallbackImp : public UpdatePrxCallback
{
public:
    UpdateAgentCallbackImp(const NotifyUpdateEventPtr& notifyPtr, const string& ip) : _notify(notifyPtr), _ip(ip)
    {
    }

    void callback_updateConfig(tars::Int32 ret,const std::string& result);
    void callback_updateConfig_exception(tars::Int32 ret);

private:
    NotifyUpdateEventPtr    _notify;
    string  _ip;
};


class ApiRegImp : public tars::Servant
{
public:

    virtual ~ApiRegImp() {}

    virtual void initialize();

    virtual void destroy();

    /**
      * @brief 处理客户端的主动请求
      * @param current
      * @param response
      * @return int
      */
    virtual int doRequest(tars::TarsCurrentPtr current, vector<char>& response);
    
    //路由数据请求回包
    static void doGetRouteResponse(TarsCurrentPtr &current, int ret,const vector<RouterData>& routerDataList,const string& errMsg);

    //IDC分组信息请求回包
    void doGetIdcGroupResponse(TarsCurrentPtr current, int ret, const vector<IDCGroupInfo> &idcGroupInfoList);

    //IDC优先级信息请求回包
    void doGetIdcPriorityResponse(TarsCurrentPtr current, int ret, const vector<IDCPriority> &idcPriority);

    //非获取请求、获取失败回包
    static void doResponse(TarsCurrentPtr current, int ret, const string& errMsg = "");
    void doResponse(TarsCurrentPtr current, int ret, rapidjson::Document &document, const string& errMsg);

    //添加IDC分组规则、优先级规则，返回规则的ID
    static void doAddIdcResponse(TarsCurrentPtr current, int ret, const string &id);

private:
    /**
      * @brief 获取请求参数,包括方法名，key，接口参数(如果有)
      * @param req
      * @param method
      * @param key
      * @param methodParam
      * @param errMsg
      * @return 详见API_ERROR_CODE
      */
    int getReqParam(const tars::TC_HttpRequest& req,string& method,string& key,string& methodParam,string& errMsg) const;
    
    /**
      * @brief 检查请求参数里指定的版本号是否合法
      * @param req
      * @return bool
      */
    bool IsApiVersionValid(const tars::TC_HttpRequest &req) const;

private:
    /**
      * @brief 添加新模块或者新的ip port路由信息
      * @param jData
      * @return 详见API_ERROR_CODE
      */
    int addServer(TarsCurrentPtr current, rapidjson::Document &jData);

    /**
      * @brief 更新指定模块的路由属性，包括set,idc 分组，权重，可用状态，流量开关
      * @param jData
      * @return 详见API_ERROR_CODE
      */
    int updateServer(TarsCurrentPtr current, rapidjson::Document &jData);

    /**
      * @brief 删除模块或某个ip port路由信息
      * @param jData
      * @return 详见API_ERROR_CODE
      */
    int delServer(TarsCurrentPtr current, rapidjson::Document &jData);

    /**
      * @brief 获取某个业务集下所有服务或者某个服务的路由数据信息
      * @param jData
      * @return 详见API_ERROR_CODE
      */
    int getServer(TarsCurrentPtr current, rapidjson::Document &jData);

    /**
      * @brief 针对某个服务的路由端口上报心跳
      * @param jData
      * @return 详见API_ERROR_CODE
      */
    int keepalive(TarsCurrentPtr current, rapidjson::Document &jData);

    /**
      * @brief 向名字服务注册一个业务集名称
      * @param jData
      * @return 详见API_ERROR_CODE
      */
    int addServiceGroup(TarsCurrentPtr current, rapidjson::Document &jData);

    /**
      * @brief 获取所有或者指定业务集信息(业务集名称,key,user)
      * @param jData
      * @return 详见API_ERROR_CODE
      */
    int getServiceGroup(TarsCurrentPtr current, rapidjson::Document &jData);

    /**
      * @brief 更新业务集对应的管理人员
      * @param jData
      * @return 详见API_ERROR_CODE
      */
    int updateServiceGroup(TarsCurrentPtr current, rapidjson::Document &jData);

    /**
      * @brief 获取idc分组信息
      * @param jData
      * @return 详见API_ERROR_CODE
      */
    int getIdcGroup(TarsCurrentPtr current, rapidjson::Document &jData);

    /**
      * @brief 下线agent
      * @param jData
      * @return 详见API_ERROR_CODE
      */
    int deleteAgent(TarsCurrentPtr current, rapidjson::Document &jData);
    
    /**
      * @brief  更新某个或者多个agent的路由中心地址
      * @param jData
      * @return 详见API_ERROR_CODE
      */
    int updateAgentLocator(TarsCurrentPtr current, rapidjson::Document &jData);
    UpdatePrx getAgentPrx(const string & nodeName);

    /**********************************IDC相关*****************************************/
    //分组
    int addIdcGroupRule(TarsCurrentPtr current, rapidjson::Document &jData);

    int modifyIdcGroupRule(TarsCurrentPtr current, rapidjson::Document &jData);

    int delIdcGroupRule(TarsCurrentPtr current, rapidjson::Document &jData);

    int getAllIdcGroupRule(TarsCurrentPtr current, rapidjson::Document& jData);

    //优先级
    int addIdcPriority(TarsCurrentPtr current, rapidjson::Document &jData);

    int modifyIdcPriority(TarsCurrentPtr current, rapidjson::Document &jData);

    int delIdcPriority(TarsCurrentPtr current, rapidjson::Document &jData);

    int getAllIdcPriority(TarsCurrentPtr current, rapidjson::Document& jData);

 

   /*******************************SeerAgent管理********************************/
   int getAgentBaseInfo(TarsCurrentPtr current, rapidjson::Document &jData);
   
   int updateAgentGrayState(TarsCurrentPtr current, rapidjson::Document &jData);
   
   int getAgentPackageInfo(TarsCurrentPtr current, rapidjson::Document &jData);
   
   int updateAgentPackageInfo(TarsCurrentPtr current, rapidjson::Document &jData);
   
   int deleteAgentPackageInfo(TarsCurrentPtr current, rapidjson::Document &jData);
   
   int getAgentOstypeInfo(TarsCurrentPtr current, rapidjson::Document &jData);
   
   int addAgentPackageInfo(TarsCurrentPtr current, rapidjson::Document &jData);

   /**
     * @brief 下载安装agent的脚本
     */
   void downloadInstallScript(tars::TarsCurrentPtr current);
   /**
     * @brief 下载agent安装包
     * @param reqParam wget请求参数
     */
   void downloadAgentPackage(tars::TarsCurrentPtr current,const string& reqParam);

   /**
     * 通过wget方式获取locator列表
     */
   void getLocatorList(tars::TarsCurrentPtr current);
private:
   //支持的API名称 
   typedef int (ApiRegImp::*CommandHandler)(TarsCurrentPtr, rapidjson::Document& );
   std::map<string, CommandHandler> _supportMethods;
    
   //业务集鉴权key缓存,key=serviceGrp,value=skey
   map<string,string> _mapServiceGrpKey;
};

#endif


