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

#ifndef __APIROUTER_FACTORY_H_
#define __APIROUTER_FACTORY_H_

#include "util/tc_thread_mutex.h"
#include "util/tc_lock.h"
#include "util/tc_singleton.h"
#include "util/tc_timeprovider.h"

#include "ApiRouterManager.h"


/***********************************************
* 文件定义了管理每个客户端请求过来的唯一标识ApiRouterManager
************************************************/
class ApiRouterFactory :  public TC_ThreadRecMutex,public TC_Singleton<ApiRouterFactory>
{
public:
    ApiRouterFactory() {}
    ~ApiRouterFactory();
    
    ApiRouterManager* getRouterMr(const Tseer::AgentRouterRequest &req);

    void timeout();
private:
    map<string,ApiRouterManager*> _apiRouterManager;
};

#endif

