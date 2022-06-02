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

#ifndef DBUSAPI_H
#define DBUSAPI_H

#include <QDBusObjectPath>
#include <QVector>

typedef QVector<QDBusObjectPath> QDBusObjectPathVector;

/* TODO:修改成使用KiranSystemDaemon Account后端模块 */
namespace DBusApi
{
template <typename T>
bool getProperty(const QString& service, const QString& obj, const QString& propertyName, T& retValue);
namespace AccountsService
{
bool listCachedUsers(QDBusObjectPathVector& userObjects);

bool findUserByName(const QString& name, QDBusObjectPath& obj);

bool getUserObjectUserNameProperty(const QDBusObjectPath& obj, QString& userName);
bool getUserObjectUserNameProperty(const QString& obj, QString& userName);

bool getUserObjectIconFileProperty(const QDBusObjectPath& userObj, QString& iconFile);
bool getUserObjectIconFileProperty(const QString& obj, QString& iconFile);

bool getRootIconFileProperty(QString& iconFile);
}  // namespace AccountsService
}  // namespace DBusApi
#endif  // DBUSAPI_H
