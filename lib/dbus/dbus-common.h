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
#pragma once

#include <QString>

#define FREEDESKTOP_DBUS_PROPERTIES_INTERFACE "org.freedesktop.DBus.Properties"
#define FREEDESKTOP_DBUS_PROPERTIES_CHANGED_METHOD "PropertiesChanged"
#define FREEDESKTOP_DBUS_PROPERTIES_GET_METHOD "Get"

namespace Kiran
{
namespace SessionGuard
{
namespace DBus
{
inline QString dbusName2GeneralPath(const QString& interfaceName)
{
    auto splitRes = interfaceName.split('.');
    auto generalPath = "/" + splitRes.join('/');
    return generalPath;
};
}  // namespace DBus

}  // namespace SessionGuard
}  // namespace Kiran