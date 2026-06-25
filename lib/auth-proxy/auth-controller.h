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
 * Author:     liuxinhao <liuxinhao@kylinsec.com.cn>
 */

#pragma once

#include <kiran-authentication-service/kas-authentication-i.h>
#include <QDateTime>
#include <QObject>
#include <QTimer>
#include "auth-base.h"

/**
 * 认证代理:对PAM认证相关和认证服务的一层封装,提供认证队列接口可以将消息存储延时发出或做相关过滤
 */
namespace Kiran
{
namespace SessionGuard
{
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

    // 是否在认证中（与底层 LightDM/PAM 状态一致）
    bool inAuthentication() const;
    // 底层 LightDM/PAM 是否仍在认证（与 inAuthentication 等价，供 UI 显式区分语义）
    bool underlyingInAuthentication() const;
    // 是否已经认证通过
    bool isAuthenticated() const;
    // 当前认证的用户
    QString authenticationUser() const;
    // 开始认证；返回 false 表示未向底层发起新会话（合并或排队等待上一轮结束）
    bool authenticate(const QString& username);
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
    // 已向底层 LightDM/PAM 发起认证
    void authenticationStarted();

    // 认证模式通知信号,开始认证过后通知
    void notifyAuthMode(KADAuthMode authMode);
    // 支持的认证类型变化信号,认证开始时经由PAM传来的认证服务消息
    void supportedAuthTypeChanged(QList<KADAuthType> authTypes);
    // 当前认证类型发生改变信号，切换界面显示,从PAM传来的认证类型变化信号
    void authTypeChanged(KADAuthType authType);

private:
    bool doAuthenticate(const QString& username);
    void tryStartPendingAuthenticate();

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

    // 会话边界与 prompt-wait（避免在无 prompt 时向 PAM 送答导致 LightDM 断言；支持用户先于 prompt 提交时暂存再转发）。
    // m_authSeq：每次 authenticate / switchAuthType 递增，用于丢弃旧会话上的排队回复、与排队内容配对。
    quint64 m_authSeq = 0;
    // m_promptsWaiting：尚未向下游转发的 UI prompt 计数，与底层一问一答对齐后才允许 respond。
    int m_promptsWaiting = 0;
    // m_hasQueuedResponse / m_queuedResponseSeq / m_queuedResponse：尚无可用 prompt 时缓存用户输入，onShowPrompt 时再 flush。
    bool m_hasQueuedResponse = false;
    quint64 m_queuedResponseSeq = 0;
    QString m_queuedResponse;

    // 当前进行中的认证轮次；用于丢弃过期 authenticationComplete。
    quint64 m_ongoingAuthSeq = 0;
    quint64 m_completedAuthSeq = 0;

    // 上一轮 PAM 未结束时排队启动的新认证
    QString m_pendingAuthenticateUser;
    QTimer* m_waitAuthEndTimer = nullptr;
    int m_waitAuthEndPollCount = 0;
};
}  // namespace SessionGuard
}  // namespace Kiran
