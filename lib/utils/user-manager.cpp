/**
 * Copyright (c) 2020 ~ 2023 KylinSec Co., Ltd.
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

#include "user-manager.h"
#include "auxiliary.h"
#include "dbus-common.h"

#include <pwd.h>
#include <qt5-log-i.h>
#include <sys/types.h>
#include <unistd.h>
#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusObjectPath>

#define ACCOUNT_SERVICE_DBUS "org.freedesktop.Accounts"
#define ACCOUNT_SERVICE_PATH "/org/freedesktop/Accounts"
#define ACCOUNT_SERVICE_INTERFACE "org.freedesktop.Accounts"

#define ACCOUNT_SERVICE_USER_PROPERTY_ICON "IconFile"
#define ACCOUNT_SERVICE_USER_PROPERTY_XSESSION "XSession"
#define ACCOUNT_SERVICE_USER_PROPERTY_USER_NAME "UserName"

#define DM_SERVICE_DBUS "org.freedesktop.DisplayManager"
#define DM_SERVICE_SEAT_INTERFACE "org.freedesktop.DisplayManager.Seat"
#define DM_SERVICE_SEAT_SWITCH_TO_GREETER_METHOD "SwitchToGreeter"

#define TIMEOUT_MS 300

QVariant getUserObjectProperty(const QString& objectPath, const QString& propertyName)
{
    QDBusMessage methodGetProperty = QDBusMessage::createMethodCall(ACCOUNT_SERVICE_DBUS,
                                                                    objectPath,
                                                                    FREEDESKTOP_DBUS_PROPERTIES_INTERFACE,
                                                                    FREEDESKTOP_DBUS_PROPERTIES_GET_METHOD);
    methodGetProperty << QString("org.freedesktop.Accounts.User") << propertyName;

    QDBusMessage reply = QDBusConnection::systemBus().call(methodGetProperty,
                                                           QDBus::Block, TIMEOUT_MS);

    if (reply.type() != QDBusMessage::ReplyMessage)
    {
        KLOG_WARNING() << "get accountservice user property failed:" << reply.errorMessage();
        return QString("");
    }

    QList<QVariant> argList = reply.arguments();
    if (argList.size() == 0)
    {
        KLOG_WARNING() << "get accountservice user property failed,reply no arguments";
        return "";
    }

    QVariant firstArg = argList.takeFirst();
    QDBusVariant busVariant = firstArg.value<QDBusVariant>();
    QVariant variant = busVariant.variant();
    return variant;
}

QString findUserObjectByName(const QString& user)
{
    QDBusMessage methodFindUserByName = QDBusMessage::createMethodCall(ACCOUNT_SERVICE_DBUS,
                                                                       ACCOUNT_SERVICE_PATH,
                                                                       ACCOUNT_SERVICE_INTERFACE,
                                                                       "FindUserByName");
    methodFindUserByName << user;
    QDBusMessage reply = QDBusConnection::systemBus().call(methodFindUserByName,
                                                           QDBus::Block, TIMEOUT_MS);
    if (reply.type() != QDBusMessage::ReplyMessage)
    {
        KLOG_WARNING() << "accountsService FindUserByName error:" << reply.errorMessage();
        return QString("");
    }
    QList<QVariant> args = reply.arguments();
    if (args.size() == 0)
    {
        KLOG_WARNING() << "no arguments";
        return "";
    }
    QVariant firstArg = args.takeFirst();
    QDBusObjectPath objPath = firstArg.value<QDBusObjectPath>();
    return objPath.path();
}

namespace Kiran
{
namespace SessionGuard
{
namespace UserManager
{
QString getUserIcon(const QString& name)
{
    QString iconPath;
    QString userPath = findUserObjectByName(name);
    RETURN_VAL_IF_TRUE(userPath.isEmpty(), iconPath);

    QVariant var = getUserObjectProperty(userPath, ACCOUNT_SERVICE_USER_PROPERTY_ICON);
    RETURN_VAL_IF_FALSE(var.isValid(), iconPath);
    RETURN_VAL_IF_FALSE(var.type() == QVariant::String, iconPath);

    iconPath = var.toString();
    return iconPath;
}

QString getUserLastSession(const QString& name)
{
    QString session;
    QString userPath = findUserObjectByName(name);
    RETURN_VAL_IF_TRUE(userPath.isEmpty(), session);

    auto var = getUserObjectProperty(userPath, ACCOUNT_SERVICE_USER_PROPERTY_XSESSION);
    RETURN_VAL_IF_FALSE(var.isValid(), session);
    RETURN_VAL_IF_FALSE(var.type() == QVariant::String, session);

    session = var.toString();
    return session;
}

QStringList getCachedUsers()
{
    QDBusMessage msgMethodCall = QDBusMessage::createMethodCall(ACCOUNT_SERVICE_DBUS,
                                                                ACCOUNT_SERVICE_PATH,
                                                                ACCOUNT_SERVICE_INTERFACE,
                                                                "ListCachedUsers");

    QDBusMessage msgReply = QDBusConnection::systemBus().call(msgMethodCall,
                                                              QDBus::Block,
                                                              TIMEOUT_MS);
    if (msgReply.type() != QDBusMessage::ReplyMessage)
    {
        KLOG_WARNING() << "get cached user failed,reply type" << msgReply.type();
        return {};
    }

    QList<QVariant> args = msgReply.arguments();
    if (args.size() < 1)
    {
        KLOG_WARNING() << "get cache user failed,reply arg size invalid!";
        return {};
    }

    QVariant firstArg = args.takeFirst();
    QDBusArgument dbusArg = firstArg.value<QDBusArgument>();
    if (dbusArg.currentSignature() != "ao")
    {
        KLOG_WARNING() << "get cached user failed,dbus arg signature invalid:" << dbusArg.currentSignature();
        return {};
    }

    QList<QDBusObjectPath> userObjects;
    dbusArg >> userObjects;

    QStringList users;
    for (auto userObject : userObjects)
    {
        QVariant var = getUserObjectProperty(userObject.path(), ACCOUNT_SERVICE_USER_PROPERTY_USER_NAME);
        if (!var.isNull() && var.type() == QVariant::String)
        {
            users << var.toString();
        }
    }
    return users;
}

QString getCurrentUser()
{
    uid_t uid = getuid();

    long bufSize = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (bufSize == -1)
    {
        KLOG_WARNING() << "autodetect getpw_r bufsize failed.";
        return QString("");
    }

    std::vector<char> buf(bufSize);
    struct passwd pwd;
    struct passwd* pResult = nullptr;
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

bool switchToGreeter()
{
    QString seatPath = qgetenv("XDG_SEAT_PATH");
    QDBusMessage methodSwitchToGreeter = QDBusMessage::createMethodCall(DM_SERVICE_DBUS,
                                                                        seatPath,
                                                                        DM_SERVICE_SEAT_INTERFACE,
                                                                        DM_SERVICE_SEAT_SWITCH_TO_GREETER_METHOD);

    QDBusMessage reply = QDBusConnection::systemBus().call(methodSwitchToGreeter, QDBus::Block, TIMEOUT_MS);

    if (reply.type() != QDBusMessage::ReplyMessage)
    {
        KLOG_WARNING() << seatPath << "switch to greeter failed," << reply.errorMessage();
        return false;
    }

    return true;
}

}  // namespace UserManager

}  // namespace SessionGuard
}  // namespace Kiran