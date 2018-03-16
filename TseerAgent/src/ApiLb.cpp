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

#include "ApiLb.h"
#include <sstream>


int LoadBalance::getRouters(vector<Tseer::RouterNodeInfo> &nodeInfoList, string &errMsg)
{
    ostringstream os;
    os << FILE_FUN << "Unsupported method invocation.";
    errMsg = os.str();
    return -1;
}

int LoadBalance::getRouter(Tseer::RouterNodeInfo& nodeInfo, string &errMsg)
{
    ostringstream os;
    os << FILE_FUN << "Unsupported method invocation.";
    errMsg = os.str();
    return -1;
}

int LoadBalance::getRouter(unsigned long long key, Tseer::RouterNodeInfo& nodeInfo, string &errMsg)
{
    ostringstream os;
    os << FILE_FUN << "Unsupported method invocation.";
    errMsg = os.str();
    return -1;
}

