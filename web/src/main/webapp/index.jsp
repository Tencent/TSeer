<%@ page language="java" contentType="text/html; charset=UTF-8" pageEncoding="UTF-8"%>
<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<title>名字服务</title>
</head>
<body>
<jsp:include page="./top.jsp" flush="true"/>

<style type="text/css">

    html {overflow-x: initial !important;}:root { --bg-color: #ffffff; --text-color: #333333; --code-block-bg-color: inherit; }
    html { font-size: 14px; background-color: var(--bg-color); color: var(--text-color); font-family: "Helvetica Neue", Helvetica, Arial, sans-serif; -webkit-font-smoothing: antialiased; }
    body { margin: 0px; padding: 0px; height: auto; bottom: 0px; top: 0px; left: 0px; right: 0px; font-size: 1rem; line-height: 1.42857; overflow-x: hidden; background: inherit; }
    a:active, a:hover { outline: 0px; }
    .in-text-selection, ::selection { background: rgb(181, 214, 252); text-shadow: none; }
    #write { margin: 0px auto; height: auto; width: inherit; word-break: normal; word-wrap: break-word; position: relative; padding-bottom: 70px; white-space: pre-wrap; overflow-x: visible; }
    .for-image #write { padding-left: 8px; padding-right: 8px; }
    body.typora-export { padding-left: 30px; padding-right: 30px; }
    @media screen and (max-width: 500px) {
        body.typora-export { padding-left: 0px; padding-right: 0px; }
        .CodeMirror-sizer { margin-left: 0px !important; }
        .CodeMirror-gutters { display: none !important; }
    }
    .typora-export #write { margin: 0px auto; }
    #write > p:first-child, #write > ul:first-child, #write > ol:first-child, #write > pre:first-child, #write > blockquote:first-child, #write > div:first-child, #write > table:first-child { margin-top: 30px; }
    #write li > table:first-child { margin-top: -20px; }
    img { max-width: 100%; vertical-align: middle; }
    input, button, select, textarea { color: inherit; font-style: inherit; font-variant: inherit; font-weight: inherit; font-stretch: inherit; font-size: inherit; line-height: inherit; font-family: inherit; }
    input[type="checkbox"], input[type="radio"] { line-height: normal; padding: 0px; }
    ::before, ::after, * { box-sizing: border-box; }
    #write p, #write h1, #write h2, #write h3, #write h4, #write h5, #write h6, #write div, #write pre { width: inherit; }
    #write p, #write h1, #write h2, #write h3, #write h4, #write h5, #write h6 { position: relative; }
    h1 { font-size: 2rem; }
    h2 { font-size: 1.8rem; }
    h3 { font-size: 1.6rem; }
    h4 { font-size: 1.4rem; }
    h5 { font-size: 1.2rem; }
    h6 { font-size: 1rem; }
    p { -webkit-margin-before: 1rem; -webkit-margin-after: 1rem; -webkit-margin-start: 0px; -webkit-margin-end: 0px; }
    .typora-export p { white-space: normal; }
    .mathjax-block { margin-top: 0px; margin-bottom: 0px; -webkit-margin-before: 0rem; -webkit-margin-after: 0rem; }
    .hidden { display: none; }
    .md-blockmeta { color: rgb(204, 204, 204); font-weight: bold; font-style: italic; }
    a { cursor: pointer; }
    sup.md-footnote { padding: 2px 4px; background-color: rgba(238, 238, 238, 0.7); color: rgb(85, 85, 85); border-radius: 4px; }
    #write input[type="checkbox"] { cursor: pointer; width: inherit; height: inherit; margin: 4px 0px 0px; }
    tr { break-inside: avoid; break-after: auto; }
    thead { display: table-header-group; }
    table { border-collapse: collapse; border-spacing: 0px; width: 100%; overflow: auto; break-inside: auto; text-align: left; }
    table.md-table td { min-width: 80px; }
    .CodeMirror-gutters { border-right: 0px; background-color: inherit; }
    .CodeMirror { text-align: left; }
    .CodeMirror-placeholder { opacity: 0.3; }
    .CodeMirror pre { padding: 0px 4px; }
    .CodeMirror-lines { padding: 0px; }
    div.hr:focus { cursor: none; }
    pre { white-space: pre-wrap; }
    .CodeMirror-gutters { margin-right: 4px; }
    .md-fences { font-size: 0.9rem; display: block; break-inside: avoid; text-align: left; overflow: visible; white-space: pre; background: var(--code-block-bg-color); position: relative !important; }
    .md-diagram-panel { width: 100%; margin-top: 10px; text-align: center; padding-top: 0px; padding-bottom: 8px; overflow-x: auto; }
    .md-fences .CodeMirror.CodeMirror-wrap { top: -1.6em; margin-bottom: -1.6em; }
    .md-fences.mock-cm { white-space: pre-wrap; }
    .show-fences-line-number .md-fences { padding-left: 0px; }
    .show-fences-line-number .md-fences.mock-cm { padding-left: 40px; }
    .footnotes { opacity: 0.8; font-size: 0.9rem; padding-top: 1em; padding-bottom: 1em; }
    .footnotes + .footnotes { margin-top: -1em; }
    .md-reset { margin: 0px; padding: 0px; border: 0px; outline: 0px; vertical-align: top; background: transparent; text-decoration: none; text-shadow: none; float: none; position: static; width: auto; height: auto; white-space: nowrap; cursor: inherit; -webkit-tap-highlight-color: transparent; line-height: normal; font-weight: normal; text-align: left; box-sizing: content-box; direction: ltr; }
    li div { padding-top: 0px; }
    blockquote { margin: 1rem 0px; }
    li p, li .mathjax-block { margin: 0.5rem 0px; }
    li { margin: 0px; position: relative; }
    blockquote > :last-child { margin-bottom: 0px; }
    blockquote > :first-child { margin-top: 0px; }
    .footnotes-area { color: rgb(136, 136, 136); margin-top: 0.714rem; padding-bottom: 0.143rem; }
    @media print {
        html, body { border: 1px solid transparent; height: 99%; break-after: avoid; break-before: avoid; }
        .typora-export * { -webkit-print-color-adjust: exact; }
        h1, h2, h3, h4, h5, h6 { break-after: avoid-page; orphans: 2; }
        p { orphans: 4; }
        html.blink-to-pdf { font-size: 13px; }
        .typora-export #write { padding-left: 1cm; padding-right: 1cm; padding-bottom: 0px; break-after: avoid; }
        .typora-export #write::after { height: 0px; }
        @page { margin: 20mm 0mm; }
    }
    .footnote-line { margin-top: 0.714em; font-size: 0.7em; }
    a img, img a { cursor: pointer; }
    pre.md-meta-block { font-size: 0.8rem; min-height: 2.86rem; white-space: pre-wrap; background: rgb(204, 204, 204); display: block; overflow-x: hidden; }
    p .md-image:only-child { display: inline-block; width: 100%; text-align: center; }
    #write .MathJax_Display { margin: 0.8em 0px 0px; }
    .mathjax-block { white-space: pre; overflow: hidden; width: 100%; }
    p + .mathjax-block { margin-top: -1.143rem; }
    .mathjax-block:not(:empty)::after { display: none; }
    [contenteditable="true"]:active, [contenteditable="true"]:focus { outline: none; box-shadow: none; }
    .task-list { list-style-type: none; }
    .task-list-item { position: relative; padding-left: 1em; }
    .task-list-item input { position: absolute; top: 0px; left: 0px; }
    .math { font-size: 1rem; }
    .md-toc { min-height: 3.58rem; position: relative; font-size: 0.9rem; border-radius: 10px; }
    .md-toc-content { position: relative; margin-left: 0px; }
    .md-toc::after, .md-toc-content::after { display: none; }
    .md-toc-item { display: block; color: rgb(65, 131, 196); text-decoration: none; }
    .md-toc-inner:hover { }
    .md-toc-inner { display: inline-block; cursor: pointer; }
    .md-toc-h1 .md-toc-inner { margin-left: 0px; font-weight: bold; }
    .md-toc-h2 .md-toc-inner { margin-left: 2em; }
    .md-toc-h3 .md-toc-inner { margin-left: 4em; }
    .md-toc-h4 .md-toc-inner { margin-left: 6em; }
    .md-toc-h5 .md-toc-inner { margin-left: 8em; }
    .md-toc-h6 .md-toc-inner { margin-left: 10em; }
    @media screen and (max-width: 48em) {
        .md-toc-h3 .md-toc-inner { margin-left: 3.5em; }
        .md-toc-h4 .md-toc-inner { margin-left: 5em; }
        .md-toc-h5 .md-toc-inner { margin-left: 6.5em; }
        .md-toc-h6 .md-toc-inner { margin-left: 8em; }
    }
    a.md-toc-inner { font-size: inherit; font-style: inherit; font-weight: inherit; line-height: inherit; }
    .footnote-line a:not(.reversefootnote) { color: inherit; }
    .md-attr { display: none; }
    .md-fn-count::after { content: "."; }
    .md-tag { opacity: 0.5; }
    .md-comment { color: rgb(162, 127, 3); opacity: 0.8; font-family: monospace; }
    code { text-align: left; }
    h1 .md-tag, h2 .md-tag, h3 .md-tag, h4 .md-tag, h5 .md-tag, h6 .md-tag { font-weight: initial; opacity: 0.35; }
    a.md-print-anchor { border-width: initial !important; border-style: none !important; border-color: initial !important; display: inline-block !important; position: absolute !important; width: 1px !important; right: 0px !important; outline: none !important; background: transparent !important; text-decoration: initial !important; text-shadow: initial !important; }
    .md-inline-math .MathJax_SVG .noError { display: none !important; }
    .mathjax-block .MathJax_SVG_Display { text-align: center; margin: 1em 0em; position: relative; text-indent: 0px; max-width: none; max-height: none; min-height: 0px; min-width: 100%; width: auto; display: block !important; }
    .MathJax_SVG_Display, .md-inline-math .MathJax_SVG_Display { width: auto; margin: inherit; display: inline-block !important; }
    .MathJax_SVG .MJX-monospace { font-family: monospace; }
    .MathJax_SVG .MJX-sans-serif { font-family: sans-serif; }
    .MathJax_SVG { display: inline; font-style: normal; font-weight: normal; line-height: normal; zoom: 90%; text-indent: 0px; text-align: left; text-transform: none; letter-spacing: normal; word-spacing: normal; word-wrap: normal; white-space: nowrap; float: none; direction: ltr; max-width: none; max-height: none; min-width: 0px; min-height: 0px; border: 0px; padding: 0px; margin: 0px; }
    .MathJax_SVG * { transition: none; }
    .md-diagram-panel > svg { max-width: 100%; }
    [lang="flow"] svg, [lang="mermaid"] svg { max-width: 100%; }


    :root { --side-bar-bg-color: #fafafa; --control-text-color: #777; }
    @font-face { font-family: "Open Sans"; font-style: normal; font-weight: normal; src: local("Open Sans Regular"), url("./github/400.woff") format("woff"); }
    @font-face { font-family: "Open Sans"; font-style: italic; font-weight: normal; src: local("Open Sans Italic"), url("./github/400i.woff") format("woff"); }
    @font-face { font-family: "Open Sans"; font-style: normal; font-weight: bold; src: local("Open Sans Bold"), url("./github/700.woff") format("woff"); }
    @font-face { font-family: "Open Sans"; font-style: italic; font-weight: bold; src: local("Open Sans Bold Italic"), url("./github/700i.woff") format("woff"); }
    html { font-size: 16px; }
    body { font-family: "Open Sans", "Clear Sans", "Helvetica Neue", Helvetica, Arial, sans-serif; color: rgb(51, 51, 51); line-height: 1.6; }
    #write { max-width: 860px; margin: 0px auto; padding: 20px 30px 100px; }
    #write > ul:first-child, #write > ol:first-child { margin-top: 30px; }
    body > :first-child { margin-top: 0px !important; }
    body > :last-child { margin-bottom: 0px !important; }
    a { color: rgb(65, 131, 196); }
    h1, h2, h3, h4, h5, h6 { position: relative; margin-top: 1rem; margin-bottom: 1rem; font-weight: bold; line-height: 1.4; cursor: text; }
    h1:hover a.anchor, h2:hover a.anchor, h3:hover a.anchor, h4:hover a.anchor, h5:hover a.anchor, h6:hover a.anchor { text-decoration: none; }
    h1 tt, h1 code { font-size: inherit; }
    h2 tt, h2 code { font-size: inherit; }
    h3 tt, h3 code { font-size: inherit; }
    h4 tt, h4 code { font-size: inherit; }
    h5 tt, h5 code { font-size: inherit; }
    h6 tt, h6 code { font-size: inherit; }
    h1 { padding-bottom: 0.3em; font-size: 2.25em; line-height: 1.2; border-bottom: 1px solid rgb(238, 238, 238); }
    h2 { padding-bottom: 0.3em; font-size: 1.75em; line-height: 1.225; border-bottom: 1px solid rgb(238, 238, 238); }
    h3 { font-size: 1.5em; line-height: 1.43; }
    h4 { font-size: 1.25em; }
    h5 { font-size: 1em; }
    h6 { font-size: 1em; color: rgb(119, 119, 119); }
    p, blockquote, ul, ol, dl, table { margin: 0.8em 0px; }
    li > ol, li > ul { margin: 0px; }
    hr { height: 4px; padding: 0px; margin: 16px 0px; background-color: rgb(231, 231, 231); border-width: 0px 0px 1px; border-style: none none solid; border-top-color: initial; border-right-color: initial; border-left-color: initial; border-image: initial; overflow: hidden; box-sizing: content-box; border-bottom-color: rgb(221, 221, 221); }
    body > h2:first-child { margin-top: 0px; padding-top: 0px; }
    body > h1:first-child { margin-top: 0px; padding-top: 0px; }
    body > h1:first-child + h2 { margin-top: 0px; padding-top: 0px; }
    body > h3:first-child, body > h4:first-child, body > h5:first-child, body > h6:first-child { margin-top: 0px; padding-top: 0px; }
    a:first-child h1, a:first-child h2, a:first-child h3, a:first-child h4, a:first-child h5, a:first-child h6 { margin-top: 0px; padding-top: 0px; }
    h1 p, h2 p, h3 p, h4 p, h5 p, h6 p { margin-top: 0px; }
    li p.first { display: inline-block; }
    ul, ol { padding-left: 30px; }
    ul:first-child, ol:first-child { margin-top: 0px; }
    ul:last-child, ol:last-child { margin-bottom: 0px; }
    blockquote { border-left: 4px solid rgb(221, 221, 221); padding: 0px 15px; color: rgb(119, 119, 119); }
    blockquote blockquote { padding-right: 0px; }
    table { padding: 0px; word-break: initial; }
    table tr { border-top: 1px solid rgb(204, 204, 204); margin: 0px; padding: 0px; }
    table tr:nth-child(2n) { background-color: rgb(248, 248, 248); }
    table tr th { font-weight: bold; border: 1px solid rgb(204, 204, 204); text-align: left; margin: 0px; padding: 6px 13px; }
    table tr td { border: 1px solid rgb(204, 204, 204); text-align: left; margin: 0px; padding: 6px 13px; }
    table tr th:first-child, table tr td:first-child { margin-top: 0px; }
    table tr th:last-child, table tr td:last-child { margin-bottom: 0px; }
    .CodeMirror-gutters { border-right: 1px solid rgb(221, 221, 221); }
    .md-fences, code, tt { border: 1px solid rgb(221, 221, 221); background-color: rgb(248, 248, 248); border-radius: 3px; font-family: Consolas, "Liberation Mono", Courier, monospace; padding: 2px 4px 0px; font-size: 0.9em; }
    .md-fences { margin-bottom: 15px; margin-top: 15px; padding: 8px 1em 6px; }
    .task-list { padding-left: 0px; }
    .task-list-item { padding-left: 32px; }
    .task-list-item input { top: 3px; left: 8px; }
    @media screen and (min-width: 914px) {
    }
    @media print {
        html { font-size: 13px; }
        table, pre { break-inside: avoid; }
        pre { word-wrap: break-word; }
    }
    .md-fences { background-color: rgb(248, 248, 248); }
    #write pre.md-meta-block { padding: 1rem; font-size: 85%; line-height: 1.45; background-color: rgb(247, 247, 247); border: 0px; border-radius: 3px; color: rgb(119, 119, 119); margin-top: 0px !important; }
    .mathjax-block > .code-tooltip { bottom: 0.375rem; }
    #write > h3.md-focus::before { left: -1.5625rem; top: 0.375rem; }
    #write > h4.md-focus::before { left: -1.5625rem; top: 0.285714rem; }
    #write > h5.md-focus::before { left: -1.5625rem; top: 0.285714rem; }
    #write > h6.md-focus::before { left: -1.5625rem; top: 0.285714rem; }
    .md-image > .md-meta { border: 1px solid rgb(221, 221, 221); border-radius: 3px; font-family: Consolas, "Liberation Mono", Courier, monospace; padding: 2px 4px 0px; font-size: 0.9em; color: inherit; }
    .md-tag { color: inherit; }
    .md-toc { margin-top: 20px; padding-bottom: 20px; }
    .sidebar-tabs { border-bottom: none; }
    #typora-quick-open { border: 1px solid rgb(221, 221, 221); background-color: rgb(248, 248, 248); }
    #typora-quick-open-item { background-color: rgb(250, 250, 250); border-color: rgb(254, 254, 254) rgb(229, 229, 229) rgb(229, 229, 229) rgb(238, 238, 238); border-style: solid; border-width: 1px; }
    #md-notification::before { top: 10px; }
    .on-focus-mode blockquote { border-left-color: rgba(85, 85, 85, 0.12); }
    header, .context-menu, .megamenu-content, footer { font-family: "Segoe UI", Arial, sans-serif; }
    .file-node-content:hover .file-node-icon, .file-node-content:hover .file-node-open-state { visibility: visible; }
    .mac-seamless-mode #typora-sidebar { background-color: var(--side-bar-bg-color); }
    .md-lang { color: rgb(180, 101, 77); }

</style>

<style type="text/css">
    body ul li{
        list-style: inherit;
    }
</style>

<div  id='write'  class = 'is-node'><h1><a name='header-n0' class='md-header-anchor '></a>Tseer产品介绍</h1><div class='md-toc' mdtype='toc'><p class="md-toc-content"><span class="md-toc-item md-toc-h1" data-ref="n0"><a class="md-toc-inner" href="#header-n0">Tseer产品介绍</a></span><span class="md-toc-item md-toc-h2" data-ref="n3"><a class="md-toc-inner" href="#header-n3">一.Tseer简介</a></span><span class="md-toc-item md-toc-h2" data-ref="n14"><a class="md-toc-inner" href="#header-n14">二.研发背景</a></span><span class="md-toc-item md-toc-h2" data-ref="n23"><a class="md-toc-inner" href="#header-n23">三.Tseer架构</a></span><span class="md-toc-item md-toc-h2" data-ref="n49"><a class="md-toc-inner" href="#header-n49">四.Tseer功能的特点</a></span><span class="md-toc-item md-toc-h3" data-ref="n50"><a class="md-toc-inner" href="#header-n50">1.负载均衡</a></span><span class="md-toc-item md-toc-h3" data-ref="n70"><a class="md-toc-inner" href="#header-n70">2.故障容错</a></span><span class="md-toc-item md-toc-h3" data-ref="n88"><a class="md-toc-inner" href="#header-n88">3.调用优化</a></span><span class="md-toc-item md-toc-h3" data-ref="n122"><a class="md-toc-inner" href="#header-n122">4.两种接入方式</a></span></p></div><h2><a name='header-n3' class='md-header-anchor '></a>一.Tseer简介</h2><p></p><p>Tseer是一套解决多框架服务集群之间服务发现的工具，基于名字进行路由实现，性能优越，接入友好，在腾讯内部被广泛采用，目前日均承载百亿级别的请求量。</p><p>在服务发现的核心功能之上，Tseer还支持多种优秀的负载均衡算法，提供可靠的故障容错策略。针对发展迅速的海量服务，还支持就近接入，按SET逻辑分组，全量调度三种不同的路由策略。实现了高度智能的调度优化，有效解决了业务跨地区跨机房调用等业界难题，最大化保证了服务的可用，对业务质量有显著提升。</p><p>Tseer拥有web管理界面和API接入两种方式可供用户根据需求自由选择，通过代理节点和代理服务器机制为需要频繁发布变更的业务提供透明的服务发现功能，学习成本很低，操作也很方便，对于业务维护人员十分友好。</p><p>同时Tseer轻巧灵便，以旁路模式工作，对业务的侵入性突破业内新低，无论新业务还是已经上线的业务都十分适用，极其友好。是微服务框架和分布式服务集群实现服务发现的优秀解决方案。</p><h2><a name='header-n14' class='md-header-anchor '></a>二.研发背景</h2><p>在传统的单体式应用中，变更发布相对较少，系统中的网络位置也很少变化，偶尔的变更也可以通过手动更改配置的方式来应对。但是在当前海量服务的大环境下，这种架构已经无法高效稳定的支撑高速增长的业务。越来越庞大的分布式服务集群和微服务框架已经逐渐成为主流。</p><p>但是新型架构为业务提供更好支撑的同时，频繁的发布更新与动态伸缩也导致了网络位置的频繁变化，在这种情况下业务维护人员手动更改配置这种大规模重复性工作不仅增大了出错的风险，其低效也会限制业务的高速发展。往往配置还没改完，新的变更就需要发布了。所以就必须要一个自动化的服务发现工具来解决这些问题。</p><p>然而这些也并不是问题的全部。在保证访问成功的前提下，响应时间作为服务质量中最重要的指标，是影响业务发展最关键的一环。多业务集之间复杂的调用关系再加上跨地区跨网络调用等其他因素，响应时间达不到预期是持续困扰整个业务发展周期的棘手问题。与此同时，无论是采用物理机还是虚拟机，节点挂掉导致的不可用时有发生，如何有效容错也是亟待解决的问题。</p><p>基于这些问题，我们开发出了Tseer。</p><h2><a name='header-n23' class='md-header-anchor '></a>三.Tseer架构</h2><p><img src='/images/seer_framework.png' alt='tseer' /></p><p>整个Tseer的结构分为三部分：Tseerserver、业务客户端（主调）、业务服务端（被调）。</p><ul><li><p>Tseerserver</p><p>Tseerserver是整个Tseer的枢纽与核心模块。
    当新节点上线时，需要先通过WEB管理平台在Tseer服务集群注册，将其网络位置信息记录在Tseer系统中。当需要对节点进行下线或者其他修改时，也需要在WEB管理平台就行相关操作。被调节点也会定时上报心跳给Tseerserver，server端会屏蔽心跳超时的节点使其无法被调用。</p></li></ul><ul><li><p>业务客户端</p><p>业务客户端是需要调用其他服务的节点，称之为主调，是服务发现功能的使用者。
    Tseer为业务客户端提供了：安装Agent与API调用两种方式来从Tseerserver获得需要调用的服务（被调）的地址来完成调用。</p></li></ul><ul><li><p>业务服务端</p><p>业务服务端是需要被调用的节点，称之为被调，是服务的提供者。
    当新节点上线时，被调需要在Tseerserver注册。不论同一个被调服务集群有多少个节点，注册时该服务集群都需要注册一个统一的名字。主调在调用逻辑中只需要写明需要调用的服务的名字，Tseer会根据被调名字来返回被调地址。当被调需要扩容时，只需要把新节点加在该服务对应的名字下面即可。业务人员无需管理被调集群下繁多的服务节点信息，十分方便。</p></li></ul><h2><a name='header-n49' class='md-header-anchor '></a>四.Tseer功能的特点</h2><h3><a name='header-n50' class='md-header-anchor '></a>1.负载均衡</h3><p>当同一业务集群中某些节点被频繁调用而另一些节点没有承担合理的负载时，不仅业务的服务质量和响应时间会大幅下降，同时也会造成资源的浪费。</p><p>Tseer系统中，当主调发起调用时，会针对被调名字下所有可用节点为调用提供四种负载均衡方式来保障各个节点的合理负载，分别是</p><ul><li>轮询</li><li>随机</li><li>静态权重</li><li>一致性哈希</li></ul><p>用户还可以使用调用分组的方式来自定义负载均衡实现，调用分组会在下文中提到。</p><h3><a name='header-n70' class='md-header-anchor '></a>2.故障容错</h3><p>为了解决节点故障导致的业务不可用与服务质量降低，Tseer还提供了可靠的故障容错机制。</p><p>当主调进行一次调用之后，会将调用结果上报。如果调用失败Tseer会暂时将该节点屏蔽来避免故障节点被反复调用，Tseer会定时探测被屏蔽的节点，当发现故障节点恢复服务时，会重新将其激活。</p><blockquote><p>对于任意被调节点，满足下列条件之一则屏蔽该节点：</p><p>1.在一个检测周期（60秒）内调用失败次数达到2次，且调用错误数占总调用次数的50%以上</p><p>2.在5秒内连续调用失败5次以上</p></blockquote><p>对于被屏蔽的节点Tseer Agent/Api将每隔30秒对已屏蔽的节点进行重试。</p><p>同时当Tseer故障时，主调也能根据缓存信息继续调用。</p><p></p><h3><a name='header-n88' class='md-header-anchor '></a>3.调用优化</h3><p>Tseer为调用逻辑提供IDC分组、Set分组、All三种方式来解决跨地区调用等问题。</p><ul><li><p>All</p><p>为主调提供所有可用被调节点地址</p></li><li><p>IDC分组</p></li></ul><p> IDC分组可以近似的看作就近接入。</p><p>该方法按照两个层次进行划分。第一个是物理小组，是最小的组调度单位，即按照节点所在的机房或者区域分配统 一的组名。第二个是物理小组组成的逻辑组，可以理解为按照更大的区域来划分的统一的组名。</p><p>针对IDC的逻辑分组，Tseer还定义了调用优先级策略。即部分逻辑组不可用时，会按照优先级策略返回可用被调节点地址列表。</p><ul><li>Set分组</li></ul><p>IDC分组主要是在区域概念上去划分分组，实现就近访问策略，在后台服务架构中，业务规模达到一定数量时，如果要对某几个服务节点实现根据容量、灰度，分区域管理的隔离控制，IDC分组是无法满足的，而Set分组则是对IDC分组的再细化。</p><p>Set分组的命名规则为： Set名.Set地区.Set组。其中Set组是最小区分单元的名称，支持通配符*，表示Set地区下的所有分组。比如0,1,2,3,4或者a,b,c,d。</p><p>Set分组的调用逻辑如下：</p><p>1.主调（客户端）和被调（服务端）都启用了Set分组，并且Set名要一致才认为是启用同SET内</p><p>2.启用Set分组的主调和被调只能访问同Set内的节点</p><p>3.主调启用Set分组，被调没有启用Set分组，则默认会走按IDC分组查询的逻辑（前提是启用了IDC分组）</p><h3><a name='header-n122' class='md-header-anchor '></a>4.两种接入方式</h3><p>根据服务客户端是否在其物理机中部署Tseer Agent，Tseer的使用方式可以分为Agent 和Tseer API两种方式：</p><ul><li>Agent 方式</li></ul><p><strong>名字路由</strong></p><p>Agent 方式下，Tseer Agent会定期缓存被调方的信息。并根据调用方指定的负载均衡策略将被调节点信息返给调用方。如果调用方希望通过服务特性来实现负载均衡，Tseer也支持按照调用方指定的分组策略将被调的组信息返给调用方。</p><p><strong>数据上报</strong></p><p>每次调用完成后，调用方需要调用Tseer Api提供的上报接口上报调用信息，调用信息将由Tseer Api即使上报至Tseer Agent。Tseer Agent将根据调用信息剔除失效被调节点。</p><p><strong>容错</strong>
    使用Agent 方式时，如果Tseer Agent失效，Tseer Api将会从内存中返回已访问过的节点给主调，如果Tseer Api缓存失效，此时Tseer Api将会从本地磁盘中的缓存文件恢复缓存信息提供给主调。需要注意的是此时Tseer Api提供给主调服务的信息为有损信息，Tseer Api不保证节点一定健康。</p><ul><li>Tseer Api方式</li></ul><p><strong>名字路由</strong></p><p>Agent 方式与Tseer Api方式的区别在于是否需要在主调的宿主机中部署Tseer Agent。Tseer Api会直接访问Tseer server。并且被调方信息缓存、负载均衡以及失效节点剔除都在Tseer Api中完成。</p><p>Tseer Api会定时拉取Tseerserver的后端信息并屏蔽不可用的被调节点。</p><p><strong>容错</strong></p><p>Tseerserver故障时，Tseer Api会将内存中缓存的信息返回给主调。当内存缓存不可用时，Tseer Api将会用本地磁盘中的缓存恢复内存缓存。</p><p></p><p><strong>Agent Api方式 与Tseer Api方式对比</strong></p><p></p><p><img src='/images/seer_index_table.png' alt='tseer' /></p><p></p><p></p><p></p><p></p><p></p><p></p><p></p><p></p><p></p><p></p><p></p></div>

<script>
    $('.md-toc-inner').click(function(e){
        e.preventDefault();
        var href = $(this).attr('href');
        if(href.indexOf('#') === 0){
            var name = href.substring(1);
            var top = $('[name="'+name+'"]').offset().top;
            $(window).scrollTop(top - 60);
        }
    });
</script>
</body>
</html>

