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
 * Created by xilizheng on 2016/5/28.
 */
var $uploadDlg;

/**
 * 上传操作
 */
function upload(){
    Util.showMask('正在上传，请稍候...');
    $('#upload_package').submit();
}

// 上传成功后的回调函数
function uploadCallback(retCode, data) {
    Util.hideMask();
    if ('0' === retCode) {
        cloudjs($uploadDlg).dialog('destroy');
        Util.showMsg('success', '上传成功');
        setTimeout(function(){
            location.reload();
        }, 1000);
    } else {
        Util.showMsg('error', '上传失败，原因：' + data, 0, 1);
    }
}

// 手动上传
function uploadPackage(obj) {
    $uploadDlg = $("<div id='id_upload_dlg' style='padding:5px;'></div>");
    $uploadDlg.load('/package/uploadpage.action', {}, function(){
        cloudjs($uploadDlg).dialog({
            title: '上传发布包',
            width:600,
            modal:true,
            buttons:{
                '提交': function() {
                    cloudjs('#upload_package').validate({
                        rule: {
                            tgz: {
                                check: function(ele){
                                    if(cloudjs.util.isEndWith('.tgz', ele.val().toLowerCase())){
                                        return true;
                                    }else{
                                        return false;
                                    }
                                },
                                errmsg: function(ele){
                                    return '请选择一个以.tgz为后缀的发布包文件';
                                }
                            }
                        },
                        success: function(){
                            upload();
                        }
                    });
                },
                '关闭': function() {
                    cloudjs($uploadDlg).dialog('close');
                }
            },
            onClose: function(){
                cloudjs('#upload_package').validate('removeErrmsg');
                cloudjs($uploadDlg).dialog('destroy');
            },
            onDragStart: function(){
                cloudjs('#upload_package').validate('removeErrmsg');
            },
            onOpen:function(){
                $('[name="file"]').change(function(){
                    var fileName = $(this).val();
                    if(!fileName){
                        return;
                    }
                    var point = fileName.lastIndexOf('.');
                    fileName = fileName.substring(0, point);
                    var arr = fileName.split('_');
                    var md5 = '', version = '', os_version;
                    if(arr.length >= 2){
                        md5 = arr[arr.length-1];
                        version = arr[arr.length -2];
                        os_version = arr[arr.length-3];
                    }
                    !($('[name="version"]',$uploadDlg).val()) && ($('[name="version"]',$uploadDlg).val(version));
                    !($('[name="md5"]',$uploadDlg).val()) && ($('[name="os_version"]',$uploadDlg).val(os_version));
                    !($('[name="md5"]',$uploadDlg).val()) && ($('[name="md5"]',$uploadDlg).val(md5));
                });
            }
        });
    });
}

/**
 * 获取参数
 */
function getParams(){
    $('#os_version option[value="' + Util.getUrlParams('os_version') + '"]').attr('selected', 'selected');
    $('#node_type option[value="' + Util.getUrlParams('nodeOrProxy') + '"]').attr('selected', 'selected');
}

/**
 * 修改发布包
 */
function editPackage(tr){
    var form = $('#edit_dialog'),
        params;

    form.find('[name="id"]').val(tr.attr('data-id'));
    form.find('.package_name').html(tr.attr('data-package-name'));
    form.find('.version').html(tr.attr('data-version'));
    form.find('[name="nodeOrProxy"][value="' + tr.attr('data-node-type') + '"]').attr('checked', 'checked');
    form.find('[name="package_type"][value="' + tr.attr('data-package-type') + '"]').attr('checked', 'checked');
    form.find('.os_version').html(tr.attr('data-os-version'));

    cloudjs('#edit_dialog').dialog({
        title: '修改发布包',
        width:600,
        modal:true,
        buttons:{
            '提交': function() {
                cloudjs('#edit_dialog').validate({
                    success: function(){
                        params = {
                            package_name:  form.find('.package_name').html(),
                            version: form.find('.version').html(),
                            package_type: form.find('[name="package_type"]:checked').val(),
                            os_version: form.find('.os_version').html()
                        },
                        Util.showMask();
                        $.ajax({
                            url: '/package/updatePackage.action',
                            data: params,
                            dataType: 'json',
                            success: function(resp){
                                Util.hideMask();
                                if(resp.ret_code == 200){
                                    Util.showMsg('success', '修改发布包成功');

                                    setTimeout(function(){
                                        location.reload();
                                    }, 1000);
                                }else{
                                    Util.showMsg('error', resp.err_msg);
                                }
                            },
                            error: function(){
                                Util.hideMask();
                                Util.showMsg('error', '修改发布包失败，服务异常');
                            }
                        });
                    }
                });
            },
            '关闭': function() {
                cloudjs('#edit_dialog').dialog('close');
            }
        },
        onClose: function(){
            cloudjs('#edit_dialog').validate('removeErrmsg');
            cloudjs('#edit_dialog').dialog('close');
        },
        onDragStart: function(){
            cloudjs('#edit_dialog').validate('removeErrmsg');
        }
    });
}

(function () {
    getParams();

    cloudjs('#page_list').table({pageSize: 20});

    $('.del_btn').bind('click', function(){
        var me = $(this),
            tr = me.closest('tr'),
            package_id = tr.attr('data-id'),
            package_name = tr.attr('data-package-name'),
            os_version = tr.attr('data-os-version'),
            package_type = tr.attr('data-package-type'),
            version = tr.attr('data-version');

        cloudjs.dialog('confirm', {
            content: '确定要删除发布包<font color="red">' + package_name + ' ' + version + '</font>吗？',
            relative: me,
            position:'left',
            ok: function(){
                Util.showMask('正在删除，请稍候...');
                $.ajax({
                    url: '/package/delPackage.action',
                    data: {
                        id: package_id,
                        package_name: package_name,
                        os_version: os_version,
                        package_type: package_type,
                        version: version
                    },
                    dataType: 'json',
                    success: function(resp){
                        Util.hideMask();
                        if(resp.ret_code == 200){
                            Util.showMsg('success', '删除发布包成功');
                            tr.remove();

                            cloudjs('#page_list').table({
                                pageSize: parseInt($('.paging_div_right.size'), 10) || 20,
                                pageNum: parseInt($('.paging_active a').text(), 10)
                            });
                        }else{
                            Util.showMsg('error', resp.err_msg);
                        }
                    },
                    error: function(){
                        Util.hideMask();
                        Util.showMsg('error', '删除发布包失败，服务异常');
                    }
                });
            }
        });
    });

    $('.edit_btn').bind('click', function(){
        editPackage($(this).closest('tr'));
    });
    
    $(function(){
       
    });
})();