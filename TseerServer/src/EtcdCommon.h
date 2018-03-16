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

#ifndef __ETCD_COMMON_H__
#define __ETCD_COMMON_H__

#include "util/tc_common.h"
#include "Registry.h"

using namespace Tseer;

/*路由数据的value标识符*/
#define OBJNAMEEKEY  string("key")
#define PRESENTSTATEKEY string("present_state")

const string DCNODE_NETWORK_ID = "dcnode";
const string TSEER_AGENT_NET_WORK_ID = "router_agent";

#endif