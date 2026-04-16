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

package com.qq.seer.common.filter;

import java.io.IOException;

import javax.servlet.Filter;
import javax.servlet.FilterChain;
import javax.servlet.FilterConfig;
import javax.servlet.ServletException;
import javax.servlet.ServletRequest;
import javax.servlet.ServletResponse;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.json.JSONObject;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * AuthFilter provides unified authentication and authorization for management
 * routes (/interface and *.action). Anonymous requests are rejected by default.
 * 
 * Management actions (Agent management, package management, installation tools,
 * service group management) require authenticated admin role.
 */
public class AuthFilter implements Filter {

    private static final Logger log = LoggerFactory.getLogger(AuthFilter.class);

    /** HTTP header name for authenticated user identity. */
    private static final String AUTH_USER_HEADER = "X-Auth-User";

    /** Session attribute key for storing authenticated user. */
    private static final String AUTH_USER_SESSION_ATTR = "SEER_AUTH_USER";

    /** Session attribute key for storing user admin role flag. */
    private static final String AUTH_IS_ADMIN_SESSION_ATTR = "SEER_AUTH_IS_ADMIN";

    /** Admin role identifier. */
    private static final String ADMIN_ROLE = "admin";

    @Override
    public void init(FilterConfig filterConfig) throws ServletException {
        log.info("AuthFilter initialized");
    }

    @Override
    public void doFilter(ServletRequest request, ServletResponse response, FilterChain chain)
            throws IOException, ServletException {
        HttpServletRequest httpRequest = (HttpServletRequest) request;
        HttpServletResponse httpResponse = (HttpServletResponse) response;

        String requestURI = httpRequest.getRequestURI();
        String contextPath = httpRequest.getContextPath();
        if (contextPath != null && requestURI.startsWith(contextPath)) {
            requestURI = requestURI.substring(contextPath.length());
        }

        // Resolve authenticated user from session or auth header
        String authUser = resolveAuthenticatedUser(httpRequest);
        boolean isAdmin = checkAdminRole(httpRequest, authUser);

        // Reject unauthenticated requests
        if (authUser == null || authUser.isEmpty()) {
            log.warn("Anonymous access rejected for URI: {}", requestURI);
            sendAuthError(httpResponse, "Authentication required. Please login first.");
            return;
        }

        // For management actions requiring admin privilege, check role
        if (isAdminAction(requestURI, httpRequest) && !isAdmin) {
            log.warn("Non-admin user [{}] access denied for admin URI: {}", authUser, requestURI);
            sendForbiddenError(httpResponse, "Admin privilege required for this operation.");
            return;
        }

        // Store authenticated user info in request attributes for downstream use
        httpRequest.setAttribute(AUTH_USER_ATTR, authUser);
        httpRequest.setAttribute(AUTH_IS_ADMIN_ATTR, isAdmin);

        // Initialize AuthContext thread-local for downstream modules
        // (e.g., SeerAgentModule) to access current user identity
        AuthContext.setCurrentUser(authUser, authUser);

        try {
            chain.doFilter(request, response);
        } finally {
            // Clean up thread-local to prevent memory leaks
            AuthContext.clear();
        }
    }

    @Override
    public void destroy() {
        log.info("AuthFilter destroyed");
    }

    /**
     * Resolve the authenticated user identity from session or auth header.
     * In production, this should be replaced with a real authentication mechanism
     * (e.g., OAuth2, SSO, Kerberos, etc.).
     */
    private String resolveAuthenticatedUser(HttpServletRequest request) {
        // First check session for authenticated user
        Object sessionUser = request.getSession(false) != null
                ? request.getSession(false).getAttribute(AUTH_USER_SESSION_ATTR)
                : null;
        if (sessionUser != null && !sessionUser.toString().isEmpty()) {
            return sessionUser.toString();
        }

        // Fallback: check auth header (e.g., set by reverse proxy / SSO gateway)
        String headerUser = request.getHeader(AUTH_USER_HEADER);
        if (headerUser != null && !headerUser.isEmpty()) {
            return headerUser;
        }

        // No authenticated user found
        return null;
    }

