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
