# 目录

> - 概述
> - 环境说明
> - TSeer Api C++接口说明
> - TSeer Api C++接口使用样例

# 概述

TSeer Api C++ 接口是TSeer Api的一个子集，TSeer Api提供 C++/Java等接口供不同的服务使用。TSeer Api是TSeer的一部分，主要提供名字路由以及数据上报的接口。视是否部署Seer Agent，提供Agent Api以及纯Api两种访问方式。



# 使用说明

## 1. 编译出API的依赖库

- 通过git拉取源码 `git clone http://git.code.oa.com/tseer/tseer.git`
- 到api/cplus 目录下执行 `cmake .;make;make tar`
- 在当前目录下就会生成C++的API 依赖库 `Tseerapi.tgz`
- 在业务代码的makefile中包含依赖库中的 `Tseer_api.h` 头文件，并链接libtseerapi.a静态库，即可完成TseerApi的引入
 

## 2. 接口列表

| 编号   | 函数名称                 | 作用                    |
| ---- | -------------------- | --------------------- |
| 1    | ApiSetAgentIpInfo    | 初始化Agent Api          |
| 2    | ApiSetRegistryInfo   | 初始化纯Api               |
| 3    | ApiGetRoute          | 按照指定的组策略及负载均衡策略获取一个节点 |
| 4    | ApiGetRoutes         | 按照指定的组策略获取一组节点        |
| 5    | ApiRouteResultUpdate | 模调数据上报                |

**接口详情**

1. `ApiSetAgentIpInfo`初始化Agent Api

```c++
//初始化Agent API入参
struct InitAgentApiParams
{
    std::string serviceKey;        //用户在管理平台上为当前业务注册的key
    std::string reserved;          //保留字段
};

/**
 * 初始化Agent的IP，只需调用一次，成功返回0，失败返回-1
 *
 *  InitAgentApiParams结构描述：
 *  serviceKey:        API标识        
 *
 *  errMsg：调用错误时返回的错误信息
 */
int ApiSetAgentIpInfo(const InitAgentApiParams &initParams, std::string &errMsg);
```

2. `ApiSetRegistryInfo`初始化纯Api

```c++
//初始化纯API入参
struct InitRawApiParams
{
    std::string domainName;    			//Seer服务端域名
    std::string dnsFile;                   //DNS配置文件
    int registryPort;                      //Seer服务端的端口，没有特殊情况使用默认值即可
    std::string serviceKey;                //用户在管理平台上为当前业务注册的key
    std::string reserved;                  //保留字段
};

/** 初始化纯API信息，分为以下两种：
 *  设置Seer服务端的域名、Seer服务端的端口、DNS配置文件;只需调用一次，成功返回0，失败返回-1
 *  如果使用纯API，必须显式调用该方法初始化；否则默认使用Agent API方式
 */
int ApiSetRegistryInfo(const InitRawApiParams &initParams, std::string &errMsg);
```

3. `ApiGetRoute`按照指定方式获取服务的单个节点

```c++
//负载均衡类型
enum LB_TYPE
{
    LB_TYPE_LOOP,               //轮询
    LB_TYPE_RANDOM,             //随机
    LB_TYPE_STATIC_WEIGHT,      //静态权重
    LB_TYPE_CST_HASH,           //一致性哈希
    LB_TYPE_ALL                 //获取所有节点
};
    
//获取方式
enum LB_GET_TYPE
{
    LB_GET_IDC,             //按IDC
    LB_GET_SET,             //按set
    LB_GET_ALL,             //全部获取
};

//路由请求的入参与出参
struct RouterRequest
{
    std::string     obj;                       //请求服务名称，必填
    LB_GET_TYPE     lbGetType;                  //按IDC、set或all获取，选填，默认按all获取
    std::string     setInfo;                   //set信息，当按set方式获取时必填
    LB_TYPE         type;                       //负载均衡类型，选填，默认使用轮询方式
    long            hashKey;                   //hash key，选择哈希负载均衡时必填
    std::string     moduleName;                //主调模块名，模调数据上报时需要该信息；若为空API会自动用本地Ip替代，选填（推荐填写）
    std::string     ip;                        //服务Ip，出参
    int             port;                      //服务端口，出参
    bool            isTcp;                       //服务是否为TCP，否则为UDP，出参
    std::string     slaveSet;                  //服务的set信息，出参（当服务启动set的时候返回）
};

/**
 *  按指定方式获取服务的单个节点，成功返回0，失败返回-1
 *  req:    需要获取的服务的信息，同时作为入参和出参
 *  errMsg:   失败时返回的错误信息
 */
int ApiGetRoute(RouterRequest &req, std::string &errMsg);
```

4. `ApiGetRoutes`按照指定方式获取服务的全部节点

