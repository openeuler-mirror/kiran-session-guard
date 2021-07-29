/**
  * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd.
  *
  * Author:     liuxinhao <liuxinhao@kylinos.com.cn>
  *
  * This program is free software; you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation; either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program; If not, see <http: //www.gnu.org/licenses/>. 
  */
#include "auth-proxy.h"
#include "auth-msg-queue.h"
#include "auth-pam.h"
#include "kiran_authentication.h"

#include <qt5-log-i.h>
//kiran-biometrics头文件
#include <kiran-pam-msg.h>
//kiran-authentication-service头文件
#include <kiran-authentication-service/authentication_i.h>

AuthProxy::AuthProxy(QObject *parent)
    : QObject(parent),
      m_authInterface(new AuthPam(this))
{
    init();
}

AuthProxy::~AuthProxy()
{
    m_authInterface->cancelAuthentication();
}

bool AuthProxy::init()
{
    m_authServiceInterface = new ComKylinsecKiranSystemDaemonAuthenticationInterface(AUTH_SERVICE_DBUS_NAME,
                                                                                     AUTH_SERVICE_OBJECT_PATH,
                                                                                     QDBusConnection::systemBus(),
                                                                                     this);
    if (!connect(m_authServiceInterface, &ComKylinsecKiranSystemDaemonAuthenticationInterface::AuthStatus,
                 this, &AuthProxy::handleAuthServiceAuthStatusChanged) ||
        !connect(m_authServiceInterface, &ComKylinsecKiranSystemDaemonAuthenticationInterface::AuthMessages,
                 this, &AuthProxy::handleAuthServiceAuthMessages) ||
        !connect(m_authServiceInterface, &ComKylinsecKiranSystemDaemonAuthenticationInterface::AuthMethodChanged,
                 this, &AuthProxy::handleAuthServiceAuthMethodChanged))
    {
        KLOG_FATAL("connect to auth service signal failed!");
    }

    if (!connect(m_authInterface, &AuthBase::showMessage, this, &AuthProxy::handlePamAuthShowMessage) ||
        !connect(m_authInterface, &AuthBase::authenticationComplete, this, &AuthProxy::handlePamAuthComplete) ||
        !connect(m_authInterface, &AuthBase::showPrompt, this, &AuthProxy::handlePamAuthShowPrompt))
    {
        KLOG_FATAL("connect to auth interface signal failed!");
    }

    return true;
}

bool AuthProxy::setMsgQueue(AuthMsgQueueBase *msgQueue)
{
    if (inAuthentication())
    {
        KLOG_ERROR() << "in authentication,can't set message queue!";
        return false;
    }

    if (m_authMessageQueue != nullptr)
    {
        disconnect(m_authMessageQueue, &AuthMsgQueueBase::showPrompt,
                   this, &AuthProxy::handleAuthQueueShowPrompt);
        disconnect(m_authMessageQueue, &AuthMsgQueueBase::showMessage,
                   this, &AuthProxy::handleAuthQueueShowMessage);
        disconnect(m_authMessageQueue, &AuthMsgQueueBase::authenticationComplete,
                   this, &AuthProxy::handleAuthQueueComplete);
        m_authMessageQueue->deleteLater();
    }

    m_authMessageQueue = msgQueue;

    if (m_authMessageQueue)
    {
        connect(m_authMessageQueue, &AuthMsgQueueBase::showPrompt,
                this, &AuthProxy::handleAuthQueueShowPrompt);
        connect(m_authMessageQueue, &AuthMsgQueueBase::showMessage,
                this, &AuthProxy::handleAuthQueueShowMessage);
        connect(m_authMessageQueue, &AuthMsgQueueBase::authenticationComplete,
                this, &AuthProxy::handleAuthQueueComplete);
    }

    return true;
}

bool AuthProxy::inAuthentication() const
{
    return m_authInterface->inAuthentication();
}

bool AuthProxy::isAuthenticated() const
{
    return m_authInterface->isAuthenticated();
}

QString AuthProxy::authenticationUser() const
{
    return m_authInterface->authenticationUser();
}

void AuthProxy::authenticate(const QString &username)
{
    KLOG_DEBUG() << "auth proxy authenticate:" << username;
    m_authInterface->authenticate(username);
}

void AuthProxy::respond(const QString &response)
{
    if (m_promptFrom == Kiran::PROMPT_FROM_AUTH_SERVICE)
    {
        KLOG_DEBUG() << "respond to auth service";
        m_authServiceInterface->ResponseMessage(response, m_authSessionID);
    }
    else
    {
        KLOG_DEBUG() << "respond to auth";
        m_authInterface->respond(response);
    }
}

