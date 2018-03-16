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
    <title>Tseer管理平台-发布包管理</title>
</head>
<body>
<#include "../common/layout.ftl">
<style>
    .whole_input{width: 90%;}
    .validate_div{z-index: 10000!important;}
    #page_list .sort_icon{position: static; display: inline-block; vertical-align: -5px;}
</style>
<form action="/package/manage.action" method="get" id="form">
    <table class="taf_query_table">
        <tbody>
            <tr>
                <td>
                    <div class="taf_cloud_combo">
                        <span class="search_item">
                            <label>操作系统</label>
                            <select name="os_version" style="width: 181px;" id="os_version" class="ml15">
                                <option value="">全部</option>
                            <#list osList as item>
                                <option value="${item.os_version}" />${item.os_version}</label>
                            </#list>
                            </select>
                        </span>
                        <button class="undeply_bt blue_btn ml15" type="submit"><i class="icon_search_white"></i>&nbsp;查询</button>
                    </div>
                </td>
            </tr>
        </tbody>
    </table>
</form>
<div class="table_toolbar">
    <span class="button_item">
        <button class="blue_btn" onclick="uploadPackage();">上传发布包</button>
    </span>
</div>
<table class="paging_table" id="page_list">
    <thead>
        <tr>
            <th>
                <div class="th_div">
                    系统
                    <div class="sort_icon string"><i class="cloudjs_icon up"></i><i class="cloudjs_icon down"></i></div>
                </div>
            </th>
            <th>
                <div class="th_div">
                    包名
                    <div class="sort_icon string"><i class="cloudjs_icon up"></i><i class="cloudjs_icon down"></i></div>
                </div>
            </th>
            <th>
                <div class="th_div">
                    版本
                    <div class="sort_icon string"><i class="cloudjs_icon up"></i><i class="cloudjs_icon down"></i></div>
                </div>
            </th>
            <th>
                <div class="th_div">
                    发布类型
                    <div class="sort_icon string"><i class="cloudjs_icon up"></i><i class="cloudjs_icon down"></i></div>
                </div>
            </th>
            <th class="center" style="width: 100px;">操作</th>
        </tr>
    </thead>
    <tbody>
        <#if fileList?? && fileList?size != 0>
            <#list fileList as item>
                <tr data-id="${item.id}" data-package-name="${item.filename!""}" data-version="${item.version!""}" data-node-type="${item.nodeOrProxy!""}" data-package-type="${item.package_type!""}" data-os-version="${item.os_version!""}">
                    <td>${item.os_version!""}</td>
                    <td>${item.filename!""}</td>
                    <td>${item.version!""}</td>
                    <td>
                        <#if item.package_type?? && item.package_type != "">
                            <#if item.package_type == "1">
                                灰度
                            <#elseif item.package_type == "2">
                                正式
                            <#else>
                                --
                            </#if>
                        <#else>
                            --
                        </#if>
                    </td>
                    <td class="center">
                        <a href="javascript:void(0)" class="del_btn">删除</a>&nbsp;&nbsp;&nbsp;&nbsp;<a href="javascript:void(0)" class="edit_btn">修改</a>
                    </td>
                </tr>
            </#list>
        <#else>
            <tr class="tc gray">
                <td colspan="5">查无任何发布包信息</td>
            </tr>
        </#if>
    </tbody>
    <#if fileList?? && fileList?size != 0>
    <tfoot>
        <tr>
            <td colspan="5" class="tpage"></td>
        </tr>
    </tfoot>
    </#if>
</table>

<form id="edit_dialog" style="display: none;">
    <input type="hidden" name="id" />
    <table class="condition_table" style="width: 100%;">
        <tbody>
            <tr>
                <td class="tr" style="width: 110px;">
                    发布包
                </td>
                <td class="package_name"></td>
            </tr>
            <tr>
                <td class="tr">
                    版本<font color="red">*</font>
                </td>
                <td class="version">
                    <!--<input type="text" name="version" data-check="empty" class="cloudjs_input whole_input" />-->
                </td>
            </tr>
            <tr>
                <td class="tr">发布类型</td>
                <td>
                    <label><input type="radio" value="1" name="package_type" />灰度</label>
                    <label><input type="radio" value="2" name="package_type" />正式</label>
                </td>
            </tr>
            <tr>
                <td class="tr">适用的操作系统<font color="red">*</font></td>
                <td class="os_version"></td>
            </tr>
        </tbody>
    </table>
</form>

<script src="/js/util.js?rand=${RANDOM_STRING!''}"></script>
<script type="text/javascript" src="/js/jquery.form.js?rand=${RANDOM_STRING!''}"></script>
<script src="/pages/ftl/package/js/manage.js?rand=${RANDOM_STRING!''}"></script>
</body>
</html>