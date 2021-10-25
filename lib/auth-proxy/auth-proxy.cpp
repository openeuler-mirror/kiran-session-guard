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

#include "auth-proxy.h"
#include "auth-lightdm.h"
#include "auth-msg-queue.h"
#include "auth-pam.h"
#include "kiran_authentication.h"

#include <qt5-log-i.h>
#include <QDBusConnection>
//kiran-biometrics头文件
#include <kiran-pam-msg.h>

AuthProxy::AuthProxy(AuthBase *authInterface, QObject *parent)
    : QObject(parent),
      m_authInterface(authInterface)
{
}

AuthProxy::~AuthProxy()
{
    m_authInterface->cancelAuthentication();
}

bool AuthProxy::init()
{
    if (m_authInterface == nullptr || !m_authInterface->init())
    {
        KLOG_DEBUG() << "auth interface init failed";
        return false;
    }

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
        KLOG_FATAL("connect to authproxy service signal failed!");
    }

    if (!connect(m_authInterface, &AuthBase::showMessage, this, &AuthProxy::handlePamAuthShowMessage) ||
        !connect(m_authInterface, &AuthBase::authenticationComplete, this, &AuthProxy::handlePamAuthComplete) ||
        !connect(m_authInterface, &AuthBase::showPrompt, this, &AuthProxy::handlePamAuthShowPrompt))
    {
        KLOG_FATAL("connect to authproxy interface signal failed!");
    }

    return true;
}

void AuthProxy::setSessionAuthType(SessionAuthType authType)
{
    m_sessionAuthType = authType;
}

