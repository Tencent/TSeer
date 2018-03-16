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
    <title>Tseer管理平台-agentRouter安装</title>
</head>
<body>
<#include "../common/layout.ftl">
    <style type="text/css">
        .title{
            font-size: 16px;
            border-left: solid 4px #258cd6;
            padding-left: 5px;
            margin: 15px 0;
            line-height: 1;
        }
        .code_div{
            border:1px solid #d2d2d2;
            padding:5px 10px;
            font-size:14px;
            word-break: break-all;
        }
    </style>
<div>
    <div class="b title" style="font-size:16px;">Agent安装</div>
    <table class="condition_table" style="margin-top:10px;">
        <caption>生成安装命令方式</caption>
        <tr>
            <td>
                <div class="position:relative">
                    <label style="display:none;">选择操作系统：</label>
                    <select id="os_version" class="whole_input" style="width:160px;display:none;">
                        <#list osList as item>
                            <option value="${item.os_version}">${item.os_version}</option>
                        </#list>
                    </select>
                    <label style="margin-left:20px;">节点ip (一次只能填写一个IP地址)：</label><input id="ips" value="" class="cloudjs_input v_node_1" data-check="empty;ip" />&nbsp;&nbsp;<input type="button" class="cloudjs_btn" id="generateCommand" value="生成命令"/></div>
                <div id="commond_div"  style="margin-top:10px; position:relative;display: none;">
                    <a href="javascript:void(0)" id="copy_btn">[复制]</a>
                    <span style="margin-left: 10px;color:#333;">（说明：请复制以下命令到机器<span id="machine_ip"></span>上执行,要求执行命令的用户有/usr/local/app/目录的读写权限）</span>
                    <div id="command" class="code_div"></div>
                </div>
            </td>
        </tr>
    </table>
</div>

<script src="/js/util.js?rand=${RANDOM_STRING!''}"></script>
<script src="/pages/ftl/router_manager/js/agentRouterInstall.js?rand=${RANDOM_STRING!''}"></script>
</body>
</html>