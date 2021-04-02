/**
 *@file  auth-msg-queue.h
 *@brief 
 *@auth  liuxinhao <liuxinhao@kylinos.com.cn>
 *@copyright (c) 2021 KylinSec. All rights reserved.
 */
#ifndef LIGHTDM_KIRAN_GREETER_AUTH_MSG_QUEUE_H
#define LIGHTDM_KIRAN_GREETER_AUTH_MSG_QUEUE_H

#include <QFutureWatcher>
#include <QMutex>
#include <QQueue>
#include <QSemaphore>
#include <QThread>
#include <QWaitCondition>

class AuthMsgQueue : public QThread
{
    Q_OBJECT
    Q_ENUMS(PamMsgType MessageType PromptType)
public:
    enum PamMsgType
    {
        PMT_MSG,
        PMT_PROMPT,
        PMT_AUTHENTICATION_COMPLETE
    };
    enum PromptType
    {
        PROMPT_TYPE_QUESTION,
        PROMPT_TYPE_SECRET
    };
    enum MessageType
    {
        MESSAGE_TYPE_INFO,
        MESSAGE_TYPE_ERROR
    };
    struct PamMessage
    {
        /// 消息类型
        PamMsgType type;
        /// 显示内容
        QString text;
        union extra
        {
            MessageType msgType;
            PromptType  promptType;
            struct AuthenticationCompleteResult
            {
                bool isSuccess;
                bool reAuthentication;
            } completeResult;
        } extra;
    };

    AuthMsgQueue(QObject *parent = nullptr);
    ~AuthMsgQueue();

    void setMsgInterval(int seconds);

    void startDispatcher();
    void stopDispatcher();

    void clean();
    void append(const PamMessage &msg);

protected:
    void run() override;

signals:
    /* 显示prompt消息信号 */
    void showPrompt(const QString &prompt, AuthMsgQueue::PromptType type);
    /* 显示消息 */
    void showMessage(const QString &message, AuthMsgQueue::MessageType type);
    /* 认证完成 */
    void authenticateComplete(bool success, bool reAuth);

private:
    quint64 getUPTime();
    bool    fetchMessageFromQueue(PamMessage &msg, int ms);
    void    pamMessageDispatcher();
    void    dumpMsg(const PamMessage &msg);

private:
    QQueue<PamMessage> m_queue;
    QSemaphore         m_queueSem;
    QMutex             m_queueMutex;

    quint64 m_showInterval = 2;
    quint64 m_msgShowTime  = 0;

    QMutex         m_cancelWaitMutex;
    QWaitCondition m_cancelWaitCondition;

    bool m_havePrompted = false;
};

#endif  //LIGHTDM_KIRAN_GREETER_AUTH_MSG_QUEUE_H
