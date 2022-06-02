/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd.
 * kiran-session-guard is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     liuxinhao <liuxinhao@kylinos.com.cn>
 */

#ifndef __KIRAN_GREETER_ACCOUNTS_TOOL_H__
#define __KIRAN_GREETER_ACCOUNTS_TOOL_H__

#include "dbus/accounts-interface.h"
#include "dbus/accounts-user-interface.h"
#include <qt5-log-i.h>

namespace AccountsTool
{
    bool isUserEnabled(const QString& account)
    {
        bool enable = true;

        //通过账户名寻找到账户对象
        AccountsInterface accountsInterface(QDBusConnection::systemBus());
        auto reply = accountsInterface.FindUserByName(account);
        reply.waitForFinished();
        if(reply.isError())
        {
            KLOG_ERROR() << "find user by name failed!" << reply.error();
            return enable;
        }

        auto userObjPath = reply.value();
        UserInterface userInterface(userObjPath.path(),QDBusConnection::systemBus());
        if(!userInterface.isValid())
        {
            KLOG_ERROR() << "user interface is invalid!";
            return enable;
        }

        return !userInterface.locked();
    }
}

#endif // __KIRAN_GREETER_ACCOUNTS_TOOL_H__
