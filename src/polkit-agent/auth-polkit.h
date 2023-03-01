#pragma once
#include "auth-base.h"
#include <PolkitQt1/Identity>

namespace PolkitQt1
{
    namespace Agent
    {
        class Session;
    }
}

GUARD_BEGIN_NAMESPACE
class AuthControllerInterface;
GUARD_END_NAMESPACE

GUARD_POLKIT_AGENT_BEGIN_NAMESPACE
class AuthPolkit : public QObject,public AuthBase
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
    AuthControllerInterface* m_controllerInterface;
    QString m_cookie;
    PolkitQt1::Agent::Session* m_session = nullptr;
    PolkitQt1::Identity m_identity;
    bool m_inAuth = false;
    bool m_gainedAuthorization = false;
};
GUARD_POLKIT_AGENT_END_NAMESPACE