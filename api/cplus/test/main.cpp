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

#include "Tseer_api.h"
#include "Tseer_comm.h"
#include <iostream>
#include <stdlib.h>
#include <assert.h>
#include <getopt.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <unistd.h>
#include <sys/syscall.h>

using namespace std;
using namespace Tseerapi;

string obj = "Tseer.TseerServer.RegistryObj";
string setname = "";
string gettypestr = "";
string lbtypestr = "";
string cntstr = "5";
size_t cnt = 5;
bool pure = false;
string dns = "localhost";
int threadNum = 1;

LB_GET_TYPE getype = LB_GET_IDC;
LB_TYPE lbType = LB_TYPE_LOOP;
int exit_num = 0;

void usage()
{
    cout << "Usage : testapi [OPTION]" << endl;
    cout << "--obj=objname,default=Tseer.TseerServer.RegistryObj" << endl;
    cout << "--set="<<endl;
    cout << "--gettype=set/idc/all"<<endl;
    cout << "--lbtype=loop/random/static/hash/all"<<endl;
    cout << "--cnt=1"<<endl;
    cout << "--pure,default is set"<<endl;
       cout << "--thrd=1"<<endl;
    cout << "--dns=localhost"<<endl;
    exit(0);
}
void checkarg(int argc, char* argv[]);

void testapi(void* arg)
{
    size_t num = 0;
    if(pure)
    {
        string errMsg;
        InitRawApiParams  para;
              para.domainName = dns;
        int ret = ApiSetRegistryInfo(para,errMsg);
        assert(ret ==0);
    }
    else
    {
        string errMsg;
        InitAgentApiParams  para;
        int ret = ApiSetAgentIpInfo(para,errMsg);
        assert(ret ==0);
    }
    
    while( num < cnt)
    {
        try
        {    
                    if(lbType == LB_TYPE_ALL)
                    {
                        RoutersRequest req;
                        req.obj = obj;
                        req.lbGetType = getype;
                        req.setInfo = setname;
                        string err("");
                        int ret = ApiGetRoutes(req, err);
                        if(ret == 0)
                        {
                            for(size_t i =0; i < req.nodeInfoVec.size();i++)
                            {
                                cout <<syscall(SYS_gettid)<< "|ip:" << req.nodeInfoVec[i].ip << "|port:" << req.nodeInfoVec[i].port << "|tcp:" << req.nodeInfoVec[i].isTcp <<"|ret:"<<ret<<"|"<<err<<endl;
                            }
                        }
                    }
                    else
                    {
                    RouterRequest req;
                    req.moduleName = "Tseer.TestApi";
                    req.obj = obj;

                    //4种获取负载均衡方式
                    req.type = lbType;
                    //3种获取方式
                    req.lbGetType = getype;
                    req.setInfo = setname;
                    
                    string err("");
                    //获取路由ip port
                    int ret = ApiGetRoute(req, err);
                    if(ret == 0)
                    {
                        //发起业务请求
                        int ret = 0;  //业务请求结果
                        if(req.port == 1234)
                        {
                                       ret = -1;
                        }
                        int usetime = 100;//业务请求耗时
                        //上报请求结果
                        ret = ApiRouteResultUpdate(req, ret, usetime, err);
                           cout <<syscall(SYS_gettid)<< "|ip:" << req.ip << "|port:" << req.port << "|tcp:" << req.isTcp <<"|ret:"<<ret<<"|"<<err<<endl;
                    }
                    else
                    {
                        cout <<syscall(SYS_gettid)<<"|ret:"<<ret<<"|"<<err<<endl;
                    }
                    }
        }catch(exception& ex)
        {
            cout <<ex.what()<<endl;
        }
              sleep(1);
        num ++;
    }

     __sync_fetch_and_add(&exit_num, 1);
}

void testMulit()
{
    pthread_t    _tid;
    cout << "thread nu:" << threadNum << endl;
    for(int i = 0; i < threadNum; i++)
    {
       pthread_create(&_tid,
               0,
               (void *(*)(void*)) & testapi,
               (void*)NULL);
        sleep(1);
    }    
}


int main(int argc, char* argv[]) 
{    
    checkarg(argc,argv);
    testMulit();
    while(1)
    {
        sleep(1);
        if(__sync_bool_compare_and_swap(&exit_num,threadNum,1))
        {
            break;
        }
    }
    return 0;
}


void checkarg(int argc, char* argv[])
{
   int c;
   while(1)
   {
       int i = 0;
       static struct option long_opts[] = {
                            {"obj",required_argument,NULL,0},
                            {"set",required_argument,NULL,1},
                            {"gettype",required_argument,NULL,2},
                            {"lbtype",required_argument,NULL,3},
                            {"cnt",required_argument,NULL,4},
                            {"dns",required_argument,NULL,5},
                            {"pure",no_argument,NULL,6},
                            {"thrd",required_argument,NULL,7},
                             {"help",required_argument,NULL,8},
                            {0,0,0,0}
       };
       c = getopt_long(argc,argv,"o:s:g:l:c:d:pt:h",long_opts,&i);
       if(c==-1)
        break;
       
       switch(c)
       {
            case 0:
                obj = optarg;
                break;
            case 1:
                setname = optarg;
                break;
            case 2:
                gettypestr = optarg;
                break;
            case 3:
                lbtypestr = optarg;
                break;
            case 4:
                cntstr = optarg;
                break;
            case 5:
                dns = optarg;
                break;
            case 6:
                pure = true;
                break;
            case 7:
                threadNum = atoi(optarg);
                break;
            case 8:
                usage();
       }
   }
   if(obj.empty())
        usage();
   cnt = atoi(cntstr.c_str());
   if(gettypestr != "")
   {
       if(gettypestr == "set")
       {
           getype= LB_GET_SET;
           if(setname.empty())
           {
               usage();
           }
       }
       if(gettypestr == "all")
       {
           getype = LB_GET_ALL;
       }
       
       if(gettypestr == "idc")
       {
           getype = LB_GET_IDC;
       }   
   }

    if(lbtypestr != "")
    {
        if(lbtypestr == "loop")
        {
            lbType = LB_TYPE_LOOP;
        }
        
        if(lbtypestr == "random")
        {
            lbType = LB_TYPE_RANDOM;
        }
        
        if(lbtypestr == "static")
        {
            lbType = LB_TYPE_STATIC_WEIGHT;
        }   

        if(lbtypestr == "hash")
        {
            lbType = LB_TYPE_CST_HASH;
        }   
         if(lbtypestr == "all")
        {
            lbType = LB_TYPE_ALL;
        }          
    }   
    
    cout <<"obj:"<<obj<<endl
        <<"type:"<<getype<<endl
        <<"lbtype:"<<lbType<<endl
        <<"set:"<<setname<<endl
        <<"pureApi:"<<std::boolalpha<<pure<<endl
        <<"dns:"<<dns<<endl
        <<"cnt:"<<cnt<<endl;
}

