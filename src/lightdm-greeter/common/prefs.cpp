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

#include "prefs.h"
#include "dbus-common.h"

#include <QDBusConnection>
#include <QSettings>

namespace Kiran
{
namespace SessionGuard
{
namespace Greeter
{
Prefs* Prefs::m_instance = nullptr;
void Prefs::globalInit()
{
    m_instance = new Prefs();
    m_instance->init();
}

Prefs::Prefs()
    : KSDGreeterProxy(KSDGreeterProxy::staticInterfaceName(),
                      DBus::dbusName2GeneralPath(KSDGreeterProxy::staticInterfaceName()),
                      QDBusConnection::systemBus())
{
}

void Prefs::init()
{
    QDBusConnection::systemBus().connect(KSDGreeterProxy::staticInterfaceName(),
                                         DBus::dbusName2GeneralPath(KSDGreeterProxy::staticInterfaceName()),
                                         FREEDESKTOP_DBUS_PROPERTIES_INTERFACE,
                                         FREEDESKTOP_DBUS_PROPERTIES_CHANGED_METHOD,
                                         this, SLOT(onPropertyChanged(QDBusMessage)));

    QSettings settings("/usr/share/lightdm-kiran-greeter/greeter.ini", QSettings::IniFormat);

    /// common
    settings.beginGroup("Common");

    auto hiddenSession = settings.value("hidden-sessions","");
    m_hiddenSessions = hiddenSession.toStringList();

    auto needHideUsers = settings.value("hide-users","");
    m_hiddenUsers = needHideUsers.toStringList();

    auto numInitState = settings.value("numlock-init-state");
    m_numlockInitState = numInitState.toBool();

    auto defaultLoginUser = settings.value("default-login-user","");
    m_defaultLoginUser = defaultLoginUser.toString();

    auto showFullName = settings.value("show-fullname",false);
    m_showFullName = showFullName.toBool();

    auto monitorAlwaysOn = settings.value("monitor-always-on",true);
    m_monitorAlwaysOn = monitorAlwaysOn.toBool();

    settings.endGroup();

    /// power
    settings.beginGroup("Power");

    auto powerOffEnable = settings.value("can-poweroff");
    m_canPowerOff = powerOffEnable.toBool();

    auto rebootEnable = settings.value("can-reboot");
    m_canReboot = rebootEnable.toBool();

    auto suspendEnable = settings.value("can-suspend");
    m_canSuspend = suspendEnable.toBool();

    auto hibernateEnable = settings.value("can-hibernate");
    m_canHibernate = hibernateEnable.toBool();
}

Prefs::~Prefs()
{
}

void Prefs::onPropertyChanged(QDBusMessage msg)
{
    QList<QVariant> arguments = msg.arguments();
    QVariantMap changedProps = qdbus_cast<QVariantMap>(arguments.at(1).value<QDBusArgument>());
    for (auto iter = changedProps.begin(); iter != changedProps.end(); iter++)
    {
        emit propertyChanged(iter.key(), iter.value());
    }
}

QStringList Prefs::hiddenSessions()
{
    return m_hiddenSessions;
}

QStringList Prefs::hiddenUsers()
{
    return m_hiddenUsers;
}

QString Prefs::getDefaultLoginUser()
{
    return m_defaultLoginUser;
}

bool Prefs::canPowerOff()
{
    return m_canPowerOff;
}

bool Prefs::canReboot()
{
    return m_canReboot;
}

bool Prefs::canSuspend()
{
    return m_canSuspend;
}

bool Prefs::canHibernate()
{
    return m_canHibernate;
}

bool Prefs::numlockInitState()
{
    return m_numlockInitState;
}

bool Prefs::showFullName()
{
    return m_showFullName;
}

bool Prefs::monitorAlwaysOn()
{
    return m_monitorAlwaysOn;
}
}  // namespace Greeter
}  // namespace SessionGuard
}  // namespace Kiran