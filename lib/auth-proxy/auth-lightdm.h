//
// Created by lxh on 2021/8/6.
//

#ifndef KIRAN_SESSION_GUARD_LIB_AUTH_PROXY_AUTH_LIGHTDM_H_
#define KIRAN_SESSION_GUARD_LIB_AUTH_PROXY_AUTH_LIGHTDM_H_

#include "auth-base.h"
#include <QLightDM/Greeter>

class AuthLightdm : public AuthBase
{
    Q_OBJECT
public:
    explicit AuthLightdm(QLightDM::Greeter* greeterAuth, QObject* parent = nullptr);
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
    QLightDM::Greeter* m_greeterAuth;

};

#endif  //KIRAN_SESSION_GUARD_LIB_AUTH_PROXY_AUTH_LIGHTDM_H_
