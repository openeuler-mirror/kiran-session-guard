#include "unix-signal-monitor.h"

#include <QApplication>
#include <QSocketNotifier>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <unistd.h>

int UnixSignalMonitor::sigTermFd[2] = {0,0};

UnixSignalMonitor::UnixSignalMonitor(QObject *parent) : QObject(parent)
{
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigTermFd))
       qFatal("Couldn't create TERM socketpair");
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
