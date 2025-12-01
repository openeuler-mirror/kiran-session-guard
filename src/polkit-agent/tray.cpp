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
#include "tray.h"
#include <QSystemTrayIcon>
#include <PolkitQt1/Subject>
#include <QDebug>
#include <QTimer>
#include <QMenu>

namespace Kiran
{
namespace SessionGuard
{
namespace PolkitAgent
{
Tray::Tray(PolkitQt1::Subject& subject, QObject* parent)
    : QObject(parent),
      m_subject(subject)
{
    init();
}

Tray::~Tray()
{
    delete m_updateTemporaryAuthorizationIconTimer;
    delete m_trayIcon;
    delete m_menu;
}

void Tray::init()
{
    m_menu = new QMenu();
    m_menu->addAction(tr("Revoke All Temporary Authorizations"), this, &Tray::revokeAllTemporaryAuthorizations);

    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setIcon(QIcon::fromTheme("dialog-password"));
    m_trayIcon->setVisible(false);
    m_trayIcon->setToolTip(tr("Temporary Authorization"));
    m_trayIcon->setContextMenu(m_menu);

    m_updateTemporaryAuthorizationIconTimer = new QTimer(this);
    m_updateTemporaryAuthorizationIconTimer->setInterval(200);
    connect(m_updateTemporaryAuthorizationIconTimer, &QTimer::timeout, this, &Tray::enumerateTemporaryAuth);

    auto authority = PolkitQt1::Authority::instance();

    // 配置变更时，重新枚举临时授权, 定时器触发
    connect(authority, &PolkitQt1::Authority::configChanged, this, [this]()
            { m_updateTemporaryAuthorizationIconTimer->start(); });

    // 枚举临时授权完成时，更新图标
    connect(authority, &PolkitQt1::Authority::enumerateTemporaryAuthorizationsFinished,
            this, &Tray::updateTemporaryAuthorizationIcon);
}

void Tray::enumerateTemporaryAuth()
{
    auto authority = PolkitQt1::Authority::instance();
#if POLKITQT1_VERSION<= QT_VERSION_CHECK(0, 114, 0)
    // FIXME:
    // openEuler-22.03-LTS polkit-qt5-1-0.114.0-1
    // 只声明了 PolkitQt1::Authority::enumerateTemporaryAuthorizations 方法，没有实现。
    // 暂时使用 PolkitQt1::Authority::enumerateTemporaryAuthorizationsSync 方法代替
    const PolkitQt1::TemporaryAuthorization::List temporaryAuthorizations = authority->enumerateTemporaryAuthorizationsSync(m_subject);
    updateTemporaryAuthorizationIcon(temporaryAuthorizations);
#else
    authority->enumerateTemporaryAuthorizations(m_subject);
#endif
}

void Tray::updateTemporaryAuthorizationIcon(const PolkitQt1::TemporaryAuthorization::List& temporaryAuthorizations)
{
    if (temporaryAuthorizations.isEmpty())
    {
        m_trayIcon->setVisible(false);
    }
    else
    {
        m_trayIcon->setVisible(true);
        if (temporaryAuthorizations.size() == 1)
        {
            m_trayIcon->setToolTip(tr("Temporary Authorization"));
        }
        else
        {
            m_trayIcon->setToolTip(tr("Temporary Authorization (%1)").arg(temporaryAuthorizations.size()));
        }
    }
}

void Tray::revokeAllTemporaryAuthorizations()
{
    auto authority = PolkitQt1::Authority::instance();
    qInfo() << "revoke all temporary authorizations";
    authority->revokeTemporaryAuthorizations(m_subject);
}

}  // namespace PolkitAgent
}  // namespace SessionGuard
}  // namespace Kiran