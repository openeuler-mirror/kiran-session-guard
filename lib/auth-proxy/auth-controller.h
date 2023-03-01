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

#pragma once

#include <QObject>
#include "auth-base.h"
#include <kiran-authentication-service/kas-authentication-i.h>

/**
 * 认证代理:对PAM认证相关和认证服务的一层封装,提供认证队列接口可以将消息存储延时发出或做相关过滤
 */
GUARD_BEGIN_NAMESPACE
class Q_DECL_IMPORT AuthController : public QObject, public AuthControllerInterface
{
    Q_OBJECT
public:
    Q_ENUMS(PromptType MessageType KADAuthType PromptFromEnum)
    explicit AuthController(QObject* parent = nullptr);
    ~AuthController();

    // 传入底层认证接口初始化,初始化成功AuthBase生存周期将由AuthController接管,外部不释放!!
    bool init(AuthBase* authInterface);
    bool isValid() const;

    // 是否在认证中
    bool inAuthentication() const;
    // 是否已经认证通过
    bool isAuthenticated() const;
    // 当前认证的用户
    QString authenticationUser() const;
    // 开始认证
    void authenticate(const QString& username);
    // 回复prompt类型消息
    void respond(const QString& response);
    // 取消认证
    void cancelAuthentication();

    // 是否能切换认证类型
    bool canSwitchAuthType();
    // 切换认证类型
    void switchAuthType(KADAuthType authType);

    KADAuthType currentAuthType();
    QList<KADAuthType> getSupportedAuthType();
    
signals:
    // 显示message信息
    void showMessage(QString text, MessageType type);
    // 显示prompt消息
    void showPrompt(QString text, PromptType type);
    // 认证完成结果
    void authenticationComplete(bool authRes);

    // 认证模式通知信号,开始认证过后通知
    void notifyAuthMode(KADAuthMode authMode);
    // 支持的认证类型变化信号,认证开始时经由PAM传来的认证服务消息
    void supportedAuthTypeChanged(QList<KADAuthType> authTypes);
    // 当前认证类型发生改变信号，切换界面显示,从PAM传来的认证类型变化信号
    void authTypeChanged(KADAuthType authType);

private:
    bool isAuthDaemonCommand(const QString& msg);
    bool processAuthDaemonCommand(const QString& msg);
    void onNotifyAuthMode(KADAuthMode mode);
    void onRequestLoginUserSwitchable();
    void onNotifySupportAuthType(QList<KADAuthType> authTypes);
    void onNotifyAuthType(KADAuthType authType);
    void onRequestAuthType();

    virtual void onShowMessage(const QString& text, MessageType type) override;
    virtual void onShowPrompt(const QString& text, PromptType type) override;
    virtual void onAuthComplete() override;

private:
    AuthBase* m_authInterface = nullptr;

    bool m_isInited = false;
    QString m_userName;
    bool m_haveErrorMsg = false;
    bool m_canSwitchAuthType = false;
    KADAuthType m_specifyAuthType = KAD_AUTH_TYPE_NONE;
    KADAuthType m_currentAuthType = KAD_AUTH_TYPE_NONE;
    QList<KADAuthType> m_supportedAuthType;
};
GUARD_END_NAMESPACE
