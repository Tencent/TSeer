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

    var isAll = '1';

    var defaultPortItem = [
        {name:'port_name',type:'input'},
        {name:'ip',type:'input'},
        {name:'port',type:'input'},
        {name:'protocol',type:'radio'},
        {name:'enable_set',type:'radio'},
        {name:'set_name',type:'input'},
        {name:'set_area',type:'input'},
        {name:'set_group',type:'input'},
        {name:'enable_idc_group',type:'radio'},
        {name:'ip_group_name',type:'cascade'},
        {name:'weight',type:'input'},
        // {name:'present_state',type:'radio'},
        {name:'grid_flag',type:'radio'},
        {name:'enable_heartbeat',type:'radio'}
    ];

    var defaultPortValue = {
        port_name:'',
        ip:'',
        port:'',
        protocol:'tcp',
        enable_set:'N',
        set_name:'',
        set_area:'',
        set_group:'',
        enable_idc_group:'N',
        ip_group_name:'',
        weight:'-1',
        present_state:'active',
        grid_flag:'NORMAL',
        enable_heartbeat: 'N'
    };


    var idcGroupList = null;

    function search(){
        $.ajax({
            url: '/interface?interface_name=routerapi_get_service_group_list&interface_params={"is_all":"'+isAll+'"}',
            // url: 'http://localhost/ife/vif?id=90',
            dataType:'json',
            success:function(data){
                if(data.ret_code == 200){
                    renderTable(data.data || []);
                }else{
                    renderTable('拉取业务集列表失败，错误原因：' + data.err_msg);
                }
            },
            error:function(){
                renderTable('拉取业务集列表失败');
            }
        });
    }

    function renderTable(data){
        if(typeof data == 'string'){
            $('#service_group_div').html('<div class="ta_c""><p class="red ta_c">'+data+'</p></div>');
        }else{
            var html = '';
            for(var i =0; i<data.length; i++){
                var moduleData = data[i]['module_array'];
                var hasPermit = data[i]['hasPrivileges'] == '1';

                html += ' <div class="biz_set_box">';
                html += '<div class="top">';
                html += '<span class="ib b auto_ignore tips" style="width:300px;padding-right:20px;" title="'+data[i]['service_group']+'">业务集：'+data[i]['service_group']+'</span>';
                html += '</div><div class="main">';
                html += ' <p class="left_text">模块(共<span class="num">'+moduleData.length+'</span>个)：</p>';
                for(var j = 0; j<moduleData.length;j++){
                    html+='<a href="/router_manager/ip_port_list.action?service_group='+data[i]['service_group']+'&module_name='+moduleData[j]+'" class="module_a">'+moduleData[j]+'</a>';
                }
                if(hasPermit){
                    html += '<a href="javascript:void(0)" class="add_module_btn tips"  data-service_group="' + data[i]['service_group'] + '" title="添加模块" >+</a>';
                }else{
                    html += '<span class="gray ib">（您无操作此业务集的权限）</span>';
                }
                html += '</div></div>';
            }
            $('#service_group_div').html(html);
            cloudjs('.tips').tips({
                position:'down',
                track:true,
                hover:true,
                hideOnFocus:false
            });
        }
    }

    function bindEvent(){
        $('#add_service_group_btn').click(function(){
            addserviceGroup();
        });
        $('.add_module_btn').live('click', function(){
            var serviceGroup = $(this).data('service_group');
            addModule(serviceGroup);
        });

        $('#app_name,#server_name').change(function(){
            var appName = $('#app_name').val();
            var serverName = $('#server_name').val();
            $('.app_name_span').text(appName);
            $('.server_name_span').text(serverName);
        });


    }

    //添加业务集
    function addserviceGroup() {
        $('#new_service_group').val('');
        cloudjs('#add_service_group_win').dialog({
            title:'添加业务集',
            width:600,
            modal:true,
            onClose:function(){
                cloudjs($('#new_service_group')).validate('removeErrmsg');
            },
            buttons:{
                '确定':function(){
                    cloudjs($('#new_service_group')).validate({
                        context:'#add_service_group_win',
                        rule: {
                            nameRule: {  //新增的一种校验格式，用于校验输入的内容是否是jpg格式的文件名
                                check: function(ele){
                                    var val = ele.val();
                                    var reg = /^([a-zA-Z_]{1}[a-zA-Z0-9_]*)$/;
                                    if(!val || reg.test(val)){
                                        return true;
                                    }else{
                                        return false;
                                    }
                                },
                                errmsg: function(){
                                    return '只能填英文，数字，下划线，大小写敏感，数字不能在开头';
                                }
                            }
                        },
                        success:function(){
                            var newserviceGroup = $('#new_service_group').val();
                            $.ajax({
                                url:'/interface?skey=&operator=&interface_name=routerapi_add_service_group&interface_params={"service_group":"'+newserviceGroup+'"}',
                                dataType:'json',
                                success:function(data){
                                    if(data.ret_code == 200){
                                        Util.showMsg('success', '添加业务集成功');
                                        cloudjs('#add_service_group_win').dialog('close');
                                        search();
                                    }else{
                                        Util.showMsg('error', '添加业务集失败，失败原因：'+data.err_msg,5000,1);
                                    }
                                },
                                error:function () {
                                    Util.showMsg('error', '添加业务集失败',5000,1);
                                }
                            });
                        }
                    });
                },
                '取消':function(){
                    this.close();
                }
            }
        }, function(){
            cloudjs('.c_tip').tips({
                zIndex:11000
            });
        });
    }

    //添加模块
    function addModule(serviceGroup){
        $('#ip_port_table tbody').html('');
        addPortConfTr(null, serviceGroup);
        $('#service_group_str').text(serviceGroup);
        $('#app_name').val('');
        $('#server_name').val('');

        var width = $('body').width()-60;
        var height = $(window).height() - 300;

        cloudjs('#add_module_win').dialog({
            title:'添加模块',
            width:width,
            height:height,
            modal:true,
            onClose:function(){
                cloudjs('#add_module_form .v_input').validate('removeErrmsg');
            },
            onOpen:function(){
                var dia = $('#add_module_win').parents('.dialog_div');
                var bWidth = $('body').width();
                var bPLeft = parseInt($('body').css('padding-left'));
                var ownWidth = dia.outerWidth();
                var left = bPLeft + (bWidth - ownWidth)/2;
                dia.css('left', left);
            },
            buttons:{
                '确定':function(){
                    cloudjs('#add_module_form .v_input').validate({
                        rule:{
                            set_not_null:{
                                check: function(ele){
                                    var val = ele.val();
                                    if($(ele).parents('tr').find('[name^="enable_set_"]:checked').val() == 'Y' && $.trim(val) == ''){
                                        return false;
                                    }else{
                                        return true;
                                    }
                                },
                                errmsg: function(){
                                    return '不能为空';
                                }
                            },
                            idc_not_null:{
                                check: function(ele){
                                    var val = ele.val();
                                    if($(ele).parents('tr').find('[name^="enable_idc_group_"]:checked').val() == 'Y' && $.trim(val) == ''){
                                        return false;
                                    }else{
                                        return true;
                                    }
                                },
                                errmsg: function(){
                                    return '不能为空';
                                }
                            },
                            nameRule: {  //新增的一种校验格式，用于校验输入的内容是否是jpg格式的文件名
                                check: function(ele){
                                    var val = ele.val();
                                    var reg = /^([a-zA-Z_]{1}[a-zA-Z0-9_]*)$/;
                                    if(!val || reg.test(val)){
                                        return true;
                                    }else{
                                        return false;
                                    }
                                },
                                errmsg: function(){
                                    return '只能填英文，数字，下划线，大小写敏感，数字不能在开头';
                                }
                            },
                            not_chinese:{
                                check: function(ele){
                                    var val = ele.val();
                                    var reg = new RegExp("[\\u4E00-\\u9FFF]+","g");
                                    return !(reg.test(val));
                                },
                                errmsg: function(){
                                    return '不能存在中文字符';
                                }
                            }
                        },
                        success:function(){
                            if(!checkIsRepeated()){
                                Util.openDlg('alert', '存在端口名，IP，端口完全一样的端口信息，请修改');
                                return;
                            }
                            var appName = $('#app_name').val();
                            var serverName = $('#server_name').val();
                            var portList = [];
                            $('#ip_port_table tbody tr').each(function(){
                                var obj = {};
                                for(var i = 0 ; i< defaultPortItem.length; i++){
                                    var name = defaultPortItem[i]['name'];
                                    if(defaultPortItem[i]['type'] == 'radio'){
                                        obj[name] = $('[name^="'+name+'_"]:checked',$(this)).val();
                                    }else if(defaultPortItem[i]['type'] == 'cascade'){
                                        obj[name] = $('[name="'+name+'"]',$(this)).parent().find('.combo_value').val();
                                    }else{
                                        if(name == 'port_name'){
                                            obj[name] = serviceGroup + '.' + appName + '.' + serverName + '.' + $('[name="'+name+'"]',$(this)).val();
                                        }else{
                                            obj[name] = $('[name="'+name+'"]',$(this)).val();
                                        }

                                    }
                                }
                                portList.push(obj);
                            });
                            $.ajax({
                                url:'/interface?skey=&operator=denisfan&interface_name=routerapi_add_ip_port',
                                data:{interface_params: JSON.stringify({
                                    service_group :serviceGroup,
                                    app_name: appName,
                                    servername: serverName,
                                    port_list:portList
                                })},
                                type:'post',
                                dataType:'json',
                                success:function(data){
                                    if(data.ret_code == 200){
                                        Util.showMsg('success','添加模块成功');
                                        serviceGroup = '';
                                        cloudjs('#add_module_win').dialog('close');
                                        search();
                                    }else{
                                        Util.showMsg('error','添加模块失败，失败原因：'+data.err_msg, 5000, 1);
                                    }
                                },
                                error:function () {
                                    Util.showMsg('error','添加模块失败', 5000, 1);
                                }
                            });
                        }
                    });
                },
                '取消':function(){
                    this.close();
                }
            }
        }, function(){
            cloudjs('.c_tip').tips({
                zIndex:11000
            });
        });
    }

    var count  = 0;
    function addPortConfTr(model, serviceGroup){
        var tr = '';
        tr += '<tr>' +
            '<td class="ta_c">' + serviceGroup + '.<span class="app_name_span"></span>.<span class="server_name_span"></span>.<input type="text" name="port_name" style="width:100px;" class="v_input cloudjs_input" data-check="empty;nameRule"></td>'+
            '<td class="ta_c"><input type="text" name="ip" style="width:120px;" class="v_input cloudjs_input" data-check="empty;ip"></td>'+
            '<td class="ta_c"><input type="text" name="port" style="width:50px;" class="v_input cloudjs_input" data-check="empty;inum"></td>'+
            '<td class="ta_c"><label style="display: inline-block"><input type="radio" name="protocol_'+count+'" value="tcp" checked="checked" />tcp</label>&nbsp;&nbsp;'+
            '<label style="display: inline-block"><input type="radio" name="protocol_'+count+'" value="udp" />udp</label></td>'+
            '<td class="ta_c"><label style="display: inline-block"><input type="radio" name="enable_set_'+count+'" value="Y" />是</label>&nbsp;&nbsp;'+
            '<label style="display: inline-block"><input type="radio" name="enable_set_'+count+'" value="N" checked="checked" />否</label></td>'+
            '<td class="ta_c"><input type="text" title="不能存在中文字符" name="set_name" style="width:100px;" class="v_input cloudjs_input c_tip" data-check="set_not_null;not_chinese"></td>'+
            '<td class="ta_c"><input type="text" title="不能存在中文字符" name="set_area" style="width:100px;" class="v_input cloudjs_input c_tip" data-check="set_not_null;not_chinese"></td>'+
            '<td class="ta_c"><input type="text" title="不能存在中文字符" name="set_group" style="width:100px;" class="v_input cloudjs_input c_tip" data-check="set_not_null;not_chinese"></td>'+
            '<td class="ta_c"><label style="display: inline-block"><input type="radio" name="enable_idc_group_'+count+'" value="Y" />是</label>&nbsp;&nbsp;'+
            '<label style="display: inline-block"><input type="radio" name="enable_idc_group_'+count+'" value="N" checked="checked" />否</label></td>'+
            '<td class="ta_c"><input type="text" name="ip_group_name" class="v_input" data-check="idc_not_null"></td>'+
            '<td class="ta_c"><input type="text" name="weight" value="-1" style="width:50px;" class="v_input cloudjs_input" data-check="empty;inum" ></td>'+
            // '<td class="ta_c"><label style="display: inline-block"><input type="radio" name="present_state_'+count+'" value="active" checked="checked"/>active</label>&nbsp;&nbsp;'+
            // '<label style="display: inline-block"><input type="radio" name="present_state_'+count+'" value="inactive" />inactive</label></td>'+
            '<td class="ta_c"><label style="display: inline-block"><input type="radio" name="grid_flag_'+count+'" value="NORMAL" checked="checked" />开启</label>&nbsp;&nbsp;'+
            '<label style="display: inline-block"><input type="radio" name="grid_flag_'+count+'" value="NO_FLOW" />不开启</label></td>' +
            '<td class="ta_c"><label style="display: inline-block"><input type="radio" name="enable_heartbeat_'+count+'" value="Y" checked="checked" />开启</label>&nbsp;&nbsp;'+
            '<label style="display: inline-block"><input type="radio" name="enable_heartbeat_'+count+'" value="N" />不开启</label></td>' +
            '<td class="ta_c">' +
            '<a href="javascript:void(0);" class="del_ip_port_btn" title="删除"><i class="icon-remove" style="color:#c90000;font-size:16px;"></i></a>&nbsp;&nbsp;' +
            '<a href="javascript:void(0);" class="copy_ip_port_btn tips" title="克隆，支持批量克隆"><i class="icon-copy" style="color:#0C61FF;font-size:16px;"></i></a>' +
            '</td>' +
            '</tr>';
        tr = $(tr);
        if(!model){
            model = defaultPortValue;
        }
        var isNode = false;
        if(model instanceof  jQuery){
            isNode = true;
        }
        for(var i = 0; i < defaultPortItem.length; i++){
            var name = defaultPortItem[i]['name'];
            var value;
            if(defaultPortItem[i]['type'] == 'radio'){
                value = '';
                if(isNode){
                    value = $('[name^="'+name+'_"]:checked', model).val();
                }else{
                    value = model[name];
                }
                $('[name^="'+name+'_"]',tr).removeAttr('checked');
                $('[name^="'+name+'_"][value="'+value+'"]',tr).attr('checked','checked');
                if(name == 'enable_set' && value == "N"){
                    $('[name="set_name"],[name="set_area"],[name="set_group"]',tr).attr('disabled', 'disabled').val('');
                }
                if(name == 'enable_idc_group' && value == "N"){
                    $('[name="ip_group_name"]', tr).attr('disabled', 'disabled').val('');
                }
            } else if(defaultPortItem[i]['type'] == 'cascade'){
                value = '';
                if(isNode){
                    value = $('[name="'+name+'"]',model).parent().find('.combo_value').val();
                }else{
                    value = model[name];
                }
                (function(input, defaultValue){
                    getIdcGroupList(function(data){
                        cloudjs(input).combobox({
                            data: data,
                            checkValue: true,
                            width: 280,
                            defaultValue: defaultValue||''
                        });
                    });
                })($('[name="'+name+'"]',tr), value);
            }else{
                value = '';
                if(isNode){
                    value = $('[name="'+name+'"]',model).val();
                }else{
                    value = model[name];
                }
                $('[name="'+name+'"]',tr).val(value);
            }
        }
        count++;
        $('#ip_port_table tbody').append($(tr));

        $('.app_name_span', $(tr)).text($('#app_name').val());
        $('.server_name_span', $(tr)).text($('#server_name').val());

        $('.del_ip_port_btn',tr).click(function(){
            if($('#ip_port_table tbody tr').length<=1){
                Util.showMsg('error','不允许删除，至少要保留一项',3000, 1);
            }else{
                $(this).parents('tr').eq(0).remove();
            }
        });
        $('.copy_ip_port_btn',tr).click(function(){
            var tr = $(this).parents('tr').eq(0);
            addPortConfTr(tr, serviceGroup);
        });
        $('[name^="enable_set_"]',tr).change(function(){
            var tr = $(this).parents('tr').eq(0);
            if($('[name^="enable_set_"]:checked',tr).val() == 'N'){
                $('[name="set_name"],[name="set_area"],[name="set_group"]',tr).attr('disabled', 'disabled').val('');
            }else{
                $('[name="set_name"],[name="set_area"],[name="set_group"]',tr).removeAttr('disabled');
            }

        });
        $('[name^="enable_idc_group_"]',tr).change(function(){
            var tr = $(this).parents('tr').eq(0);
            if($('[name^="enable_idc_group_"]:checked',tr).val() == 'N'){
                cloudjs('[name="ip_group_name"]', tr).combobox('setValue', '');
                cloudjs('[name="ip_group_name"]', tr).combobox('disable');
            }else{
                cloudjs('[name="ip_group_name"]', tr).combobox('enable');
            }

        });
    }

    //获取IDC列表，发起请求并记录结果，下次不重复发起
    function getIdcGroupList(callback){
        if(idcGroupList){
            callback && callback(idcGroupList);
        }
        var arr = [];
        $.ajax({
            url:'/interface?skey=&operator=&interface_name=routerapi_get_idc_group_list&interface_params={}',
            dataType:'json',
            success:function(data){
                if(data.ret_code == 200){
                    data = data.data;
                    for(var i = 0 ; i<data.length; i++){
                        arr.push({text:data[i]['value'],value:data[i]['key']});
                    }
                }else{
                    Util.showMsg('error', '拉取IDC组列表失败，错误原因'+data.err_msg, 5000, 1);
                }
                idcGroupList = arr;
                callback && callback(arr);
            },
            error:function(){
                Util.showMsg('error','拉取IDC组列表失败', 5000, 1);
                idcGroupList = arr;
                callback && callback(arr);
            }
        });
    }

    //检测填写的端口是否重复
    function checkIsRepeated(){
        var obj = {},result = true;
        $('#ip_port_table tbody tr').each(function(){
            var port_name = $('[name="port_name"]', $(this)).val();
            var ip =  $('[name="ip"]', $(this)).val();
            var port =  $('[name="port"]', $(this)).val();
            var key = port_name+ '+' + ip + '+' + port;
            if(obj[key]){
                result = false;
                return false;
            }else{
                obj[key] = true;
            }
        });
        return result;
    }

    $(function(){
        search();
        bindEvent();
    });

    cloudjs('.c_tip').tips({
        zIndex:11000
    });

})(jQuery, this);
