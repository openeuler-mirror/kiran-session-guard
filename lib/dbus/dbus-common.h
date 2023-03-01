#pragma once

#include <QString>
#include "guard-global.h"

#define FREEDESKTOP_DBUS_PROPERTIES_INTERFACE "org.freedesktop.DBus.Properties"
#define FREEDESKTOP_DBUS_PROPERTIES_CHANGED_METHOD "PropertiesChanged"
#define FREEDESKTOP_DBUS_PROPERTIES_GET_METHOD "Get"

GUARD_BEGIN_NAMESPACE

namespace DBus
{
inline QString dbusName2GeneralPath(const QString& interfaceName)
{
    auto splitRes = interfaceName.split('.');
    auto generalPath = "/" + splitRes.join('/');
    return generalPath;
};
}  // namespace DBus

GUARD_END_NAMESPACE