#ifndef UNIXSIGNALMONITOR_H
#define UNIXSIGNALMONITOR_H

#include <QObject>

class QSocketNotifier;
class UnixSignalMonitor : public QObject
{
    Q_OBJECT
public:
    explicit UnixSignalMonitor(QObject *parent = nullptr);
    static bool setup_unix_signal_handlers();
    //unix signal handlers
    static void termSignalHandler(int unused);

public slots:
    void handlerSigTerm();

private:
    static int sigTermFd[2];
    QSocketNotifier *m_snTerm;
};

#endif // UNIXSIGNALMONITOR_H
