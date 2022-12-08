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

#ifndef KIRAN_SESSION_GUARD_LIB_AUTH_PROXY_AUTH_LIGHTDM_H_
#define KIRAN_SESSION_GUARD_LIB_AUTH_PROXY_AUTH_LIGHTDM_H_

#include "auth-base.h"
#include <QLightDM/Greeter>
#include <QSharedPointer>

/**
 * 简单的对QLightDM::Greeter进行了一层包装，只是为了给上层认证代理AuthProxy提供统一的接口
 */
class AuthLightdm : public AuthBase
{
    Q_OBJECT
public:
    explicit AuthLightdm(QSharedPointer<QLightDM::Greeter> greeterAuth, QObject* parent = nullptr);
    ~AuthLightdm() override;

    bool init() override;
    bool authenticate(const QString& userName) override;
    void cancelAuthentication() override;

    bool isAuthenticated() const override;
    bool inAuthentication() const override;
    QString authenticationUser() const override;

    void respond(const QString& response) override;

private slots:
    void handleGreeterAuthShowPrompt(QString text,QLightDM::Greeter::PromptType type);
    void handleGreeterAuthShowMessage(QString text,QLightDM::Greeter::MessageType type);
    void handleGreeterAuthComplete();

private:
    QSharedPointer<QLightDM::Greeter> m_greeterPtrAuth;

};

#endif  //KIRAN_SESSION_GUARD_LIB_AUTH_PROXY_AUTH_LIGHTDM_H_
