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

#include "ApiRouterFactory.h"
#include "RollLogger.h"

ApiRouterFactory::~ApiRouterFactory()
{
    TC_LockT<TC_ThreadRecMutex> lock(*this);
    for (map<string, ApiRouterManager*>::iterator it = _apiRouterManager.begin(); it != _apiRouterManager.end(); it++)
    {
        delete (it->second);
    }
}

ApiRouterManager* ApiRouterFactory::getRouterMr(const Tseer::AgentRouterRequest &req)
{
    TC_LockT<TC_ThreadRecMutex> lock(*this);
    string key;
    if (!req.identifier.empty())
    {
        key = req.identifier; // pid
    }
    else
    {
        key = req.obj;
    }

    map<string, ApiRouterManager*>::const_iterator it = _apiRouterManager.find(key);
    if (it != _apiRouterManager.end())
        return it->second;
    
    ApiRouterManager* pApi = new ApiRouterManager(key);
    _apiRouterManager[key] = pApi;
    return pApi;
}

void ApiRouterFactory::timeout()
{
    TC_LockT<TC_ThreadRecMutex> lock(*this);
    map<string, ApiRouterManager*>::iterator it = _apiRouterManager.begin();
    while(it != _apiRouterManager.end())
    {
        if (it->second->timeout())
        {
            delete (it->second);
            _apiRouterManager.erase(it++);
        }
        else
        {
            ++it;
        }
    }   
}

