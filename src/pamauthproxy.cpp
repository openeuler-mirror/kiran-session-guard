#include "pamauthproxy.h"
#include <QDebug>
#include <QDateTime>

#define PAM_SERVICE_NAME "mate-screensaver"
#define MAX_TRIES 5

void no_fail_delay(int status, unsigned int delay, void *appdata_ptr)
{

}

PamAuthProxy::PamAuthProxy(PamAuthCallback *callback, QObject *parent)
    : QThread(parent),
      m_pamStatus(-1),
      m_authCallback(callback),
      m_pamHandler(nullptr),
      m_authComplete(false),
      m_conversationReply(nullptr),
      m_conversationRelplyRet(false),
      m_cancelAuthFlag(false)
{
    connect(this,&QThread::finished,this,&PamAuthProxy::slotThreadFinished);
}

PamAuthProxy::~PamAuthProxy()
{
    cancelAuthenticate();
}

/**
 * @brief  获取是否在认证过程中
 * @return 是否在认证过程中
 */
bool PamAuthProxy::isInAuthenticate()
{
    return isRunning();
}

/**
 * @brief 开始创建线程进行用户验证，验证过程中调用构造中传入的接口类
 * @param userName
 * @return 是否开启成功
 */
bool PamAuthProxy::startAuthenticate(const char *userName)
{
    const struct pam_conv conv = {
        &PamAuthProxy::conversation,
        this
    };
    bool bRes = false;
    QString display;

    if(isRunning()||
       userName==nullptr){
        return false;
    }

    if( m_pamHandler ){
        closePamHandler();
    }

    if( (m_pamStatus = pam_start(PAM_SERVICE_NAME,userName,&conv,&m_pamHandler)) != PAM_SUCCESS ){
        m_pamHandler = nullptr;
        qWarning() << "pam_start failed:" << m_pamStatus << pam_strerror(m_pamHandler,m_pamStatus);
        goto out;
    }

    qInfo("pam_start %s %s -> %d %s",
          PAM_SERVICE_NAME,userName,
          m_pamStatus,pam_strerror(m_pamHandler,m_pamStatus));

    pam_set_item(m_pamHandler,PAM_FAIL_DELAY,(void*)no_fail_delay);

    display = qgetenv("DISPLAY");
    if(display.isEmpty()){
        display = ":0.0";
    }

    if( (m_pamStatus = pam_set_item(m_pamHandler,PAM_TTY,display.data())) != PAM_SUCCESS ){
        qWarning("pam_set_item PAM_TTY %s failed,%s",
                 display.toStdString().c_str(),
                 pam_strerror(m_pamHandler,m_pamStatus));
        goto out;
    }

    pam_set_item(m_pamHandler,PAM_USER_PROMPT,"Username:");
    m_authUserName = userName;
    qInfo() << "start thread";
    start();
    bRes = true;
out:
    if(!bRes){
        closePamHandler();
    }
    return bRes;
}

/**
 * @brief 取消用户认证，退出线程，并进行清理
 */
void PamAuthProxy::cancelAuthenticate()
{
    if(m_pamHandler==nullptr||!isRunning()){
        return ;
    }
    //请求线程退出
    requestInterruption();

    m_cancelAuthFlag = true;

    //唤醒可能的阻塞
    m_waitCondition.wakeAll();

    wait();

    //状态复位
    closePamHandler();

    qInfo() << "end cancelAuthenticate";
}

/**
 * @brief 调用该方法来进行传入之前请求的回复，并唤醒等待线程把该参数填入
 * @param 标志成功与否
 * @param 回复的内容
 */
void PamAuthProxy::response(bool ret, const QString &resp)
{
    if( !isRunning() ){
        qWarning() << "thread not runing,response failed";
        return;
    }

    if( m_conversationReply==nullptr ){
        qWarning() << "conversation reply is null,response failed";
        return;
    }
    m_conversationRelplyRet = ret;
    *m_conversationReply = (char*)malloc(resp.length()+1);
    strcpy(*m_conversationReply,resp.toStdString().c_str());
    m_waitCondition.wakeAll();
}

/**
 * @brief 认证线程结束的槽，对线程中设置的状态进行判断
 *        发出不同的认证信号，并对该次认证进行清理
 */
void PamAuthProxy::slotThreadFinished()
{
    qInfo() << "authenticate thread finished";
    emit authenticateComplete(m_authComplete);
    closePamHandler();
}

///FIXME: struct pam_response 释放？ pam_response->msg 释放？
/**
 * @brief   该静态方法是填充到pam_conv结构体中，在pam的认证过程中，需要进行用户交互的回调
 *        若m_proxyCallback不为空，则会调用m_proxyCallback中对应的方法，把response中的msg赋值给m_conversationReply中
 *        并利用条件变量阻塞，直到外部调用response对m_conversationReply进行唤醒线程回复或cancelAuthenticate取消掉认证,
 *        才会继续
 * @param num_msg      该次调用需处理的消息数量
 * @param msg          消息内容
 * @param resp         对每个消息的回应，需进行分配内存，需对resp->msg进行分配和填充
 * @param appdata_ptr  pam_start中参数struct pam_conv中的参数
 * @return 处理成功 PAM_SUCCESS 处理失败 PAM_CONV_ERR
 */
