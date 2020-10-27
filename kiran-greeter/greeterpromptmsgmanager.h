#ifndef GREETERPROMPTMSGMANAGER_H
#define GREETERPROMPTMSGMANAGER_H

#include <QObject>
#include <QLightDM/Greeter>
#include <QThread>
#include <QQueue>
#include <QSemaphore>
#include <QWaitCondition>
#include <QMutex>


enum LoginMode{
    LOGIN_BY_USER_LIST,
    LOGIN_BY_INPUT_USER
};

/**
 * @brief 为了解决消息显示覆盖的问题,提供LightdmGreeter认证消息的一个队列
 */
class GreeterPromptMsgManager:public QThread
{
    Q_OBJECT
    enum LightdmPromptMsgType{
        LIGHTDM_MSG,
        LIGHTDM_PROMPT,
        LIGHTDM_AUTHENTICATION_COMPLETE
    };
    struct LightdmPromptMsg{
        /// 消息类型
        LightdmPromptMsgType type;
        /// 消息内容
        union detail{
            QLightDM::Greeter::MessageType msgType;
            QLightDM::Greeter::PromptType promptType;
            struct AuthenticationCompeteResult{
                bool isSuccess;
                bool reAuthentication;
            } authenticationCompleteResult;
        }info;
        /// 消息中携带的内容
        QString text;
    };

public:
    explicit GreeterPromptMsgManager(QLightDM::Greeter *greeter,QObject *parent = Q_NULLPTR);
    ~GreeterPromptMsgManager();

    /**
     * @brief 设置消息间隔
     * @param sec 消息间隔(s)
     */
    void setMessageInterval(int sec);

    /**
     * @brief 清空所有消息,重置消息队列,复位所有标志位
     */
    void reset();

    /**
     * @brief 设置登录模式，用该信息返回不同认证错误的信息
     * @param loginMode
     */
    void setLoginMode(LoginMode loginMode);

private Q_SLOTS:
    /**
     * @brief 连接到QLightdm::Greeter::addMsgToQueue的槽函数,将Msg消息加入到队列中,排队提供给UI
     * @param text 内容
     * @param type 类型
     */
    void addMsgToQueue(QString text,QLightDM::Greeter::MessageType type);
    /**
     * @brief 连接到QLightdm::Greeter::addPromptToQueue的槽函数,将Prompt消息加入到队列中,排队提供给UI
     * @param text 内容
     * @param type 类型
     */
    void addPromptToQueue(QString text,QLightDM::Greeter::PromptType type);
    /**
     * @brief 连接到QLightdm::Greeter::authenticationComplete的槽函数,将认证结果加入到队列中,排队提供给UI
     */
    void addAuthenticationCompleteToQueue();
Q_SIGNALS:
    /**
     * @brief 经消息队列发出的显示Msg消息信号
     * @param text 内容
     * @param type 类型
     */
    void showMessage(QString text,QLightDM::Greeter::MessageType type);
    /**
     * @brief 经消息队列发出的显示Prmpt消息信号
     * @param text 内容
     * @param type 类型
     */
    void showPrompt(QString text,QLightDM::Greeter::PromptType type);
    /**
     * @brief 经消息队列发出的认证完成消息信号
     * @param success 是否认证完成
     * @param reAuthentication 认证失败时,是否需要重新开启认证
     */
    void authenticationComplete(bool success,bool reAuthentication);
private:
    /**
     * @brief 将消息追加到队列
     * @param msg 需追加消息
     */
    void addMsgItemToQueue(const LightdmPromptMsg& msg);
    /**
     * @brief 尝试从消息队列中获取消息
     * @param msg 获取到的消息
     * @param ms 超时时间(ms)
     * @return 是否获取成功
     */
    bool getMsgItemFromQueue(LightdmPromptMsg& msg,int ms);
    /**
     * @brief 获取开机到现在的时间(作为消息的时间标志)
     * @return 开机到现在的时间
     */
    qint64 getUpTime();

    QString dumpMsgInfoToString(const LightdmPromptMsg& msg);
protected:
    /**
     * @brief 线程中运行,负责对消息队列中的消息进行分发和阻塞等待
     */
    virtual void run() Q_DECL_OVERRIDE;
private:
    /// 消息等待间隔(秒)
    int m_messageInterval = 3;
    /// 上条Msg类型的消息显示的时间(从开机到显示的时间uptime)
    qint64 m_messageShowTime = 0;
    /// 消息信号量
    QSemaphore m_semaphore;
    /// 维护的消息队列
    QQueue<LightdmPromptMsg> m_msgQueue;
    QMutex m_msgQueueMutex;
    QLightDM::Greeter*  m_greeter = nullptr;
    /// 标志该次认证是否有Prompted消息(做为认证失败,提供错误信息和判断是否重新开始认证的辅助条件)
    bool m_havePrompted = false;
    /// 标志该次认证是否有Error Msg消息(做为认证失败,提供错误信息辅助条件)
    bool m_havePAMError = false;
    /// 分发消息阻塞条件变量互斥锁
    QMutex m_sleepCancelMutex;
    /// 分发消息阻塞条件变量(提供线程外取消阻塞的方法)
    QWaitCondition m_sleepCancelCondition;
    LoginMode m_loginMode;
};

#endif // LIGHTDMGREETERQUEUEWRAPPER_H
