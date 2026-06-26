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

#include "auth-lightdm.h"
#include <qt5-log-i.h>
#include <QDateTime>

using namespace ::Kiran::SessionGuard;

namespace Kiran
{
namespace SessionGuard
{
namespace Greeter
{
AuthLightdm::AuthLightdm(QSharedPointer<QLightDM::Greeter> greeterAuth, QObject *parent)
    : QObject(parent),
      AuthBase(),
      m_greeterPtr(greeterAuth)
{
    connect(m_greeterPtr.data(), &QLightDM::Greeter::showPrompt, this, &AuthLightdm::onGreeterAuthShowPrompt);
    connect(m_greeterPtr.data(), &QLightDM::Greeter::showMessage, this, &AuthLightdm::onGreeterAuthShowMessage);
    connect(m_greeterPtr.data(), &QLightDM::Greeter::authenticationComplete, this, &AuthLightdm::onGreeterAuthComplete);
}

AuthLightdm::~AuthLightdm()
{
}

bool AuthLightdm::init(AuthControllerInterface *controllerInterface)
{
    bool bRes = m_greeterPtr->connectSync();
    if (!bRes)
    {
        KLOG_ERROR() << "can't connect greeter auth!";
    }
    m_interface = controllerInterface;
    return bRes;
}

bool AuthLightdm::authenticate(const QString &userName)
{
    KLOG_INFO() << "AuthLightdm: authenticate"
                << "user=" << userName
                << "inAuth=" << m_greeterPtr->inAuthentication()
                << "epochMs=" << QDateTime::currentMSecsSinceEpoch();
    m_greeterPtr->authenticate(userName);
    return true;
}

void AuthLightdm::cancelAuthentication()
{
    KLOG_INFO() << "AuthLightdm: cancelAuthentication"
                << "inAuth=" << m_greeterPtr->inAuthentication()
                << "epochMs=" << QDateTime::currentMSecsSinceEpoch();
    m_greeterPtr->cancelAuthentication();
}

bool AuthLightdm::isAuthenticated() const
{
    return m_greeterPtr->isAuthenticated();
}

bool AuthLightdm::inAuthentication() const
{
    return m_greeterPtr->inAuthentication();
}

QString AuthLightdm::authenticationUser() const
{
    return m_greeterPtr->authenticationUser();
}

void AuthLightdm::respond(const QString &response)
{
    KLOG_INFO() << "AuthLightdm: respond"
                << "len=" << response.size()
                << "inAuth=" << m_greeterPtr->inAuthentication()
                << "user=" << m_greeterPtr->authenticationUser()
                << "epochMs=" << QDateTime::currentMSecsSinceEpoch();
    m_greeterPtr->respond(response);
}

void AuthLightdm::onGreeterAuthShowPrompt(QString text, QLightDM::Greeter::PromptType type)
{
    KLOG_INFO() << "AuthLightdm: showPrompt"
                << "type=" << (int)type
                << "len=" << text.size()
                << "inAuth=" << m_greeterPtr->inAuthentication()
                << "user=" << m_greeterPtr->authenticationUser()
                << "epochMs=" << QDateTime::currentMSecsSinceEpoch();
    m_interface->onShowPrompt(text, type == QLightDM::Greeter::PromptTypeSecret ? PromptTypeSecret : PromptTypeQuestion);
}

void AuthLightdm::onGreeterAuthShowMessage(QString text, QLightDM::Greeter::MessageType type)
{
    KLOG_INFO() << "AuthLightdm: showMessage"
                << "type=" << (int)type
                << "text=" << text
                << "inAuth=" << m_greeterPtr->inAuthentication()
                << "user=" << m_greeterPtr->authenticationUser()
                << "epochMs=" << QDateTime::currentMSecsSinceEpoch();
    m_interface->onShowMessage(text, type == QLightDM::Greeter::MessageTypeInfo ? MessageTypeInfo : MessageTypeError);
}

void AuthLightdm::onGreeterAuthComplete()
{
    KLOG_INFO() << "AuthLightdm: authenticationComplete"
                << "authenticated=" << m_greeterPtr->isAuthenticated()
                << "inAuth=" << m_greeterPtr->inAuthentication()
                << "user=" << m_greeterPtr->authenticationUser()
                << "epochMs=" << QDateTime::currentMSecsSinceEpoch();
    m_interface->onAuthComplete();
}

}  // namespace Greeter
}  // namespace SessionGuard
}  // namespace Kiran