void AuthProxy::cancelAuthentication()
{
    KLOG_DEBUG() << "cancel authentication";
    m_authInterface->cancelAuthentication();
}

void AuthProxy::handlePamAuthComplete()
{
    ///TODO:根据合适场景写入一个错误描述
    if (m_authMessageQueue != nullptr)
    {
        KLOG_DEBUG() << "add auth complete msg to queue";
        m_authMessageQueue->appendAuthCompleteMsg(m_authInterface->isAuthenticated(), "");
    }
    else
    {
        KLOG_DEBUG() << "auth complete";
        stopAuthSession(m_authSessionID);
        emit authenticationComplete(m_authInterface->isAuthenticated(), "");
    }
}

void AuthProxy::handlePamAuthShowPrompt(QString text, Kiran::PromptType type)
{
    if (text == ASK_AUTH_SID)
    {
        KLOG_DEBUG() << "handle auth ask auth sid";
        //create auth
        QString authSessionID;
        if (!createAuthSession(authSessionID))
        {
            KLOG_ERROR() << "can't create auth session for" << m_authInterface->authenticationUser();
            m_authInterface->respond("");
            return;
        }

        KLOG_DEBUG() << "created auth session:" << authSessionID;

        //respond to pam
        m_authSessionID = authSessionID;
        m_authInterface->respond(m_authSessionID);

        //start auth
        if(!startAuthSession(m_authInterface->authenticationUser(),m_authSessionID))
        {
            KLOG_ERROR() << "can't start auth session:" << m_authSessionID;
            //TODO:认证服务开启认证会话失败，应中止PAM、停止认证服务认证会话
        }
        return;
    }
    else if (text == ASK_FACE) ///兼容老版本PAM直接通过prompt消息通知
    {
        KLOG_DEBUG() << "handle auth ask face";
        emit authTypeChanged(Kiran::AUTH_TYPE_FACE);
        m_authInterface->respond(REP_FACE);
        return;
    }
    else if (text == ASK_FPINT) ///兼容老版本PAM直接通过prompt消息通知
    {
        KLOG_DEBUG() << "handle auth ask fingerprint";
        emit authTypeChanged(Kiran::AUTH_TYPE_FINGER);
        m_authInterface->respond(REP_FPINT);
        return;
    }

    if (m_authMessageQueue != nullptr)
    {
        KLOG_DEBUG() << "append prompt to message queue" << text;
        m_authMessageQueue->append(Kiran::PROMPT_FROM_PAM, type, text);
    }
    else
    {
        KLOG_DEBUG() << "prompt message:" << text;
        m_promptFrom = Kiran::PROMPT_FROM_PAM;
        emit showPrompt(text, type);
    }
}

void AuthProxy::handlePamAuthShowMessage(QString text, Kiran::MessageType type)
{
    if (m_authMessageQueue != nullptr)
    {
        KLOG_DEBUG() << "append message to message queue" << text;
        m_authMessageQueue->append(type, text);
    }
    else
    {
        KLOG_DEBUG() << "message:" << text;
        emit showMessage(text, type);
    }
}

void AuthProxy::handleAuthServiceAuthStatusChanged(QString userName, int state, QString sid)
{
    if (sid != m_authSessionID)
    {
        return;
    }

    KLOG_DEBUG() << "auth status changed:" << userName
                 << "state:" << ((state == (int)SESSION_AUTH_SUCCESS) ? "success" : "failed");
}

void AuthProxy::handleAuthServiceAuthMessages(QString msg, int type, QString sid)
{
    if (sid != m_authSessionID)
    {
        return;
    }

    switch (type)
    {
    case AUTH_SERVICE_PROMPT_ECHO_OFF:
    case AUTH_SERVICE_PROMPT_ECHO_ON:
    {
        Kiran::PromptType promptType = type == AUTH_SERVICE_PROMPT_ECHO_ON ? Kiran::PromptTypeQuestion : Kiran::PromptTypeSecret;
        if (m_authMessageQueue != nullptr)
        {
            KLOG_DEBUG() << "append prompt message to message queue:" << msg;
            m_authMessageQueue->append(Kiran::PROMPT_FROM_AUTH_SERVICE, promptType, msg);
        }
        else
        {
            KLOG_DEBUG() << "prompt message:" << msg;
            m_promptFrom = Kiran::PROMPT_FROM_AUTH_SERVICE;
            emit showPrompt(msg, promptType);
        }
        break;
    }
    case AUTH_SERVICE_TEXT_INFO:
    case AUTH_SERVICE_ERROR_MSG:
    {
        Kiran::MessageType messageType = type == AUTH_SERVICE_TEXT_INFO ? Kiran::MessageTypeInfo : Kiran::MessageTypeError;
        if (m_authMessageQueue != nullptr)
        {
            KLOG_DEBUG() << "append message to message queue:" << msg;
            m_authMessageQueue->append(messageType, msg);
        }
        else
        {
            KLOG_DEBUG() << "message:" << msg;
            emit showMessage(msg, messageType);
        }
    }
    default:
        break;
    }
}
void AuthProxy::handleAuthQueueShowMessage(QString text, Kiran::MessageType type)
{
    KLOG_DEBUG() << "auth queue: show message" << text;
    emit showMessage(text, type);
}

