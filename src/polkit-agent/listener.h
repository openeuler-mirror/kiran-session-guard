#pragma once
#include <PolkitQt1/Agent/Listener>
#include <QObject>
#include "guard-global.h"

GUARD_POLKIT_AGENT_BEGIN_NAMESPACE
class Dialog;
class Listener : public PolkitQt1::Agent::Listener
{
    Q_OBJECT
public:
    Listener(QObject *parent = nullptr);
    ~Listener();

public slots:

    /**
     * \brief 发起身份认证
     * 此方法将在已注册的身份验证代理上调用
     * \param actionId   要进行身份认证的操作
     * \param message    要呈现给用户的消息
     * \param iconName   操作图标的名层
     * \param details    描述操作的详细信息
     * \param cookie     身份验证请求的cookie
     * \param identities 用户可以选择作为其进行身份验证的Identity对象列表
     * \param result 这个AsyncResult必须在身份验证完成时使用complete()方法完成
     * 可以将它传递给Session类的构造函数，然后调用Session ->result()->complete()来标记操作已完成
     */
    virtual void initiateAuthentication(const QString &actionId,
                                        const QString &message,
                                        const QString &iconName,
                                        const PolkitQt1::Details &details,
                                        const QString &cookie,
                                        const PolkitQt1::Identity::List &identities,
                                        PolkitQt1::Agent::AsyncResult *result) override;

    /**
     * \brief 完成来自polkit守护进程的身份验证请求
     */
    virtual bool initiateAuthenticationFinish() override;

    /**
     * \brief 取消来自polkit守护进程的身份验证请求。
     */
    virtual void cancelAuthentication() override;

private slots:
    void onAuthDialogCompleted(bool isSuccess);
    void onAuthDialogCancelled();

private:
    bool m_inProcess = false;
    PolkitQt1::Agent::AsyncResult *m_result = nullptr;
    Dialog *m_authDialog = nullptr;
};
GUARD_POLKIT_AGENT_END_NAMESPACE