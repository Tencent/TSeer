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

#ifndef __NODE_TEST_H__
#define __NODE_TEST_H__

//测试添加一个Agent节点
void addOneAgentNode();

//测试删除一个Agent节点
void delOneAgentNode();

//测试上报Agent节点的心跳
void keepAliveAgent();

//测试添加一个Node节点
void registerNode();

//测试删除一个Node节点
void destroyNode();

//测试上报Node节点的心跳
void keepAlive();

void getagentbaseinfo();
void updateagentgraystate();
void getagentpackageinfo();
void updateagentpackageinfo();
void deleteagentpackage();
void updateagentlocator();
#endif