#ifndef PAMWRAPPER_H
#define PAMWRAPPER_H

#include <QThread>
#include <QObject>
#include <security/pam_appl.h>
#include <QWaitCondition>
#include <QMutex>

class PamAuthProxy : public QThread
{
    Q_OBJECT
    Q_ENUMS(PromptType MessageType)
public:
    enum PromptType {
        PromptTypeQuestion,
        PromptTypeSecret
    };
    enum MessageType {
        MessageTypeInfo,
        MessageTypeError
    };
    enum AuthState{
        AUTH_STATE_NOT_RUNNING,
        AUTH_STATE_RUNNING,
        AUTH_STATE_WAIT_RESPONSE,
        AUTH_STATE_WAIT_RE_AUTH
    };
    explicit PamAuthProxy(QObject *parent = nullptr);
    virtual ~PamAuthProxy();

    AuthState state();

    bool isAuthenticated();
    bool inAuthentication();
    void startAuthenticate(const QString& userName);
    void cancelAuthenticate();
    void stopAuthenticate();
    void response(const QString& str);
    void reAuthenticate();

signals:
    void showMessage(QString text, PamAuthProxy::MessageType type);
    void showPrompt(QString text, PamAuthProxy::PromptType type);
    void authenticationComplete();

private:
    static int conversation(int num_msg, const struct pam_message **msg,struct pam_response **resp, void *appdata_ptr);
    void waitForResponse();
    void waitForReAuthenticate();
    void resetFlag();
    void start(Priority = InheritPriority);

protected:
    virtual void run() Q_DECL_OVERRIDE;

private:
    AuthState m_state = AUTH_STATE_NOT_RUNNING;

    QMutex m_waitMutex;
    QWaitCondition m_waitCondition;

    QMutex m_reAuthMutex;
    QWaitCondition m_reAuthCOndition;

    QString m_authUserName;
    bool m_authRes = false;

    QPair<bool,QString> m_conversationRep;
};

#endif // PAMWRAPPER_H