int PamAuthProxy::conversation(int num_msg, const pam_message **msg, pam_response **resp, void *appdata_ptr)
{
    PamAuthProxy* This = static_cast<PamAuthProxy*>(appdata_ptr);
    struct pam_response *reply = NULL;
    bool res;
    int ret;
    int replies;

    reply = (struct pam_response *) calloc (num_msg, sizeof (*reply));
    if (reply == nullptr){
        return PAM_CONV_ERR;
    }

    res = true;
    ret = PAM_SUCCESS;

    for(replies=0;replies<num_msg&&ret==PAM_SUCCESS;replies++){
        bool visiable = false;
        bool replyRet = true;
        if(This->m_cancelAuthFlag){
            qInfo("cancel authenticate flag is set,skip msg [%s]",
                  msg[replies]->msg?msg[replies]->msg:"null");
            goto next_msg;
        }
        qInfo() << "conversation: " << msg[replies]->msg;
        switch (msg [replies]->msg_style) {
        case PAM_PROMPT_ECHO_ON:
            visiable=true;
        case PAM_PROMPT_ECHO_OFF:
            if( This->m_authCallback ){
                This->m_authCallback->requestResponse(msg[replies]->msg,visiable);
                This->waitForResponse(replyRet,reply[replies].resp);
                //qInfo() << msg[replies]->msg << " reply:" << reply[replies].resp;
            }
            break;
        case PAM_ERROR_MSG:
            if( This->m_authCallback ){
                This->m_authCallback->onDisplayError(msg[replies]->msg);
            }
            break;
        case PAM_TEXT_INFO:
            if( This->m_authCallback ){
                This->m_authCallback->onDisplayError(msg[replies]->msg);
            }
            break;
        default:
            break;
        }
next_msg:
        qInfo() << "replyRet" << replyRet;
        if( replyRet && !This->isInterruptionRequested() ){
            reply [replies].resp_retcode = PAM_SUCCESS;
        }else{
            int i;
            for (i = 0; i <= replies; i++){
                if( reply[i].resp!=nullptr )
                    free (reply [i].resp);
            }
            free (reply);
            reply = nullptr;
            ret = PAM_CONV_ERR;
        }
    }

    *resp = reply;
    return ret;
}

/**
 * @brief PamAuthProxy::conversation中调用，阻塞等待直到外部调用response或cancelAuthenticate
 * @param replyRet
 * @param convReply
 */
void PamAuthProxy::waitForResponse(bool& replyRet,char *&convReply)
{
    m_conversationReply = &convReply;
    m_conversationRelplyRet = false;
    m_waitMutex.lock();
    m_waitCondition.wait(&m_waitMutex);
    m_waitMutex.unlock();
    replyRet = m_conversationRelplyRet;
    m_conversationReply = nullptr;
}

/**
 * @brief QThread::start的包装，只是让外部控制不了进程的启动
 * @param priority
 */
void PamAuthProxy::start(QThread::Priority priority)
{
    QThread::start();
}

/**
 * @brief 关闭PAM认证，并复位标志位
 */
void PamAuthProxy::closePamHandler()
{
    if(m_pamHandler!=nullptr){
        pam_end(m_pamHandler,m_pamStatus);
        m_pamHandler = nullptr;
    }
    m_authUserName = "";
    m_pamStatus = -1;
    m_authComplete = false;
    m_conversationReply = nullptr;
    m_conversationRelplyRet = false;
    m_cancelAuthFlag = false;
}

/**
 * @brief 线程中执行认证
 */
void PamAuthProxy::run()
{
    int status = -1;
    int tries = 0;
    qInfo() << "AuthProxy Thread Run";
    do{
        qInfo() << "start pam_authenticate";
        status = pam_authenticate(m_pamHandler,PAM_SILENT);
        qInfo() << "end pam_authenticate";
        if( status==PAM_AUTH_ERR ){
            qInfo() << "pam_authenticate error,PAM_AUTH_ERR";
            emit authenticateError();
        }
    }while( (status==PAM_AUTH_ERR) && ((tries++)<MAX_TRIES) && !isInterruptionRequested() );

    if (status != PAM_SUCCESS){
        qWarning("pam_authenticate failed,%s",pam_strerror(m_pamHandler,status));
        goto done;
    }

    status = pam_acct_mgmt(m_pamHandler,PAM_SILENT);
    if (status != PAM_SUCCESS){
        qWarning("pam_acct_mgmt failed,%s",pam_strerror(m_pamHandler,status));
        goto done;
    }

    status = pam_setcred (m_pamHandler, PAM_REINITIALIZE_CRED);
    if (status != PAM_SUCCESS){
        qWarning("pam_setcred failed,%s",pam_strerror(m_pamHandler,status));
        goto done;
    }
    qInfo() << "AuthProxy Thread Finished";
    m_authComplete = true;
done:
    m_pamStatus = status;
    return;
}
