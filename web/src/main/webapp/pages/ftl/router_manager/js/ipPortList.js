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
    var serviceGroup = $('#service_group').val();
    var appName = $('#app_name').val();
    var serverName = $('#servername').val();
    var hasPrivileges = $('#hasPrivileges').val() == 'true';
    var ipPortData = {};
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
        {name:'present_state',type:'radio'},
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
        // present_state:'active',
        grid_flag:'NORMAL',
        enable_heartbeat: 'N'
    };
    var idcGroupList = null;

    function getTableData(){
        $.ajax({
            url:'/interface?interface_name=routerapi_get_server&interface_params={"service_group":"'+serviceGroup+'","app_name":"'+appName+'","servername":"'+serverName+'"}',
            // url:'http://localhost/ife/vif?id=91',
            dataType:'json',
            success:function(data){
                if(data.ret_code == 200){
                    var temp = [];
                    if(data.data && data.data[0]){
                        temp = data.data[0];
                    }
                    
                    renderTable(temp.port_list || []);
                }else{
                    renderTable('拉取ip端口列表失败，错误原因：' + data.err_msg);
                }
            },
            error:function(){
                renderTable('拉取ip端口列表失败');
            }
        });
    }

    function renderTable(data){
        if(typeof data == 'string'){
            $('#ip_port_list tbody').html('<tr><td colspan="15"><p class="red ta_c">'+data+'</td></tr>');
        }else{
            ipPortData = {};
            var html = '';
            for(var i = 0 ; i<data.length; i++){
                var key = data[i]['port_name']+'+'+data[i]['ip']+'+'+data[i]['port'];
                ipPortData[key] = data[i];
                html+='<tr data-key="'+key+'">';
                html += '<td class="ta_c"><input type="checkbox" /></td>';
                html += '<td class="ta_c">'+data[i]['port_name']+'</td>';
                html += '<td class="ta_c">'+data[i]['ip']+'</td>';
                html += '<td class="ta_c">'+data[i]['port']+'</td>';
                html += '<td class="ta_c">'+data[i]['protocol']+'</td>';
                html += '<td class="ta_c">'+(data[i]['enable_set']=='Y'?'是':'否')+'</td>';
                html += '<td class="ta_c">'+data[i]['set_name']+'</td>';
                html += '<td class="ta_c">'+data[i]['set_area']+'</td>';
                html += '<td class="ta_c">'+data[i]['set_group']+'</td>';
                html += '<td class="ta_c">'+(data[i]['enable_idc_group']=='Y'?'是':'否')+'</td>';
                html += '<td class="ta_c">'+data[i]['ip_group_name']+'</td>';
                html += '<td class="ta_c">'+data[i]['weight']+'</td>';
                html += '<td class="ta_c">'+(data[i]['present_state']=='active'?'<span class="green">active</span>':'<span class="red">inactive</span>')+'</td>';
                html += '<td class="ta_c">'+data[i]['grid_flag']+'</td>';
                html += '<td class="ta_c">'+(data[i]['enable_heartbeat']=='Y'?'开启':'不开启')+'</td>';
                html += '<td class="ta_c">';
                if(hasPrivileges){
                    html+='<a href="javascript:void(0);" class="del_ip_port_btn tips" title="删除"><i class="icon-remove" style="color:#c90000;"></i></a>&nbsp;&nbsp;';
                    html+='<a href="javascript:void(0);" class="copy_ip_port_btn tips" title="克隆"><i class="icon-copy" style="color:#0C61FF;"></i></a>&nbsp;&nbsp;';
                    html+='<a href="javascript:void(0);" class="edit_ip_port_btn tips" title="编辑"><i class="icon-edit" style="color:green;"></i></a>';
                }
                html+='</td>';
                html += '</tr>';
            }
            $('#ip_port_list tbody').html(html);
            $('#ip_port_list .del_ip_port_btn').click(function(){
                doDel($(this).parents('tr').eq(0));
            });
            $('#ip_port_list .edit_ip_port_btn').click(function(){
                var key = $(this).parents('tr').eq(0).data('key');
                showEditDialog(ipPortData[key]);
            });
            $('#ip_port_list .copy_ip_port_btn').click(function(){
                var key = $(this).parents('tr').eq(0).data('key');
                showAddDialog(ipPortData[key]);
            });
            cloudjs('#ip_port_list').wsdtable({
                curPage:1,
                pageSize:10000
            });
            cloudjs('.tips').tips({
                position:'down',
                track:true,
                hover:true,
                hideOnFocus:false
            });
        }
    }

    function bindEvent(){
        $('#add_ip_port_btn').click(function(){
            showAddDialog();
        });
        $('#del_ip_port_btn').click(function(){
            doDel($('#ip_port_list tbody [type="checkbox"]:checked').parents('tr'));
        });

        cloudjs('.tips').tips({
            position:'down',
            track:true,
            hover:true,
            hideOnFocus:false
        });

        $('#edit_win [name="enable_set"]').change(function(){
            var node = $('[name="set_name"],[name="set_area"],[name="set_group"]', '#edit_win');
            node.removeClass('err_msg');
            node.parents('td').find('.err_msg').remove();
            if($('#edit_win [name="enable_set"]:checked').val() == 'Y'){
                node.removeAttr('disabled');
            }else{
                node.val('');
                node.attr('disabled','disabled');
            }
        });
        $('#edit_win [name="enable_idc_group"]').change(function(){
            var node = $('[name="ip_group_name"]', '#edit_win');
            node.removeClass('err_msg');
            node.parents('td').find('.err_msg').remove();
            if($('#edit_win [name="enable_idc_group"]:checked').val() == 'Y'){
                cloudjs(node).combobox('enable');
            }else{
                cloudjs(node).combobox('setValue', '');
                cloudjs(node).combobox('disable');
            }
        });
        $('#check_all').change(function(){
            if($(this).prop('checked')){
                $('#ip_port_list tbody [type="checkbox"]').attr('checked', 'checked');
            }else{
                $('#ip_port_list tbody [type="checkbox"]').removeAttr('checked');
            }
        });
    }

    function showEditDialog(portData){
        var title = '修改端口',type='update';
        for(var i = 0 ; i< defaultPortItem.length; i++){
            var name = defaultPortItem[i]['name'];
            var input = $('#edit_win [name="'+name+'"]');
            if(defaultPortItem[i]['type'] == 'radio'){
                $('#edit_win [name="'+name+'"][value="'+portData[name]+'"]').attr('checked','checked');
                if(name == 'enable_set'){
                    if( portData[name] == 'N'){
                        $('[name="set_name"],[name="set_area"],[name="set_group"]', '#edit_win').attr('disabled','disabled');
                    }else{
                        $('[name="set_name"],[name="set_area"],[name="set_group"]', '#edit_win').removeAttr('disabled');
                    }
                }
                if(name == 'enable_idc_group'){
                    if( portData[name] == 'N'){
                        $('[name="ip_group_name"]', '#edit_win').attr('disabled','disabled');
                    }else{
                        $('[name="ip_group_name"]', '#edit_win').removeAttr('disabled');
                    }
                }
            }else if(defaultPortItem[i]['type'] == 'cascade'){
                var value = portData[name];
                (function(input, value){
                    getIdcGroupList(function(data){
                        cloudjs(input).combobox({
                            data:data,
                            defaultValue: value || '',
                            checkValue:true,
                            width:280
                        });
                    });
                })(input, value);
            }else{
                input.val(portData[name]);
            }
            if($('.'+name+'_text', '#edit_win').length){
                $('.'+name+'_text', '#edit_win').text(portData[name]);
            }
        }

        cloudjs('#edit_win').dialog({
            modal:true,
            title:title,
            width:800,
            height:250,
            onClose:function(){
                cloudjs('#edit_win .v_node').validate('removeErrmsg');
            },
            buttons:{
                确定:function(){
                    cloudjs('#edit_win .v_node').validate({
                        context:'#edit_win',
                        rule:{
                            set_not_null:{
                                check: function(ele){
                                    var val = ele.val();
                                    if($('#edit_win [name="enable_set"]:checked').val() == 'Y' && $.trim(val) == ''){
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
                                    if($('#edit_win [name="enable_idc_group"]:checked').val() == 'Y' && $.trim(val) == ''){
                                        return false;
                                    }else{
                                        return true;
                                    }
                                },
                                errmsg: function(){
                                    return '不能为空';
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
                            var data = {};
                            for(var i = 0 ; i< defaultPortItem.length; i++){
                                var name = defaultPortItem[i]['name'];
                                if(defaultPortItem[i]['type'] == 'radio'){
                                    data[name] = $('#edit_win [name="'+name+'"]:checked').val();
                                }else if(defaultPortItem[i]['type'] == 'cascade'){
                                    data[name] = $('#edit_win [name="'+name+'"]').parent().find('.combo_value').val()||'';
                                }else{
                                    data[name] = $('#edit_win [name="'+name+'"]').val();
                                }
                            }
                            if(data.enable_set == 'N'){
                                data.set_name = '';
                                data.set_area = '';
                                data.set_group = '';
                            }
                            if(data.enable_idc_group == 'N'){
                                data.ip_group_name = '';
                            }
                            submitPortData([data],type, function(){
                                cloudjs('#edit_win').dialog('close');
                            });
                        }
                    });
                },
                取消:function(){
                    this.close();
                }
            }
        }, function(){
            cloudjs('.c_tip').tips({
                zIndex:11000
            });
        });
    }

    function showAddDialog(obj){
        $('#ip_port_table tbody').html('');
        addPortConfTr(obj);
        var width = $('body').width()-60;
        var height = $(window).height() - 300;

        cloudjs('#add_win').dialog({
            title:'添加端口',
            width:width,
            height:height,
            modal:true,
            onClose:function(){
                cloudjs('#ip_port_table .v_input').validate('removeErrmsg');
            },
            onOpen: function(){
                var dia = $('#add_win').parents('.dialog_div');
                var bWidth = $('body').width();
                var bPLeft = parseInt($('body').css('padding-left'));
                var ownWidth = dia.outerWidth();
                var left = bPLeft + (bWidth - ownWidth)/2;
                dia.css('left', left);
            },
            buttons:{
                '确定':function(){
                    cloudjs('#ip_port_table .v_input').validate({
                        context:'#ip_port_table',
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
                            submitPortData(portList, 'add', function(){
                                cloudjs('#add_win').dialog('close');
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
    function addPortConfTr(model){
        var tr = '';
        tr += '<tr>' +
            '<td class="ta_c">'+ serviceGroup + '.' + appName + '.' + serverName + '.' + '<input type="text" name="port_name" style="width:100px;" class="v_input cloudjs_input" data-check="empty;nameRule"></td>'+
            '<td class="ta_c"><input type="text" name="ip" style="width:120px;" class="v_input cloudjs_input" data-check="empty;ip"></td>'+
            '<td class="ta_c"><input type="text" name="port" style="width:50px;" class="v_input cloudjs_input" data-check="empty;inum"></td>'+
            '<td class="ta_c"><label style="display: inline-block"><input type="radio" name="protocol_'+count+'" value="tcp" checked="checked" />tcp</label>&nbsp;&nbsp;'+
            '<label style="display: inline-block"><input type="radio" name="protocol_'+count+'" value="udp" />udp</label></td>'+
            '<td class="ta_c"><label style="display: inline-block"><input type="radio" name="enable_set_'+count+'" value="Y" />是</label>&nbsp;&nbsp;'+
            '<label style="display: inline-block"><input type="radio" name="enable_set_'+count+'" value="N" checked="checked" />否</label></td>'+
            '<td class="ta_c"><input type="text" title="不能存在中文字符" name="set_name" style="width:50px;" class="v_input cloudjs_input c_tip" data-check="set_not_null;not_chinese"></td>'+
            '<td class="ta_c"><input type="text" title="不能存在中文字符" name="set_area" style="width:50px;" class="v_input cloudjs_input c_tip" data-check="set_not_null;not_chinese"></td>'+
            '<td class="ta_c"><input type="text" title="不能存在中文字符" name="set_group" style="width:50px;" class="v_input cloudjs_input c_tip" data-check="set_not_null;not_chinese"></td>'+
            '<td class="ta_c"><label style="display: inline-block"><input type="radio" name="enable_idc_group_'+count+'" value="Y" />是</label>&nbsp;&nbsp;'+
            '<label style="display: inline-block"><input type="radio" name="enable_idc_group_'+count+'" value="N" checked="checked" />否</label></td>'+
            '<td class="ta_c"><input type="text" name="ip_group_name" class="v_input" data-check="idc_not_null" style="width: 50px;"></td>'+
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
				if(name == 'port_name'){
                    value = value.replace(serviceGroup + '.' + appName + '.' + serverName + '.', '');
                }
                $('[name="'+name+'"]',tr).val(value);
            }
        }
        count++;
        $('#ip_port_table tbody').append($(tr));
        $('.del_ip_port_btn',tr).click(function(){
            if($('#ip_port_table tbody tr').length<=1){
                cloudjs.message({
                    type:'error',
                    content:'不允许删除，至少要保留一项',
                    relative:'#add_win',
                    showCloseIcon:true,
                    showLeftIcon:true
                });
            }else{
                $(this).parents('tr').eq(0).remove();
            }
        });
        $('.copy_ip_port_btn',tr).click(function(){
            var tr = $(this).parents('tr').eq(0);
            addPortConfTr(tr);
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
                   Util.showMsg('error','拉取IDC组列表失败，错误原因'+data.err_msg, 5000, 1);
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
            var portNameNode = $('[name="port_name"]', $(this));
            var ipNode =  $('[name="ip"]', $(this));
            var portNode =  $('[name="port"]', $(this));
            var key = portNameNode.val()+ '+' + ipNode.val() + '+' + portNode.val();
            if(obj[key]){
                result = false;
                portNameNode.add(ipNode).add(portNameNode).addClass('red_border');
                return false;
            }else{
                portNameNode.add(ipNode).add(portNameNode).removeClass('red_border');
                obj[key] = true;
            }
        });
        return result;
    }

    function submitPortData(data,type, callback){
        var interfaceName = '';
        var typeStr = '';
        if(type == 'add'){
            typeStr = '添加';
            interfaceName = 'routerapi_add_ip_port';
        }else{
            typeStr = '修改';
            interfaceName = 'routerapi_update_ip_port';
        }
        $.ajax({
            url:'/interface?interface_name='+interfaceName,
            data:{
                interface_params:JSON.stringify({
                    service_group: serviceGroup,
                    app_name: appName,
                    servername: serverName,
                    port_list: data
                })
            },
            dataType:'json',
            type:'post',
            success:function(data){
                if(data.ret_code == 200){
                    Util.showMsg('success',typeStr+'端口名成功');
                    getTableData();
                    callback && callback();
                }else{
                    Util.showMsg('error',typeStr+'添加端口名失败，失败原因：'+data.err_msg, 5000, 1);
                }
            },
            error:function(){
                Util.showMsg('error',typeStr+'添加端口名失败', 5000, 1);
            }
        });
    }

    function doDel(trs){
        if(!trs.length){
            Util.openDlg('alert', '请选择要删除的端口');
            return;
        }
        Util.openDlg('confirm', '确定要删除选中的端口吗？', function(){
            var params = [];
            trs.each(function(){
                var key = $(this).data('key');
                var obj = ipPortData[key];
                if(obj){
                    params.push({
                        port_name: obj['port_name'],
                        ip: obj['ip'],
                        port: obj['port']
                    });
                }
            });
            $.ajax({
                url:'/interface?interface_name=routerapi_del_ip_port',
                data:{
                    interface_params:JSON.stringify({
                        service_group:serviceGroup,
                        app_name:appName,
                        servername:serverName,
                        port_list: params
                    })
                },
                dataType:'json',
                success:function(data){
                    if(data.ret_code == 200){
                        Util.showMsg('success','删除成功');
                        getTableData();
                    }else{
                        Util.showMsg('error','删除失败，错误原因：'+data.err_msg, 5000, 1);
                    }
                },
                error:function(){
                    Util.showMsg('error','删除失败', 5000, 1);
                }
            });
        });
    }


    $(function(){
        bindEvent();
        getTableData();
    });

    cloudjs('.c_tip').tips({
        zIndex:11000
    });

})(jQuery, this);