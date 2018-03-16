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