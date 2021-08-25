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

#ifndef LIGHTDM_KIRAN_GREETER_KIRAN_GREETER_PREFS_H
#define LIGHTDM_KIRAN_GREETER_KIRAN_GREETER_PREFS_H

#include "greeter-dbus-interface.h"

class KiranGreeterPrefs : public GreeterDBusInterface
{
    Q_OBJECT
public:
    static KiranGreeterPrefs* instance();
    ~KiranGreeterPrefs();

    QStringList hiddenSessions();
    QStringList hiddenUsers();
    bool canPowerOff();
    bool canReboot();
    bool canSuspend();
    bool canHibernate();

private:
    KiranGreeterPrefs();

signals:
    void propertyChanged(QString propertyName, QVariant property);

private Q_SLOTS:
    void handlePropertiesChanged(QDBusMessage msg);

private:
    QStringList m_hiddenSessions;
    QStringList m_hiddenUsers;
    bool m_canPowerOff = true;
    bool m_canReboot = true;
    bool m_canSuspend = true;
    bool m_canHibernate = true;
};

#endif  //LIGHTDM_KIRAN_GREETER_KIRAN_GREETER_PREFS_H
