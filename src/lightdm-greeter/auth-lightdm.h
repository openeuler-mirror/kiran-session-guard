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

#include <QLightDM/Greeter>
#include <QSharedPointer>
#include "auth-base.h"

/**
 * 简单的对QLightDM::Greeter进行了一层包装，只是为了给上层认证代理AuthProxy提供统一的接口
 */
namespace Kiran
{
namespace SessionGuard
{
namespace Greeter
{
class AuthLightdm : public QObject, public AuthBase
{
    Q_OBJECT
public:
    explicit AuthLightdm(QSharedPointer<QLightDM::Greeter> greeterAuth, QObject* parent = nullptr);
    ~AuthLightdm() override;

    bool init(AuthControllerInterface* controllerInterface) override;

    bool loginUserSwitchable() override { return true; };

    bool authenticate(const QString& userName) override;
    void cancelAuthentication() override;

    bool isAuthenticated() const override;
    bool inAuthentication() const override;
    QString authenticationUser() const override;

    void respond(const QString& response) override;

private slots:
    void onGreeterAuthShowPrompt(QString text, QLightDM::Greeter::PromptType type);
    void onGreeterAuthShowMessage(QString text, QLightDM::Greeter::MessageType type);
    void onGreeterAuthComplete();

private:
    AuthControllerInterface* m_interface = nullptr;
    QSharedPointer<QLightDM::Greeter> m_greeterPtr;
};
}  // namespace Greeter
}  // namespace SessionGuard
}  // namespace Kiran