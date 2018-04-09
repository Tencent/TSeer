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
    <title>发布包上传</title>
</head>
<body>
<form id="upload_package" name="upload_package" encoding="multipart/form-data" enctype="multipart/form-data" target="hidden_frame" method="post" autocomplete="off" action="/DoCompileUploadPackage">
    <input type="hidden" name="uin" id="uin" />
    <input type="hidden" name="skey" id="skey" />

    <table class="condition_table" style="width: 100%;">
        <tbody>
            <tr>
                <td class="tr" style="width: 110px;">
                    发布包<font color="red">*</font>
                </td>
                <td>
                    <input type="file" name="file" class="cloudjs_input whole_input" data-check="tgz" />
                </td>
            </tr>
            <tr>
                <td class="tr">
                    版本<font color="red">*</font>
                </td>
                <td>
                    <input type="text" name="version" data-check="empty" class="cloudjs_input whole_input" />
                </td>
            </tr>
            <tr>
                <td class="tr">
                    MD5值<font color="red">*</font>
                </td>
                <td>
                    <input type="text" name="md5" data-check="empty" class="cloudjs_input whole_input" />
                </td>
            </tr>
            <tr>
                <td class="tr">发布类型</td>
                <td>
                    <label><input type="radio" value="1" name="package_type" />灰度</label>
                    &nbsp;&nbsp;
                    <label><input type="radio" value="2" name="package_type" />正式</label>
                </td>
            </tr>
            <tr>
                <td class="tr">适用的操作系统<font color="red">*</font></td>
                <td>
                    <select name="os_version" id="os_version" class="whole_input">
                    <#list osList as item>
                        <option value="${item.os_version}">${item.os_version}</option>
                    </#list>
                    </select>
                </td>
            </tr>
        </tbody>
    </table>
</form>
<iframe name="hidden_frame" id="hidden_frame" style="display:none;" src="javascript:false"></iframe>
</body>
</html>
