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
 * Created by denisfan on 2016/12/15.
 */
$(function () {
    initOpTime();

    $('#query_btn').click(function () {
        loadTable($.noop);
    });
    initOpTypeData();
    loadTable();

    function showDetailData(title, dataObj) {

        var str = [];

        str.push('<table class="paging_table"><thead><th width="100">属性</th><th>值</th></thead><tbody>');

        var i = 0, value;


        for (var p in dataObj) {
            if (dataObj.hasOwnProperty(p)) {
                str.push('<tr><td class="bg">' + p + '</td>');

                value = dataObj[p] || '';

                if (typeof value == 'object') {
                    try{
                        value = JSON.stringify(value);
                    }catch(e){

                    }
                }
                str.push('<td class="bg">' + value + '</td></tr>');
                i++;
            }
        }

        str.push('</tbody></table>');
        showDialog(title, str.join(''));
    }

    function showDialog(title, content) {
        $('#dialog_show_div').html(content);
        cloudjs('#dialog_show_div').dialog({
            title: title,
            modal: true,
            width: 800,
            height: 400,
            buttons: {
                '关闭': function () {
                    this.close();
                }
            }
        });
    }

    function initOpTypeData() {
        $.ajax({
            url: '/interface?skey=&operator=&interface_name=op_type_list&interface_params={}',
            dataType: 'json',
            success: function (resp) {
                if (resp.ret_code == 200) {
                    var data = resp.data || [],
                        tempData = [{value: '', text: '全部'}];
                    $.each(data, function (index, obj) {
                        tempData.push({value: obj.value, text: obj.key});
                    });
                    initOpTypeCascade(tempData, tempData[0].value || '');
                } else {
                    Util.showMsg('error', '获取操作类型列表失败，错误信息：' + resp.err_msg);
                }
            },
            error: function () {
                Util.showMsg('error', '获取操作类型列表失败，后台异常！');
            }
        });
    }

    function initOpTypeCascade(data, defaultKey) {
        cloudjs('#op_type').combobox({
            checkKey: true,
            data: data,
            defaultValue: defaultKey
        });
    }

    function initOpTime() {
        var startDateTextBox = $('#last_update_time_start_input');
        var endDateTextBox = $('#last_update_time_end_input');

        cloudjs('#last_update_time_start_input,#last_update_time_end_input').calendar({
            timeFormat: 'yyyy-mm-dd hh:mm:ss',
            months: 1
        });
        var newDate = new Date();

        var year, month, date, hour, minute, second;
        year = newDate.getFullYear();
        month = getFullNum(newDate.getMonth() + 1);
        date = getFullNum(newDate.getDate());
        // hour = getFullNum(newDate.getHours());
        // minute = getFullNum(newDate.getMinutes());
        // second = getFullNum(newDate.getSeconds());

        endDateTextBox.val(year + '-' + month + '-' + date + ' ' + 23 + ':' + 59 + ':' + 59);
        //endDateTextBox.val(year+'-'+month+'-'+date);

        newDate.setMonth(newDate.getMonth() - 1);

        year = newDate.getFullYear();
        month = getFullNum(newDate.getMonth() + 1);
        date = getFullNum(newDate.getDate());
        // hour = getFullNum(newDate.getHours());
        // minute = getFullNum(newDate.getMinutes());
        // second = getFullNum(newDate.getSeconds());

        startDateTextBox.val(year + '-' + month + '-' + date + ' ' + '00' + ':' + '00' + ':' + '00');
        //startDateTextBox.val(year+'-'+month+'-'+date);
    }

    function getFullNum(num) {
        num = (num < 10 ? (0 + '' + num) : num);
        return num;
    }

    function initZclip() {
        cloudjs(".copy_before_data_class,.copy_after_data_class").clip({
            swfPath: "/global-libs/cloudJs/js/Clipboard.swf",
            onCopy: function () {
                var text = $(this).closest('td').find('.span_data').html();
                return text;
            },
            afterCopy: function () {
                Util.showMsg('success', '复制成功');
            }
        });
    }

    function initShowDetail() {
        $('.before_data_class,.after_data_class').click(function () {
            var span = $(this).prev();
            var dataDetail = $.trim(span.html()),
                dataTitle = span.attr('data-title');
            if (dataDetail == '' || dataDetail == '{}') {
                Util.showMsg('error', '无可查看数据');
            } else {
                var data;
                try {
                    data = eval('(' + dataDetail + ')');
                } catch (e) {
                    showDialog(dataTitle, dataDetail);
                }
                showDetailData(dataTitle, data);
            }
        });
    }


    function loadTable() {
        var url = '/op_log/log_show_data_table.action',
            f_op_type = $('#op_type').parent().find('.combo_value').val(),
        // f_data_flag = $('#data_flag_input').val(),
        // f_app_name = $('#app_name_input').val(),
        // f_server_name = $('#server_name_input').val(),
        // f_node_name = $('#node_name_input').val(),
        // f_before_data = $('#before_data_input').val(),
            f_after_data = $('#after_data_input').val(),
            f_op_username = $('#op_username').val(),
            f_last_update_time_start = $('#last_update_time_start_input').val(),
            f_last_update_time_end = $('#last_update_time_end_input').val(),

            curPage = $('#cur_page').val() || '1', pageSize = $('#page_size').val() || '20';
        f_op_username = f_op_username.replace(/;$/, '');

        url = urlParam(url, 'f_op_type', f_op_type);
        // url = urlParam(url, 'f_data_flag', f_data_flag);
        // url = urlParam(url, 'f_app_name', f_app_name);
        // url = urlParam(url, 'f_server_name', f_server_name);
        // url = urlParam(url, 'f_node_name', f_node_name);
        // url = urlParam(url, 'f_before_data', f_before_data);
        url = urlParam(url, 'f_after_data', f_after_data);
        url = urlParam(url, 'f_op_username', f_op_username);
        url = urlParam(url, 'f_last_update_time_start', f_last_update_time_start);
        url = urlParam(url, 'f_last_update_time_end', f_last_update_time_end);

        url = urlParam(url, 'curPage', curPage);
        url = urlParam(url, 'pageSize', pageSize);
        url = urlParam(url, 'times', odfl.uniq());

        if (new Date(f_last_update_time_end).getTime() - new Date(f_last_update_time_start).getTime() > 90 * 60 * 60 * 24 * 1000) {
            Util.openDlg('alert', '搜索的操作开始和结束时间区间需在90天内，请修改查询参数');
            return;
        }


        $('#main_body').load(encodeURI(url), function () {
            initZclip();
            initShowDetail();
        });

    }

    function urlParam(url, name, value) {
        var newUrl = "", reg, tmp;
        url = url.split('#');
        name = name || '';
        value = value || '';
        if (!name) return '';
        reg = new RegExp("(^|)" + name + "=([^&]*)(|$)");
        tmp = name + "=" + value;
        if (url[0].match(reg) != null) {
            newUrl = url[0].replace(eval(reg), tmp);
        } else {
            if (url[0].match("[\?]")) {
                newUrl = url[0] + "&" + tmp;
            } else {
                newUrl = url[0] + "?" + tmp;
            }
        }
        url[0] = newUrl;
        newUrl = url.join('#');
        return newUrl;
    }

    odfl('#hide_btn').submit({modal: true});

});