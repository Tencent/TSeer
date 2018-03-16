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

#include "global.h"
#include "tc_common.h"

#include <string>

namespace Tseerapi
{

//全局变量设置，接口名称需要查看相关TARS文件来确定

#define TseerVER "TseerAgentCPlusAPI_v0.1"

//纯api的版本号
#define PUREAPIVER "TseerRawCPlusAPI_v0.1"

const std::string g_agent_api_version = TseerVER;
const std::string g_agent_router_obj = "Tseer.TseerAgent.RouterObj";
const std::string g_agent_router_func = "getRouterNode";
const std::string g_agent_call_stat_report_func = "reportStat";

std::string g_agent_addr = "127.0.0.1";
unsigned short g_agent_router_port = 8865;
unsigned int g_agent_timeout = 500;

bool g_is_using_registry = false;
bool g_is_using_domain_to_get_registry = true;
const std::string g_pure_api_version = PUREAPIVER;
const std::string g_registry_obj = "Tseer.TseerServer.QueryObj";
std::string g_registry_dns = "localhost";
std::string g_registry_ips;
unsigned short g_registry_port = 9903;
const std::string g_registry_get_all_func = "findObjectById4Any";
const std::string g_registry_get_idc_func = "findObjectByIdInSameGroup";
const std::string g_registry_get_set_func = "findObjectByIdInSameSet";
unsigned int g_registry_timeout = 500;

unsigned int g_cache_expire_interval = 15;

unsigned int g_node_normal_expire_interval = 5;
unsigned int g_node_short_expire_interval = 3;
unsigned int g_node_long_expire_interval = 30;
std::string g_registry_dns_conf_file;
std::string g_tseer_client_key = "";
size_t g_tseer_client_report_interval = 60 * 30;

const std::string g_TSEERAPI_MASTER_KEY_STR = "TSEERAPI_KEY";
const std::string g_TSEERAPI_VERSION_STR = "TSEERAPI_VERSION";
std::string g_reserved_value = "";

int g_init_times = 0;

//根据RouterRequest推导出某服务的路由表KEY
std::string getKeyFromReq(const InnerRouterRequest &req)
{
    switch (req.lbGetType)
    {
    case LB_GET_ALL:
        return req.obj + "|ALL";
    case LB_GET_IDC:
        return req.obj + "|IDC";
    case LB_GET_SET:
        return req.obj + "|SET|" + req.setInfo;
    default:
        return req.obj;
    }
}

std::string getUnkeyFromReq(const InnerRouterRequest &req)
{
    switch (req.lbGetType)
    {
    case LB_GET_ALL:
        return req.obj + "|ALL|" + TC_Common::tostr<int>(req.type);
    case LB_GET_IDC:
        return req.obj + "|IDC|" + TC_Common::tostr<int>(req.type);
    case LB_GET_SET:
        return req.obj + "|SET|" + req.setInfo + TC_Common::tostr<int>(req.type);
    default:
        return req.obj;
    }
}

std::string mergeErrMsg(const std::string &currErr, const std::string &subErr)
{
    return currErr + "\n\t" + subErr;
}

}
