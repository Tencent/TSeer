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

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Tseer管理平台-IP端口列表</title>
</head>
<body>
<#include "../common/layout.ftl">
    <style type="text/css">
        #ip_port_table input[type="text"]{
            width:80px;
        }
        .paging_table .combo_container{
            background-color:#FFF;
        }
        .paging_table .combo_container.combo_disabled{
            background-color:#eeeeee;
        }
        .paging_table .combo_container .combo_panel{
            text-align: left;
        }
        .red_border{
            border-color:#F00;
        }
        .paging_table i{
            font-size:16px;
        }
        .paging_table a{
            text-decoration: none;
        }
        body .dialog_div_body{
            overflow:visible !important;
        }
        #ip_port_table th,#ip_port_table td,
        #ip_port_list th,#ip_port_list td{
            padding:5px 1px;
        }

    </style>
<input type="hidden" id="service_group" value="${service_group!''}" />
<input type="hidden" id="app_name" value="${app_name!''}" />
<input type="hidden" id="servername" value="${servername!''}" />
<input type="hidden" id="hasPrivileges" value="${hasPrivileges?string('true','false')}" />
<div style="position: relative">
    <table class="condition_table" style="width:auto; border-collapse: collapse;word-wrap: break-word;">
        <tr>
            <td class="tit" width="80">业务集</td>
            <td style="padding:2px 15px;">${service_group!''}</td>
            <td class="tit" width="80">模块</td>
            <td class="bg" style="padding:2px 15px;">${app_name!''}.${servername!''}</td>
            <td style="border:none;">
                <#if hasPrivileges?? && hasPrivileges == true>
                    <input type="button" class="cloudjs_btn_white" id="del_ip_port_btn" value="删除端口" />&nbsp;&nbsp;
                    <input type="button" class="cloudjs_btn" id="add_ip_port_btn" value="添加端口" />
                <#else>
                    <span class="gray">（您没有操作该模块的权限）</span>
                </#if>

            </td>
        </tr>
    </table>
    <div class="h8"></div>
    <table class="paging_table" id="ip_port_list">
        <thead>
        <tr>
            <th class="center"><input type="checkbox" id="check_all"></th>
            <th class="center sort">服务端口名</th>
            <th class="center sort">IP</th>
            <th class="center sort">端口</th>
            <th class="center sort">监听协议</th>
            <th class="center sort">启用set</th>
            <th class="center sort">set名</th>
            <th class="center sort">set地区</th>
            <th class="center sort">set组</th>
            <th class="center sort">启用IDC分组</th>
            <th class="center sort">IDC分组名</th>
            <th class="center sort">权重值</th>
            <th class="center sort">可用状态</th>
            <th class="center sort">开启流量</th>
            <th class="center sort">心跳超时检测</th>
            <th class="center">操作</th>
        </tr>
        </thead>
        <tbody></tbody>
    </table>
</div>

<div id="edit_win" style="display: none;overflow:visible !important;">
    <form id="port_form" name="port_form">
        <table class="condition_table">
            <tr>
                <td class="tit">服务端口名</td>
                <td class="bg">
                    <input type="hidden" name="port_name" readonly/>
                    <span  class="port_name_text"></span>
                </td>
                <td class="tit">IP</td>
                <td class="bg">
                    <input type="hidden" name="ip" readonly />
                    <span  class="ip_text"></span>
                </td>
            </tr>
            <tr>
                <td class="tit">端口</td>
                <td class="bg">
                    <input type="hidden" name="port" readonly />
                    <span  class="port_text"></span>
                </td>
                <td class="tit">监听协议</td>
                <td class="bg">
                   <span style="display: none">
                        <label><input type="radio" name="protocol" value="tcp" checked="checked" />tcp</label>&nbsp;&nbsp;
                        <label><input type="radio" name="protocol" value="udp" />udp</label>
                   </span>
                    <span class="protocol_text"></span>
                </td>
            </tr>
            <tr>
                <td class="tit">启用Set</td>
                <td class="bg">
                    <label><input type="radio" name="enable_set" value="Y" />是</label>&nbsp;&nbsp;
                    <label><input type="radio" name="enable_set" value="N" checked="checked" />否</label>
                </td>
                <td class="tit">set名</td>
                <td class="bg"><input type="text" title="不能存在中文字符" name="set_name" disabled="disabled" class="v_node cloudjs_input c_tip" data-check="set_not_null;not_chinese" /></td>
            </tr>
            <tr>
                <td class="tit">set地区</td>
                <td class="bg"><input type="text" title="不能存在中文字符" name="set_area" disabled="disabled" class="v_node cloudjs_input c_tip" data-check="set_not_null;not_chinese" /></td>
                <td class="tit">set组</td>
                <td class="bg"><input type="text" title="不能存在中文字符" name="set_group" disabled="disabled" class="v_node cloudjs_input c_tip" data-check="set_not_null;not_chinese" /></td>
            </tr>
            <tr>
                <td class="tit">启用IDC分组</td>
                <td class="bg">
                    <label><input type="radio" name="enable_idc_group" value="Y" />是</label>&nbsp;&nbsp;
                    <label><input type="radio" name="enable_idc_group" value="N" checked="checked" />否</label>
                </td>
                <td class="tit">IDC分组名</td>
                <td class="bg"><input type="text" name="ip_group_name" disabled="disabled" class="v_node cloudjs_input" data-check="idc_not_null" /></td>
            </tr>
            <tr>
                <td class="tit">权重</td>
                <td class="bg"><input type="text" name="weight" class="tips v_node cloudjs_input" title="-1表示不启用，大于等于0表示启用静态权重" data-check="empty;inum" /></td>
                <td class="tit">是否开启流量</td>
                <td class="bg">
                    <label><input type="radio" name="grid_flag" value="NORMAL" checked="checked" />开启</label>&nbsp;&nbsp;
                    <label><input type="radio" name="grid_flag" value="NO_FLOW"/>不开启</label>
                </td>
            </tr>
            <tr>
                <td class="tit">心跳超时检测</td>
                <td class="bg" colspan="3">
                    <label><input type="radio" name="enable_heartbeat" value="Y" checked="checked" />开启</label>&nbsp;&nbsp;
                    <label><input type="radio" name="enable_heartbeat" value="N" />不开启</label>
                </td>
            </tr>
        </table>
    </form>
</div>

<div id="add_win" style="display: none;">
    <p style="margin:5px 0px"><b>端口信息</b></p>
    <table class="paging_table" id="ip_port_table">
        <thead>
        <th class="center">服务端口名</th>
        <th class="center">IP</th>
        <th class="center">端口</th>
        <th class="center">监听协议</th>
        <th class="center">启用set</th>
        <th class="center">set名</th>
        <th class="center">set地区</th>
        <th class="center">set组</th>
        <th class="center">启用IDC分组</th>
        <th class="center">IDC分组名</th>
        <th class="center">权重值</th>
        <th class="center">开启流量</th>
        <th class="center">心跳超时检测</th>
        <th class="center">操作</th>
        </thead>
        <tbody>
        </tbody>
    </table>
</div>
<script src="/js/util.js?rand=${RANDOM_STRING!''}"></script>
<script src="/pages/ftl/router_manager/js/ipPortList.js?rand=${RANDOM_STRING!''}"></script>
</body>
</html>