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
#ifndef KIRAN_SCREENSAVER_DIALOG_SRC_KIRAN_AUTH_PROXY_H_
#define KIRAN_SCREENSAVER_DIALOG_SRC_KIRAN_AUTH_PROXY_H_

#include <QObject>
#include <kiran-authentication-service/authentication_i.h>
#include "auth-base.h"
#include "auth-msg-queue-base.h"

class AuthPam;
class ComKylinsecKiranSystemDaemonAuthenticationInterface;

/**
 * 认证代理:对PAM认证相关和认证服务的一层封装,提供认证队列接口可以将消息存储延时发出或做相关过滤
 */
class Q_DECL_IMPORT AuthProxy : public QObject
{
    Q_OBJECT
public:
    Q_ENUMS(PromptType MessageType AuthType PromptFromEnum)
    explicit AuthProxy(AuthBase* authInterface, QObject* parent = nullptr);
    ~AuthProxy();

    bool init();

public:
    /// 设置认证服务的认证类型，此认证类型只在走认证服务流程时生效，直接通过PAM进行认证时不会生效
    void setSessionAuthType(SessionAuthType authType);
    /// 设置认证消息队列，可将消息排队处理或过滤
    bool setMsgQueue(AuthMsgQueueBase* msgQueue);
    /// 是否在认证中
    bool inAuthentication() const;
    /// 是否已经认证通过
    bool isAuthenticated() const;
    /// 当前认证的用户
    QString authenticationUser();

public:
    /// 开始认证
    void authenticate(const QString& username);
    /// 回复prompt类型消息
    void respond(const QString& response);
    /// 取消认证
    void cancelAuthentication();

signals:
    /// 认证类型发生改变信号，切换界面显示
    void authTypeChanged(Kiran::AuthType authType);
    /// 显示message信息
    void showMessage(QString text, Kiran::MessageType type);
    /// 显示prompt消息
    void showPrompt(QString text, Kiran::PromptType type);
    /// 认证完成结果
    void authenticationComplete(bool authRes);

private slots:
    //处理认证相关信号
    void handlePamAuthComplete();
    void handlePamAuthShowPrompt(QString text, Kiran::PromptType type);
    void handlePamAuthShowMessage(QString text, Kiran::MessageType type);

    //处理认证服务信号
    void handleAuthServiceAuthStatusChanged(QString userName, int state, QString sid);
    void handleAuthServiceAuthMessages(QString msg, int type, QString sid);
    void handleAuthServiceAuthMethodChanged(int method, const QString& sid);

    //处理认证消息队列发来的信号
    void handleAuthQueueShowMessage(QString text, Kiran::MessageType type);
    void handleAuthQueueShowPrompt(QString text, Kiran::PromptType type, Kiran::PromptFromEnum promptFrom);
    void handleAuthQueueComplete(bool authRes);


private:
    bool createAuthSession(QString& authSessionID);
    bool startAuthSession(const QString& userName, const QString& authSessionID);
    bool stopAuthSession(const QString& authSessionID);

private:
    AuthBase* m_authInterface = nullptr;
    AuthMsgQueueBase* m_authMessageQueue = nullptr;

    Kiran::PromptFromEnum m_promptFrom = Kiran::PROMPT_FROM_PAM;
    bool m_haveErrorMsg = false;

    ComKylinsecKiranSystemDaemonAuthenticationInterface* m_authServiceInterface = nullptr;
    QString m_authSessionID = "";
    SessionAuthType m_sessionAuthType = SESSION_AUTH_TYPE_TOGETHER_WITH_USER;
};

#endif  //KIRAN_SCREENSAVER_DIALOG_SRC_KIRAN_AUTH_PROXY_H_
