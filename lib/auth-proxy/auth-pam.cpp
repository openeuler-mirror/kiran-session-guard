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


#include "auth-pam.h"
#include "pam-message.h"

#include <qt5-log-i.h>
#include <sys/wait.h>
#include <unistd.h>
#include <QJsonObject>
#include <QSocketNotifier>
#include <fcntl.h>

#define CHECKPASS_PATH "/usr/libexec/kiran-session-guard-checkpass"

enum PipeChannelEnum{
    CHANNEL_READ = 0,
    CHANNEL_WRITE
};

AuthPam::AuthPam(QObject *parent)
    : AuthBase(parent)
{
}

AuthPam::~AuthPam()
{
}

bool AuthPam::init()
{
    return true;
}

bool AuthPam::authenticate(const QString &userName)
{
    if (inAuthentication())
    {
        cancelAuthentication();
    }

    if (pipe(m_toParentPipe) == -1 || pipe(m_toChildPipe) == -1)
    {
        KLOG_ERROR() << "can't create pipe before fork," << strerror(errno);
        return false;
    }

    /* Don't allow the daemon end of the pipes to be accessed in child processes */
    fcntl(m_toParentPipe[CHANNEL_READ],F_SETFD,FD_CLOEXEC);
    fcntl(m_toChildPipe[CHANNEL_WRITE],F_SETFD,FD_CLOEXEC);

    m_userName = userName;

    pid_t forkPid = fork();

    //fork出错
    if (forkPid == -1)
    {
        KLOG_ERROR() << "fork error," << strerror(errno);
        close(m_toParentPipe[CHANNEL_WRITE]);
        close(m_toParentPipe[CHANNEL_READ]);
        close(m_toChildPipe[CHANNEL_WRITE]);
        close(m_toChildPipe[CHANNEL_READ]);
        m_userName = "";
        return false;
    }

    //认证子进程
    if (forkPid == 0)
    {
        if (execlp(CHECKPASS_PATH,
                   QString::number(m_toChildPipe[CHANNEL_READ]).toStdString().c_str(),
                   QString::number(m_toParentPipe[CHANNEL_WRITE]).toStdString().c_str(),
                   m_userName.toStdString().c_str(), nullptr) == -1)
        {
            KLOG_ERROR() << "execl failed," << strerror(errno);
        }
        ::_exit(-1);
    }

    m_inAuthenticating = true;

    //父进程
    m_authPid = forkPid;

    //关闭主进程不需要用到的管道文件描述符
    close(m_toParentPipe[CHANNEL_WRITE]);
    m_toParentPipe[CHANNEL_WRITE] = 0;

    close(m_toChildPipe[CHANNEL_READ]);
    m_toChildPipe[CHANNEL_READ] = 0;

    //监听管道可读消息
    m_socketNotifier = new QSocketNotifier(m_toParentPipe[CHANNEL_READ], QSocketNotifier::Read);
    connect(m_socketNotifier, &QSocketNotifier::activated, this, &AuthPam::handlePipeActivated);

    return true;
}

void AuthPam::respond(const QString &response)
{
    if (!inAuthentication())
    {
        return;
    }
    PromptReplyEvent promptReplyEvent(true, response);
    kiran_pam_message_send_event(m_toChildPipe[CHANNEL_WRITE], &promptReplyEvent);
}

bool AuthPam::inAuthentication() const
{
    return m_inAuthenticating;
}

bool AuthPam::isAuthenticated() const
{
    return m_isAuthenticated;
}

QString AuthPam::authenticationUser() const
{
    return m_userName;
}

void AuthPam::cancelAuthentication()
{
    if (m_authPid != 0)
    {
        kill(m_authPid, SIGKILL);
        waitpid(m_authPid, nullptr, 0);
        m_authPid = 0;
    }

    if (m_toChildPipe[CHANNEL_WRITE] != 0)
    {
        close(m_toChildPipe[CHANNEL_WRITE]);
    }

    if (m_toParentPipe[CHANNEL_READ] != 0)
    {
        close(m_toParentPipe[CHANNEL_READ]);
    }

    m_isAuthenticated = false;
    m_inAuthenticating = false;
    m_hasSendCompleteSignal = false;

    if (m_socketNotifier)
    {
        disconnect(m_socketNotifier, &QSocketNotifier::activated, this, &AuthPam::handlePipeActivated);
        delete m_socketNotifier;
        m_socketNotifier = nullptr;
    }

    m_userName.clear();
}

void AuthPam::handlePipeActivated()
{
    QJsonDocument doc;

    PamEvent *pamEvent = nullptr;
    if (!kiran_pam_message_recv_event(m_toParentPipe[CHANNEL_READ], &pamEvent))
    {
        handleChildExit();
        return;
    }

    switch (pamEvent->type())
    {
    case PamEvent::Error:
        KLOG_ERROR() << "recv checkpass pam error:" << pamEvent->text();
        break;
    case PamEvent::PromptRequest:
    {
        auto event = dynamic_cast<PromptRequestEvent *>(pamEvent);
        Kiran::PromptType promptType = event->secret() ? Kiran::PromptTypeSecret : Kiran::PromptTypeQuestion;
        KLOG_DEBUG() << "recv checkpass prompt message\n"
                     << "type:" << promptType << "\n"
                     << "text:" << event->text();
        emit showPrompt(event->text(), promptType);
        break;
    }
    case PamEvent::Message:
    {
        auto event = dynamic_cast<MessageEvent *>(pamEvent);
        Kiran::MessageType messageType = event->isError() ? Kiran::MessageTypeError : Kiran::MessageTypeInfo;
        KLOG_DEBUG() << "recv checkpass authproxy message\n"
                     << "type:" << messageType << "\n"
                     << "text:" << event->text();
        emit showMessage(event->text(), messageType);
        break;
    }
    case PamEvent::Complete:
    {
        auto event = dynamic_cast<CompleteEvent *>(pamEvent);
        m_hasSendCompleteSignal = true;
        KLOG_DEBUG() << "recv checkpass authproxy complete\n"
                     << "complete:" << event->isComplete() << "\n"
                     << "result:  " << event->authResult() << "\n"
                     << "text:    " << event->text();
        m_isAuthenticated = event->authResult();
        emit authenticationComplete();
        break;
    }
    case PamEvent::LAST:
    case PamEvent::PromptReply:
    default:
        KLOG_ERROR() << "can't supported this event" << pamEvent->type();
        break;
    }

    kiran_pam_message_free(&pamEvent);
}

void AuthPam::handleChildExit()
{
    KLOG_DEBUG() << "handle child process exit";
    waitpid(-1, nullptr, WNOHANG);
    KLOG_DEBUG() << "child process exit finished";

    m_inAuthenticating = false;

    if (m_toParentPipe[CHANNEL_READ])
    {
        close(m_toParentPipe[CHANNEL_READ]);
    }

    if (m_toChildPipe[CHANNEL_WRITE])
    {
        close(m_toChildPipe[CHANNEL_WRITE]);
    }

    if (m_socketNotifier != nullptr)
    {
        delete m_socketNotifier;
        m_socketNotifier = nullptr;
    }

    //中途意外结束，未发认证完成信号的时候，补上认证完成信号
    if (!m_hasSendCompleteSignal)
    {
        emit authenticationComplete();
    }
}