    /**
     * Check if the given user has admin role.
     * In production, this should be replaced with a real role/permission system.
     */
    private boolean checkAdminRole(HttpServletRequest request, String authUser) {
        if (authUser == null || authUser.isEmpty()) {
            return false;
        }
        // Check session for admin flag
        Object isAdminObj = request.getSession(false) != null
                ? request.getSession(false).getAttribute(AUTH_IS_ADMIN_SESSION_ATTR)
                : null;
        if (isAdminObj != null) {
            return Boolean.TRUE.equals(isAdminObj) || ADMIN_ROLE.equals(isAdminObj.toString());
        }
        // Default: non-admin unless explicitly set
        return false;
    }

    /**
     * Determine if the requested URI and parameters indicate an admin-level action.
     * These include: Agent deletion, gray release management, package management,
     * service group management, and installation tools.
     * 
     * Per security audit: explicit authorization required for:
     * - delete_agent / routerapi_update_agent_locator / release.action
     * - add_service_group / add_ip_port / del_ip_port / update_ip_port
     */
    private boolean isAdminAction(String requestURI, HttpServletRequest request) {
        // /interface endpoint: check interface_name parameter for specific admin actions
        if (requestURI.equals("/interface")) {
            String interfaceName = request.getParameter("interface_name");
            if (interfaceName != null) {
                // Explicit authorization for sensitive interface actions
                if (interfaceName.equals("deleteagent")
                        || interfaceName.equals("routerapi_update_agent_locator")
                        || interfaceName.equals("addservicegroup")
                        || interfaceName.equals("addipport")
                        || interfaceName.equals("delipport")
                        || interfaceName.equals("updateipport")
                        || interfaceName.equals("release")
                        || interfaceName.equals("updateagentpackageinfo")
                        || interfaceName.equals("deleteagentpackage")
                        || interfaceName.equals("updateagentgraystate")
                        || interfaceName.equals("addserver")
                        || interfaceName.equals("delserver")
                        || interfaceName.equals("updateserver")
                        || interfaceName.equals("updateagentlocator")
                        || interfaceName.equals("getservicegroup")) {
                    return true;
                }
            }
        }

        // *.action routes: specific admin actions
        // Agent management
        if (requestURI.equals("/package/grayReleasedPage.action")
                || requestURI.equals("/package/incGrayReleasedPage.action")) {
            return true;
        }
        // Package management
        if (requestURI.equals("/package/manage.action")
                || requestURI.equals("/package/uploadpage.action")
                || requestURI.equals("/package/updatePackage.action")
                || requestURI.equals("/package/delPackage.action")
                || requestURI.equals("/package/release.action")) {
            return true;
        }
        // Installation tools
        if (requestURI.equals("/router_manager/agent_router_install_page.action")
                || requestURI.equals("/router_manager/agent_router_install_command.action")) {
            return true;
        }
        // Service group management
        if (requestURI.equals("/router_manager/service_group_list.action")
                || requestURI.equals("/router_manager/ip_port_list.action")) {
            return true;
        }

        return false;
    }

    /**
     * Send an authentication error response (401) in JSON format.
     */
    private void sendAuthError(HttpServletResponse response, String message) throws IOException {
        response.setStatus(HttpServletResponse.SC_UNAUTHORIZED);
        response.setContentType("application/json; charset=UTF-8");
        JSONObject jsonObj = new JSONObject();
        jsonObj.put("ret_code", 401);
        jsonObj.put("err_msg", message);
        jsonObj.put("data", new JSONObject());
        response.getWriter().write(jsonObj.toString());
    }

    /**
     * Send a forbidden error response (403) in JSON format.
     */
    private void sendForbiddenError(HttpServletResponse response, String message) throws IOException {
        response.setStatus(HttpServletResponse.SC_FORBIDDEN);
        response.setContentType("application/json; charset=UTF-8");
        JSONObject jsonObj = new JSONObject();
        jsonObj.put("ret_code", 403);
        jsonObj.put("err_msg", message);
        jsonObj.put("data", new JSONObject());
        response.getWriter().write(jsonObj.toString());
    }
}
