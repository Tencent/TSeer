# **目录**

>- 概述
>- 开发环境搭建说明
>- API接口说明
>- API接口使用样例

# **概述**

​        为分布式名字发现服务提供java客户端jar包，提供查询服务节点信息，模调数据上报等功能接口。本文简单说明api接口，并介绍api接口的使用方法，方便用户快速上手。

# **开发环境搭建说明**

#### **环境依赖**

- JDK1.7或以上版本
- Maven2.2.1或以上版本

   JDK与maven的安装方法请自行查找，不在这里介绍。

#### **构建工程**

​       通过IDE或是命令行创建一个maven项目，这里以eclipse为例，File -> New -> Project -> Maven Project -> maven-archetype-webapp(或是archetype-quickstart,以自己的实际需求为准)，生成一个maven项目。

#### 依赖配置

​       在刚才创建的项目的pom.xml中添加Tseer依赖。

```xml
<dependency>
  	<groupId>qq-cloud-central</groupId>
  	<artifactId>Tseer-client</artifactId>
  	<version>1.0.1</version>
</dependency>
```
# API接口说明

#### 1.API key的申请

​        首先需要为自己的业务在管理页面上申请一个key, 该key将作为初始化API的参数，为API设置key的方法如下。

```java
RouterConfig.setSeerApiKey("your server's API key");  //填入自己业务集的真实API key
```



#### 2.接口说明

| 编号   | 接口或类的名称       | 功能说明                                     |
| ---- | :------------ | ---------------------------------------- |
| 1    | RouterFactory | 实例化路由接口的工厂类，单例类，可以分别使用Agent方式和纯api方式两种方式实例化路由接口。 |
| 2    | RouterConfig  | 名字路由的配置类。                                |
| 3    | Router        | 名字路由对外提供的接口，可用于查询服务节点，上报模调数据。            |

#### 3.接口方法说明

**3.1初始化路由接口**

| 方法原型                        | 参数说明 | 返回值              | 所属类           |
| --------------------------- | ---- | ---------------- | ------------- |
| Router createAgentRouter(); | 无参数  | Router的Agent方式实例 | RouterFactory |
| Router createApiRouter();   | 无参数  | Router的纯api方式实例  | RouterFactory |

**3.2获取单个服务节点**

| 概述   | 按照指定方式获取服务的单个节点                          |
| :--- | ---------------------------------------- |
| 方法原型 | Result<RouterResponse> getRouter(RouterRequest request); |
| 参数说明 | request: 入参，需要获取服务的相关信息，主要包括请求服务名，获取方式，负载均衡方式等。 |
| 返回值  | 返回值中包括返回码和返回路由信息(RouterResponse)，返回码为0表示成功，-1表示失败，失败时会包含返回的错误信息。 |
| 备注   | RouterRequest类结构和RouterResponse类结构见下方代码。 |



RouterRequest:

```java
public class RouterRequest {

    private String obj; //请求服务名称，必填
    private LBGetType lbGetType; //按IDC、set或all获取，默认ALL,选填
    private String setInfo; //set信息，当按set方式获取时必填
    private LBType lbType; //负载均衡类型，默认轮询，选填
    private long hashKey; //hash key，选择哈希负载均衡时必填
    private String callModuleName; //主调模块名，模调数据上报时需要该信息，若为空则用本地Ip替代，可选（推荐填写）
    ....省略getter和setter方法
}
```



LBGetType:

```java
public enum LBGetType {
    LB_GET_IDC(0), //按IDC
    LB_GET_SET(1), //按set
    LB_GET_ALL(2); //全部获取
}
```



LBType:

```java
public enum LBType {
    LB_TYPE_LOOP(0), //轮询
    LB_TYPE_RANDOM(1), //随机
    LB_TYPE_STATIC_WEIGHT(2), //静态权重
    LB_TYPE_CST_HASH(3), //一致性哈希
    LB_TYPE_ALL(4);  //获取所有节点，调用getRouter方法时不要设置成此类型,否则默认返回列表中的第一个节点
}
```



RouterResponse:

```java
public class RouterResponse {
	
	private ServerNode serverNode; //返回的服务节点
	private String responseMsg;   //返回信息
	...
}
```



ServerNode:

```java
public class ServerNode {
	private String sIP;    //服务的IP
	private int port;    //服务端口
	private int timeout; //节点访问超时时间
	private boolean bTcp;  //是否是tcp通信
	...
}
```



**3.3获取服务的节点列表**

| 概述   | 按照指定方式获取服务的所有节点                          |
| ---- | ---------------------------------------- |
| 方法原型 | Result<RoutersResponse> getRouters(RoutersRequest request); |
| 参数说明 | request: 入参，需要获取服务的相关信息，主要包括请求服务名，获取方式。  |
| 返回值  | 返回值中包括返回码和返回路由信息(RoutersResponse)，返回码为0表示成功，-1表示失败，失败时会包含返回的错误信息 |
| 备注   | RoutersRequest类结构和RoutersResponse类结构见下方代码。 |

RoutersRequest:

```java
public class RoutersRequest {
	
	private String obj;
	private LBGetType lbGetType;
	private String setInfo;
	...
}
```

RoutersResponse:

```java
public class RoutersResponse {
	
	private String responseMsg;
	private List<ServerNode> node;    //返回节点列表
	...
}
```

