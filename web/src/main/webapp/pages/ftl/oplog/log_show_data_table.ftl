<!--
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
-->

<input type="hidden" id="total_count" value="${total!}">
<input type="hidden" id="cur_page" value="${curPage!}">
<input type="hidden" id="page_size" value="${pageSize!}">
<table class="paging_table page_list" id="page_list">
	<thead>
        <tr>
            <th class="sort">操作类型</th>
            <!--<th class="sort">操作标识</th>-->
            <!--<th class="sort">应用名称</th>-->
            <!--<th class="sort">服务名称</th>-->
            <!--<th class="sort">节点名称</th>-->
            <!--<th>修改前数据</th>-->
            <th class="center">修改后数据</th>
            <!--<th>数据对比</th>-->
            <th class="sort">操作人</th>
            <th class="sort">操作时间</th>
        </tr>
    </thead>
    <tbody>
    	<#if dataList?exists && (dataList?size>0)>
            <#list dataList as item>
                <tr class="row_tr">
			      	<td>${item.f_op_type_desc!}</td>
			      	<!--<td>${item.f_data_flag!}</td>-->
			      	<!--<td>${item.f_app_name!}</td>-->
			      	<!--<td>${item.f_server_name!}</td>-->
			      	<!--<td>${item.f_node_name!}</td>-->
			      	<!--<td class="ta_c"><span data-title="修改前数据" style="display:none;" class="span_data">${item.f_before_data!}</span><a class="before_data_class" href="javascript:void(0);">查看数据</a>&nbsp;|&nbsp;<a class="copy_before_data_class" href="javascript:void(0);">拷贝数据</a></td>-->
			      	<td class="center" style="position: relative"><span data-title="修改后数据" style="display:none;" class="span_data">${item.f_after_data!}</span><a class="after_data_class" href="javascript:void(0);">查看数据</a>&nbsp;|&nbsp;<a class="copy_after_data_class" href="javascript:void(0);">拷贝数据</a></td>
			      	<!--<td class="ta_c"><a href="/compare/compare.jsp?f_id=${item.f_id!}" target="_blank">对比</a></td>-->
                    <td>${item.f_op_username!}</td>
			      	<td>${item.f_last_update_time!}</td>
			    </tr>
            </#list>
        <#else>
            <tr>
                <td colspan="4" class="center">查无任何操作日志信息</td>
            </tr>
        </#if>
    </tbody>
    <tfoot>
        <tr>
            <td class="pagebox" colspan="4"></td>
        </tr>
    </tfoot>
</table>
<script type="text/javascript">
	$(function(){
		var url = '/op_log/log_show_data_table.action',
           	curPage = '${curPage!1}', 
           	pageSize = '${pageSize!20}', 
           	total = '${total!}',
			f_op_type = $('#op_type').parent().find('.combo_value').val(),
//            f_data_flag = $('#data_flag_input').val(),
//            f_app_name = $('#app_name_input').val(),
//            f_server_name = $('#server_name_input').val(),
//            f_node_name = $('#node_name_input').val(),
//            f_before_data = $('#before_data_input').val(),
            f_after_data = $('#after_data_input').val(),
            f_op_username = $('#op_username').val(),
            f_last_update_time_start = $('#last_update_time_start_input').val(),
            f_last_update_time_end = $('#last_update_time_end_input').val();

        f_op_username = f_op_username.replace(/;$/,'');

		if(new Date(f_last_update_time_end).getTime() - new Date(f_last_update_time_start).getTime() > 90 * 60 * 60 * 24 * 1000){
			Util.openDlg('alert','搜索的操作开始和结束时间区间需在90天内，请修改查询参数');
			return;
		}

       	cloudjs('.page_list').wsdtable({
           	total:total,
           	pageSize:pageSize,
           	curPage:curPage,
           	ajaxUrl:url,
           	paramsJson:{
				f_op_type: f_op_type,
//				f_data_flag: f_data_flag,
//				f_app_name: f_app_name,
//				f_server_name: f_server_name,
//				f_node_name: f_node_name,
//				f_before_data: f_before_data,
				f_after_data: f_after_data,
				f_op_username: f_op_username,
				f_last_update_time_start: f_last_update_time_start,
				f_last_update_time_end: f_last_update_time_end,
              	curPage: curPage,
              	pageSize: pageSize
           	}
      	});
    });
</script>
