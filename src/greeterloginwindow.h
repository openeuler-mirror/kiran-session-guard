#ifndef GREETERLOGINWINDOW_H
#define GREETERLOGINWINDOW_H

#include <QWidget>
#include <QPixmap>
#include <QLightDM/Greeter>
#include <QLightDM/Power>
#include <QLightDM/SessionsModel>
#include <QLightDM/UsersModel>
#include <QWindow>
#include <QStateMachine>

#include "userinfo.h"
#include "capslocksnoop.h"
#include "auth-msg-queue.h"

namespace Ui {
class GreeterLoginWindow;
}

/* 标志按钮的两种功能 */
enum ButtonType{
    BUTTON_SWITCH_TO_MANUAL_LOGIN,
    BUTTON_RETURN,
};

enum LoginMode{
    LOGIN_MODE_USER_LIST,     /** < 通过用户列表登录 **/
    LOGIN_MODE_MANUAL         /** < 通过手动输入用户密码名方式登录 **/
};

class QProcess;
class QScreen;
class QMenu;
class GreeterLoginWindow : public QWidget
{
    Q_OBJECT
public:
    explicit GreeterLoginWindow(QWidget *parent = nullptr);
    virtual ~GreeterLoginWindow() override;

    void setEditPromptFocus(int ms=0);
private:
    //初始化LightdmGreeter，并连接信号槽
    void initLightdmGreeter();
    //初始化配置
    void initSettings();
    //初始UI
    void initUI();
    //初始化菜单
    void initMenu();
    //设置TIPS
    void setTips(AuthMsgQueue::MessageType type, const QString& text);
    //认证用户名
    void startAuthUser(const QString& username, QString userIcon=QString(""));
    //用户列表模式重设UI
    void resetUIForUserListLogin();
    //手动登录模式重设UI
    void resetUIForManualLogin();
    Q_INVOKABLE void startUpdateTimeTimer();
    Q_INVOKABLE void updateTimeLabel();
    QString getCurrentDateTime();
    static void capsLockStatusChanged(bool on,void* user_data);

    ///切换到输入框输入
    void switchToPromptEdit();
    ///当前用户为自动登录用户时,切换到自动登录按钮显示,
    void switchToAutoLogin();
    void switchToReAuthentication();

private slots:
    void slotTextConfirmed(const QString& text);
    void slotButtonClicked();
    /*用户列表切换，重新开始认证 */
    void slotUserActivated(const UserInfo&userInfo);

    void slotShowMessage(QString text, AuthMsgQueue::MessageType type);
    void slotShowprompt(QString text, AuthMsgQueue::PromptType type);
    void slotAuthenticationComplete(bool sucess,bool reAuthentication);

protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::GreeterLoginWindow *ui;

    QLightDM::Greeter m_greeter;
    QLightDM::UsersModel m_userModel;
    QLightDM::PowerInterface m_powerIface;

    AuthMsgQueue m_authQueue;

    QMenu* m_powerMenu;
    QMenu* m_sessionMenu;

    //配置项 允许手动登录
    bool m_noListButotnVisiable;
    //配置项 显示用户列表
    bool m_showUserList;
    //标志当前登录的模式,当前是手动输入用户名或选择用户进行登录
    //TODO:input mode移动到这
    LoginMode m_loginMode;
    //标志按钮当前的作用
    bool m_buttonType;
    //选中的session的名称
    QString m_session;
    CapsLockSnoop m_snoop;

    bool m_havePAMError = false;
    bool m_havePrompted = false;
};

#endif // GREETERLOGINWINDOW_H
