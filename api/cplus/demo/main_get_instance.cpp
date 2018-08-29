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

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include "include/Tseer_api.h"
#include "include/Tseer_comm.h"

using namespace std;
using namespace Tseerapi;

int main(int argc, char *argv[]) {
    const char* service_name = NULL;
    int option = 0x001;

    int opt;
    opterr = 0;
    while ((opt = getopt(argc, argv, "iptah")) != -1) {
        switch(opt) {
            case 'a':
                option |= 0x111;
                break;
            case 'i':
                option |= 0x001;
                break;
            case 'p':
                option |= 0x010;
                break;
            case 't':
                option |= 0x100;
                break;
            case 'h':
                printf("Usage: %s [OPTION] service_name\n", argv[0]);
                printf("  -a\tprint all fileds\n");
                printf("  -i\tprint ip\n");
                printf("  -p\tprint port\n");
                printf("  -t\tprint tag\n");
                printf("  -h\tprint help message\n");
                exit(0);
            default:
                break;
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Error: no service name, use -h for help\n");
        exit(1);
    }

    service_name = argv[optind];
    string sErr;

    InitAgentApiParams initParams;
    if (ApiSetAgentIpInfo(initParams, sErr) != 0) {
        fprintf(stderr, "Error: init agent error\n");
        exit(2);
    }

    RoutersRequest reqs;
    reqs.obj        = service_name;
    reqs.lbGetType  = LB_GET_ALL;
    if (ApiGetRoutes(reqs, sErr) == 0) {
        vector<NodeInfo>::iterator vIter;
        for (vIter = reqs.nodeInfoVec.begin(); vIter != reqs.nodeInfoVec.end(); ++vIter) {
            bool need_tab = false;
            if (option & 0x001) {
                if (need_tab) printf("\t");
                printf("%s", vIter->ip.c_str());
                need_tab = true;
            }

            if (option & 0x010) {
                if (need_tab) printf("\t");
                printf("%d", vIter->port);
                need_tab = true;
            }

            if (option & 0x100) {
                if (need_tab) printf("\t");
                printf("%s", vIter->slaveSet.empty() ? "-" : vIter->slaveSet.c_str());
                need_tab = true;
            }

            if (need_tab) printf("\n");
        }
    } else {
        fprintf(stderr, "Error: get service [%s] info failed\n", service_name);
        exit(3);
    }

    return 0;
}