**3.4上报模调数据**

| 概述   | 上报调用服务的结果（成功、超时或是异常、耗时），用户完成一次业务请求后，最好调用一次上报接口上报真实的业务请求结果，该结果除了帮助SeerAgent和纯api进行异常服务节点屏蔽，还会上报到监控平台 |
| ---- | ---------------------------------------- |
| 方法原型 | Result<String>  resultReport(RouterRequest req, RouterResponse res, int ret, long timecost); |
| 参数说明 | req:getRouter方法的入参。res:getRouter方法的返回值。ret:getRouter方法的返回码。timecost:调用服务时长(ms) |
| 返回值  | 返回值包括返回码与返回信息说明，返回码0表示成功，-1表示失败。         |


# API使用样例

#### 1.获取单个服务节点，并上报模调数据

```java
        Router router = RouterFactory.getInstance().createAgentRouter();  //初始化路由，采用agent模式,如果要采用纯api方式，初始化方法如下条注释。
        //Router router = RouterFactory.getInstance().createApiRouter(); 
		
        RouterRequest request  = new RouterRequest();   //创建查询单节点请求。
        request.setObj("tencent.tencentServer.HelloService");  //设置请求被调服务的名字，需要提前在管理平台上注册接入，按照实际注册的服务名填写，必填。
        request.setLbGetType(LBGetType.LB_GET_SET);     //设置获取节点方式，可以选用按set分组，IDC分组或是全部获取，默认使用ALL, 选填。
        request.setSetInfo("sz.a.b");                   //set信息，选择按set分组时必填。
        request.setLbType(LBType.LB_TYPE_CST_HASH);     //选取节点采用的负载均衡算法，当前可选轮询，静态权重，随机和一致性hash，默认使用轮询，选填 。
        request.setHashKey((long)10086);                //hash key, 选择一致性hash方法时必填。
        request.setCallModuleName("seerClientDemo");    //主调模块名，模调数据上报时需要改信息，选填，为空用本地IP填充。按照实际值填写
        
        Result<RouterResponse> result = router.getRouter(request);  //调用获取路由节点接口，传入request作为参数
        int ret_code = result.getRet();                 //返回结果码
        if (ret_code == 0) {                            //如果结果码为0表示表示调用成功
        	String IP = result.getData().getIP();       //获取返回节点IP
            int port = result.getData().getPort();      //获取返回节点port
            boolean istcp = result.getData().isbTcp();  //获取返回节点是否采用tcp通信，true表示采用tcp,false表示采用udp
            
        	System.out.println("get HelloService node: IP " + IP + " port " + port + 
        			" protol " + (istcp ? " tcp": " udp"));
        	
	        //模拟调用服务，访问返回得到的被调节点
        	long start_time = System.currentTimeMillis();
        	try {
        		Thread.sleep(100);
        	} catch (Exception e) {
        		e.printStackTrace();
        	}
        	
        	int call_result = 0;  //调用结果返回值，为0表示调用成功，-1表示调用异常，-2表示调用超时。注意：这里的返回是指调用被调完成之后的结果而不是指调用getRouter接口的返回值！！！
        	long timecost = System.currentTimeMillis()-start_time;  //调用被调服务的时长，单位是毫秒。
        	        	
        	router.resultReport(request, result.getData(), call_result, timecost); //调用结果上报接口，将查询请求和返回的查询结果以及调用服务返回值，调用服务时长信息一起上报。
        } else {
        	System.out.println(result.getData().getResponseMsg());   //查询路由失败，打印失败的相关信息。
        	System.out.println(result.getData().getIP());
        }
```

#### 2.获取被调服务的的节点列表

```java
		RouterConfig.setSeerApiKey("apikey_from_ClientDemo");  //设置全局 API key,选填，强烈建议填写。按照实际apikey填写，这里只是示例。
		
		Router router = RouterFactory.getInstance().createAgentRouter();  //初始化路由，采用agent模式,如果要采用纯api方式，初始化方法如下条注释。
//      Router router = RouterFactory.getInstance().createApiRouter(); 
		
		RoutersRequest request = new RoutersRequest();  //创建查询节点列表请求，注意与单节点请求类区分，这里多了一个s。
		request.setObj("tencent.tencentServer.HelloService");   //设置请求被调服务的名字，需要提前在管理平台上注册接入，按照实际注册的服务名填写，必填。
		request.setLbGetType(LBGetType.LB_GET_SET);     //设置获取节点方式，可以选用按set分组，IDC分组或是全部获取，默认使用ALL, 选填。
		request.setSetInfo("sz.a.b");                   //set信息，选择按set分组时必填。
		
		Result<RoutersResponse> result = router.getRouters(request);
        int ret_code = result.getRet();                 //返回结果码
        if (ret_code == 0) {                            //如果结果码为0表示表示调用成功
        	List<ServerNode> nodes = result.getData().getNode(); //获取节点列表        	
        	for (ServerNode node : nodes) {
        		String IP = node.getsIP();
        		int port = node.getPort();
        		boolean istcp = node.isbTcp();
        		System.out.println("get HelloService node: IP " + IP + " port " + port + 
            			" protol " + (istcp ? " tcp": " udp"));
        	}
        } else {
        	System.out.println(result.getData().getResponseMsg());   //查询路由失败，打印失败的相关信息。
        }
```

