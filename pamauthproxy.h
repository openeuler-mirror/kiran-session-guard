#ifndef PAMWRAPPER_H
#define PAMWRAPPER_H

#include <QThread>
#include <QObject>
#include <security/pam_appl.h>
#include <QWaitCondition>
#include <QMutex>

///FIXME:不能在requestResonse中调用PamAuthProxy::response
class PamAuthCallback{
    friend class PamAuthProxy;
public:
    PamAuthCallback() = default;
    virtual ~PamAuthCallback() = default;
private:
    /**
     * @brief requestResponse
     *        PAM认证线程调用请求回复，对应在界面上就是填写msg到输入框的Label
     *        并在用户确认输入后，点击确认或回车之后，调用PamProxy的response继续进行回应
     * @param msg       用户需输入的内容
     * @param visiable  输入的内容是否可见
     */
    virtual void requestResponse(const char* msg,bool visiable)    = 0;
    /**
     * @brief onDisplayError
     *        PAM认证线程，会调用该方法来显示错误信息
     * @param msg 错误信息
     */
    virtual void onDisplayError(const char* msg)    = 0;
    /**
     * @brief onDisplayTextInfo
     *        PAM认证线程调用该方法来进行显示信息
     * @param msg 信息
     */
    virtual void onDisplayTextInfo(const char* msg) = 0;
};

class PamAuthProxy : public QThread
{
    Q_OBJECT
public:
    explicit PamAuthProxy(PamAuthCallback* callback,QObject *parent = nullptr);
    virtual ~PamAuthProxy();
    bool isInAuthenticate();
    bool startAuthenticate(const char*userName);
    void cancelAuthenticate();
    void response(bool ret, const QString& resp);
signals:
    void authenticateError();
    void authenticateComplete(bool isSuccess);
private slots:
    void slotThreadFinished();
private:
    static int conversation(int num_msg, const struct pam_message **msg,
                            struct pam_response **resp, void *appdata_ptr);
    void waitForResponse(bool& replyRet, char*& convReply);
    void start(Priority = InheritPriority);
    void closePamHandler();
protected:
    virtual void run() Q_DECL_OVERRIDE;
private:
    int m_pamStatus;
    QMutex m_waitMutex;
    QWaitCondition m_waitCondition;
    PamAuthCallback* m_authCallback;
    pam_handle_t* m_pamHandler;
    QString m_authUserName;
    bool m_authComplete;
    char** m_conversationReply;
    bool m_conversationRelplyRet;
    bool m_cancelAuthFlag;
};

#endif // PAMWRAPPER_H
