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
    bool canPowerOff();
    bool canReboot();
    bool canSuspend();
    bool canHibernate();
    bool numlockInitState();

private:
    Prefs();
    void init();

signals:
    void propertyChanged(QString name, QVariant value);

private slots:
    void onPropertyChanged(QDBusMessage messsage);

private:
    static Prefs* m_instance;
    QStringList m_hiddenSessions;
    QStringList m_hiddenUsers;
    bool m_numlockInitState = true;
    bool m_canPowerOff = true;
    bool m_canReboot = true;
    bool m_canSuspend = true;
    bool m_canHibernate = true;
};
}  // namespace Greeter
}  // namespace SessionGuard
}  // namespace Kiran