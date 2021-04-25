#include "log.h"
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QMap>
#include <QMutex>
#include <QScopedPointer>
#include <QTextStream>
#include <iostream>
#include <zlog_ex.h>

#define LOG_FILENAME "qt-file"
#define LOG_FUNCTION "qt-function"

Log::~Log()
{
}

Log *Log::instance()
{
    static QMutex              mutex;
    static QScopedPointer<Log> pInst;

    if (Q_UNLIKELY(!pInst))
    {
        QMutexLocker locker(&mutex);
        if (pInst.isNull())
        {
            pInst.reset(new Log);
        }
    }

    return pInst.data();
}

bool Log::init()
{
    if (m_initOver)
    {
        return false;
    }

    //NOTE: do initialization

    m_initOver = true;
    return true;
}

void Log::setLogLevel(QtMsgType type)
{
    m_msgType = type;
}

void Log::write(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    static QMap<QtMsgType, zlog_level> msgDescMap = {
        {QtDebugMsg, ZLOG_LEVEL_DEBUG},
        {QtWarningMsg, ZLOG_LEVEL_WARN},
        {QtCriticalMsg, ZLOG_LEVEL_ERROR},
        {QtFatalMsg, ZLOG_LEVEL_FATAL},
        {QtInfoMsg, ZLOG_LEVEL_INFO},
    };

    if( type<m_msgType ){
        std::cerr<<"ignore:" << std::endl;
        return;
    }

    std::string fileName = context.file?context.file:"no-file";
    std::string function = context.function?context.function:"no-func";
    int line = context.line;
    std::string strMsg = msg.toStdString();

    auto zlogLevelIter = msgDescMap.find(type);
    if(zlogLevelIter==msgDescMap.end()){
        std::cerr<<"can't find zlog level." << std::endl;
        return;
    }

    dzlog(fileName.c_str(),
          fileName.size(),
          function.c_str(),
          function.size(),
          line,
          zlogLevelIter.value(),
          "%s",
          strMsg.c_str());
}

bool Log::isInited() const
{
    return m_initOver;
}

void Log::messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Log *log = Log::instance();
    if (!log->isInited())
    {
        std::cerr << "not initialized,call Log::init" << std::endl;
        std::cerr << msg.toStdString() << std::endl;
        return;
    }
    log->write(type, context, msg);
}

Log::Log() : m_msgType(QtDebugMsg), m_initOver(false)
{
}