bool AuthProxy::setMsgQueue(AuthMsgQueueBase *msgQueue)
{
    if (inAuthentication())
    {
        KLOG_ERROR() << "in authentication,can't set message queue!";
        return false;
    }

    if (m_authMessageQueue)
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

QString AuthProxy::authenticationUser()
{
    return m_authInterface->authenticationUser();
}

void AuthProxy::authenticate(const QString &username)
{
    KLOG_DEBUG() << "authproxy proxy authenticate:" << username;

    // 重置消息队列
    if (m_authMessageQueue)
    {
        m_authMessageQueue->stopDispatcher();
        m_authMessageQueue->startDispatcher();
    }

    // 重置标志位
    m_haveErrorMsg = false;

    // 开始进行PAM认证
    m_authInterface->authenticate(username);
}

void AuthProxy::respond(const QString &response)
{
    if (m_promptFrom == Kiran::PROMPT_FROM_AUTH_SERVICE)
    {
        KLOG_DEBUG() << "respond to authproxy service";
        unsigned char *encrypted = nullptr;

        int length = kiran_authentication_rsa_public_encrypt((char *)response.toStdString().c_str(),
                                                             response.length(),
                                                             (unsigned char *)m_authPublicKey.data(),
                                                             &encrypted);
        QByteArray encryptedArray((char *)encrypted, length);
        free(encrypted);

        QString base64temp = encryptedArray.toBase64();
        m_authServiceInterface->ResponseMessage(base64temp, m_authSessionID);
    }
    else
    {
        KLOG_DEBUG() << "respond to authproxy";
        m_authInterface->respond(response);
    }
}

void AuthProxy::cancelAuthentication()
{
    KLOG_DEBUG() << "cancel authentication";
    m_authInterface->cancelAuthentication();
    if (m_authMessageQueue)
        m_authMessageQueue->stopDispatcher();
    stopAuthSession(m_authSessionID);
}

void AuthProxy::handlePamAuthComplete()
{
    KLOG_DEBUG() << "handle auth interface complete";
    //关闭认证服务，认证session
    if (!m_authSessionID.isEmpty())
    {
        auto stopAuthReply = m_authServiceInterface->StopAuth(m_authSessionID);
        stopAuthReply.waitForFinished();
        if (stopAuthReply.isError())
        {
            KLOG_ERROR() << "stop auth service session failed," << stopAuthReply.error();
        }
        m_authSessionID.clear();
    }

    //认证完成并且失败时，检查认证过程中是否存在过错误消息
    //如果没存在过错误消息，编造一个错误消息
    QString authResultDesc;
    if (!m_authInterface->isAuthenticated())
    {
        if (!m_haveErrorMsg)
        {
            authResultDesc = tr("Failed to authenticate");
        }
    }

    if (m_authMessageQueue != nullptr)
    {
        KLOG_DEBUG() << "add authproxy complete msg to queue";
        if (!authResultDesc.isEmpty())
        {
            m_authMessageQueue->append(Kiran::MessageTypeError, authResultDesc);
        }
        m_authMessageQueue->appendAuthCompleteMsg(m_authInterface->isAuthenticated());
    }
    else
    {
        KLOG_DEBUG() << "authproxy complete";
        stopAuthSession(m_authSessionID);
        if (!authResultDesc.isEmpty())
        {
            emit showMessage(authResultDesc, Kiran::MessageTypeError);
        }
        emit authenticationComplete(m_authInterface->isAuthenticated());
    }
}

void AuthProxy::handlePamAuthShowPrompt(QString text, Kiran::PromptType type)
{
    KLOG_DEBUG() << "auth interface prompt:" << type << text;

    if (text == ASK_AUTH_SID)
    {
        KLOG_DEBUG() << "handle authproxy ask authproxy sid";
        //create authproxy
        QString authSessionID;
        QByteArray authPublicKey;
        if (!createAuthSession(authSessionID, authPublicKey))
        {
            KLOG_ERROR() << "can't create authproxy session for" << m_authInterface->authenticationUser();
            m_authInterface->respond("");
            return;
        }

        KLOG_DEBUG() << "created authproxy session:" << authSessionID;

        //respond to pam
        m_authSessionID = authSessionID;
        m_authPublicKey = authPublicKey;
        m_authInterface->respond(m_authSessionID);

        //start authproxy
        if (!startAuthSession(m_authInterface->authenticationUser(), m_authSessionID))
        {
            KLOG_ERROR() << "can't start authproxy session:" << m_authSessionID;
        }
        return;
    }
    else if (text == ASK_FACE)  ///兼容老版本PAM直接通过prompt消息通知
    {
        KLOG_DEBUG() << "handle authproxy ask face";
        emit authTypeChanged(Kiran::AUTH_TYPE_FACE);
        m_authInterface->respond(REP_FACE);
        return;
    }
    else if (text == ASK_FPINT)  ///兼容老版本PAM直接通过prompt消息通知
    {
        KLOG_DEBUG() << "handle authproxy ask fingerprint";
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
    KLOG_DEBUG() << "auth interface message:" << type << text;
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

    KLOG_DEBUG() << "authproxy status changed:" << userName
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
    KLOG_DEBUG() << "authproxy queue: show message" << text;
    emit showMessage(text, type);
}

void AuthProxy::handleAuthQueueShowPrompt(QString text, Kiran::PromptType type, Kiran::PromptFromEnum promptFrom)
{
    KLOG_DEBUG() << "authproxy queue: prompt" << text;
    m_promptFrom = promptFrom;
    emit showPrompt(text, type);
}

void AuthProxy::handleAuthQueueComplete(bool authRes)
{
    KLOG_DEBUG() << "authproxy queue: complete";
    stopAuthSession(m_authSessionID);
    emit authenticationComplete(authRes);
}

void AuthProxy::handleAuthServiceAuthMethodChanged(int method, const QString &sid)
{
    if (sid != m_authSessionID)
    {
        return;
    }

    // clang-format off
    //map<SessionAuthMethod,tuple<AuthType,AuthTypeDesc>>
    const std::map<int, std::tuple<Kiran::AuthType, QString>> AuthTypeTransMap = {
        {SESSION_AUTH_METHOD_PASSWORD, {Kiran::AUTH_TYPE_PASSWD, QStringLiteral("Password"),}},
        {SESSION_AUTH_METHOD_FINGERPRINT, {Kiran::AUTH_TYPE_FINGER, QStringLiteral("Fingerprint")}},
        {SESSION_AUTH_METHOD_FACE, {Kiran::AUTH_TYPE_FACE, QStringLiteral("Face")}}
    };
    // clang-format on

    ///不处理认证类型为空的情况
    if (method == SESSION_AUTH_METHOD_NONE)
    {
        KLOG_ERROR() << "authproxy service authproxy method SESSION_AUTH_METHOD_NONE";
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

    KLOG_DEBUG() << "authproxy type changed:" << authTypeString << authType;
    emit authTypeChanged(authType);
}

bool AuthProxy::createAuthSession(QString &authSessionID, QByteArray &authPKey)
{
    KLOG_DEBUG() << "create authproxy session";
    auto createAuthReply = m_authServiceInterface->CreateAuth();
    createAuthReply.waitForFinished();
    if (createAuthReply.isError())
    {
        KLOG_ERROR() << "create authproxy failed," << createAuthReply.error();
        return false;
    }

    authSessionID = createAuthReply.argumentAt(0).toString();

    QString tempStr = createAuthReply.argumentAt(1).toString();
    authPKey = QByteArray::fromBase64(tempStr.toUtf8());

    KLOG_DEBUG() << "create authproxy session finished"
                 << "\n\tauth session id:" << authSessionID
                 << "\n\tauth public key:" << authPKey;
    return true;
}

bool AuthProxy::startAuthSession(const QString &userName, const QString &authSessionID)
{
    KLOG_DEBUG() << "start authproxy session" << userName << authSessionID;
    auto startAuthReply = m_authServiceInterface->StartAuth(userName,
                                                            m_authSessionID,
                                                            m_sessionAuthType,
                                                            true);
    startAuthReply.waitForFinished();
    if (startAuthReply.isError())
    {
        KLOG_ERROR() << "start authproxy for" << m_authInterface->authenticationUser() << startAuthReply.error();
        return false;
    }
    KLOG_DEBUG() << "start authproxy session finished";
    return true;
}

bool AuthProxy::stopAuthSession(QString &authSessionID)
{
    bool bRet = true;

    KLOG_DEBUG() << "cancelAndStop authproxy session" << authSessionID;

    if (authSessionID.isEmpty())
    {
        return false;
    }

    auto stopAuthReply = m_authServiceInterface->StopAuth(authSessionID);
    stopAuthReply.waitForFinished();
    if (stopAuthReply.isError())
    {
        bRet = false;
        KLOG_ERROR() << "cancelAndStop authproxy" << authSessionID << "failed," << stopAuthReply.error();
    }

    KLOG_DEBUG() << "cancelAndStop authproxy session finished";
    authSessionID.clear();
    return bRet;
}