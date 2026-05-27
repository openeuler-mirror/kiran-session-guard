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
#pragma once

#include "ksd_greeter_proxy.h"
#include <QFileSystemWatcher>
#include <QSettings>
#include <QTimer>

namespace Kiran
{
namespace SessionGuard
{
namespace Greeter
{
class Prefs : public KSDGreeterProxy
{
    Q_OBJECT
public:
    ~Prefs();
    static void globalInit();
    static void globalDeinit() { delete m_instance; };
    static Prefs* getInstance() { return m_instance; };

    QStringList hiddenSessions();
    QStringList hiddenUsers();
    QString getDefaultLoginUser();
    bool canPowerOff();
    bool canReboot();
    bool canSuspend();
    bool canHibernate();
    bool numlockInitState();
    bool showFullName();
    bool monitorAlwaysOn();
    bool facePreviewEnabled() const;

    /// Inherited from KSDGreeterProxy, falls back to greeter.ini when service unavailable
    QString background();
    QString autologin_user();
    qulonglong autologin_timeout();
    bool allow_manual_login();
    bool hide_user_list();
    quint16 scale_mode();
    quint16 scale_factor();

private:
    Prefs();
    void init();

signals:
    void propertyChanged(QString name, QVariant value);

private slots:
    void onPropertyChanged(QDBusMessage messsage);
    void handleIniFileChanged();
private:
    void initFromSettings();
    void setupSettingsFileWatcher();

private:
    static Prefs* m_instance;
    QSettings* m_iniSettings;
    QFileSystemWatcher* m_iniFileWatcher;

    QStringList m_hiddenSessions;
    QStringList m_hiddenUsers;
    QString m_defaultLoginUser;
    bool m_showFullName = false;
    bool m_numlockInitState = true;
    bool m_monitorAlwaysOn = true;
    bool m_facePreviewEnabled = false;
    bool m_canPowerOff = true;
    bool m_canReboot = true;
    bool m_canSuspend = true;
    bool m_canHibernate = true;

    // 用于 com.kylinsec.Kiran.SystemDaemon.Greeter 不存在时使用
    // 特别是低版本系统上没有适配com.kylinsec.Kiran.SystemDaemon.Greeter时
    bool m_ksdServiceAvailable = false;
    QString m_background;
    QString m_autologinUser;
    qulonglong m_autologinTimeout = 0;
    bool m_allowManualLogin = true;
    bool m_hideUserList = false;
    quint16 m_scaleMode = 0;
    quint16 m_scaleFactor = 1;
};
}  // namespace Greeter
}  // namespace SessionGuard
}  // namespace Kiran