void AuthProxy::handleAuthQueueShowPrompt(QString text, Kiran::PromptType type, Kiran::PromptFromEnum promptFrom)
{
    KLOG_DEBUG() << "auth queue: prompt" << text;
    m_promptFrom = promptFrom;
    emit showPrompt(text, type);
}

void AuthProxy::handleAuthQueueComplete(bool authRes, QString msg)
{
    KLOG_DEBUG() << "auth queue: complete" << msg;
    stopAuthSession(m_authSessionID);
    emit authenticationComplete(authRes, msg);
}

void AuthProxy::handleAuthServiceAuthMethodChanged(int method, QString sid)
{
    if (sid != m_authSessionID)
    {
        return;
    }

    static const std::map<int, std::tuple<Kiran::AuthType, QString>> AuthTypeTransMap = {
        {SESSION_AUTH_METHOD_PASSWORD, {Kiran::AUTH_TYPE_PASSWD, "Password"}},
        {SESSION_AUTH_METHOD_FINGERPRINT, {Kiran::AUTH_TYPE_FINGER, "Fingerprint"}},
        {SESSION_AUTH_METHOD_FACE, {Kiran::AUTH_TYPE_FACE, "Face"}}};

    ///不处理认证类型为空的情况
    if (method == SESSION_AUTH_METHOD_NONE)
    {
        KLOG_ERROR() << "auth service auth method SESSION_AUTH_METHOD_NONE";
        return;
    }

    Kiran::AuthType authType;
    QString authTypeString;
    for (const auto &iter : AuthTypeTransMap)
    {
        if (method & iter.first)
        {
            Kiran::AuthType tempType;
            QString tempTypeString;
            std::tie(tempType, tempTypeString) = iter.second;
            authType = tempType | authType;
            authTypeString.append((authTypeString.isEmpty() ? "" : "|") + tempTypeString);
        }
    }

    KLOG_DEBUG() << "auth type changed:" << authTypeString << authType;
    emit authTypeChanged(authType);
}

bool AuthProxy::createAuthSession(QString &authSessionID)
{
    KLOG_DEBUG() << "create auth session";
    auto createAuthReply = m_authServiceInterface->CreateAuth();
    createAuthReply.waitForFinished();
    if (createAuthReply.isError())
    {
        KLOG_ERROR() << "create auth failed," << createAuthReply.error();
        return false;
    }
    authSessionID = createAuthReply.value();
    KLOG_DEBUG() << "create auth session finished" << authSessionID;
    return true;
}

bool AuthProxy::startAuthSession(const QString &userName, const QString &authSessionID)
{
    KLOG_DEBUG() << "start auth session" << userName << authSessionID;
    auto startAuthReply = m_authServiceInterface->StartAuth(userName,
                                                            m_authSessionID,
                                                            SESSION_AUTH_TYPE_DEFAULT,
                                                            true);
    startAuthReply.waitForFinished();
    if (startAuthReply.isError())
    {
        KLOG_ERROR() << "start auth for" << m_authInterface->authenticationUser() << startAuthReply.error();
        return false;
    }
    KLOG_DEBUG() << "start auth session finished";
    return true;
}

bool AuthProxy::stopAuthSession(const QString &authSessionID)
{
    KLOG_DEBUG() << "stop auth session" << authSessionID;

    if(authSessionID.isEmpty())
    {
        return false;
    }

    auto stopAuthReply = m_authServiceInterface->StopAuth(authSessionID);
    stopAuthReply.waitForFinished();

    if(stopAuthReply.isError())
    {
        KLOG_ERROR() << "stop auth" << authSessionID << "failed," << stopAuthReply.error();
        return false;
    }

    KLOG_DEBUG() << "stop auth session finished";
    return true;
}
