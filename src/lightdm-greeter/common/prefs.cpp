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

#define GREETER_SETTINGS_FILE "/usr/share/lightdm-kiran-greeter/greeter.ini"

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
    , m_iniSettings(nullptr)
    , m_iniFileWatcher(nullptr)
{
}

void Prefs::init()
{
    m_ksdServiceAvailable = QDBusConnection::systemBus().interface()->isServiceRegistered(KSDGreeterProxy::staticInterfaceName());

    if (m_ksdServiceAvailable)
    {
        QDBusConnection::systemBus().connect(KSDGreeterProxy::staticInterfaceName(),
                                             DBus::dbusName2GeneralPath(KSDGreeterProxy::staticInterfaceName()),
                                             FREEDESKTOP_DBUS_PROPERTIES_INTERFACE,
                                             FREEDESKTOP_DBUS_PROPERTIES_CHANGED_METHOD,
                                             this, SLOT(onPropertyChanged(QDBusMessage)));
    }

    initFromSettings();
}

void Prefs::setupSettingsFileWatcher()
{
    if (m_iniFileWatcher)
        return;

    m_iniSettings = new QSettings(GREETER_SETTINGS_FILE, QSettings::IniFormat, this);
    m_iniFileWatcher = new QFileSystemWatcher(this);

    if (QFile::exists(GREETER_SETTINGS_FILE))
    {
        m_iniFileWatcher->addPath(GREETER_SETTINGS_FILE);
    }

    connect(m_iniFileWatcher, &QFileSystemWatcher::fileChanged, this, &Prefs::handleIniFileChanged);
}

void Prefs::handleIniFileChanged()
{
    QTimer::singleShot(500, this, [this]() {
        if (!QFile::exists(GREETER_SETTINGS_FILE))
        {
            return;
        }

        if (!m_iniFileWatcher->files().contains(GREETER_SETTINGS_FILE))
        {
            m_iniFileWatcher->addPath(GREETER_SETTINGS_FILE);
        }

        m_iniSettings->sync();

        // 只在服务不可用时从INI重新加载值
        if (!m_ksdServiceAvailable)
        {
            m_background = m_iniSettings->value("Greeter/background", "").toString();
            m_autologinUser = m_iniSettings->value("Greeter/autologin_user", "").toString();
            m_autologinTimeout = m_iniSettings->value("Greeter/autologin_timeout", 0).toULongLong();
            m_allowManualLogin = m_iniSettings->value("Greeter/allow_manual_login", true).toBool();
            m_hideUserList = m_iniSettings->value("Greeter/hide_user_list", false).toBool();
            m_scaleMode = m_iniSettings->value("Greeter/scale_mode", 0).toUInt();
            m_scaleFactor = m_iniSettings->value("Greeter/scale_factor", 1).toUInt();
            
            // TODO: 按理说这里是要比较原来的值和新的值的变化，然后触发 Prefs::propertyChanged 信号
            // 但实际上没必要，因为当前程序没有地方处理过这个信号
            // 因为登录锁屏运行时，正常来说用户不会去修改配置（含dbus、ini），而如果是通过界面修改，登录锁屏并没有启动，待下次启动会直接使用新值
        }
    });
}

void Prefs::initFromSettings()
{
    QSettings settings(GREETER_SETTINGS_FILE, QSettings::IniFormat);

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

    m_facePreviewEnabled = settings.value("face-preview-enabled", true).toBool();

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

    settings.endGroup();

    // greeter DBus service 不存在时使用
    settings.beginGroup("Greeter");
    m_background = settings.value("background", "").toString();
    m_autologinUser = settings.value("autologin_user", "").toString();
    m_autologinTimeout = settings.value("autologin_timeout", 0).toULongLong();
    m_allowManualLogin = settings.value("allow_manual_login", true).toBool();
    m_hideUserList = settings.value("hide_user_list", false).toBool();
    m_scaleMode = settings.value("scale_mode", 0).toUInt();
    m_scaleFactor = settings.value("scale_factor", 1).toUInt();
    settings.endGroup();

    setupSettingsFileWatcher();
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

bool Prefs::facePreviewEnabled() const
{
    return m_facePreviewEnabled;
}

QString Prefs::background()
{
    if (m_ksdServiceAvailable)
    {
        return KSDGreeterProxy::background();
    }
    return m_background;
}

QString Prefs::autologin_user()
{
    if (m_ksdServiceAvailable)
    {
        return KSDGreeterProxy::autologin_user();
    }
    return m_autologinUser;
}

qulonglong Prefs::autologin_timeout()
{
    if (m_ksdServiceAvailable)
    {
        return KSDGreeterProxy::autologin_timeout();
    }
    return m_autologinTimeout;
}

bool Prefs::allow_manual_login()
{
    if (m_ksdServiceAvailable)
    {
        return KSDGreeterProxy::allow_manual_login();
    }
    return m_allowManualLogin;
}

bool Prefs::hide_user_list()
{
    if (m_ksdServiceAvailable)
    {
        return KSDGreeterProxy::hide_user_list();
    }
    return m_hideUserList;
}

quint16 Prefs::scale_mode()
{
    if (m_ksdServiceAvailable)
    {
        return KSDGreeterProxy::scale_mode();
    }
    return m_scaleMode;
}

quint16 Prefs::scale_factor()
{
    if (m_ksdServiceAvailable)
    {
        return KSDGreeterProxy::scale_factor();
    }
    return m_scaleFactor;
}
}  // namespace Greeter
}  // namespace SessionGuard
}  // namespace Kiran