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


#include "auth-lightdm.h"
#include <qt5-log-i.h>

AuthLightdm::AuthLightdm(QLightDM::Greeter* greeterAuth, QObject* parent)
    : AuthBase(parent),
      m_greeterAuth(greeterAuth)
{
    connect(m_greeterAuth,&QLightDM::Greeter::showPrompt,this,&AuthLightdm::handleGreeterAuthShowPrompt);
    connect(m_greeterAuth,&QLightDM::Greeter::showMessage,this,&AuthLightdm::handleGreeterAuthShowMessage);
    connect(m_greeterAuth,&QLightDM::Greeter::authenticationComplete, this,&AuthLightdm::handleGreeterAuthComplete);
}

AuthLightdm::~AuthLightdm()
{
}

bool AuthLightdm::init()
{
    bool bRes = m_greeterAuth->connectSync();
    if( !bRes )
    {
        KLOG_ERROR() << "can't connect greeter auth!";
    }
    return bRes;
}

bool AuthLightdm::authenticate(const QString &userName)
{
    m_greeterAuth->authenticate(userName);
    return true;
}

void AuthLightdm::cancelAuthentication()
{
    m_greeterAuth->cancelAuthentication();
}

bool AuthLightdm::isAuthenticated() const
{
    return m_greeterAuth->isAuthenticated();
}

bool AuthLightdm::inAuthentication() const
{
    return m_greeterAuth->inAuthentication();
}

QString AuthLightdm::authenticationUser() const
{
    return m_greeterAuth->authenticationUser();
}

void AuthLightdm::respond(const QString &response)
{
    m_greeterAuth->respond(response);
}

void AuthLightdm::handleGreeterAuthShowPrompt(QString text, QLightDM::Greeter::PromptType type)
{
    emit AuthBase::showPrompt(text,type==QLightDM::Greeter::PromptTypeSecret?Kiran::PromptTypeSecret:Kiran::PromptTypeQuestion);
}

void AuthLightdm::handleGreeterAuthShowMessage(QString text, QLightDM::Greeter::MessageType type)
{
    emit AuthBase::showMessage(text,type==QLightDM::Greeter::MessageTypeInfo?Kiran::MessageTypeInfo:Kiran::MessageTypeError);
}

void AuthLightdm::handleGreeterAuthComplete()
{
    emit AuthBase::authenticationComplete();
}
