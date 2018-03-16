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

#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <string>

#include "servant/Communicator.h"

std::string g_key = "webadmin";

std::string g_url = "http://127.0.0.1:9904/v1/interface";

std::string g_srvgrp = "apitest222";

std::string g_user = "";
std::string g_idcname = "idctest";
tars::Communicator g_com;

int g_ttime = 0;

int g_iloadInterval = 0;

#endif