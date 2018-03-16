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
 * Created by denisfan on 2016/9/5.
 */
(function($, w, undefined){

    function bindEvent(){
        $('#generateCommand').click(function(){
            $('#ips').val($.trim($('#ips').val()));
            cloudjs('.v_node_1').validate({
                success:function(){
                    var ips = $('#ips').val();
                    var os_version = $('#os_version').val();
                    cloudjs.validate('removeErrmsg');
                    getCommond(ips, os_version);
                }
            });
        });

        $('#post_frame').load(function(){
            Util.hideMask();
        });
    }

    //第一种方式，获取安装命令
    function getCommond(ips, os_version){
        Util.showMask('正在获取安装命令');
        $.ajax({
            url:'/router_manager/agent_router_install_command.action?ip='+ips+'&os_version='+os_version,
            dataType:'json',
            success:function(data){
                Util.hideMask();
                if(data.ret_code == 200){
                    $('#commond_div').show();
                    $('#command').text(data.data.command);
                    $('#ip_a').text(data.data.ip);
                    $('#machine_ip').text(ips);
                    cloudjs('#copy_btn').clip({
                        swfPath: '/global-libs/cloudJs/js/Clipboard.swf',
                        onCopy: function(){
                            return $('#command').text();
                        },
                        afterCopy: function(){
                            Util.showMsg('success', '复制成功');
                        }
                    });
                }else{
                    Util.showMsg('error', '获取安装命令失败，失败原因：'+data.err_msg, 8000, 1 );
                }
            },
            error:function(){
                Util.hideMask();
                Util.showMsg('error', '获取安装命令失败', 8000, 1 );
            }
        });
    }


    $(function(){
        bindEvent();
    });

})(jQuery, this);