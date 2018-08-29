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

#ifndef __TSEER_API_GLOBAL_H_
#define __TSEER_API_GLOBAL_H_

/**********************************************
* 文件声明常用的全局变量、函数
**********************************************/

#include <string>
#include "Tseer_inner_comm.h"

namespace Tseerapi
{

/***************************************************************
*Agent信息
***************************************************************/

//Agent版本号
extern const std::string g_agent_api_version;

//Agent获取名字服务的Obj名称
extern const std::string g_agent_router_obj;

//获取服务节点的接口名
extern const std::string g_agent_router_func;

//上报调用结果数据的接口名
extern const std::string g_agent_call_stat_report_func;

//Agent的IP地址
extern std::string g_agent_addr;

//Agent的路由服务的端口
extern unsigned short g_agent_router_port;

//向Agent通信的超时时间（毫秒）
extern unsigned int g_agent_timeout;

/****************************************************************
*纯API配置
****************************************************************/

//是否使用路由，默认为假
extern bool g_is_using_registry;

//使用域名获取主控还是使用IP方式，默认使用主控方式（为真）
extern bool g_is_using_domain_to_get_registry;

//纯api版本号
extern const std::string g_pure_api_version;

//Tseer服务端Obj名称
extern const std::string g_registry_obj;

//Tseer服务端的DNS域名
extern std::string g_registry_dns;

//Tseer服务端的IP地址，多个ip用|分割
extern std::string g_registry_ips;
extern std::string g_registry_ip;

//Tseer服务端的端口
extern unsigned short g_registry_port;

//Tseer服务端获取全部服务节点的接口
extern const std::string g_registry_get_all_func;

//Tseer服务端获取同一IDC服务节点的接口
extern const std::string g_registry_get_idc_func;

//Tseer服务端获取同一SET服务节点的接口
extern const std::string g_registry_get_set_func;

//向Tseer服务端通信的超时时间（毫秒）
extern unsigned int g_registry_timeout;

/***************************************************************
*其它
****************************************************************/
//uniCache更新间隔时间(s)
extern unsigned int g_cache_expire_interval;

//节点过期时间(s)
extern unsigned int g_node_normal_expire_interval;
extern unsigned int g_node_short_expire_interval;
extern unsigned int g_node_long_expire_interval;

//Tseer服务端的DNS服务器配置文件地址
extern std::string g_registry_dns_conf_file;

//Tseer客户端标识key
extern std::string g_tseer_client_key;
//Tseer客户端上报自身属性时间间隔(30分钟)
extern size_t g_tseer_client_report_interval;

//两个关键字：客户端Key、Tseer api版本
extern const std::string g_TSEERAPI_MASTER_KEY_STR;
extern const std::string g_TSEERAPI_VERSION_STR;

//保留用的关键字
//extern const std::string g_RESERVED_STR;
extern std::string g_reserved_value;

//初始化次数
extern int g_init_times;
/***************************************************************
*通用函数
****************************************************************/

//根据InnerRouterRequest推导出某服务的路由表KEY
std::string getKeyFromReq(const InnerRouterRequest &req);

//根据InnerRouterRequest推导出某服务的GetRouter方式缓存的Key
std::string getUnkeyFromReq(const InnerRouterRequest &req);

//合并错误信息,currErr本层错误信息，subErr上层的错误信息
std::string mergeErrMsg(const std::string &currErr, const std::string &subErr);

//定义缓存文件的路径
#ifndef ROUTERSCACHE_PATH
#define ROUTERSCACHE_PATH "/tmp/.routersCache/"
#endif

#ifndef FILE_FUN
#define FILE_FUN  __FILE__<<":"<<__FUNCTION__<<":"<<__LINE__<<"|"
#endif
}

#endif
