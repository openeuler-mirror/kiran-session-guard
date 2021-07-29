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
 
#ifndef PAM_AUTH_SRC_PAM_AUTH_H_
#define PAM_AUTH_SRC_PAM_AUTH_H_

#include <QObject>
#include "auth-base.h"

class QProcess;
class QSocketNotifier;
class AuthPam : public AuthBase
{
    Q_OBJECT
public:
    explicit AuthPam(QObject* = nullptr);
    ~AuthPam() override;

public:
    virtual bool init() override;
    virtual bool authenticate(const QString& userName) override;
    virtual void respond(const QString& response) override;
    virtual bool inAuthentication() const override;
    virtual bool isAuthenticated() const override;
    virtual QString authenticationUser() const override;
    virtual void cancelAuthentication() override;

private slots:
    void handlePipeActivated();

private:
    void handleChildExit();

private:
    QString m_userName;

    bool m_isAuthenticated = false;
    bool m_inAuthenticating = false;
    bool m_hasSendCompleteSignal = false;

    pid_t m_authPid = 0;
    int m_toParentPipe[2] = {0,0};
    int m_toChildPipe[2] = {0,0};
    QSocketNotifier* m_socketNotifier = nullptr;
};

#endif//PAM_AUTH_SRC_PAM_AUTH_H_
