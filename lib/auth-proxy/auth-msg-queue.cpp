/**
  * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd.
  *
  * Author:     liuxinhao <liuxinhao@kylinos.com.cn>
  *
  * This program is free software; you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation; either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program; If not, see <http: //www.gnu.org/licenses/>. 
  */

#include "auth-msg-queue.h"
#include <QThread>
#include <qt5-log-i.h>
#include <sys/sysinfo.h>

AuthMsgQueue::AuthMsgQueue(QObject *parent)
    : AuthMsgQueueBase(parent),
      m_dispatcherThread(new QThread),
      m_dispatcherWorker(new DispatcherWorker())
{
    qRegisterMetaType<Kiran::PromptType>("Kiran::PromptType");
    qRegisterMetaType<Kiran::MessageType>("Kiran::MessageType");
    qRegisterMetaType<Kiran::AuthType>("Kiran::AuthType");
    qRegisterMetaType<Kiran::PromptFromEnum>("Kiran::PromptFromEnum");

    connect(m_dispatcherWorker,&DispatcherWorker::showMessage,this,&AuthMsgQueue::showMessage);
    connect(m_dispatcherWorker,&DispatcherWorker::showPrompt,this,&AuthMsgQueue::showPrompt);
    connect(m_dispatcherWorker,&DispatcherWorker::authenticationComplete,this,&AuthMsgQueue::authenticationComplete);
}

AuthMsgQueue::~AuthMsgQueue()
{
    stopDispatcher();
    delete m_dispatcherWorker;
    delete m_dispatcherThread;
}

void AuthMsgQueue::startDispatcher()
{
    if(!m_dispatcherThread->isRunning())
    {
        m_dispatcherThread->start();
        m_dispatcherWorker->moveToThread(m_dispatcherThread);
        if( !QMetaObject::invokeMethod(m_dispatcherWorker,"doDispatcher",Qt::QueuedConnection) )
        {
            KLOG_ERROR() << "can't invoke method 'doDispatcher'";
        }
    }
}

void AuthMsgQueue::stopDispatcher()
{
    if(m_dispatcherThread->isRunning())
    {
        m_dispatcherThread->requestInterruption();
        m_dispatcherWorker->cancelAndStop();

        m_dispatcherThread->quit();
        m_dispatcherThread->wait();
    }
    m_dispatcherWorker->cancelAndStop();
}

void AuthMsgQueue::setInterval(int seconds)
{
    m_dispatcherWorker->setInterval(seconds);
}

int AuthMsgQueue::interval()
{
    return m_dispatcherWorker->interval();
}

void AuthMsgQueue::append(Kiran::PromptFromEnum promptFrom,
                          Kiran::PromptType promptType,
                          const QString &msg)
{
    m_dispatcherWorker->append(promptFrom,promptType,msg);
}

void AuthMsgQueue::append(Kiran::MessageType messageType,
                          const QString &msg)
{
    m_dispatcherWorker->append(messageType,msg);
}

void AuthMsgQueue::appendAuthCompleteMsg(bool authRes)
{
    m_dispatcherWorker->appendAuthCompleteMsg(authRes);
}

DispatcherWorker::DispatcherWorker(QObject *parent) : QObject(parent)
{

}

DispatcherWorker::~DispatcherWorker()
{
    cancelAndStop();
}

void DispatcherWorker::cancelAndStop()
{
    m_queueSem.acquire(m_queueSem.available());

    m_queueLock.lock();
    m_queue.clear();
    m_queueLock.unlock();

    m_cancelWaitCond.wakeAll();
    m_cancelWaitMutex.unlock();
    m_msgShowTime = 0;
}

void DispatcherWorker::setInterval(int seconds)
{
    m_interval = seconds;
}

int DispatcherWorker::interval()
{
    return m_interval;
}

void DispatcherWorker::append(Kiran::PromptFromEnum promptFrom, Kiran::PromptType promptType, const QString &msg)
{
    AuthMsg authMsg;
    authMsg.type = AUTH_MSG_TYPE_PROMPT;
    authMsg.msg = msg;
    authMsg.extra.prompt_.promptType = promptType;
    authMsg.extra.prompt_.promptFrom = promptFrom;

    append(authMsg);
}

void DispatcherWorker::append(Kiran::MessageType messageType, const QString &msg)
{
    AuthMsg authMsg;
    authMsg.type = AUTH_MSG_TYPE_MESSAGE;
    authMsg.msg = msg;
    authMsg.extra.message_.messageType = messageType;

    append(authMsg);
}

void DispatcherWorker::appendAuthCompleteMsg(bool authRes)
{
    AuthMsg authMsg;
    authMsg.type = AUTH_MSG_TYPE_COMPLETE;
    authMsg.msg = "";
    authMsg.extra.complete_.authRes = authRes;

    append(authMsg);
}

void DispatcherWorker::append(const AuthMsg &authMsg)
{
    m_queueLock.lock();
    m_queue.enqueue(authMsg);
    m_queueLock.unlock();

    m_queueSem.release();
}

void DispatcherWorker::doDispatcher()
{
    forever
    {
        if (QThread::currentThread()->isInterruptionRequested())
        {
            return;
        }

        ///阻塞尝试从队列中取出认证事件
        AuthMsg msg;
        if (!fetchMsgFromQueue(msg, 300))
        {
            continue;
        }

        quint64 upTime = getUpTime();
        quint64 msgShowTime = m_msgShowTime + m_interval;

        ///第一个prompt消息不等待
        ///其他消息需要排队延时显示
        if (!(msg.type == AUTH_MSG_TYPE_PROMPT && !m_havePrompted) &&
            (msgShowTime > upTime))
        {
            ///阻塞等待到可以显示的时机，可以取消
            if (m_cancelWaitCond.wait(&m_cancelWaitMutex, (msgShowTime - upTime) * 1000))
            {
                continue;
            }
        }

        m_msgShowTime = getUpTime();

        ///消息分发
        switch (msg.type)
        {
        case AUTH_MSG_TYPE_MESSAGE:
            emit showMessage(msg.msg, msg.extra.message_.messageType);
            break;
        case AUTH_MSG_TYPE_PROMPT:
            m_havePrompted = true;
            emit showPrompt(msg.msg,msg.extra.prompt_.promptType,msg.extra.prompt_.promptFrom);
            break;
        case AUTH_MSG_TYPE_COMPLETE:
            emit authenticationComplete(msg.extra.complete_.authRes);
            break;
        }
    }
}

bool DispatcherWorker::fetchMsgFromQueue(AuthMsg &msg, int ms)
{
    if (m_queueSem.tryAcquire(1, ms))
    {
        QMutexLocker locker(&m_queueLock);
        if (m_queue.isEmpty())
        {
            return false;
        }
        msg = m_queue.dequeue();
        return true;
    }
    return false;
}

quint64 DispatcherWorker::getUpTime()
{
    struct sysinfo sysInfo{};
    int res = 0;
    qint64 uptime = 0;

    res = sysinfo(&sysInfo);
    if (res != 0)
    {
        KLOG_WARNING(" get up time failed,%s",strerror(res));
        return uptime;
    }

    uptime = sysInfo.uptime;
    return uptime;
}