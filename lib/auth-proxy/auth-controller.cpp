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

#include "auth-controller.h"
#include "auxiliary.h"
#include "kiran_authentication.h"

#include <qt5-log-i.h>
#include <QDBusConnection>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>

namespace Kiran
{
namespace SessionGuard
{
AuthController::AuthController(QObject* parent)
    : QObject(parent)
{
    qRegisterMetaType<PromptType>("PromptType");
    qRegisterMetaType<MessageType>("MessageType");
    qRegisterMetaType<KADAuthType>("KADAuthType");
}

AuthController::~AuthController()
{
    m_authInterface->cancelAuthentication();
    delete m_authInterface;
}

bool AuthController::init(AuthBase* authInterface)
{
    RETURN_VAL_IF_TRUE(m_isInited, true);
    RETURN_VAL_IF_FALSE(authInterface->init(this), false);
    m_authInterface = authInterface;
    m_isInited = true;
    return true;
}

bool AuthController::isValid() const
{
    return m_isInited;
}

bool AuthController::inAuthentication() const
{
    RETURN_VAL_IF_FALSE(isValid(), false);
    return m_authInterface->inAuthentication();
}

bool AuthController::isAuthenticated() const
{
    RETURN_VAL_IF_FALSE(isValid(), false);
    return m_authInterface->isAuthenticated();
}

QString AuthController::authenticationUser() const
{
    RETURN_VAL_IF_FALSE(isValid(), "");
    return m_userName;
}

void AuthController::authenticate(const QString& username)
{
    ++m_authSeq;

    KLOG_INFO() << "AuthController: authenticate requested"
                << "user=" << username
                << "seq=" << m_authSeq;
    m_promptsWaiting = 0;
    m_hasQueuedResponse = false;
    m_queuedResponseSeq = 0;
    m_queuedResponse.clear();

    m_haveErrorMsg = false;
    m_canSwitchAuthType = false;
    m_supportedAuthType.clear();
    m_userName = username;
    m_currentAuthType = KAD_AUTH_TYPE_NONE;

    if( username != m_userName )
    {
        m_specifyAuthType = KAD_AUTH_TYPE_NONE;
    }

    m_authInterface->authenticate(username);
}

void AuthController::respond(const QString& response)
{
    const bool authReady = m_authInterface->inAuthentication();
    const bool promptWaiting = m_promptsWaiting > 0;
    if (!authReady || !promptWaiting)
    {
        m_hasQueuedResponse = true;
        m_queuedResponseSeq = m_authSeq;
        m_queuedResponse = response;
        KLOG_INFO() << "AuthController: respond queued"
                    << "authReady=" << authReady
                    << "promptWaiting=" << promptWaiting
                    << "seq=" << m_queuedResponseSeq
                    << "len=" << response.size();
        return;
    }

    KLOG_INFO() << "AuthController: respond forwarded"
                << "seq=" << m_authSeq
                << "len=" << response.size()
                << "promptsWaitingBefore=" << m_promptsWaiting;
    m_promptsWaiting = qMax(0, m_promptsWaiting - 1);
    m_hasQueuedResponse = false;
    m_queuedResponseSeq = 0;
    m_queuedResponse.clear();
    m_authInterface->respond(response);
}

void AuthController::cancelAuthentication()
{
    RETURN_IF_FALSE(m_authInterface->inAuthentication());
    KLOG_DEBUG() << "cancel auth";
    m_promptsWaiting = 0;
    m_hasQueuedResponse = false;
    m_queuedResponseSeq = 0;
    m_queuedResponse.clear();
    m_authInterface->cancelAuthentication();
}

bool AuthController::canSwitchAuthType()
{
    return m_canSwitchAuthType;
}

void AuthController::switchAuthType(KADAuthType authType)
{
    RETURN_IF_FALSE(m_canSwitchAuthType);
    RETURN_IF_FALSE(m_supportedAuthType.contains(authType));
    RETURN_IF_FALSE(m_currentAuthType != authType);

    KLOG_INFO() << "AuthController: switchAuthType requested"
                << "authType=" << (int)authType
                << "inAuth=" << inAuthentication()
                << "currentAuthType=" << (int)m_currentAuthType
                << "seqBefore=" << m_authSeq;

    ++m_authSeq;
    KLOG_INFO() << "AuthController: switchAuthType seq start"
                << "seq=" << m_authSeq;
    m_promptsWaiting = 0;
    m_hasQueuedResponse = false;
    m_queuedResponseSeq = 0;
    m_queuedResponse.clear();

    m_specifyAuthType = authType;
    if (inAuthentication())
    {
        cancelAuthentication();
    }

    m_haveErrorMsg = false;
    // 调用AuthBase的开始认证接口，不调用AuthController::authenticate,缓存一些上次认证的值
    KLOG_DEBUG() << "switch auth type reauth:" << m_authInterface->authenticate(m_userName);
}

KADAuthType AuthController::currentAuthType()
{
    return m_currentAuthType;
}

QList<KADAuthType> AuthController::getSupportedAuthType()
{
    return m_supportedAuthType;
}

bool AuthController::isAuthDaemonCommand(const QString& msg)
{
    return msg.startsWith(KAP_PROTO_JSON_PREFIX);
}

bool AuthController::processAuthDaemonCommand(const QString& msg)
{
    auto cmdStr = msg.midRef(strlen(KAP_PROTO_JSON_PREFIX), -1);
    QJsonDocument jsonDoc = QJsonDocument::fromJson(cmdStr.toUtf8());

#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    int protoID = jsonDoc[KAP_PJK_KEY_HEAD][KAP_PJK_KEY_CMD].toInt(-1);
#else
    QJsonValue val = jsonDoc.object()[KAP_PJK_KEY_HEAD];
    int protoID = val.toObject()[KAP_PJK_KEY_CMD].toInt(-1);
#endif
    if (protoID == -1)
    {
        return false;
    }

    KLOG_INFO() << "AuthController: daemonCmd recv"
                << "protoID=" << protoID
                << "seq=" << m_authSeq;

    switch (protoID)
    {
    case KAP_REQ_CMD_NOTIFY_AUTH_MODE:
    {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
        auto authMode = jsonDoc[KAP_PJK_KEY_BODY][KAP_PJK_KEY_AUTH_MODE].toInt(-1);
#else
        QJsonValue val = jsonDoc.object()[KAP_PJK_KEY_BODY];
        auto authMode = val.toObject()[KAP_PJK_KEY_AUTH_MODE].toInt(-1);
#endif
        KLOG_INFO() << "AuthController: daemonCmd notify authMode"
                    << "authMode=" << authMode
                    << "seq=" << m_authSeq;
        if (authMode < KAD_AUTH_MODE_NONE || authMode > KAD_AUTH_MODE_LAST)
        {
            KLOG_WARNING() << "invalid auth mode" << authMode;
            return false;
        }
        onNotifyAuthMode((KADAuthMode)authMode);
        KLOG_INFO() << "AuthController: daemonCmd handled"
                    << "protoID=" << protoID;
        return true;
    }
    case KAP_REQ_CMD_LOGIN_USER_SWITCHABLE:
    {
        onRequestLoginUserSwitchable();
        KLOG_INFO() << "AuthController: daemonCmd handled"
                    << "protoID=" << protoID;
        return true;
    }
    case KAP_REQ_CMD_NOTIFY_SUPPORT_AUTH_TYPE:
    {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
        auto authTypesArray = jsonDoc[KAP_PJK_KEY_BODY][KAP_PJK_KEY_AUTH_TYPES].toArray(QJsonArray());
#else
        QJsonValue val = jsonDoc.object()[KAP_PJK_KEY_BODY];
        const QJsonObject object =  val.toObject();
        auto authTypesArray = object[KAP_PJK_KEY_AUTH_TYPES].toArray(QJsonArray());
#endif
        if (authTypesArray.isEmpty())
        {
            KLOG_WARNING() << "invalid auth types";
            return false;
        }

        QList<KADAuthType> supportAuthTypes;
        auto varList = authTypesArray.toVariantList();
        for (const auto& var : varList)
        {
            bool toIntOk = false;
            int authType = var.toInt(&toIntOk);
            if (!toIntOk || authType <= KAD_AUTH_TYPE_NONE || authType >= KAD_AUTH_TYPE_LAST)
            {
                KLOG_WARNING() << "invalid auth types";
                return false;
            }
            supportAuthTypes << (KADAuthType)authType;
        }
        KLOG_INFO() << "AuthController: daemonCmd notify supportAuthTypes"
                    << "count=" << supportAuthTypes.size()
                    << "types=" << supportAuthTypes
                    << "seq=" << m_authSeq;

        onNotifySupportAuthType(supportAuthTypes);
        KLOG_INFO() << "AuthController: daemonCmd handled"
                    << "protoID=" << protoID;
        return true;
    }
    case KAP_REQ_CMD_AUTH_TYPE:
    {
        onRequestAuthType();
        KLOG_INFO() << "AuthController: daemonCmd handled"
                    << "protoID=" << protoID;
        return true;
    }
    case KAP_REQ_CMD_NOTIFY_AUTH_TYPE:
    {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
        auto authType = jsonDoc[KAP_PJK_KEY_BODY][KAP_PJK_KEY_AUTH_TYPE].toInt(-1);
#else
        QJsonValue val = jsonDoc.object()[KAP_PJK_KEY_BODY];
        auto authType = val.toObject()[KAP_PJK_KEY_AUTH_TYPES].toInt(-1);
#endif
        if (authType <= KAD_AUTH_TYPE_NONE || authType >= KAD_AUTH_TYPE_LAST)
        {
            KLOG_WARNING() << "invalid auth types";
            return false;
        }
        onNotifyAuthType((KADAuthType)authType);
        KLOG_INFO() << "AuthController: daemonCmd handled"
                    << "protoID=" << protoID;
        return true;
    }
    default:
        KLOG_WARNING() << "Unknown protocol command id:" << protoID;
        break;
    }
    return false;
}

void AuthController::onNotifyAuthMode(KADAuthMode mode)
{
    KLOG_DEBUG() << "notify auth mode" << mode;
    m_canSwitchAuthType = mode == KAD_AUTH_MODE_OR;
    emit notifyAuthMode(mode);
}

void AuthController::onRequestLoginUserSwitchable()
{
    QJsonDocument jsonRspDoc{QJsonObject{{KAP_PJK_KEY_BODY, QJsonObject{{KAP_PJK_KEY_LOGIN_USER_SWITCHABLE, m_authInterface->loginUserSwitchable()}}}}};
    // 协议应答必须立即送达，不能走 prompt-wait 排队（否则 OR 模式/支持类型通知不到）。
    RETURN_IF_FALSE(m_authInterface->inAuthentication());
    KLOG_INFO() << "AuthController: daemonCmd respond loginUserSwitchable"
                << "seq=" << m_authSeq;
    m_authInterface->respond(jsonRspDoc.toJson(QJsonDocument::Compact));
}

void AuthController::onNotifySupportAuthType(QList<KADAuthType> authTypes)
{
    KLOG_DEBUG() << "notify support auth type:" << authTypes;
    if ( m_canSwitchAuthType 
        /*&& m_specifyAuthType == KAD_AUTH_TYPE_NONE*/ )
    {
        m_supportedAuthType = authTypes;

        // 指定的认证类型，已不在最新的认证列表之中，更新为默认值
        if( m_specifyAuthType != KAD_AUTH_TYPE_NONE )
        {
            if( !m_supportedAuthType.contains(m_specifyAuthType) )
            {
                m_specifyAuthType = KAD_AUTH_TYPE_NONE;
            }
        }
        
        emit supportedAuthTypeChanged(m_supportedAuthType);
    }
}

void AuthController::onNotifyAuthType(KADAuthType authType)
{
    KLOG_DEBUG() << "notify auth type:" << authType;
    m_currentAuthType = authType;
    emit authTypeChanged(m_currentAuthType);
}

void AuthController::onRequestAuthType()
{
    KADAuthType specifyAuthType = KAD_AUTH_TYPE_PASSWORD;
    if (m_specifyAuthType != KAD_AUTH_TYPE_NONE)
    {
        specifyAuthType = m_specifyAuthType;
    }
    else if (!m_supportedAuthType.isEmpty())
    {
        specifyAuthType = m_supportedAuthType.first();
    }
    KLOG_DEBUG() << "request auth type:" << specifyAuthType;
    QJsonDocument jsonRspDoc{QJsonObject{{KAP_PJK_KEY_BODY, QJsonObject{{KAP_PJK_KEY_AUTH_TYPE, specifyAuthType}}}}};
    // 协议应答必须立即送达，不能走 prompt-wait 排队。
    RETURN_IF_FALSE(m_authInterface->inAuthentication());
    KLOG_INFO() << "AuthController: daemonCmd respond authType"
                << "specifyAuthType=" << (int)specifyAuthType
                << "seq=" << m_authSeq;
    m_authInterface->respond(jsonRspDoc.toJson(QJsonDocument::Compact));
}

void AuthController::onAuthComplete()
{
    KLOG_INFO() << "AuthController: authComplete"
                << "seq=" << m_authSeq
                << "success=" << m_authInterface->isAuthenticated();

    // 认证完成并且失败时，检查认证过程中是否存在过错误消息
    // 如果没存在过错误消息，编造一个错误消息
    QString authResultDesc;
    auto success = m_authInterface->isAuthenticated();
    if (!success)
    {
        if (!m_haveErrorMsg)
        {
            authResultDesc = tr("Failed to authenticate");
        }
    }

    if (!authResultDesc.isEmpty())
    {
        emit showMessage(authResultDesc, MessageTypeError);
    }

    emit authenticationComplete(m_authInterface->isAuthenticated());
}

void AuthController::onShowPrompt(const QString& text, PromptType type)
{
    if (type == PromptTypeQuestion && isAuthDaemonCommand(text))
    {
        KLOG_INFO() << "AuthController: onShowPrompt daemonCmd"
                    << "seq=" << m_authSeq;
        if (!processAuthDaemonCommand(text))
        {
            KLOG_WARNING() << "Error processing authentication service command" << text;
        }
        return;
    }

    KLOG_INFO() << "AuthController: onShowPrompt"
                << "type=" << (int)type
                << "inAuth=" << inAuthentication()
                << "promptsWaitingBefore=" << m_promptsWaiting
                << "seq=" << m_authSeq;
    m_promptsWaiting++;
    emit showPrompt(text, type);

    if (m_hasQueuedResponse && m_queuedResponseSeq == m_authSeq && m_promptsWaiting > 0)
    {
        const auto seq = m_queuedResponseSeq;
        QTimer::singleShot(0, this, [this, seq]() {
            if (!m_hasQueuedResponse || m_queuedResponseSeq != seq)
            {
                return;
            }
            if (!m_authInterface || !m_authInterface->inAuthentication())
            {
                return;
            }
            if (m_promptsWaiting <= 0)
            {
                return;
            }
            KLOG_INFO() << "AuthController: respond flush on prompt"
                        << "seq=" << seq
                        << "len=" << m_queuedResponse.size()
                        << "promptsWaitingBefore=" << m_promptsWaiting;
            const QString rsp = m_queuedResponse;
            m_promptsWaiting = qMax(0, m_promptsWaiting - 1);
            m_hasQueuedResponse = false;
            m_queuedResponseSeq = 0;
            m_queuedResponse.clear();
            m_authInterface->respond(rsp);
        });
    }
}

void AuthController::onShowMessage(const QString& text, MessageType type)
{
    if (type == MessageTypeInfo && isAuthDaemonCommand(text))
    {
        if (!processAuthDaemonCommand(text))
        {
            KLOG_WARNING() << "Error processing authentication service command" << text;
        }
        return;
    }

    if (type == MessageTypeError)
    {
        m_haveErrorMsg = true;
    }

    KLOG_DEBUG() << "auth controller message:" << type << text;
    emit showMessage(text, type);
}
}  // namespace SessionGuard
}  // namespace Kiran