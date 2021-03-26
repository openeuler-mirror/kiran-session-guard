#ifndef LOG_H
#define LOG_H

#include <QString>
#include <QMutex>

class Log
{
public:
    ~Log ();
    static Log *instance ();
    static void messageHandler (QtMsgType type, const QMessageLogContext &context, const QString &msg);
    bool init (QString filePath);
    void setLogLevel (QtMsgType type);
    void write (QtMsgType type, const QMessageLogContext &context, const QString &msg);
    bool isInited ();
private:
    Log ();
    QMutex m_mutex;
    QString m_savePath;
    QtMsgType m_msgType;
    bool m_initOver;
};

#endif // LOG_H
