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
#include "listener.h"
#include "dialog.h"

#include <kiran-log/qt5-log-i.h>
#include <PolkitQt1/Details>
#include <QApplication>
#include <QDBusConnection>
#include <QDBusError>
#include <QScreen>
#include <QTimer>
#include <QX11Info>

namespace Kiran
{
namespace SessionGuard
{
namespace PolkitAgent
{
Listener::Listener(QObject *parent)
    : PolkitQt1::Agent::Listener(parent)
{
}

Listener::~Listener()
{
}

void Listener::initiateAuthentication(const QString &actionId,
                                      const QString &message,
                                      const QString &iconName,
                                      const PolkitQt1::Details &details,
                                      const QString &cookie,
                                      const PolkitQt1::Identity::List &identities,
                                      PolkitQt1::Agent::AsyncResult *result)
{
    if (m_inProcess)
    {
        result->setError(tr("Existing authentication is in progress, please try again later"));
        result->setCompleted();
        return;
    }

    m_inProcess = true;
    m_result = result;

    AuthInfo info;
    info.actionID = actionId;
    info.message = message;
    info.iconName = iconName;
    info.details = details;
    info.cookie = cookie;
    info.identities = identities;
    info.result = result;
    // info.dump();

    m_authDialog = new Dialog();
    if (!m_authDialog->init(info))
    {
        result->setError(tr("Existing authentication is in progress, please try again later"));
        result->setCompleted();

        m_result = nullptr;
        m_inProcess = false;

        m_authDialog->deleteLater();
        m_authDialog = nullptr;
        return;
    }

    connect(m_authDialog, &Dialog::completed, this, &Listener::onAuthDialogCompleted);
    connect(m_authDialog, &Dialog::cancelled, this, &Listener::onAuthDialogCancelled);

    auto screen = QApplication::screenAt(QCursor::pos());
    if (screen != nullptr)
    {
        QRect screenGeometry = screen->geometry();
        m_authDialog->move(screenGeometry.x() + (screenGeometry.width() - m_authDialog->width()) / 2,
                           screenGeometry.y() + (screenGeometry.height() - m_authDialog->height()) / 2);
    }

    QX11Info::setAppTime(QX11Info::getTimestamp());
    m_authDialog->show();
    m_authDialog->activateWindow();
    return;
}

bool Listener::initiateAuthenticationFinish()
{
    return true;
}

void Listener::cancelAuthentication()
{
    if (m_inProcess)
    {
        m_inProcess = false;
        m_authDialog->hide();
        m_authDialog->deleteLater();
        m_authDialog = nullptr;

        m_result->setError("Authentication Cancelled");
        m_result->setCompleted();
        m_result = nullptr;
    }
}

void Listener::onAuthDialogCompleted(bool isSuccess)
{
    if (!isSuccess)
    {
        m_result->setError("Authentication Error");
    }
    m_inProcess = false;

    m_result->setCompleted();
    m_result = nullptr;

    m_authDialog->deleteLater();
    m_authDialog = nullptr;
}

void Listener::onAuthDialogCancelled()
{
    KLOG_DEBUG() << "dialog cancelled,cancel authentication";
    cancelAuthentication();
}
}  // namespace PolkitAgent
}  // namespace SessionGuard
}  // namespace Kiran