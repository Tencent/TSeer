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
    <title>Tseer管理平台-Agent管理</title>

</head>
<body>
<#include "../common/layout.ftl">
    <style>
        #page_list .sort_icon{position: static; display: inline-block; vertical-align: -2px;}
        .locator_ip{
            border-bottom:1px dashed #999;
        }
        .th_filter_wrap{
            position:relative;
        }
        .filter_th_div{
            position:absolute;
            display: none;
            background: #FFF;
            border: 1px solid #CCC;
            width: 102px;
            right: 0px;
            padding: 5px 8px;
            z-index: 2;
        }
        .filter_th_div label{
            display: block;
            margin: 2px;
        }
        .filter_th_div label input[type="checkbox"]{
            vertical-align: -2px;
            margin-right:2px;
        }
    </style>
<input type="hidden" id="username" value="${username!''}" />
<input type="hidden" id="isAdmin" value="${isAdmin!''}" />
<table class="condition_table">
    <tbody>
    <tr>
        <td class="tit">IP</td>
        <td><input class="cloudjs_input" name="ip" /></td>
        <td class="tit">当前状态</td>
        <td>
            <select name="present_state" class="cloudjs_input" style="width:171px;">
                <option value="">全部</option>
                <option value="active">active</option>
                <option value="inactive">inactive</option>
            </select>
        </td>
        <td class="tit">OS版本</td>
        <td><input class="cloudjs_input" name="os_version" /></td>
        <td>
            <button class="undeply_bt blue_btn ml15" type="submit" id="search_btn"><i class="icon_search_white"></i>&nbsp;查询</button>
        </td>
    </tr>
    </tbody>
</table>
<div class="table_toolbar" style="margin-top:8px;">
    <span class="button_item">
        <#if isAdmin?? && (isAdmin == 'true')>
            <button class="action-do blue_btn" id="gray_release">设置灰度</button>
            <button class="action-do blue_btn ml15" id="formal_release">取消灰度</button>
            <button class="action-do blue_btn ml15" id="update_agent_locator">更新主控地址</button>
        </#if>
        <button class="action-do gray_btn ml15" id="undeploy_btn">下线</button>
        <div id="th_filter_wrap" class="fr th_filter_wrap">
            <button class="gray_btn" id="th_filter_btn">过滤表格项&nbsp;<img src="/images/select_down.png" style="vertical-align: -2px;" /></button>
            <div class="filter_th_div" id="filter_th_div">
                <label><input class="filter_chk" type="checkbox" data-filter-index="1">IP</label>
                <label><input class="filter_chk" type="checkbox" data-filter-index="2">当前状态</label>
                <label><input class="filter_chk" type="checkbox" data-filter-index="3">当前版本</label>
                <label><input class="filter_chk" type="checkbox" data-filter-index="4">安装时间</label>
                <label><input class="filter_chk" type="checkbox" data-filter-index="5">OS版本</label>
                <label><input class="filter_chk" type="checkbox" data-filter-index="6">是否灰度</label>
                <label><input class="filter_chk" type="checkbox" data-filter-index="7">灰度版本</label>
                <label><input class="filter_chk" type="checkbox" data-filter-index="8">正式版本</label>
                <label><input class="filter_chk" type="checkbox" data-filter-index="9">主控地址</label>
                <label><input class="filter_chk" type="checkbox" data-filter-index="10">上次心跳时间</label>
            </div>
        </div>
    </span>
</div>
<div id="table_div"></div>

<div id="update_agent_locator_div" style="display: none;">
    <table class="condition_table">
        <tr>
            <td class="tit" width="130">agent机器列表</td>
            <td>
                <span id="iplist"></span>
            </td>
        </tr>
        <tr>
            <td class="tit">更新的主控地址列表</td>
            <td><input type="text" class="cloudjs_input" style="width:600px;" id="locator" />&nbsp;&nbsp;<i class="icon_question_trans ctip"  title="待更新的主控地址列表，格式：ip:port|ip:port，多个地址用|号分隔，默认为空，由服务端自动获取最新的主控地址去更新"></i></td>
        </tr>
    </table>
</div>
<script src="/js/util.js?rand=${RANDOM_STRING!''}"></script>
<script src="/global-libs/js/jquery.cookie.js"></script>
<script src="/pages/ftl/package/js/grayReleasedPage.js?rand=${RANDOM_STRING!''}"></script>
</body>
</html>