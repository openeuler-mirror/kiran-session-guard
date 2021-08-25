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


#include "unix-signal-monitor.h"

#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>
#include <QApplication>
#include <QSocketNotifier>
#include <qt5-log-i.h>

int UnixSignalMonitor::sigTermFd[2] = {0, 0};

UnixSignalMonitor::UnixSignalMonitor(QObject *parent) : QObject(parent)
{
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigTermFd))
        KLOG_FATAL("Couldn't create TERM socketpair");
    m_snTerm = new QSocketNotifier(sigTermFd[1], QSocketNotifier::Read, this);
    connect(m_snTerm, &QSocketNotifier::activated, this, &UnixSignalMonitor::handlerSigTerm);
}

bool UnixSignalMonitor::setup_unix_signal_handlers()
{
    struct sigaction term;
    term.sa_handler = UnixSignalMonitor::termSignalHandler;
    sigemptyset(&term.sa_mask);
    term.sa_flags = 0;
    term.sa_flags |= SA_RESTART;

    if (sigaction(SIGTERM, &term, 0))
        return false;

    return true;
}

void UnixSignalMonitor::termSignalHandler(int unused)
{
    char a = 1;
    //收到SIGTERM信号,激活SocketNotifier
    ::write(sigTermFd[0], &a, sizeof(a));
}

void UnixSignalMonitor::handlerSigTerm()
{
    //异步处理SIGTERM
    m_snTerm->setEnabled(false);
    char tmp;
    ::read(sigTermFd[1], &tmp, sizeof(tmp));
    qApp->quit();
    exit(EXIT_SUCCESS);
    m_snTerm->setEnabled(true);
}
