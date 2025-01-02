/**
 * Copyright (c) 2020 ~ 2024 KylinSec Co., Ltd.
 * kiran-session-guard is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     liuxinhao <liuxinhao@kylinsec.com.cn>
 */
#include "user-utils.h"
#include <qt5-log-i.h>
#include <unistd.h>
#include <pwd.h>

namespace UserUtils
{
QString getCurrentUser()
{
    uid_t uid = getuid();
    KLOG_INFO() << "current uid:" << uid;

    long bufSize = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (bufSize == -1)
    {
        KLOG_WARNING() << "autodetect getpw_r bufsize failed.";
        return QString("");
    }

    std::vector<char> buf(bufSize);
    struct passwd pwd;
    struct passwd *pResult = nullptr;
    int iRet = 0;

    do
    {
        iRet = getpwuid_r(uid, &pwd, &buf[0], bufSize, &pResult);
        if (iRet == ERANGE)
        {
            bufSize *= 2;
            buf.resize(bufSize);
        }
    } while ((iRet == EINTR) || (iRet == ERANGE));

    if (iRet != 0)
    {
        KLOG_ERROR() << "getpwuid_r failed,error: [" << iRet << "]" << strerror(iRet);
        return QString("");
    }

    if (pResult == nullptr)
    {
        KLOG_ERROR() << "getpwuid_r no matching password record was found";
        return QString("");
    }

    return pResult->pw_name;
}
QString getUserFullName(const QString &name)
{
    struct passwd pwdBuffer;
    struct passwd* resultPwd = nullptr;
    std::vector<char> bufferArray;
    
    auto bufferSize = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (bufferSize == -1)
    {
        bufferSize = 16384;
    }
    bufferArray.resize(bufferSize);

    auto iRes = getpwnam_r(name.toStdString().c_str(), &pwdBuffer,
                           &bufferArray[0], bufferArray.size(),
                           &resultPwd);
    if (iRes != 0 ||
        resultPwd == nullptr ||
        resultPwd->pw_gecos == nullptr)
    {
        return QString();
    }

    // pw_gecos为为段以','分割的字符串,存储用户全名/办公室/电话等信息
    QString gecosInfo = resultPwd->pw_gecos;
    auto gecosSplitRes = gecosInfo.split(',');
    if (gecosSplitRes.size() < 1)
    {
        return QString();
    }
    QString realName = gecosSplitRes.at(0);

    return realName;
}
}  // namespace UserInfo