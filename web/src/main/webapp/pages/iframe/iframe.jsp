<%@page language="java" contentType="text/html; charset=UTF-8" pageEncoding="UTF-8"%>
<%
    String js_name = request.getParameter("js_name");
%>
<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
<title>跨域js调用</title>
<script src="/global-libs/js/jquery.min.js" ></script>
<script type="text/javascript" src="/global-libs/odfl/odfl.js"></script>
<%
    if(js_name != null && js_name.length() > 0 ){
%>
        <script type="text/javascript" src="js/<%=js_name%>.js?{WFE_TIMESTAMP}"></script>
<%
    }
%>
</head>
<body>
</body>
</html>