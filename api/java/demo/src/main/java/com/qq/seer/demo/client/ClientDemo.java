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

package com.qq.seer.demo.client;

import java.util.List;

import com.qq.cloud.router.client.LBGetType;
import com.qq.cloud.router.client.LBType;
import com.qq.cloud.router.client.Result;
import com.qq.cloud.router.client.Router;
import com.qq.cloud.router.client.RouterConfig;
import com.qq.cloud.router.client.RouterFactory;
import com.qq.cloud.router.client.RouterRequest;
import com.qq.cloud.router.client.RouterResponse;
import com.qq.cloud.router.client.RoutersRequest;
import com.qq.cloud.router.client.RoutersResponse;
import com.qq.cloud.router.client.ServerNode;
/**
 * seer api java客户端使用样例
 *
 */
public class ClientDemo {
	
	/**
	 * 获取单个节点的ipport并且上报调用结果示例
	 */
	public static void getRouterAndReportDemo() {
        RouterConfig.setSeerApiKey("apikey_from_ClientDemo");  //设置全局 API key,选填，强烈建议填写。按照实际apikey填写，这里只是示例。
		
		Router router = RouterFactory.getInstance().createAgentRouter();  //初始化路由，采用agent模式,如果要采用纯api方式，初始化方法如下条注释。
//      Router router = RouterFactory.getInstance().createApiRouter(); 
		
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
	}
	
	/**
	 * 获取被调服务的节点列表示例
	 */
	public static void getRoutersDemo() {
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
	}
	
	public static void main(String[] args) {
		getRouterAndReportDemo();
		getRoutersDemo();
	}

}