```c++
//获取方式
enum LB_GET_TYPE
{
    LB_GET_IDC,             //按IDC
    LB_GET_SET,             //按set
    LB_GET_ALL,             //全部获取
};

//路由节点信息
struct NodeInfo
{
	std::string     ip;                        //服务Ip，出参
	int             port;                      //服务端口，出参
	bool            isTcp;                       //服务是否为TCP，否则为UDP，出参
	std::string     slaveSet;                  //服务的set信息，出参（当服务启动set的时候返回）   
};   

//路由请求的入参与出参
struct RoutersRequest
{
    std::string     obj;                       //请求服务名称，必填
    LB_GET_TYPE     lbGetType;                  //按IDC、set或all获取，选填，默认按all获取
    std::string     setInfo;                   //set信息，当按set方式获取时必填

    std::vector<NodeInfo> nodeInfoVec;
};

/**
 *  按指定方式获取服务的全部节点，成功返回0，失败返回-1
 *  req:    需要获取的服务的信息，同时作为入参和出参
 *  errMsg:   失败时返回的错误信息
 */
int ApiGetRoutes(RoutersRequest &req, std::string &errMsg);
```

5. `ApiRouteUpdateResult`上报服务调用结果

```c++
//负载均衡类型
enum LB_TYPE
{
    LB_TYPE_LOOP,               //轮询
    LB_TYPE_RANDOM,             //随机
    LB_TYPE_STATIC_WEIGHT,      //静态权重
    LB_TYPE_CST_HASH,           //一致性哈希
    LB_TYPE_ALL                 //获取所有节点
};
    
//获取方式
enum LB_GET_TYPE
{
    LB_GET_IDC,             //按IDC
    LB_GET_SET,             //按set
    LB_GET_ALL,             //全部获取
};

//路由请求的入参与出参
struct RouterRequest
{
    std::string     obj;                       //请求服务名称，必填
    LB_GET_TYPE     lbGetType;                  //按IDC、set或all获取，选填，默认按all获取
    std::string     setInfo;                   //set信息，当按set方式获取时必填
    LB_TYPE         type;                       //负载均衡类型，选填，默认使用轮询方式
    long            hashKey;                   //hash key，选择哈希负载均衡时必填
    std::string     moduleName;                //主调模块名，模调数据上报时需要该信息；若为空API会自动用本地Ip替代，选填（推荐填写）
    std::string     ip;                        //服务Ip，出参
    int             port;                      //服务端口，出参
    bool            isTcp;                       //服务是否为TCP，否则为UDP，出参
    std::string     slaveSet;                  //服务的set信息，出参（当服务启动set的时候返回）
};

/**
 *  上报调用服务结果，成功返回0，失败返回-1
 *  req：        被调服务的信息，用户应该使用ApiGetRoute接口的出参req作为此接口的入参
 *  ret：       调用被调服务的结果，0为成功，-1为异常，-2为超时（异常是除成功、超时以外的所有情况）
 *  timeCost：  调用服务时长
 *  errMsg：       失败时返回的错误信息
 */
int ApiRouteResultUpdate(const RouterRequest &req, int ret, int timeCost, std::string &errMsg);
```



## 3. Seer Api C++使用样例

### Demo1 使用Agent Api方式获取节点

```c++
#include <iostream>
#include "Tseer_api.h"
#include "Tseer_comm.h"

using namespace std;
using namespace Tseerapi;

int main()
{
	int iRet = 0;
	string errMsg = "";

	InitAgentApiParams initParams;
	iRet = ApiSetAgentIpInfo(initParams, errMsg);
	if (iRet != 0) {
		cout << "init Agent error" << endl;
		return 0;
	}

	//初始化ApiGetRoute的参数
	RouterRequest req;
	req.obj = "tencent.tencentServer.HelloService";
	req.lbGetType = LB_GET_SET;
	req.setInfo = "sz.a.b";
	req.type = LB_TYPE_LOOP;
	req.moduleName = "seerClientDemo";
	iRet = ApiGetRoute(req, errMsg);
	cout << "[out]iRet: " << iRet << " errMsg: " << errMsg << endl;
	cout << "[out]ip: " << req.ip << endl;
	cout << "[out]port: " << req.port << endl;
	cout << "[out]isTcp: " << req.isTcp << endl;

	if (iRet == 0) {
		//进行调用
		sleep(5);
		//调用结束上报调用结果
		ApiRouteResultUpdate(req, 0, 1, errMsg);
		cout << "[report]errMsg: " << errMsg << endl;
	}
}
```

### Demo2 使用Agent Api方式获取一组节点

