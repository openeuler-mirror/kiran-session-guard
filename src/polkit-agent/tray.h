/**
 * Copyright (c) 2020 ~ 2025 KylinSec Co., Ltd.
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
#include <PolkitQt1/Subject>
#include <PolkitQt1/Authority>

class QSystemTrayIcon;
class QTimer;
class QMenu;

namespace Kiran
{
namespace SessionGuard
{
namespace PolkitAgent
{
class Tray : public QObject
{
    Q_OBJECT
public:
    explicit Tray(PolkitQt1::Subject& subject, QObject *parent = nullptr);
    ~Tray();

private:
    void init();

private slots:
    void enumerateTemporaryAuth();
    void updateTemporaryAuthorizationIcon(const PolkitQt1::TemporaryAuthorization::List &temporaryAuthorizations);
    void revokeAllTemporaryAuthorizations();

private:
    PolkitQt1::Subject m_subject;
    QSystemTrayIcon *m_trayIcon = nullptr;
    QTimer *m_updateTemporaryAuthorizationIconTimer = nullptr;
    QMenu *m_menu = nullptr;
};
}  // namespace PolkitAgent
}  // namespace SessionGuard
}  // namespace Kiran