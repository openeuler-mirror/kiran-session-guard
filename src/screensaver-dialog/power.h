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
#include <QObject>


QT_BEGIN_NAMESPACE
class QDBusInterface;
QT_END_NAMESPACE

namespace Kiran
{
namespace SessionGuard
{
namespace Locker
{
class Prefs;
class Power : public QObject
{
    Q_OBJECT
private:
    explicit Power(Prefs* prefs, QObject* parent = nullptr);

public:
    ~Power();

    static void globalInit(Prefs* prefs);
    static void globalDeinit() { delete m_instance; };
    static Power* getInstance() { return m_instance; };

    bool canPoweroff();
    bool canReboot();
    bool canSuspend();

    bool poweroff();
    bool reboot();
    bool suspend();

private:
    static Power* m_instance;
    Prefs* m_prefs;
    QDBusInterface* m_loginProxy;
    QDBusInterface* m_smProxy;
};
}  // namespace Locker
}  // namespace SessionGuard
}  // namespace Kiran