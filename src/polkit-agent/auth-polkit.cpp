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
#include "auth-polkit.h"
#include <qt5-log-i.h>
#include <PolkitQt1/Agent/Session>
#include <PolkitQt1/Identity>
#include "auth-controller-i.h"
#include "auxiliary.h"

using namespace PolkitQt1;

namespace Kiran
{
namespace SessionGuard
{
namespace PolkitAgent
{
AuthPolkit::AuthPolkit(const QString& cookie)
    : AuthBase(),
      m_cookie(cookie)
{
}

AuthPolkit::~AuthPolkit()
{
    // session需手动结束,不手动结束,认证子进程会一直保持
    if (m_session)
    {
        m_session->cancel();
        m_session->deleteLater();
    }
}

bool AuthPolkit::init(AuthControllerInterface* controllerInterface)
{
    m_controllerInterface = controllerInterface;
    return true;
}

bool AuthPolkit::authenticate(const QString& userName)
{
    if (m_session)
    {
        m_session->cancel();
        m_session->deleteLater();
        m_session = nullptr;
    }

    Identity id = Identity::fromString("unix-user:" + userName);
    RETURN_VAL_IF_FALSE(id.isValid(), false);

    m_session = new Agent::Session(id, m_cookie, nullptr);
    connect(m_session, &Agent::Session::completed, this, &AuthPolkit::handleSessionCompleted);
    connect(m_session, &Agent::Session::request, this, &AuthPolkit::handleSessionRequest);
    connect(m_session, &Agent::Session::showError, this, &AuthPolkit::handleSessionShowError);
    connect(m_session, &Agent::Session::showInfo, this, &AuthPolkit::handleSessionShowInfo);

    m_inAuth = true;
    m_session->initiate();
    return true;
}

void AuthPolkit::respond(const QString& response)
{
    RETURN_IF_FALSE(m_session);
    m_session->setResponse(response);
}

bool AuthPolkit::inAuthentication() const
{
    return m_inAuth;
}

bool AuthPolkit::isAuthenticated() const
{
    return m_gainedAuthorization;
}

QString AuthPolkit::authenticationUser() const
{
    RETURN_VAL_IF_FALSE(m_session, "");
    return m_identity.toString();
}

void AuthPolkit::cancelAuthentication()
{
    RETURN_IF_FALSE(m_session);
    RETURN_IF_FALSE(m_inAuth);

    // 阻塞信号发出，取消的时候应不发出完成信号
    m_session->blockSignals(true);
    m_session->cancel();
    delete m_session;
    m_session = nullptr;

    m_inAuth = false;
    m_gainedAuthorization = false;
}

void AuthPolkit::handleSessionCompleted(bool gainedAuthorization)
{
    m_gainedAuthorization = gainedAuthorization;

    m_session->blockSignals(true);
    m_session->cancel();
    m_session->deleteLater();
    m_session = nullptr;

    m_inAuth = false;

    m_controllerInterface->onAuthComplete();
}

void AuthPolkit::handleSessionRequest(const QString& request, bool echo)
{
    m_controllerInterface->onShowPrompt(request, echo ? PromptTypeQuestion : PromptTypeSecret);
}

void AuthPolkit::handleSessionShowError(const QString& text)
{
    m_controllerInterface->onShowMessage(text, MessageTypeError);
}

void AuthPolkit::handleSessionShowInfo(const QString& text)
{
    m_controllerInterface->onShowMessage(text, MessageTypeInfo);
}
}  // namespace PolkitAgent
}  // namespace SessionGuard
}  // namespace Kiran