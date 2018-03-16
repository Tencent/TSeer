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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <new>
#include "ApiLbFactory.h"

#include "ApiLb_loop.h"
#include "ApiLb_random.h"
#include "ApiLb_consistenthash.h"
#include "ApiLb_static_weight.h"
#include "ApiLb_all.h"

LoadBalance* LoadBalanceFactory::CreateLoadBalance(Tseer::LB_TYPE type)
{
    LoadBalance *lb = NULL;
    switch(type)
    {
    case Tseer::LB_TYPE_LOOP:
        lb = new(std::nothrow) LBLoop;
        break;
    case Tseer::LB_TYPE_RANDOM:
        lb = new(std::nothrow) LBRandom;
        break;
    case Tseer::LB_TYPE_CST_HASH:
        lb = new(std::nothrow) LBConsistentHash;
        break;
    case Tseer::LB_TYPE_STATIC_WEIGHT:
        lb = new(std::nothrow) LBStaticWeight;
        break;
   case Tseer::LB_TYPE_ALL:
        lb = new(std::nothrow) LBAll;
        break;
    default:
        lb = NULL;
        break;
    }

    return lb;
}

