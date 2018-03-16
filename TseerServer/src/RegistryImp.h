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

#ifndef __REGISTRY_IMP_H__
#define __REGISTRY_IMP_H__

#include "util/tc_common.h"
#include "util/tc_config.h"

#include "Registry.h"

using namespace Tseer;

/*
 * 提供给node调用的接口类
 */
class RegistryImp: public Registry
{
public:
    /**
     * 构造函数
     */
    RegistryImp(){};

    /**
     * 初始化
     */
    virtual void initialize();

    /**
     ** 退出
     */
    virtual void destroy() {};

    /**
     * node启动的时候往registry注册一个session
     *
     * @param name node名称
     * @param ni   node详细信息
     * @param li    node机器负载信息
     *
     * @return 注册是否成功
     */
    virtual int registerNode(const string & name, const NodeInfo & ni, const LoadInfo & li, tars::TarsCurrentPtr current);


    /**
     * node上报心跳负载
     *
     * @param name node名称
     * @param li    node机器负载信息
     *
     * @return 心跳接收状态
     */
    virtual int keepAlive(const string& name, const LoadInfo & li, tars::TarsCurrentPtr current);

    /**
     * 更新server状态
     *
     * @param nodeName : node id
     * @param app:       应用
     * @param serverName: server 名
     * @param state :  server状态
     *
     * @return server信息列表
     */
    virtual int updateServer(const ServerStateInfo & stateInfo, tars::TarsCurrentPtr current);

    /**
     * 量批更新server状态
     *
     * @param vecStateInfo : 批量server状态
     *
     * @return server信息列表
     */
    virtual int updateServerBatch(const std::vector<ServerStateInfo> & vecStateInfo, tars::TarsCurrentPtr current);

    /**
     * node停止，释放node的会话
     *
     * @param name    node名称
     */
    virtual int destroy(const string & name, tars::TarsCurrentPtr current);

    /**
     * node启动的时候往registry注册一个session
     *
     * @param nodeInfo 注册信息
     *
     * @return 注册是否成功
     */
    virtual Int32 registerNodeExt(const NodeInstanceInfo & nodeInfo,tars::TarsCurrentPtr current);
    /**
     * node上报心跳及机器负载
     *
     * @param nodeName    node名称
     * @param load    node机器负载信息
     */
    virtual Int32 keepAliveExt(const NodeInstanceInfo & nodeInfo,tars::TarsCurrentPtr current);


    virtual Int32 checkChange(const PackageInfo & reqInfo, PackageInfo &rspinfo, tars::TarsCurrentPtr current);

    
    virtual Int32 updatePacket(const UpdateReq & req, UpdateRsp &rsp, tars::TarsCurrentPtr current);

    /**
    * web上传发布包到patch服务器
    * 注意 :由于taf的发布包大小限制默认10MB，因此调用次接口时，如果发布包大于10MB，必须拆分包按顺序同步上传
    * @param stPackageInfo, 待上传的数据流信息
    * @param result, 失败信息
    *
    * @return 0,表示成功，<0:表示失败
    */
    virtual Int32 pushPackage(const Tseer::PushPackageInfo & pushPkInfo,std::string &result,tars::TarsCurrentPtr current);
    
private:
    void genFailRsp(const UpdateReq & req, UpdateRsp &rsp);
    string intStateToString(const ServerState &state);
};

#endif
