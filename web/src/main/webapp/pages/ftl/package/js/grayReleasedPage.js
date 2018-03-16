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

var $node_type = '';

var sIp = '',
    sBgName = '',
    sProduct = '',
    sChargename = '',
    sPresentState = '',
    sOsVersion = '',
    sGrayVer = '',
    sFormalVer = '',
    sCurVer = '';


var oldCookieVersion = '5559622';
var newCookieVersion = '2017091901';

//清除原先demo预留的cookie
$.cookie('agent_hide_th_'+oldCookieVersion, '', {expires: -1});


/**
 * 获取查询参数
 */
function search() {
    sIp = $('[name="ip"]').val();
    sBgName = $('[name="bg_name"]').parent().find('.combo_value').val() || '';
    sProduct = $('[name="product"]').parent().find('.combo_value').val() || '';
    sChargename = $('[name="chargename"]').val();
    sPresentState = $('[name="present_state"]').val();
    sOsVersion = $('[name="os_version"]').val();
    sGrayVer = $('[name="gray_ver"]').val();
    sFormalVer = $('[name="formal_ver"]').val();
    sCurVer = $('[name="cur_ver"]').val();
    doSearch();
}

/**
 * 执行查询
 */
function doSearch() {
    $('#table_div').load('/package/incGrayReleasedPage.action', {
        ip: sIp,
        bg_name: sBgName,
        product: sProduct,
        chargename: sChargename,
        present_state: sPresentState,
        os_version: sOsVersion,
        gray_ver: sGrayVer,
        formal_ver: sFormalVer,
        cur_ver: sCurVer
    }, function () {
        cloudjs('#table_div>table').table({
            pageSize: 20,
            onBodyChange:function(){
                $('.all, .item', '#table_div').removeAttr('checked');
                filterTh();
            }
        });
        $('#table_div .all').bind('click', function () {
            if ($(this).is(':checked')) {
                $('.item:visible').attr('checked', 'checked');
            } else {
                $('.item').removeAttr('checked');
            }
        });
        cloudjs('#table_div .locator_ip_td').tips({
            position: 'down',
            hover: true
        });
    });
}
/**
 * 表格过滤下拉框事件
 */
function initFilterTalbeDiv(){
    $('#th_filter_wrap').hover(function(){
        $('#filter_th_div').show();
    },function(){
        $('#filter_th_div').hide();
    });

    $('#filter_th_div .filter_chk').change(function(){
        var arr = [];
        $('#filter_th_div .filter_chk').each(function(){
            if(!$(this).prop('checked')){
                arr.push($(this).data('filter-index'));
            }
        });
        $.cookie('agent_hide_th_'+newCookieVersion, arr.join(';'),{expires: 20000});
        doFilterTh(arr);
    });

    filterTh();

}

function  filterTh(){
    var filterArr =  $.cookie('agent_hide_th_'+newCookieVersion);
    filterArr = (filterArr === null || filterArr === undefined )?null : (filterArr.split(';'));
    doFilterTh(filterArr);
}


/**
    执行表格项过滤
 */
function doFilterTh(arr) {
    var defaultHideTh = [6,7,8,9,10];
    if(arr === null){
        arr = defaultHideTh;
    }
    $('#page_list thead th, #page_list tbody td').each(function () {
        var index = $(this).index();
        var isShow = true;
        for (var i = 0, len = arr.length; i < len; i++) {
            if (index == arr[i]) {
                isShow = false;
            }
        }
        if(isShow || index == 0){  //第一列checkbox列强制一定显示
            $(this).show();
        }else{
            $(this).hide();
        }
    });

    $('#filter_th_div .filter_chk').attr('checked', 'checked');
    $.each(arr, function () {
        $('#filter_th_div .filter_chk[data-filter-index="' + this.toString() + '"]').removeAttr('checked');
    });
}


/**
 * 灰度/正式登记
 * @param  {String} type 灰度/正式   灰度版本(2)还是正式版本(3)
 */
function grayPackage(type) {
    var data = {
        ips: '',
        server: $node_type,
        package_type: type
    };
    var text = '';

    $('.item:checked').each(function () {
        data.ips += $(this).attr('data-ip') + ';';
    });
    data.os_version = $('.item:checked').eq(0).attr('data-os-version');
    
    if (type === '1') {
        text = '设置';
    } else {
        text = '取消';
    }

    Util.showMask('正在' + text + '灰度，请稍候...');
    $.ajax({
        url: '/package/release.action',
        data: data,
        type: 'post',
        dataType: 'json',
        success: function (resp) {
            Util.hideMask();
            if (resp.ret_code == 200) {
                if (type === '2') {
                    Util.showMsg('success', text + '灰度成功，请等待后台进行发布');
                } else {
                    Util.showMsg('success', text + '灰度成功');
                }
            } else {
                Util.showMsg('error', resp.err_msg);
            }
        },
        error: function (XMLHttpRequest, textStatus, errorThrown) {
            Util.hideMask();
            Util.showMsg('error', text + '灰度失败，服务异常');
        }
    });
}

/**
 * 发布登记验证
 * @param  {String} type 灰度/正式   灰度版本(2)还是正式版本(3)
 */
