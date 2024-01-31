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
#pragma once
#include <PolkitQt1/Identity>
#include "auth-base.h"

namespace PolkitQt1
{
namespace Agent
{
class Session;
}
}  // namespace PolkitQt1

namespace Kiran
{
namespace SessionGuard
{
class AuthControllerInterface;
}
}  // namespace Kiran

namespace Kiran
{
namespace SessionGuard
{
namespace PolkitAgent
{
class AuthPolkit : public QObject, public AuthBase
{
    Q_OBJECT
public:
    AuthPolkit(const QString& cookie);
    ~AuthPolkit();

    bool init(AuthControllerInterface* controllerInterface);
    bool authenticate(const QString& userName) override;
    void respond(const QString& response) override;
    bool inAuthentication() const override;
    bool isAuthenticated() const override;
    QString authenticationUser() const override;
    void cancelAuthentication() override;

private slots:
    void handleSessionCompleted(bool gainedAuthorization);
    void handleSessionRequest(const QString& request, bool echo);
    void handleSessionShowError(const QString& text);
    void handleSessionShowInfo(const QString& text);

private:
    AuthControllerInterface* m_controllerInterface = nullptr;
    QString m_cookie;
    PolkitQt1::Agent::Session* m_session = nullptr;
    PolkitQt1::Identity m_identity;
    bool m_inAuth = false;
    bool m_gainedAuthorization = false;
};
}  // namespace PolkitAgent
}  // namespace SessionGuard
}  // namespace Kiran