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
#include "power.h"
#include "auxiliary.h"
#include "prefs.h"

#include <qt5-log-i.h>
#include <QDBusConnectionInterface>
#include <QDBusInterface>

#define SESSION_MANAGER_DBUS "org.gnome.SessionManager"
#define SESSION_MANAGER_PATH "/org/gnome/SessionManager"
#define SESSION_MANAGER_INTERFACE "org.gnome.SessionManager"

#define LOGIN_MANAGER_DBUS "org.freedesktop.login1"
#define LOGIN_MANAGER_PATH "/org/freedesktop/login1"
#define LOGIN_MANAGER_INTERFACE "org.freedesktop.login1.Manager"
#define LOGIN_SESSION_INTERFACE "org.freedesktop.login1.Session"

#define DISPLAY_MANAGER_DBUS "org.freedesktop.DisplayManager"
#define DISPLAY_MANAGER_SEAT_PATH "/org/freedesktop/DisplayManager/Seat0"
#define DISPLAY_MANAGER_INTERFACE "org.freedesktop.DisplayManager.Seat"

#define DBUS_PROXY_TIMEOUT_MSEC 300

namespace Kiran
{
namespace SessionGuard
{
namespace Locker
{
Power::Power(Prefs* prefs, QObject* parent)
    : QObject(parent),
      m_prefs(prefs)
{
    auto checkLoginReply = QDBusConnection::sessionBus().interface()->isServiceRegistered(LOGIN_MANAGER_DBUS);
    if (checkLoginReply.value())
    {
        m_loginProxy = new QDBusInterface(LOGIN_MANAGER_DBUS, LOGIN_MANAGER_PATH, LOGIN_MANAGER_INTERFACE, QDBusConnection::sessionBus(), this);
        m_loginProxy->setTimeout(DBUS_PROXY_TIMEOUT_MSEC);
    }
    else
    {
        m_loginProxy = nullptr;
        KLOG_WARNING() << "dbus service" << LOGIN_MANAGER_DBUS << "is not detected";
    }

    auto checkSMReply = QDBusConnection::sessionBus().interface()->isServiceRegistered(SESSION_MANAGER_DBUS);
    if (checkSMReply.value())
    {
        m_smProxy = new QDBusInterface(SESSION_MANAGER_DBUS, SESSION_MANAGER_PATH, SESSION_MANAGER_INTERFACE, QDBusConnection::sessionBus(), this);
        m_smProxy->setTimeout(DBUS_PROXY_TIMEOUT_MSEC);
    }
    else
    {
        m_smProxy = nullptr;
        KLOG_WARNING() << "dbus service" << SESSION_MANAGER_DBUS << "is not detected";
    }
}

Power::~Power()
{
}

Power* Power::m_instance = nullptr;
void Power::globalInit(Prefs* prefs)
{
    m_instance = new Power(prefs);
}

bool Power::canPoweroff()
{
    bool prefsEnable = m_prefs->canPowerOff();
    RETURN_VAL_IF_TRUE(!prefsEnable || !m_loginProxy, false);

    bool loginManagerEnable = false;
    QDBusMessage msg = m_loginProxy->call(QDBus::Block, "CanPowerOff");
    if ((msg.type() == QDBusMessage::ReplyMessage) && (msg.arguments().size() >= 0))
    {
        QVariant var = msg.arguments().takeAt(0);
        if (var.toString() == "yes")
        {
            loginManagerEnable = true;
        }
    }

    return prefsEnable && loginManagerEnable;
}

bool Power::canReboot()
{
    bool prefsEnable = m_prefs->canReboot();
    RETURN_VAL_IF_TRUE(!prefsEnable || !m_loginProxy, false);

    bool loginManagerEnable = false;
    QDBusMessage msg = m_loginProxy->call(QDBus::Block, "CanReboot");
    if ((msg.type() == QDBusMessage::ReplyMessage) && (msg.arguments().size() >= 0))
    {
        QVariant var = msg.arguments().takeAt(0);
        if (var.toString() == "yes")
        {
            loginManagerEnable = true;
        }
    }

    return prefsEnable && loginManagerEnable;
}

bool Power::canSuspend()
{
    bool prefsEnable = m_prefs->canSuspend();
    RETURN_VAL_IF_TRUE(!prefsEnable || !m_loginProxy, false);

    bool loginManagerEnable = false;
    QDBusMessage msg = m_loginProxy->call(QDBus::Block, "CanSuspend");
    if ((msg.type() == QDBusMessage::ReplyMessage) && (msg.arguments().size() >= 0))
    {
        QVariant var = msg.arguments().takeAt(0);
        if (var.toString() == "yes")
        {
            loginManagerEnable = true;
        }
    }

    return prefsEnable && loginManagerEnable;
}

bool Power::poweroff()
{
    RETURN_VAL_IF_FALSE(canPoweroff() || m_smProxy, false);
    auto msg = m_smProxy->call(QDBus::Block, "RequestShutdown");
    return msg.type() == QDBusMessage::ReplyMessage;
}

bool Power::reboot()
{
    RETURN_VAL_IF_FALSE(canReboot() || m_smProxy, false);
    auto msg = m_smProxy->call(QDBus::Block, "RequestReboot");
    return msg.type() == QDBusMessage::ReplyMessage;
}

bool Power::suspend()
{
    RETURN_VAL_IF_FALSE(canSuspend() || m_loginProxy, false);
    auto msg = m_loginProxy->call(QDBus::Block, "Suspend");
    return msg.type() == QDBusMessage::ReplyMessage;
}

}  // namespace Locker
}  // namespace SessionGuard
}  // namespace Kiran