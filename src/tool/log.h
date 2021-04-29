#ifndef LOG_H
#define LOG_H

#include <QDebug>
#include <QMutex>
#include <QString>

#include <iostream>
#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1) : __FILE__)

///NOTE:可见qlogging.h，未定义"QT_MESSAGELOGCONTEXT"将导致QDebug上下文没文件、行号、函数等信息
//为了不影响Qt流程，直接使用QMessageLogger,将上下文信息塞入


///NOTE:
/// LOG_XX()    以c语言类似传参数，不支持Qt内部相关类，和qDebug("%s","hello world")使用方法一样
/// LOG_XX_S()  以流的形式传递参数，支持Qt内部相关类，和qDeubg() << "hello world" << QString << QStringList使用方法一样


#define LOG_FATAL(format, ...)                                                             \
    do                                                                                     \
    {                                                                                      \
        QMessageLogger(__FILENAME__, __LINE__, __FUNCTION__).fatal(format, ##__VA_ARGS__); \
    } while (0)
#define LOG_FATAL_S QMessageLogger(__FILENAME__, __LINE__, __FUNCTION__).fatal

#define LOG_ERROR(format, ...)                                                                \
    do                                                                                        \
    {                                                                                         \
        QMessageLogger(__FILENAME__, __LINE__, __FUNCTION__).critical(format, ##__VA_ARGS__); \
    } while (0)
#define LOG_ERROR_S QMessageLogger(__FILENAME__, __LINE__, __FUNCTION__).critical

#define LOG_WARNING(format, ...)                                                             \
    do                                                                                       \
    {                                                                                        \
        QMessageLogger(__FILENAME__, __LINE__, __FUNCTION__).warning(format, ##__VA_ARGS__); \
    } while (0)
#define LOG_WARNING_S QMessageLogger(__FILENAME__, __LINE__, __FUNCTION__).warning

#define LOG_INFO(format, ...)                                                             \
    do                                                                                    \
    {                                                                                     \
        QMessageLogger(__FILENAME__, __LINE__, __FUNCTION__).info(format, ##__VA_ARGS__); \
    } while (0)
#define LOG_INFO_S QMessageLogger(__FILENAME__, __LINE__, __FUNCTION__).info

#define LOG_DEBUG(format, ...)                                                             \
    do                                                                                     \
    {                                                                                      \
        QMessageLogger(__FILENAME__, __LINE__, __FUNCTION__).debug(format, ##__VA_ARGS__); \
    } while (0)
#define LOG_DEBUG_S QMessageLogger(__FILENAME__, __LINE__, __FUNCTION__).debug

class Log
{
public:
  ~Log();
  static Log *instance();
  static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
  bool init();
  void setLogLevel(QtMsgType type);
  void write(QtMsgType type, const QMessageLogContext &context, const QString &msg);
  bool isInited() const;

private:
  Log();
  QtMsgType m_msgType;
  bool m_initOver;
};

#endif  // LOG_H
