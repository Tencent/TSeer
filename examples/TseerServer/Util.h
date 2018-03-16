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