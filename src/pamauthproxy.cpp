#include "pamauthproxy.h"
#include <QDebug>
#include <QDateTime>

#define PAM_SERVICE_NAME "mate-screensaver"
#define MAX_TRIES 5

void no_fail_delay(int status, unsigned int delay, void *appdata_ptr) {

}

PamAuthProxy::PamAuthProxy(QObject *parent)
        : QThread(parent){
    qRegisterMetaType<PamAuthProxy::PromptType>("PamAuthProxy::PromptType");
    qRegisterMetaType<PamAuthProxy::MessageType>("PamAuthProxy::MessageType");
    connect(this,&QThread::finished,[this](){
        resetFlag();
    });
}

PamAuthProxy::~PamAuthProxy() {
    if (!isFinished()) {
        QThread::requestInterruption();
        m_waitCondition.wakeAll();
        m_reAuthCOndition.wakeAll();
        if (!QThread::wait(1000)) {
            qWarning() << "pam auth thread can't stop,terminate it.";
            QThread::terminate();
            QThread::wait();
        }
    }
    //清理标志
    resetFlag();
}

void PamAuthProxy::startAuthenticate(const QString &userName) {
    if( m_state!=AUTH_STATE_NOT_RUNNING ){
        qWarning() << "start failed,is running";
        return;
    }
    m_authUserName = userName;
    m_state = AUTH_STATE_RUNNING;
    start();
}

void PamAuthProxy::reAuthenticate() {
    if( m_state!=AUTH_STATE_WAIT_RE_AUTH ){
        qWarning() << "current state is not wait reauth";
        return;
    }
    m_reAuthCOndition.wakeAll();
}

void PamAuthProxy::response(const QString &str) {
    if( m_state != AUTH_STATE_WAIT_RESPONSE ){
        qWarning() << "current state is nor wait response";
        return;
    }
    m_conversationRep = {true, str};
    m_waitCondition.wakeAll();
    m_state = AUTH_STATE_RUNNING;
}

int PamAuthProxy::conversation(int num_msg, const pam_message **msg, pam_response **resp, void *appdata_ptr) {
    PamAuthProxy *This = static_cast<PamAuthProxy *>(appdata_ptr);
    struct pam_response *reply = NULL;
    bool res;
    int ret;
    int replies;

    reply = (struct pam_response *) calloc(num_msg, sizeof(*reply));
    if (reply == nullptr) {
        return PAM_CONV_ERR;
    }

    res = true;
    ret = PAM_SUCCESS;
    for (replies = 0; replies < num_msg && ret == PAM_SUCCESS; replies++) {
        bool visiable = false;
        bool replyRet = true;
        if (This->isInterruptionRequested()) {
            qInfo("thread interruption is set,skip msg [%s]", msg[replies]->msg ? msg[replies]->msg : "null");
            goto next_msg;
        }
        qInfo() << "conversation type[" << msg[replies]->msg_style << "]" << " " << msg[replies]->msg;
        switch (msg[replies]->msg_style) {
            case PAM_PROMPT_ECHO_ON: {
                emit This->showPrompt(msg[replies]->msg, PromptTypeQuestion);
                This->waitForResponse();
                reply[replies].resp_retcode = This->m_conversationRep.first ? PAM_SUCCESS : PAM_CONV_ERR;
                std::string str = This->m_conversationRep.second.toStdString();
                reply[replies].resp = new char[str.length() + 1];
                strcpy(reply[replies].resp, str.c_str());
                break;
            }
            case PAM_PROMPT_ECHO_OFF: {
                emit This->showPrompt(msg[replies]->msg, PromptTypeSecret);
                This->waitForResponse();
                reply[replies].resp_retcode = This->m_conversationRep.first ? PAM_SUCCESS : PAM_CONV_ERR;
                std::string str = This->m_conversationRep.second.toStdString();
                reply[replies].resp = new char[str.length() + 1];
                strcpy(reply[replies].resp, str.c_str());
                break;
            }
            case PAM_ERROR_MSG: {
                emit This->showMessage(msg[replies]->msg, MessageTypeError);
                break;
            }
            case PAM_TEXT_INFO: {
                emit This->showMessage(msg[replies]->msg, MessageTypeInfo);
                break;
            }
            default:
                break;
        }
        next_msg:
        qInfo() << "replyRet" << replyRet;
        if (replyRet && !This->isInterruptionRequested()) {
            reply[replies].resp_retcode = PAM_SUCCESS;
        } else {
            int i;
            for (i = 0; i <= replies; i++) {
                if (reply[i].resp != nullptr)
                    free(reply[i].resp);
            }
            free(reply);
            reply = nullptr;
            ret = PAM_CONV_ERR;
        }
    }
    *resp = reply;
    return ret;
}

void PamAuthProxy::waitForResponse() {
    m_waitMutex.lock();
    m_state = AUTH_STATE_WAIT_RESPONSE;
    m_waitCondition.wait(&m_waitMutex);
    m_waitMutex.unlock();
}

void PamAuthProxy::start(QThread::Priority priority) {
    QThread::start();
}

void PamAuthProxy::run() {
    const struct pam_conv conv = {
            &PamAuthProxy::conversation,
            this
    };

    pam_handle_t *pamh = nullptr;
    int iRet = 0;

    qInfo() << "pam_start-->" << m_authUserName;
    iRet = pam_start(PAM_SERVICE_NAME, m_authUserName.toStdString().c_str(), &conv, &pamh);
    if (iRet != PAM_SUCCESS) {
        //goto clean
        qWarning() << "pam_start failed," << pam_strerror(pamh, iRet);
        goto end;
    }

    while (!QThread::isInterruptionRequested()) {
        m_state = AUTH_STATE_RUNNING;
        iRet = pam_authenticate(pamh, 0);
        if (iRet != PAM_SUCCESS) {
            qWarning() << "pam_authenticate failed," << pam_strerror(pamh, iRet);
            emit authenticationComplete();
            waitForReAuthenticate();
            continue;
        }else{
            m_authRes = true;
            emit authenticationComplete();
            break;
        }
    }

end:
    pam_end(pamh, iRet);
    m_authRes = iRet == PAM_SUCCESS;
    qInfo() << "pam auth thread finished";
}

bool PamAuthProxy::isAuthenticated() {
    return m_authRes;
}

void PamAuthProxy::waitForReAuthenticate() {
    m_reAuthMutex.lock();
    m_state = AUTH_STATE_WAIT_RE_AUTH;
    m_reAuthCOndition.wait(&m_reAuthMutex);
    m_reAuthMutex.unlock();
}

PamAuthProxy::AuthState PamAuthProxy::state() {
    return m_state;
}

void PamAuthProxy::resetFlag() {
    m_state = AUTH_STATE_NOT_RUNNING;
    m_waitMutex.unlock();
    m_waitCondition.wakeAll();
    m_reAuthMutex.unlock();
    m_reAuthCOndition.wakeAll();
    m_authRes = false;
    m_conversationRep = {false,""};
}
