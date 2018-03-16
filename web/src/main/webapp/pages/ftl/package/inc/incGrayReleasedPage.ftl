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

<table class="paging_table" id="page_list">
    <thead>
        <tr>
            <th style="text-align:center; width: 40px;"><input autocomplete="off" type="checkbox" name="all" class="all"></th>
            <th>
                <div class="th_div">
                    IP
                    <div class="sort_icon string"><i class="cloudjs_icon up"></i><i class="cloudjs_icon down"></i></div>
                </div>
            </th>
            <th>
                <div class="th_div">
                    当前状态
                    <div class="sort_icon string"><i class="cloudjs_icon up"></i><i class="cloudjs_icon down"></i></div>
                </div>
            </th>
            <th>
                <div class="th_div">
                    当前版本
                    <div class="sort_icon string"><i class="cloudjs_icon up"></i><i class="cloudjs_icon down"></i></div>
                </div>
            </th>
            <th>
                <div class="th_div">
                    安装时间
                    <div class="sort_icon string"><i class="cloudjs_icon up"></i><i class="cloudjs_icon down"></i></div>
                </div>
            </th>
            <th>
                <div class="th_div">
                    OS版本
                    <div class="sort_icon string"><i class="cloudjs_icon up"></i><i class="cloudjs_icon down"></i></div>
                </div>
            </th>
            <th>
                <div class="th_div">
                    是否灰度
                    <div class="sort_icon string"><i class="cloudjs_icon up"></i><i class="cloudjs_icon down"></i></div>
                </div>
            </th>
            <th>
                <div class="th_div">
                    灰度版本
                    <div class="sort_icon string"><i class="cloudjs_icon up"></i><i class="cloudjs_icon down"></i></div>
                </div>
            </th>
            <th>
                <div class="th_div">
                    正式版本
                    <div class="sort_icon string"><i class="cloudjs_icon up"></i><i class="cloudjs_icon down"></i></div>
                </div>
            </th>
            <th>
                <div class="th_div">
                    主控地址
                    <div class="sort_icon string"><i class="cloudjs_icon up"></i><i class="cloudjs_icon down"></i></div>
                </div>
            </th>
            <th>
                <div class="th_div">
                    上次心跳时间
                    <div class="sort_icon string"><i class="cloudjs_icon up"></i><i class="cloudjs_icon down"></i></div>
                </div>
            </th>
        </tr>
    </thead>
    <tbody>
        <#if ipList?? && ipList?size != 0>
            <#list ipList as item>
                <tr>
                    <td class="tc"><input type="checkbox" class="item" data-ip="${item.ip!""}" data-gray-ver="${item.gray_ver!""}" data-formal-ver="${item.formal_ver!""}" data-chargename="${item.chargename!""}" data-os-version="${item.os_version!""}"></td>
                    <td>${item.ip!""}</td>
                    <td>
                        <#if item.present_state == 'active'>
                            <span class="green">active</span>
                            <#else>
                                <span class="red">inactive</span>
                        </#if>
                    </td>
                    <td>${item.cur_ver!"--"}</td>
                    <td>${item.last_reg_time!"--"}</td>
                    <td>${item.os_version!"--"}</td>
                    <td>
                        <#if (item.grayscale_state??) && (item.grayscale_state == '0')>无状态</#if>
                        <#if (item.grayscale_state??) && (item.grayscale_state == '1')>灰度中</#if>
                        <#if (item.grayscale_state??) && (item.grayscale_state == '2')>取消灰度</#if>
                    </td>
                    <td>${item.gray_ver!"--"}</td>
                    <td>${item.formal_ver!"--"}</td>
                    <td class="locator_ip_td" title="${item.locator!''}"><span class="locator_ip">${item.locatorIp!"--"}</span></td>
                    <td>${item.last_heartbeat!"--"}</td>
                </tr>
            </#list>
        <#else>
            <tr class="tc gray">
                <td colspan="11">查无任何IP信息</td>
            </tr>
        </#if>
    </tbody>
    <#if ipList?? && ipList?size != 0>
    <tfoot>
        <tr>
            <td colspan="11" class="tpage"></td>
        </tr>
    </tfoot>
    </#if>
</table>