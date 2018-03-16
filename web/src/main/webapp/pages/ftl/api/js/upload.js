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

/**
 * Created by denisfan on 2016/10/12.
 */
/**
 * Created by denisfan on 2016/9/5.
 */
(function($, w, undefined){
    function bindEvent(){
        $('#upload_file_input_show, #choose_file_btn').click(function(){
            $('#upload_file_input').click();
        });
        $('#upload_file_input').change(function(){
            $('#upload_file_input_show').val($(this).val());
        });
        $('#upload_iframe').load(function(){
            Util.hideMask();
        });

        // 上传成功后的回调函数
        function uploadCallback(retCode, data) {
            if(retCode == 0){
                cloudjs('<div>上传成功</div>').dialog({
                    title:'系统提示',
                    modal:true,
                    buttons:{
                        '继续上传':function(){
                            this.close();
                        },
                        '前往下载页面':function(){
                            location.href='/api_manage/download.action';
                        }
                    },
                    onClose:function(){
                        $('#upload_file_input').val('');
                        $('#upload_file_input_show').val('');
                        this.destroy();
                    }
                });
            }else{
                Util.openDlg('alert', '上传失败，错误原因：'+data);
            }
            $('#upload_iframe').attr('src', '');
        }
        w.uploadCallback = uploadCallback;

        $('#upload_file_btn').click(function(){
            if(!$('#upload_file_input').val()){
                Util.openDlg('alert', '请选择上传的API文件');
                return;
            }
            Util.showMask('正在上传');
            $('#upload_form').submit();
        });
    }

    $(function(){
        bindEvent();
    });

})(jQuery, this);