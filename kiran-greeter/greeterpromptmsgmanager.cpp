#include "greeterpromptmsgmanager.h"
#include <QDebug>
#include <QMetaType>
#include <sys/sysinfo.h>
#include <QMutexLocker>

GreeterPromptMsgManager::GreeterPromptMsgManager(QLightDM::Greeter *greeter, QObject *parent)
    :QThread (parent),
      m_greeter(greeter)
{
    qRegisterMetaType<QLightDM::Greeter::PromptType>("QLightDM::Greeter::PromptType");
    qRegisterMetaType<QLightDM::Greeter::MessageType>("QLightDM::Greeter::MessageType");
    connect(m_greeter,
            &QLightDM::Greeter::showMessage,
            this,
            &GreeterPromptMsgManager::addMsgToQueue);
    connect(m_greeter,
            &QLightDM::Greeter::showPrompt,
            this,
            &GreeterPromptMsgManager::addPromptToQueue);
    connect(m_greeter,
            &QLightDM::Greeter::authenticationComplete,
            this,
            &GreeterPromptMsgManager::addAuthenticationCompleteToQueue);
}

GreeterPromptMsgManager::~GreeterPromptMsgManager()
{
    m_sleepCancelCondition.wakeAll();
    QThread::requestInterruption();
    QThread::wait();
}

void GreeterPromptMsgManager::setMessageInterval(int ms)
{
    m_messageInterval = ms;
}

void GreeterPromptMsgManager::addMsgToQueue(QString text, QLightDM::Greeter::MessageType type)
{
    LightdmPromptMsg msg;

    msg.type = LIGHTDM_MSG;
    msg.info.msgType = type;
    msg.text = text;

    addMsgItemToQueue(msg);
}

void GreeterPromptMsgManager::addPromptToQueue(QString text, QLightDM::Greeter::PromptType type)
{
    LightdmPromptMsg msg;

    msg.type = LIGHTDM_PROMPT;
    msg.info.promptType = type;
    msg.text = text;

    addMsgItemToQueue(msg);
}

void GreeterPromptMsgManager::addAuthenticationCompleteToQueue()
{
    bool reAuthentication = false;

    /// 如果认证失败,往队列中加入一个认证错误消息
    if(!m_greeter->isAuthenticated()){
        LightdmPromptMsg authFailedMsg;
        authFailedMsg.type = LIGHTDM_MSG;
        if(m_havePrompted){
            if(!m_havePAMError){
                authFailedMsg.text = tr("Incorrect password, please try again");
            }
            reAuthentication = true;
        }else{
            if(!m_havePAMError){
                authFailedMsg.text = tr("Failed to authenticate");
            }
        }
        if(!authFailedMsg.text.isEmpty())
            addMsgItemToQueue(authFailedMsg);
    }

    LightdmPromptMsg msg;
    msg.type = LIGHTDM_AUTHENTICATION_COMPLETE;
    msg.info.authenticationCompleteResult.isSuccess = m_greeter->isAuthenticated();
    msg.info.authenticationCompleteResult.reAuthentication = reAuthentication;
    addMsgItemToQueue(msg);
}

void GreeterPromptMsgManager::reset()
{
    m_semaphore.acquire(m_semaphore.available());

    m_msgQueueMutex.lock();
    m_msgQueue.clear();
    m_msgQueueMutex.unlock();

    m_sleepCancelCondition.wakeAll();
    m_messageShowTime = 0;
    m_havePAMError = false;
    m_havePrompted = false;
}

void GreeterPromptMsgManager::addMsgItemToQueue(const GreeterPromptMsgManager::LightdmPromptMsg &msg)
{
    qInfo() <<"queue append: " << dumpMsgInfoToString(msg);
    m_msgQueueMutex.lock();
    m_msgQueue.enqueue(msg);
    m_msgQueueMutex.unlock();

    m_semaphore.release();
}

bool GreeterPromptMsgManager::getMsgItemFromQueue(GreeterPromptMsgManager::LightdmPromptMsg &msg,
                                                         int ms)
{
    if( m_semaphore.tryAcquire(1,ms) ){
        QMutexLocker mutexLock(&m_msgQueueMutex);
        if(m_msgQueue.isEmpty()){
            return false;
        }
        msg = m_msgQueue.dequeue();
        qInfo() << "queue getItem: " << dumpMsgInfoToString(msg);
        return true;
    }
    return false;
}

qint64 GreeterPromptMsgManager::getUpTime()
{
    struct sysinfo sysInfo;
    int res = 0;
    qint64 uptime = 0;

    res = sysinfo(&sysInfo);
    if(res!=0){
        qWarning("sysinfo get uptime failed,%s",strerror(res));
        return uptime;
    }

    uptime = sysInfo.uptime;
    return uptime;
}

QString GreeterPromptMsgManager::dumpMsgInfoToString(const GreeterPromptMsgManager::LightdmPromptMsg &msg)
{
    QString ret;
    static QMap<LightdmPromptMsgType,QString> msgTypeDescMap = {
        {LIGHTDM_MSG,"Message"},
        {LIGHTDM_PROMPT,"Prompt"},
        {LIGHTDM_AUTHENTICATION_COMPLETE,"Authentication Complete"}
    };

    auto iter = msgTypeDescMap.find(msg.type);
    ret = QString("[%1] text:%2")
            .arg(iter==msgTypeDescMap.end()?"none":iter.value())
            .arg(msg.text);
    return ret;
}

void GreeterPromptMsgManager::run()
{
    forever {
        /// 若线程被请求中断退出线程
        if ( QThread::currentThread()->isInterruptionRequested() ) {
            return;
        }
        LightdmPromptMsg msg;
        if(getMsgItemFromQueue(msg,300)){
            qint64 uptime = getUpTime();
            /// 第一个prompt不做等待操作
            /// 其他消息等待上一次Msg消息超过时间间隔再分发
            if( !(msg.type==LIGHTDM_PROMPT&&!m_havePrompted)
                    && ((m_messageShowTime+m_messageInterval) >= uptime) ){
                QMutexLocker mutexLock(&m_sleepCancelMutex);
                /// 等待取消条件
                /// 如果取消条件满足,不分发该条Lightdm消息
                /// 可能发生在切换切换用户认证或取消认证
                if( m_sleepCancelCondition.wait(&m_sleepCancelMutex,
                                            ((m_messageShowTime+m_messageInterval)-uptime)*1000) ){
                    continue;
                }
            }
            /// 分发消息
            switch (msg.type) {
            case LIGHTDM_MSG:
                m_messageShowTime = uptime;
                if(msg.info.msgType==QLightDM::Greeter::MessageType::MessageTypeError){
                    m_havePAMError = true;
                }
                emit showMessage(msg.text,msg.info.msgType);
                break;
            case LIGHTDM_PROMPT:
                m_havePrompted = true;
                emit showPrompt(msg.text,msg.info.promptType);
                break;
            case LIGHTDM_AUTHENTICATION_COMPLETE:
                emit authenticationComplete(msg.info.authenticationCompleteResult.isSuccess,
                                            msg.info.authenticationCompleteResult.reAuthentication);
                break;
            }
        }
    }
}
