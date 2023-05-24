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
    KLOG_DEBUG() << "start auth" << username;

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
    RETURN_IF_FALSE(m_authInterface->inAuthentication());
    KLOG_DEBUG() << "respond";
    m_authInterface->respond(response);
}

void AuthController::cancelAuthentication()
{
    RETURN_IF_FALSE(m_authInterface->inAuthentication());
    KLOG_DEBUG() << "cancel auth";
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

    int protoID = jsonDoc[KAP_PJK_KEY_HEAD][KAP_PJK_KEY_CMD].toInt(-1);
    if (protoID == -1)
    {
        return false;
    }

    switch (protoID)
    {
    case KAP_REQ_CMD_NOTIFY_AUTH_MODE:
    {
        auto authMode = jsonDoc[KAP_PJK_KEY_BODY][KAP_PJK_KEY_AUTH_MODE].toInt(-1);
        if (authMode < KAD_AUTH_MODE_NONE || authMode > KAD_AUTH_MODE_LAST)
        {
            KLOG_WARNING() << "invalid auth mode" << authMode;
            return false;
        }
        onNotifyAuthMode((KADAuthMode)authMode);
        return true;
    }
    case KAP_REQ_CMD_LOGIN_USER_SWITCHABLE:
    {
        onRequestLoginUserSwitchable();
        return true;
    }
    case KAP_REQ_CMD_NOTIFY_SUPPORT_AUTH_TYPE:
    {
        auto authTypesArray = jsonDoc[KAP_PJK_KEY_BODY][KAP_PJK_KEY_AUTH_TYPES].toArray(QJsonArray());
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

        onNotifySupportAuthType(supportAuthTypes);
        return true;
    }
    case KAP_REQ_CMD_AUTH_TYPE:
    {
        onRequestAuthType();
        return true;
    }
    case KAP_REQ_CMD_NOTIFY_AUTH_TYPE:
    {
        auto authType = jsonDoc[KAP_PJK_KEY_BODY][KAP_PJK_KEY_AUTH_TYPE].toInt(-1);
        if (authType <= KAD_AUTH_TYPE_NONE || authType >= KAD_AUTH_TYPE_LAST)
        {
            KLOG_WARNING() << "invalid auth types";
            return false;
        }
        onNotifyAuthType((KADAuthType)authType);
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
    respond(jsonRspDoc.toJson(QJsonDocument::Compact));
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
    respond(jsonRspDoc.toJson(QJsonDocument::Compact));
}

void AuthController::onAuthComplete()
{
    KLOG_DEBUG() << "auth controller auth complete";

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
        if (!processAuthDaemonCommand(text))
        {
            KLOG_WARNING() << "Error processing authentication service command" << text;
        }
        return;
    }

    KLOG_DEBUG() << "auth controller prompt:" << type << text;
    emit showPrompt(text, type);
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