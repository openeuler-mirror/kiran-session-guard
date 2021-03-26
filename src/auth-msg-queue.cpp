//
// Created by lxh on 2021/3/16.
//

#include "auth-msg-queue.h"

#include <QtConcurrent/QtConcurrent>
#include <sys/sysinfo.h>

Q_DECLARE_METATYPE(AuthMsgQueue::PromptType)

Q_DECLARE_METATYPE(AuthMsgQueue::MessageType)

AuthMsgQueue::AuthMsgQueue (QObject *parent) : QThread(parent)
{
    qRegisterMetaType<AuthMsgQueue::PromptType>("AuthMsgQueue::PromptType");
    qRegisterMetaType<AuthMsgQueue::MessageType>("AuthMsgQueue::MessageType");
}

AuthMsgQueue::~AuthMsgQueue ()
{
    m_cancelWaitCondition.wakeAll();
    QThread::requestInterruption();
    QThread::wait();
}

void AuthMsgQueue::setMsgInterval (int seconds)
{
    m_showInterval = seconds;
}

void AuthMsgQueue::startDispatcher ()
{
    stopDispatcher();
    start();
}

void AuthMsgQueue::stopDispatcher ()
{
    if (!isRunning())
        return;

    clean();
    requestInterruption();
    wait();
}

void AuthMsgQueue::clean ()
{
    m_queueSem.acquire(m_queueSem.available());

    m_queueMutex.lock();
    m_queue.clear();
    m_queueMutex.unlock();

    m_cancelWaitCondition.wakeAll();
    m_msgShowTime = 0;
    m_havePrompted = false;
}

void AuthMsgQueue::append (const AuthMsgQueue::PamMessage &msg)
{
    m_queueMutex.lock();
    m_queue.enqueue(msg);
    m_queueMutex.unlock();

    m_queueSem.release();

    qInfo() << "append:";
    dumpMsg(msg);
    qInfo() << "------------";
}

quint64 AuthMsgQueue::getUPTime ()
{
    struct sysinfo sysInfo;
    int res = 0;
    qint64 uptime = 0;

    res = sysinfo(&sysInfo);
    if (res != 0)
    {
        qWarning("sysinfo get uptime failed,%s", strerror(res));
        return uptime;
    }

    uptime = sysInfo.uptime;
    return uptime;
}

bool AuthMsgQueue::fetchMessageFromQueue (PamMessage &msg, int ms)
{
    if (m_queueSem.tryAcquire(1, ms))
    {
        QMutexLocker locker(&m_queueMutex);
        if (m_queue.isEmpty())
        {
            return false;
        }
        msg = m_queue.dequeue();
        qInfo() << "got message from queue:";
        dumpMsg(msg);
        qInfo() << "------------";
        return true;
    }
    return false;
}

void AuthMsgQueue::pamMessageDispatcher ()
{
    forever
    {
        if (QThread::currentThread()->isInterruptionRequested())
        {
            return;
        }
        PamMessage msg;
        if (!fetchMessageFromQueue(msg, 300))
        {
            continue;
        }
        quint64 upTime = getUPTime();
        quint64 msgShowTime = m_msgShowTime + m_showInterval;
        ///第一个prompt消息不等待
        ///其他消息需要排队延时显示
        if (!(msg.type == PMT_PROMPT && !m_havePrompted) &&
            (msgShowTime > upTime))
        {
            ///阻塞等待，可以取消
            if (m_cancelWaitCondition.wait(&m_cancelWaitMutex, (msgShowTime - upTime) * 1000))
            {
                continue;
            }
        }
        ///消息分发
        switch (msg.type)
        {
            case PMT_MSG:emit showMessage(msg.text, msg.extra.msgType);
                break;
            case PMT_PROMPT:m_havePrompted = true;
                emit showPrompt(msg.text, msg.extra.promptType);
                break;
            case PMT_AUTHENTICATION_COMPLETE:emit authenticateComplete(msg.extra.completeResult.isSuccess,
                                                                       msg.extra.completeResult.reAuthentication);
                break;
        }

    }
}

void AuthMsgQueue::dumpMsg (const AuthMsgQueue::PamMessage &msg)
{
    QString ret;
    static QMap<PamMsgType, QString> msgTypeDescMap = {
            {PMT_MSG,                     "Message"},
            {PMT_PROMPT,                  "Prompt"},
            {PMT_AUTHENTICATION_COMPLETE, "Authentication Complete"}
    };
    auto iter = msgTypeDescMap.find(msg.type);
    ret = QString("[%1] text:%2")
            .arg(iter == msgTypeDescMap.end() ? "none" : iter.value())
            .arg(msg.text);
    qInfo() << ret;
}

void AuthMsgQueue::run ()
{
    pamMessageDispatcher();
}
