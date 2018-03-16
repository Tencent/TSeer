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

#ifndef __UTIL_H__
#define __UTIL_H__

#include <iostream>

#include "util/tc_encoder.h"
#include "util/tc_common.h"
#include "util/tc_http.h"
#include "servant/Communicator.h"


//#define FILE_FUN __FUNCTION__ << ":" << __LINE__ << "|" << pthread_self() << "|"
#define FILE_FUN __FUNCTION__ << ":" << __LINE__

#define REGISTRY_HTTP_API "http://127.0.0.1:9904/v1/client/"

#define REGISTRY_OBJ "Tseer.TseerServer.RegistryObj@tcp -h 127.0.0.1 -p 9902 -t 30000"

#define QUERY_OBJ "Tseer.TseerServer.QueryObj@tcp -h 127.0.0.1 -p 9903 -t 30000"

std::string urlEncode(const std::string &in);

std::string getRequestUrl(const std::string &interfaceName, const std::string &params, const std::string &api = REGISTRY_HTTP_API);

int doSyncRequest(const std::string &url, tars::TC_HttpResponse &response);

#endif