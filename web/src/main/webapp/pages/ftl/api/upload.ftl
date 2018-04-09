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
    <title>API文件上传</title>
</head>
<body>
<#include "../common/layout.ftl">
<form id="upload_form" method="post" enctype="multipart/form-data" target="upload_iframe" action="/DoApiUploadPackage">
    <input type="file" id="upload_file_input" style="display:none;" name="upload_file" />
</form>


<div style="text-align: center;margin-top: 200px;">
    <h1 style="margin-bottom:20px;color:#444;">API文件上传</h1>
    <input type="text" class="text_input_h" id="upload_file_input_show" style="width:400px;" />
    <button id="choose_file_btn" class="gray_btn" style="height:35px;vertical-align: 1px;">选择API文件</button>
    <button id="upload_file_btn" class="blue_btn" style="height:35px;vertical-align: 1px;">上&nbsp;&nbsp;&nbsp;&nbsp;传</button>
</div>

<iframe style="display: none;" id="upload_iframe" name="upload_iframe"></iframe>

<script src="/js/util.js?rand=${RANDOM_STRING!''}"></script>
<script src="/pages/ftl/api/js/upload.js?rand=${RANDOM_STRING!''}"></script>
</body>
</html>