```c++
#include <iostream>
#include <vector>
#include "Tseer_api.h"
#include "Tseer_comm.h"

using namespace std;
using namespace Tseerapi;


int main()
{
	int iRet = 0;
	string errMsg = "";

	InitAgentApiParams initParams;
	iRet = ApiSetAgentIpInfo(initParams, errMsg);
	if (iRet != 0) {
		cout << "init Agent error" << endl;
		return 0;
	}

	//初始化ApiGetRoutes的参数
	RoutersRequest req;
	req.obj = "tencent.tencentServer.HelloService";
	req.lbGetType = LB_GET_SET;
	req.setInfo = "sz.a.b";

	iRet = ApiGetRoutes(req, errMsg);
	if (iRet == 0) {
		vector<NodeInfo>::iterator vIter;
		string sNodeInfo;
		for (vIter = req.nodeInfoVec.begin(); vIter != req.nodeInfoVec.end(); vIter++) {
			sNodeInfo += vIter->ip + ":";
			sNodeInfo += to_string(vIter->port) + " | ";
			if (vIter->isTcp) {
				sNodeInfo += "tcp | ";
			} else {
				sNodeInfo += "udp | ";
			}
			sNodeInfo += vIter->slaveSet;
			cout << "[NODEINFO]: " << sNodeInfo << endl;

			//调用被调
			sleep(5);

			//调用结束上报调用结果
			RouterRequest reportReq;
			reportReq.obj = req.obj;
			reportReq.lbGetType = req.lbGetType;
			reportReq.setInfo = req.setInfo;
			reportReq.ip = vIter->ip;
			reportReq.port = vIter->port;
			reportReq.isTcp = vIter->isTcp;

			ApiRouteResultUpdate(reportReq, 0, 1, errMsg);
			cout << "[report]errMsg: " << errMsg << endl;
		}
	}
}
```

### Demo3 使用纯Api方式获取节点

```c++
#include <iostream>
#include "Tseer_api.h"
#include "Tseer_comm.h"

using namespace std;
using namespace Tseerapi;

int main()
{
	int iRet = 0;
	string errMsg = "";

	InitRawApiParams initParams;
	iRet = ApiSetRegistryInfo(initParams, errMsg);
	if (iRet != 0) {
		cout << "init Agent error" << endl;
		return 0;
	}

	//初始化ApiGetRoute的参数
	RouterRequest req;
	req.obj = "tencent.tencentServer.HelloService";
	req.lbGetType = LB_GET_SET;
	req.setInfo = "sz.a.b";
	req.type = LB_TYPE_LOOP;
	req.moduleName = "seerClientDemo";
	iRet = ApiGetRoute(req, errMsg);
	cout << "[out]iRet: " << iRet << " errMsg: " << errMsg << endl;
	cout << "[out]ip: " << req.ip << endl;
	cout << "[out]port: " << req.port << endl;
	cout << "[out]isTcp: " << req.isTcp << endl;

	if (iRet == 0) {
		//进行调用
		sleep(5);
		//调用结束上报调用结果
		ApiRouteResultUpdate(req, 0, 1, errMsg);
		cout << "[report]errMsg: " << errMsg << endl;
	}
}
```

### Demo4 使用纯Api方式获取一组节点

```c++
#include <iostream>
#include <vector>
#include "Tseer_api.h"
#include "Tseer_comm.h"

using namespace std;
using namespace Tseerapi;
int main()
{
	int iRet = 0;
	string errMsg = "";

	InitRawApiParams initParams;
	iRet = ApiSetRegistryInfo(initParams, errMsg);
	if (iRet != 0) {
		cout << "init Agent error" << endl;
		return 0;
	}

	//初始化ApiGetRoute的参数
	RoutersRequest req;
	req.obj = "tencent.tencentServer.HelloService";
	req.lbGetType = LB_GET_SET;
	req.setInfo = "sz.a.b";

	iRet = ApiGetRoute(req, errMsg);
	if (iRet == 0) {
		vector<NodeInfo>::iterator vIter;
		for (vIter = nodeInfoVec.begin(); vIter != nodeInfoVec.end(); vIter++) {
			string sNodeInfo += vIter->ip + ":";
			sNodeInfo += to_string(vIter->port) + " | ";
			if (vIter->isTcp) {
				sNodeInfo += "tcp | ";
			} else {
				sNodeInfo += "udp | ";
			}
			sNodeInfo += vIter->slaveSet;
			cout << "[NODEINFO]: " << sNodeInfo << endl;

			//进行调用
			sleep(5);

			RouterRequest reportReq;
			reportReq.obj = req.obj;
			reportReq.lbGetType = req.lbGetType;

			//调用结束上报调用结果
			ApiRouteResultUpdate(req, 0, 1, errMsg);
			cout << "[report]errMsg: " << errMsg << endl;
		}
	}
}
```
