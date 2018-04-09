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
<html>
<head>
    <meta http-equiv="x-ua-compatible" content="IE=edge">
    <meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
    <title>操作日志查询</title>

</head>
<body>
<#include "../common/layout.ftl">
<input type="button" id="hide_btn" style="display:none;">

<div class="search_con">
    <table class="condition_table">
        <tr>
            <td class="tit">操作类型</td>
            <td> <input id="op_type" style="width:143px;" /></td>
            <td class="tit">操作开始时间</td>
            <td><input autocomplete="off" type="text" id="last_update_time_start_input" name="last_update_time_start_input" value="" class="cloudjs_input" readonly></td>
            <td class="tit">操作结束时间</td>
            <td><input autocomplete="off" type="text" id="last_update_time_end_input" name="last_update_time_end_input" value="" class="cloudjs_input" readonly></td>
        </tr>
        <tr>
            <td class="tit">修改后数据</td>
            <td><input autocomplete="off" type="text" id="after_data_input" name="after_data_input" value="" class="cloudjs_input"></td>
            <td class="tit">操作人</td>
            <td colspan="3"><input autocomplete="off" type="text" id="op_username" name="after_data_input" value="" class="cloudjs_input"></td>
        </tr>
        <tr>
            <td class="ta_c" colspan="6"> <button class="cloudjs_btn" id="query_btn">查&nbsp;询</button></td>
        </tr>
    </table>
    <!--<div class="clearfix">-->
        <!--<span class="search_item">-->
            <!--<label>操作类型</label>-->
            <!--<span id="op_type_wrap"></span>-->
        <!--</span>-->
        <!--<span class="search_item">-->
            <!--<label>操作开始时间</label>-->
            <!--<input autocomplete="off" type="text" id="last_update_time_start_input" name="last_update_time_start_input" value="" class="cloudjs_input" readonly>-->
        <!--</span>-->
        <!--<span class="search_item">-->
            <!--<label>操作结束时间</label>-->
            <!--<input autocomplete="off" type="text" id="last_update_time_end_input" name="last_update_time_end_input" value="" class="cloudjs_input" readonly>-->
        <!--</span>-->
        <!--<span class="search_item">-->
            <!--<label>操作标识</label>-->
            <!--<input autocomplete="off" type="text" id="data_flag_input" name="data_flag_input" value="" class="cloudjs_input">-->
        <!--</span>-->
        <!--<span class="search_item">-->
            <!--<label>应用名称</label>-->
            <!--<input autocomplete="off" type="text" id="app_name_input" name="app_name_input" value="" class="cloudjs_input">-->
        <!--</span>-->
        <!--<span class="search_item">-->
            <!--<label>服务名称</label>-->
            <!--<input autocomplete="off" type="text" id="server_name_input" name="server_name_input" value="" class="cloudjs_input">-->
        <!--</span>-->
        <!--<span class="search_item">-->
            <!--<label>节点名</label>-->
            <!--<input autocomplete="off" type="text" id="node_name_input" name="node_name_input" value="" class="cloudjs_input">-->
        <!--</span>-->
        <!--<span class="search_item">-->
            <!--<label>修改前数据</label>-->
            <!--<input autocomplete="off" type="text" id="before_data_input" name="before_data_input" value="" class="cloudjs_input">-->
        <!--</span>-->
        <!--<span class="search_item">-->
            <!--<label>修改后数据</label>-->
            <!--<input autocomplete="off" type="text" id="after_data_input" name="after_data_input" value="" class="cloudjs_input">-->
        <!--</span>-->
        <!--<span class="search_item">-->
            <!--<label>操作人</label>-->
            <!--<span id="op_username_wrap"></span>-->
        <!--</span>-->
    <!--</div>-->
    <!--<div class="clearfix" style="height:40px;padding-top:10px;">-->
        <!--<p class="ta_c f_l one_line">-->
           <!--<b class="odfl_btn_blue odfl_btn query_btn">查&nbsp;询</b>-->
        <!--</p>-->
    <!--</div>-->
</div>
<div id="main_body" style="margin-top:10px;"></div>

<div id="dialog_show_div" style="display:none;"></div>

<script src="/js/util.js?rand=${RANDOM_STRING!''}"></script>
<script type="text/javascript" src="/pages/ftl/oplog/js/log_show.js?${TAF_TIMESTAMP!}"></script>
</body>
</html>
