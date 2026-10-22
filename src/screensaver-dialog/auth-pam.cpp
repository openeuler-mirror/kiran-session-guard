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

#include <fcntl.h>
#include <qt5-log-i.h>
#include <sys/wait.h>
#include <unistd.h>
#include <QJsonObject>
#include <QSocketNotifier>

#define CHECKPASS_PATH "/usr/libexec/kiran-session-guard-checkpass"

enum PipeChannelEnum
{
    CHANNEL_READ = 0,
    CHANNEL_WRITE
};

namespace Kiran
{
namespace SessionGuard
{
namespace Locker
{
AuthPam::AuthPam(QObject *parent)
    : QObject(parent), AuthBase()
{
}

AuthPam::~AuthPam()
{
}

bool AuthPam::init(AuthControllerInterface *controllerInterface)
{
    m_interface = controllerInterface;
    return true;
}

bool AuthPam::authenticate(const QString &userName)
{
    if (inAuthentication())
    {
        cancelAuthentication();
    }
    else if (m_socketNotifier != nullptr || m_authPid != 0 ||
             m_toParentPipe[CHANNEL_READ] > 0 || m_toChildPipe[CHANNEL_WRITE] > 0)
    {
        /* Complete/cancel 后 inAuthentication 已为 false，但 WNOHANG 可能尚未收尽
         * pid，或异常路径仍残留 notifier。此处兜底收尾，避免 new notifier 泄漏旧 fd。
         * 即将 fork，kill 后允许短时阻塞 waitpid(0) 以腾出 pid。 */
        if (m_socketNotifier != nullptr)
        {
            m_socketNotifier->setEnabled(false);
        }
        if (m_authPid != 0)
        {
            if (waitpid(m_authPid, nullptr, WNOHANG) == 0)
            {
                kill(m_authPid, SIGKILL);
                waitpid(m_authPid, nullptr, 0);
            }
            m_authPid = 0;
        }
        cleanupAuthResources();
    }

    if (pipe(m_toParentPipe) == -1 || pipe(m_toChildPipe) == -1)
    {
        KLOG_ERROR() << "can't create pipe before fork," << strerror(errno);
        return false;
    }

    /* Don't allow the daemon end of the pipes to be accessed in child processes */
    fcntl(m_toParentPipe[CHANNEL_READ], F_SETFD, FD_CLOEXEC);
    fcntl(m_toChildPipe[CHANNEL_WRITE], F_SETFD, FD_CLOEXEC);

    m_userName = userName;

    pid_t forkPid = fork();

    // fork出错
    if (forkPid == -1)
    {
        KLOG_ERROR() << "fork error," << strerror(errno);
        close(m_toParentPipe[CHANNEL_WRITE]);
        close(m_toParentPipe[CHANNEL_READ]);
        close(m_toChildPipe[CHANNEL_WRITE]);
        close(m_toChildPipe[CHANNEL_READ]);
        m_toParentPipe[CHANNEL_READ] = 0;
        m_toParentPipe[CHANNEL_WRITE] = 0;
        m_toChildPipe[CHANNEL_READ] = 0;
        m_toChildPipe[CHANNEL_WRITE] = 0;
        m_userName = "";
        return false;
    }

    // 认证子进程
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
    m_hasSendCompleteSignal = false;

    // 父进程
    m_authPid = forkPid;

    // 关闭主进程不需要用到的管道文件描述符
    close(m_toParentPipe[CHANNEL_WRITE]);
    m_toParentPipe[CHANNEL_WRITE] = 0;

    close(m_toChildPipe[CHANNEL_READ]);
    m_toChildPipe[CHANNEL_READ] = 0;

    // 监听管道可读消息
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
    /* 先停 notifier，再 kill/关管：管道 POLLHUP 时若仍启用 notifier，
     * Qt 事件循环会以 poll(timeout=0) 忙等，表现为锁屏预览卡顿。 */
    if (m_socketNotifier != nullptr)
    {
        m_socketNotifier->setEnabled(false);
    }

    if (m_authPid != 0)
    {
        kill(m_authPid, SIGKILL);
        /* 不阻塞 waitpid(0)，避免 UI/D-Bus 路径上卡死；未收到则保留 pid 供后续回收 */
        if (waitpid(m_authPid, nullptr, WNOHANG) != 0)
        {
            m_authPid = 0;
        }
    }

    const bool needNotifyComplete = !m_hasSendCompleteSignal;

    m_isAuthenticated = false;
    m_inAuthenticating = false;
    cleanupAuthResources();
    m_userName.clear();

    /* 取消时补发 complete，供 AuthController → authenticationComplete(false)；
     * Quiet 路径用 m_suppressAuthComplete 吞掉本次回调。 */
    if (needNotifyComplete && m_interface != nullptr)
    {
        m_hasSendCompleteSignal = true;
        this->m_interface->onAuthComplete();
    }
    else
    {
        m_hasSendCompleteSignal = false;
    }
}

void AuthPam::cleanupAuthResources()
{
    /* 先 setEnabled(false) 再 delete。管道已 POLLHUP 时若仍启用 notifier，
     * Qt 事件循环会以 poll(timeout=0) 忙等，表现为锁屏预览卡顿。 */
    if (m_socketNotifier != nullptr)
    {
        m_socketNotifier->setEnabled(false);
        disconnect(m_socketNotifier, &QSocketNotifier::activated, this, &AuthPam::handlePipeActivated);
        delete m_socketNotifier;
        m_socketNotifier = nullptr;
    }

    if (m_toChildPipe[CHANNEL_WRITE] > 0)
    {
        close(m_toChildPipe[CHANNEL_WRITE]);
        m_toChildPipe[CHANNEL_WRITE] = 0;
    }
    if (m_toParentPipe[CHANNEL_READ] > 0)
    {
        close(m_toParentPipe[CHANNEL_READ]);
        m_toParentPipe[CHANNEL_READ] = 0;
    }
}

void AuthPam::handlePipeActivated()
{
    /* 读之前先检查：管道可能已在其他路径被 close，fd 为 0 */
    if (m_toParentPipe[CHANNEL_READ] <= 0)
    {
        handleChildExit();
        return;
    }

    PamEvent *pamEvent = nullptr;
    if (!kiran_pam_message_recv_event(m_toParentPipe[CHANNEL_READ], &pamEvent))
    {
        /* EOF/读失败：先停 notifier，再进 handleChildExit 关管并回收子进程 */
        if (m_socketNotifier != nullptr)
        {
            m_socketNotifier->setEnabled(false);
        }
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
        PromptType promptType = event->secret() ? PromptTypeSecret : PromptTypeQuestion;
        KLOG_DEBUG() << "recv checkpass prompt message\n"
                     << "type:" << promptType << "\n"
                     << "text:" << event->text();
        this->m_interface->onShowPrompt(event->text(), promptType);
        break;
    }
    case PamEvent::Message:
    {
        auto event = dynamic_cast<MessageEvent *>(pamEvent);
        MessageType messageType = event->isError() ? MessageTypeError : MessageTypeInfo;
        KLOG_DEBUG() << "recv checkpass authproxy message\n"
                     << "type:" << messageType << "\n"
                     << "text:" << event->text();
        this->m_interface->onShowMessage(event->text(), messageType);
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
        m_inAuthenticating = false;

        /* Complete 后 checkpass 很快退出，管道出现 POLLHUP。若 notifier 仍启用，
         * Qt 会以 poll(timeout=0) 忙等（预览卡顿），且 EOF 槽可能来不及停转。
         * 回调 onAuthComplete 前必须先停 notifier 并收尾；waitpid 只用 WNOHANG。 */
        if (m_socketNotifier != nullptr)
        {
            m_socketNotifier->setEnabled(false);
        }
        cleanupAuthResources();
        if (m_authPid != 0)
        {
            /* 关管后子进程会退出；WNOHANG 未收到则保留 pid，供二次 authenticate 回收 */
            if (waitpid(m_authPid, nullptr, WNOHANG) != 0)
            {
                m_authPid = 0;
            }
        }

        kiran_pam_message_free(&pamEvent);
        pamEvent = nullptr;

        this->m_interface->onAuthComplete();
        return;
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
    m_inAuthenticating = false;
    if (m_authPid != 0)
    {
        /* 与 Complete 路径一致：仅在已 reap / ECHILD 时清零，避免丢 pid 留僵尸 */
        if (waitpid(m_authPid, nullptr, WNOHANG) != 0)
        {
            m_authPid = 0;
        }
    }
    cleanupAuthResources();
    KLOG_DEBUG() << "child process exit finished";

    // 中途意外结束，未发认证完成信号的时候，补上认证完成信号
    if (!m_hasSendCompleteSignal)
    {
        KLOG_DEBUG() << "authentication subprocess exits without sending a completion signal. completion signal is sent again";
        m_hasSendCompleteSignal = true;
        this->m_interface->onAuthComplete();
    }
}
}  // namespace Locker
}  // namespace SessionGuard
}  // namespace Kiran