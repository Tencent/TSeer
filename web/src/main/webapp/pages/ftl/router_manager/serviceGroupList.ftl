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

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Seer-服务端业务集列表</title>
</head>
<body>
<#include "../common/layout.ftl">
<link href="/pages/ftl/router_manager/css/serviceGroupList.css?rand=${RANDOM_STRING!''}" rel="stylesheet" />
<div style="margin:0px 10px 0px 10px;">
    <div class="tool_div">
        <div class="f_l">
            <input type="button" value="添加业务集" id="add_service_group_btn" class="cloudjs_btn_white"/>
        </div>
        <!--<div class="f_r">-->
            <!--<label><input type="radio" name="is_all"  value="0" checked="checked" />我负责的业务集</label>&nbsp;&nbsp;-->
            <!--<label><input type="radio" name="is_all"  value="1" />全部业务集</label>-->
        <!--</div>-->
    </div>
    <div class="title">业务集列表</div>
    <div id="service_group_div"></div>
</div>

<div id="add_service_group_win" style="display: none;">
    <form name="add_service_group_form" id="add_service_group_form">
        <table class="condition_table">
            <tr>
                <td class="tit">业务集</td>
                <td>
                    <input title="只能填英文，数字，下划线，大小写敏感，数字不能在开头" type="text" value="" id="new_service_group" name="new_service_group" style="width:300px;display: inline-block;" data-check="empty;nameRule" class="cloudjs_input c_tip" />
                    <span class="red">*</span>
                </td>
            </tr>
        </table>
    </form>
</div>

<div id="add_module_win" style="display: none;">
    <form name="add_module_form" id="add_module_form">
        <p style="margin:5px 0px;"><b>业务集模块信息</b></p>
        <table class="condition_table">
            <tr>
                <td class="tit">业务集</td>
                <td>
                    <span id="service_group_str"></span>
                </td>
            </tr>
            <tr>
                <td class="tit">模块名</td>
                <td>
                    <span style="display:inline-block;" class="vt"><input type="text" title="只能填英文，数字，下划线，大小写敏感，数字不能在开头" style="width:150px" id="app_name" name="app_name" class="v_input cloudjs_input c_tip" data-check="empty;nameRule"  /></span>&nbsp;<span class="vt">.</span>&nbsp;<span style="display:inline-block;" class="vt"><input title="只能填英文，数字，下划线，大小写敏感，数字不能在开头" type="text" style="width:150px" id="server_name" name="server_name" class="v_input cloudjs_input c_tip" data-check="empty;nameRule" /></span><span class="red vt">*</span>
                    <span class="gray vt" style="margin-left:10px;">填写提示：业务.服务</span>
                </td>
            </tr>
        </table>
        <div class="h8"></div>
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
    </form>
</div>
<script src="/js/util.js?rand=${RANDOM_STRING!''}"></script>
<script src="/pages/ftl/router_manager/js/serviceGroupList.js?rand=${RANDOM_STRING!''}"></script>
</body>
</html>
