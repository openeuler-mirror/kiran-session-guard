#include "log.h"
#include <QDateTime>
#include <QDir>
#include <QDebug>
#include <QMap>
#include <QTextStream>
#include <iostream>
#include <QMutex>
#include <QScopedPointer>

Log::~Log()
{

}

Log *Log::instance()
{
    static QMutex mutex;
    static QScopedPointer<Log> pInst;

    if(Q_UNLIKELY(!pInst)){
        QMutexLocker locker(&mutex);
        if(pInst.isNull()){
            pInst.reset(new Log);
        }
    }

    return pInst.data();
}

bool Log::init(QString filePath)
{
    if(m_initOver){
        return false;
    }

    if( !filePath.isEmpty() ){
        QFile file(filePath);
        QFileInfo fileInfo(filePath);
        if(!fileInfo.dir().exists() && !fileInfo.dir().mkpath(fileInfo.dir().path())){
            fprintf(stderr,"make log file failed\n");
            return false;
        }
        if( !file.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text) ){
            return false;
        }
        file.close();
        m_savePath = filePath;
    }

    m_initOver = true;
    return true;
}

void Log::setLogLevel(QtMsgType type)
{
    m_msgType = type;
}

void Log::write(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QMutexLocker locker(&m_mutex);
    static QMap<QtMsgType,QString> msgDescMap = {
        {QtDebugMsg,   "[DEBUG]"},
        {QtWarningMsg, "[WARNING]"},
        {QtCriticalMsg,"[CRITICAL]"},
        {QtFatalMsg,   "[FATAL]"},
        {QtInfoMsg,    "[INFO]"}
    };

    QString curTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    QMap<QtMsgType,QString>::Iterator it = msgDescMap.find(type);
    QString logContent = QString("%1 %2 <%3:%4>: %5")
            .arg(curTime)
            .arg(it==msgDescMap.end()?"UNKNOW":*it,-10)
            .arg(context.function)
            .arg(context.line)
            .arg(msg);
    if( type > m_msgType ){
        if(!m_savePath.isEmpty()){
            QFile file(m_savePath);
            if( file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text) ){
                QFlags<QFile::Permission> flags = QFile::ReadOwner|QFile::WriteOwner|
                                                  QFile::ReadUser |QFile::WriteUser |
                                                  QFile::ReadGroup|QFile::WriteGroup|
                                                  QFile::ReadOther|QFile::WriteOther;
                if( file.permissions() != flags ){
                    file.setPermissions(flags);
                }
                QTextStream ts(&file);
                ts << logContent << endl;
                file.close();
            }
        }
        std::string sLogContent = logContent.toStdString();
        std::cout << sLogContent << std::endl;
    }
}

bool Log::isInited()
{
    return m_initOver;
}

void Log::messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Log* log = Log::instance();
    if( !log->isInited() ){
        qWarning() << "Log not initialized,call Log::init.";
        qDebug() << msg;
        return;
    }
    log->write(type,context,msg);
}

Log::Log():m_savePath(""),m_msgType(QtDebugMsg),m_initOver(false)
{

}
