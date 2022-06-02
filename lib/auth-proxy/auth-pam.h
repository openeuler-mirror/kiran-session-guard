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

 
#ifndef PAM_AUTH_SRC_PAM_AUTH_H_
#define PAM_AUTH_SRC_PAM_AUTH_H_

#include <QObject>
#include "auth-base.h"

class QProcess;
class QSocketNotifier;

/**
 * 封装使用fork出子进程进行PAM认证的相关接口
 */
class AuthPam : public AuthBase
{
    Q_OBJECT
public:
    explicit AuthPam(QObject* = nullptr);
    ~AuthPam() override;

public:
    bool init() override;
    bool authenticate(const QString& userName) override;
    void respond(const QString& response) override;
    bool inAuthentication() const override;
    bool isAuthenticated() const override;
    QString authenticationUser() const override;
    void cancelAuthentication() override;

private slots:
    void handlePipeActivated();

private:
    void handleChildExit();

private:
    QString m_userName;

    bool m_isAuthenticated = false;
    bool m_inAuthenticating = false;

    //标志是否该次认证是否收到过认证完成信号，避免认证子进程异常退出，外部未收认证完成的信号
    bool m_hasSendCompleteSignal = false;

    pid_t m_authPid = 0;
    int m_toParentPipe[2] = {0,0};
    int m_toChildPipe[2] = {0,0};
    QSocketNotifier* m_socketNotifier = nullptr;
};

#endif//PAM_AUTH_SRC_PAM_AUTH_H_
