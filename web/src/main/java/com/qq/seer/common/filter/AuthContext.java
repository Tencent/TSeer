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

import javax.servlet.http.HttpServletRequest;

/**
 * AuthContext provides thread-local access to the current authenticated user
 * identity information. It is populated by AuthFilter and consumed by
 * downstream modules (e.g., SeerAgentModule) to replace hardcoded admin
 * credentials.
 * 
 * Usage flow:
 * 1. AuthFilter resolves authenticated user from session/header
 * 2. AuthFilter stores user info in both request attributes and AuthContext thread-local
 * 3. Downstream code calls AuthContext.getCurrentUser() / getCurrentUserKey()
 * 4. AuthFilter cleans up thread-local after request processing
 */
public class AuthContext {

    /** Thread-local storage for current authenticated user. */
    private static final ThreadLocal<String> currentUser = new ThreadLocal<>();

    /** Thread-local storage for current authenticated user's operator key. */
    private static final ThreadLocal<String> currentUserKey = new ThreadLocal<>();

    /** Request attribute key for authenticated user (set by AuthFilter). */
    public static final String AUTH_USER_ATTR = "SEER_AUTH_USER";

    /** Request attribute key for admin role flag (set by AuthFilter). */
    public static final String AUTH_IS_ADMIN_ATTR = "SEER_AUTH_IS_ADMIN";

    /**
     * Get the current authenticated user's username.
     * @return username string, or null if not set
     */
    public static String getCurrentUser() {
        return currentUser.get();
    }

    /**
     * Get the current authenticated user's operator key.
     * In production, this should map to a real permission token or key derived
     * from the user's role and authorized scope.
     * @return operator key string, or null if not set
     */
    public static String getCurrentUserKey() {
        return currentUserKey.get();
    }

    /**
     * Set the current authenticated user info into thread-local context.
     * Called by AuthFilter after successful authentication.
     * @param user the authenticated username
     * @param userKey the authenticated user's operator key
     */
    public static void setCurrentUser(String user, String userKey) {
        currentUser.set(user);
        currentUserKey.set(userKey);
    }

    /**
     * Initialize AuthContext from request attributes (set by AuthFilter).
     * This provides a bridge between the servlet request and thread-local context
     * for code that does not have direct access to the HttpServletRequest.
     * @param request the HTTP request containing auth attributes
     */
    public static void initFromRequest(HttpServletRequest request) {
        Object user = request.getAttribute(AUTH_USER_ATTR);
        if (user != null) {
            currentUser.set(user.toString());
            // Derive userKey from the authenticated user identity
            // In production, this should be a proper key derived from role/permission system
            currentUserKey.set(user.toString());
        }
    }

    /**
     * Clear the current thread-local context.
     * Must be called after request processing to prevent memory leaks.
     */
    public static void clear() {
        currentUser.remove();
        currentUserKey.remove();
    }
}
