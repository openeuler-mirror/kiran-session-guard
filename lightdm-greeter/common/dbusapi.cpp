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

#include "dbusapi.h"

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDBusSignature>
#include <qt5-log-i.h>

#define DBUS_PROPERTY_INTERFACE "org.freedesktop.DBus.Properties"
#define METHOD_GET_PROPERTY "Get"

#define ACCOUNT_SERVICE_DBUS "org.freedesktop.Accounts"
#define ACCOUNT_SERVICE_PATH "/org/freedesktop/Accounts"
#define ACCOUNT_SERVICE_INTERFACE "org.freedesktop.Accounts"
#define METHOD_LIST_CACHED_USERS "ListCachedUsers"
#define METHOD_FIND_USER_BY_NAME "FindUserByName"

#define ACCOUNT_SERVICE_USER_INTERFACE "org.freedesktop.Accounts.User"
#define ACCOUNT_SERVICE_USER_PROPERTY_USER_NAME "UserName"
#define ACCOUNT_SERVICE_USER_PROPERTY_ICON_FILE "IconFile"

#define TIMEOUT_MS 300

template <typename T>
bool DBusApi::getProperty(const QString &service, const QString &obj,
                          const QString &propertyName, T &retValue)
{
    QDBusMessage msgMethodCall = QDBusMessage::createMethodCall(service,
                                                                obj,
                                                                DBUS_PROPERTY_INTERFACE,
                                                                METHOD_GET_PROPERTY);

    msgMethodCall << ACCOUNT_SERVICE_USER_INTERFACE << propertyName;

    QDBusMessage msgReply = QDBusConnection::systemBus().call(msgMethodCall,
                                                              QDBus::Block,
                                                              TIMEOUT_MS);

    QString mErr;
    if (msgReply.type() == QDBusMessage::ReplyMessage)
    {
        KLOG_DEBUG() << "reply:" << msgReply;
        QList<QVariant> args = msgReply.arguments();
        if (args.size() < 1)
        {
            mErr = "arguments size < 1";
            goto failed;
        }
        QVariant firstArg = args.takeFirst();
        QDBusVariant firstDBusArg = firstArg.value<QDBusVariant>();
        if (!firstDBusArg.variant().canConvert<T>())
        {
            mErr = "can't convert";
            goto failed;
        }
        retValue = firstDBusArg.variant().value<T>();
        return true;
    }
failed:
    KLOG_WARNING() << ACCOUNT_SERVICE_USER_INTERFACE << METHOD_GET_PROPERTY << propertyName
                    << msgReply.errorName() << msgReply.errorMessage() << mErr;
    return false;
}

bool DBusApi::AccountsService::listCachedUsers(QDBusObjectPathVector &userObjects)
{
    QDBusMessage msgMethodCall = QDBusMessage::createMethodCall(ACCOUNT_SERVICE_DBUS,
                                                                ACCOUNT_SERVICE_PATH,
                                                                ACCOUNT_SERVICE_INTERFACE,
                                                                METHOD_LIST_CACHED_USERS);

    QDBusMessage msgReply = QDBusConnection::systemBus().call(msgMethodCall,
                                                              QDBus::Block,
                                                              TIMEOUT_MS);
    QString mErr = "";

    if (msgReply.type() == QDBusMessage::ReplyMessage)
    {
        QList<QVariant> args = msgReply.arguments();
        if (args.size() < 1)
        {
            mErr = "arguments size < 1";
            goto failed;
        }
        QVariant firstArg = args.takeFirst();
        QDBusArgument firstDBusArg = firstArg.value<QDBusArgument>();
        if (firstDBusArg.currentSignature() != "ao")
        {
            mErr = "argument type != \"ao\"";
            goto failed;
        }
        firstDBusArg >> userObjects;
        return true;
    }

failed:
    KLOG_WARNING() << ACCOUNT_SERVICE_DBUS << METHOD_LIST_CACHED_USERS
                    << msgReply.errorName() << msgReply.errorMessage() << mErr;
    return false;
}

bool DBusApi::AccountsService::findUserByName(const QString &name, QDBusObjectPath &obj)
{
    QDBusMessage msgMethodCall = QDBusMessage::createMethodCall(ACCOUNT_SERVICE_DBUS,
                                                                ACCOUNT_SERVICE_PATH,
                                                                ACCOUNT_SERVICE_INTERFACE,
                                                                METHOD_FIND_USER_BY_NAME);

    msgMethodCall << name;

    QDBusMessage msgReply = QDBusConnection::systemBus().call(msgMethodCall,
                                                              QDBus::Block,
                                                              TIMEOUT_MS);
    QString mErr;
    if (msgReply.type() == QDBusMessage::ReplyMessage)
    {
        QList<QVariant> args = msgReply.arguments();
        if (args.size() < 1)
        {
            mErr = "arguments size < 1";
            goto failed;
        }
        QVariant firstArg = args.takeFirst();
        obj = firstArg.value<QDBusObjectPath>();
        return true;
    }
failed:
    KLOG_WARNING() << ACCOUNT_SERVICE_DBUS << METHOD_FIND_USER_BY_NAME
                    << msgReply.errorName() << msgReply.errorMessage() << mErr;
    return false;
}

bool DBusApi::AccountsService::getUserObjectUserNameProperty(const QDBusObjectPath &obj, QString &userName)
{
    return getUserObjectUserNameProperty(obj.path(),
                                         userName);
}

bool DBusApi::AccountsService::getUserObjectUserNameProperty(const QString &obj, QString &userName)
{
    return getProperty(ACCOUNT_SERVICE_DBUS,
                       obj,
                       ACCOUNT_SERVICE_USER_PROPERTY_USER_NAME,
                       userName);
}

bool DBusApi::AccountsService::getUserObjectIconFileProperty(const QDBusObjectPath &userObj, QString &iconFile)
{
    return getUserObjectIconFileProperty(userObj.path(),
                                         iconFile);
}

bool DBusApi::AccountsService::getUserObjectIconFileProperty(const QString &obj, QString &iconFile)
{
    return getProperty(ACCOUNT_SERVICE_DBUS,
                       obj,
                       ACCOUNT_SERVICE_USER_PROPERTY_ICON_FILE,
                       iconFile);
}

bool DBusApi::AccountsService::getIconFileProperty(const QString& name, QString& iconFile)
{
    QDBusObjectPath userObjectPath;
    if (!findUserByName(name, userObjectPath) )
    {
        KLOG_WARNING() << "get" << name << "object path failed";
        return false;
    }

    if (!getUserObjectIconFileProperty(userObjectPath.path(),iconFile))
    {
        KLOG_WARNING() << "get" << userObjectPath.path() << "icon file property failed";
        return false;
    }

    return true;
}

bool DBusApi::AccountsService::getRootIconFileProperty(QString &iconFile)
{
    return  getIconFileProperty("root",iconFile);
}
