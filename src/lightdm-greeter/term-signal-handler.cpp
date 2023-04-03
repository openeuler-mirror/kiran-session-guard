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

#include "term-signal-handler.h"

#include <qt5-log-i.h>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>

#include <QApplication>
#include <QSocketNotifier>

namespace Kiran
{
namespace SessionGuard
{
namespace Greeter
{
int TermSignalHandler::sigTermFd[2] = {0, 0};
TermSignalHandler::TermSignalHandler(QObject *parent) : QObject(parent)
{
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigTermFd))
    {
        KLOG_FATAL("Couldn't create TERM socketpair");
    }

    m_termNotifier = new QSocketNotifier(sigTermFd[1], QSocketNotifier::Read, this);
    connect(m_termNotifier, &QSocketNotifier::activated, this, &TermSignalHandler::handlerSigTerm);
}

bool TermSignalHandler::init()
{
    struct sigaction term;
    term.sa_handler = TermSignalHandler::termCallback;

    sigemptyset(&term.sa_mask);

    term.sa_flags = 0;
    term.sa_flags |= SA_RESTART;

    if (sigaction(SIGTERM, &term, 0) != 0)
    {
        KLOG_WARNING() << "can't install signal-catching function:" << strerror(errno);
        return false;
    }

    return true;
}

void TermSignalHandler::termCallback(int unused)
{
    char a = 1;
    ::write(sigTermFd[0], &a, sizeof(a));
}

void TermSignalHandler::handlerSigTerm()
{
    // 异步处理SIGTERM
    m_termNotifier->setEnabled(false);
    char tmp;
    ::read(sigTermFd[1], &tmp, sizeof(tmp));
    qApp->quit();
}

}  // namespace Greeter
}  // namespace SessionGuard
}  // namespace Kiran