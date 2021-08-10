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

#ifndef KIRAN_SCREENSAVER_DIALOG_SRC_AUTH_AUTH_MSG_QUEUE_H_
#define KIRAN_SCREENSAVER_DIALOG_SRC_AUTH_AUTH_MSG_QUEUE_H_

#include <QMutex>
#include <QQueue>
#include <QSemaphore>
#include <QWaitCondition>
#include "auth-msg-queue-base.h"

enum AuthMsgType
{
    AUTH_MSG_TYPE_PROMPT,
    AUTH_MSG_TYPE_MESSAGE,
    AUTH_MSG_TYPE_COMPLETE
};

struct AuthMsg
{
    AuthMsgType type;
    QString msg;
    union extra
    {
        struct Prompt
        {
            Kiran::PromptFromEnum promptFrom;
            Kiran::PromptType promptType;
        } prompt_;
        struct Message
        {
            Kiran::MessageType messageType;
        } message_;
        struct Complete
        {
            bool authRes;
        } complete_;
    } extra;
};

/**
 * 具体的消息队列实现，该方法会移动至QThread中
 * 通过调用该对象的槽函数来在另一个线程开始处理认证事件队列
 */
class DispatcherWorker : public QObject
{
    Q_OBJECT
public:
    DispatcherWorker(QObject* parent = nullptr);
    ~DispatcherWorker();

    void cancelAndStop();

    void setInterval(int seconds);
    int interval();

    void append(Kiran::PromptFromEnum promptFrom,
                Kiran::PromptType promptType,
                const QString& msg);
    void append(Kiran::MessageType messageType,
                const QString& msg);
    void appendAuthCompleteMsg(bool authRes);
    void append(const AuthMsg& authMsg);

signals:
    void showMessage(QString text, Kiran::MessageType type);
    void showPrompt(QString text, Kiran::PromptType type,Kiran::PromptFromEnum promptFrom);
    void authenticationComplete(bool authRes);

public slots:
    void doDispatcher();

private:
    bool fetchMsgFromQueue(AuthMsg& msg, int ms);
    quint64 getUpTime();

private:
    QQueue<AuthMsg> m_queue;
    QSemaphore m_queueSem;
    QMutex m_queueLock;

    int m_interval = 2;
    quint64 m_msgShowTime = 0;

    QMutex m_cancelWaitMutex;
    QWaitCondition m_cancelWaitCond;

    bool m_havePrompted = false;
};

class AuthMsgQueue : public AuthMsgQueueBase
{
    Q_OBJECT
public:
    AuthMsgQueue(QObject* parent = nullptr);
    ~AuthMsgQueue();

public:
    void startDispatcher() override;
    void stopDispatcher() override;

    void setInterval(int seconds) override;
    int interval() override;

    void append(Kiran::PromptFromEnum promptFrom,
                Kiran::PromptType promptType,
                const QString& msg) override;
    void append(Kiran::MessageType messageType,
                const QString& msg) override;
    void appendAuthCompleteMsg(bool authRes) override;

private:
    QThread* m_dispatcherThread = nullptr;
    ///线程处理，消息存储和分发都在该对象中
    DispatcherWorker* m_dispatcherWorker = nullptr;
};

#endif  //KIRAN_SCREENSAVER_DIALOG_SRC_AUTH_AUTH_MSG_QUEUE_H_
