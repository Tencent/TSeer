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

#ifndef __UPDATE_IMP_H_
#define __UPDATE_IMP_H_

#include "TseerAgentUpdate.h"

using Tseer::Update;

class UpdateImp : public Update
{
public:
    /**
     *
     */
    UpdateImp()
    {
    };

    /**
     * 销毁服务
     * @param k
     * @param v
     *
     * @return
     */
    ~UpdateImp()
    {
    };

    /**
    * 初始化
    */
    virtual void initialize();

    /**
    * 退出
    */
    virtual void destroy()
    {
    };

    /**
    * 更新agent自身配置
    * @param cfg  更新agent配置
    * @return  int 0成功 其它失败
    */
    virtual tars::Int32 updateConfig(const Tseer::AgentConfig & cfg,std::string &result,tars::TarsCurrentPtr current);

    virtual tars::Int32 uninstall(std::string &result,tars::TarsCurrentPtr current);
};

#endif
