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

#include "Util.h"
using namespace std;
using namespace tars;

string urlEncode(const string &sIn)
{
    /**
    const char *str = sIn.c_str();
    const int strSize = sIn.size();
    string sOut;

    int i;
    char ch;

    for (i = 0; i < strSize; ++i)
    {
        ch = str[i];
        if (((ch >= 'A') && (ch <= 'Z')) ||
            ((ch >= 'a') && (ch <= 'z')) ||
            ((ch >= '0') && (ch <= '9')))
        {
            sOut += ch;
        }
        else if (ch == ' ')
        {
            sOut += '+';
        }
        else if (ch == '.' || ch == '-' || ch == '_' || ch == '*')
        {
            sOut += ch;
        }
        else
        {
            char result[4] = {'\0'};
            sprintf(result, "%%%02X", (unsigned char)ch);
            sOut += result;
        }
    }
    **/
    return sIn;
}

string getRequestUrl(const string &interfaceName, const string &params, const string &api)
{
    string url = api + "?interface_name=" + interfaceName + "&interface_params=" + params;
    return url;    
}

int doSyncRequest(const string &url, tars::TC_HttpResponse &response)
{
    TC_HttpRequest stHttpReq;
    stHttpReq.setCacheControl("no-cache");
    stHttpReq.setHeader("Content-Type", "application/x-www-form-urlencoded");
    stHttpReq.setGetRequest(url);

    int iRet = stHttpReq.doRequest(response, 30000);
    if (iRet != 0)
    {
        cout << FILE_FUN << "send request failed, ret:" << iRet << endl;
        return -1;
    }

    return 0;
}