function validateVer(type) {
    var flag = true, text = '';

    if ($('.item:checked').length === 0) {
        Util.showMsg('error', '请至少选择一台机器');
        flag = false;
    }

    var osVersion = '';

    $('.item:checked').each(function () {

        if(!osVersion){
            osVersion = $(this).attr('data-os-version');
        }else if(osVersion != $(this).attr('data-os-version')){
            Util.showMsg('error', '只能批量处理同个OS版本的机器');
            flag = false;
            return false;
        }

        if (type === '2' && $(this).attr('data-gray-ver') === '') {
            Util.showMsg('error', '当前选中的机器没有灰度版本');
            flag = false;
            return false;
        }

        if (type === '3' && $(this).attr('data-formal-ver') === '') {
            Util.showMsg('error', '当前选中的机器没有正式版本');
            flag = false;
            return false;
        }
    });

    return flag;
}

/**
 * 获取参数
 */
function getParams() {
    $('#net_id option[value="' + Util.getUrlParams('net_id') + '"]').attr('selected', 'selected');
    $('#node_type option[value="' + Util.getUrlParams('nodeOrProxy') + '"]').attr('selected', 'selected');
    $node_type = Util.getUrlParams('nodeOrProxy') || 'taf.tafNode';
}


/**
 * 下线操作
 * */
function undeploy() {
    if ($('.item:checked').length === 0) {
        Util.showMsg('error', '请至少选择一台机器');
        return false;
    }
    if($('#isAdmin').val() != 'true' && !(function(){
            var userName = $('#username').val();
            if(!userName){
                return false;
            }
            var r = true;
            $('.item:checked').each(function () {
                var chargeName = $(this).data('chargename')||'';
                chargeName = chargeName.split(';');
                var i = 0 , len = chargeName.length;
                for(; i < chargeName.length; i++){
                    if(chargeName[i] == userName){
                        break;
                    }
                }
                if(i==len){
                    r = false;
                    return false;
                }
            });
            return r;
        })()){
        Util.openDlg('alert', '选中的机器中存在您无权操作的机器，不允许执行下线操作');
        return false;
    }
    Util.openDlg('confirm', '确定要下线选中的机器吗？', function () {
        var ips = '';
        $('.item:checked').each(function () {
            ips += $(this).attr('data-ip') + ';';
        });
        Util.showMask('正在下线...');
        $.ajax({
            url: '/interface?interface_name=delete_agent&&interface_params={"ipList":"' + ips + '"}',
            dataType: 'json',
            success: function (data) {
                Util.hideMask();
                if (data.ret_code == 200) {
                    Util.showMsg('success', '下线成功');
                    setTimeout(function () {
                        location.reload();
                    }, 1500);
                } else {
                    Util.showMsg('error', '下线失败，错误原因：' + data.err_msg, 8000, 1);
                }
            },
            error: function () {
                Util.hideMask();
                Util.showMsg('error', '下线失败', 8000, 1);
            }
        });
    });
}

/**
 * 更新Agent主控地址
 * */
function updateAgentLocator() {
    if ($('.item:checked').length === 0) {
        Util.showMsg('error', '请至少选择一台机器');
        return false;
    }
    var iplist = '';
    var locator = '';
    $('.item:checked').each(function () {
        iplist += $(this).data('ip') + ';';
    });
    iplist = iplist.substring(0, iplist.length - 1);
    $('#iplist').text(iplist);
    $('#locator').val('');
    cloudjs('#update_agent_locator_div').dialog({
        width: 800,
        modal: true,
        title: '更新Agent主控地址',
        onClose: function () {
            cloudjs.validate("removeErrmsg");
        },
        buttons: {
            '确定': function () {
                var self = this;
                var locator = $('#locator').val();
                $.ajax({
                    url: '/interface',
                    data: {
                        interface_name: 'routerapi_update_agent_locator',
                        interface_params: JSON.stringify({
                            iplist: iplist,
                            locator: locator
                        })
                    },
                    dataType: 'json',
                    success: function (data) {
                        if (data.ret_code == 200) {
                            Util.showMsg('success', '更新agent主控地址成功');
                            doSearch();
                            self.close();
                        } else {
                            Util.showMsg('error', data.err_msg, 8000, 1);
                        }
                    },
                    error: function () {
                        Util.showMsg('error', '更新agent主控地址失败', 8000, 1);
                    }
                });
            },
            '取消': function () {
                this.close();
            }
        }
    });
}

(function () {
    getParams();


    $('#gray_release').bind('click', function () {
        if (validateVer('1')) {
            grayPackage('1');
        }
    });

    $('#formal_release').bind('click', function () {
        if (validateVer('2')) {
            grayPackage('2');
        }
    });

    $('#undeploy_btn').bind('click', function () {
        undeploy();
    });

    $('#update_agent_locator').bind('click', function () {
        updateAgentLocator();
    });

    cloudjs('.ctip').tips({postion: 'down'});

    search();
    $('#search_btn').click(function () {
        search();
    });

    initFilterTalbeDiv();

    $(function () {
       
    });
})();