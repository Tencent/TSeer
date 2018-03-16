<link href="/css/base.css" rel="stylesheet" />
<script src="/global-libs/js/jquery.min.js" ></script>
<script type="text/javascript" src="/global-libs/odfl/odfl.js"></script>
<script src="/global-libs/cloudJs/js/cloudjs.js"></script>
<script src="/global-libs/js/json2.js"></script>
<style>
    body{
        padding-top: 60px !important;
    }
    #layout_top_bar {
        height: 50px;
        position: fixed;
        top: 0px;
        width: 100%;
        z-index: 9902;
        left: 0px;
        border-bottom: 1px solid #CCC;
    }
    #layout_top_bar .top_bar {
        background: #3F90C8;
        color: #FFF;
    }
    #layout_top_bar .top_bar .logo{
        font-size:26px;
        color:#FFF;
        padding: 20px;
        line-height:50px;
    }
    #layout_top_bar .top_bar .logo:hover{
        text-decoration:none;
    }
    #layout_top_bar .layout_nav_bar {
        float: right;
        line-height: 50px;
        font-size: 14px;
        margin-right: 5px;
    }
    #layout_top_bar .layout_nav_item {
        margin-right: 10px;
        color: #FFF;
        margin-left: 10px;
    }
    #layout_top_bar .layout_cut_line {
        font-size: 10px;
        color: #CCC;
    }
    #layout_left_bar a, #layout_top_bar a:hover{
        text-decoration:none;
        color:#FFF;
    }
</style>
<div id="layout_top_bar">
    <div class="top_bar">
        <a class="logo" href="/">Tseer管理平台</a>
        <!--
        <div class="layout_user_info">
            <span class="layout_user_name">3444340632</span><span class="layout_user_arrow"><img src="/images/select_down.png"></span>
            <a class="logout_btn" href="javascript:void(0);">退出</a>
        </div>
        -->
        <div class="layout_nav_bar">
            <a href="/router_manager/service_group_list.action" class="layout_nav_item">业务集列表</a><span class="layout_cut_line">|</span>
            <a href="/router_manager/agent_router_install_page.action" class="layout_nav_item">Agent安装工具</a><span class="layout_cut_line">|</span>
            <a href="/package/grayReleasedPage.action" class="layout_nav_item">Agent管理</a><span class="layout_cut_line">|</span>
            <a href="/package/manage.action" class="layout_nav_item">发布包管理</a>
        </div>
    </div>
</div>