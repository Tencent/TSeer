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
    <title>API文件下载</title>
    </head>
<body>
<#include "../common/layout.ftl">
    <style type="text/css">
        .api_list_div{
            padding:10px 25px;
        }
        .api_list_div ul li{
            padding:8px 1em;
        }
        .api_list_div i{
            margin-right:5px;
        }
        .api_list_div ul{
            padding:8px 0px;
        }
        .api_list_div ul li i,
        .api_list_div li a{
            color: #3498db;
            font-size: 14px;
        }
    </style>
<div class="api_list_div">
    <h2><i class="icon-list-ul"></i>API文件列表</h2>
    <ul>
        <#list fileList as item>
            <#if item??>
                <li><i class="icon-download-alt"></i><a href="/DoFileDownload?file_name=${item}" target="_blank">${item}</a></li>
            </#if>
        </#list>
    </ul>
</div>
</body>
</html>