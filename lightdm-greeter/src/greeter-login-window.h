/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd.
 * kiran-session-guard is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     liuxinhao <liuxinhao@kylinos.com.cn>
 */

#ifndef GREETERLOGINWINDOW_H
#define GREETERLOGINWINDOW_H

#include <QLightDM/Greeter>
#include <QLightDM/Power>
#include <QLightDM/SessionsModel>
#include <QLightDM/UsersModel>
#include <QPixmap>
#include <QStateMachine>
#include <QWidget>
#include <QWindow>

#include "auth-define.h"
#include "auth-msg-queue.h"
#include "capslock-snoop.h"
#include "user-info.h"
#include "filter-user-proxy-model.h"

namespace Ui
{
class GreeterLoginWindow;
}

/* 标志按钮的两种功能 */
enum ButtonType
{
    BUTTON_SWITCH_TO_MANUAL_LOGIN, /** < 标志按钮为切换到手动登录 **/
    BUTTON_RETURN,                 /** < 标志按钮为返回 **/
};

enum LoginMode
{
    LOGIN_MODE_USER_LIST, /** < 通过用户列表登录 **/
    LOGIN_MODE_MANUAL     /** < 通过手动输入用户密码名方式登录 **/
};

enum EditInputMode
{
    EDIT_INPUT_USER_NAME,     /** < 输入框当前输入用户名 **/
    EDIT_INPUT_PROMPT_RESPOND /** < 输入框当前输入pam prompt回复内容 **/
};

enum AuthType
{
    AUTH_TYPE_PASSWD,
    AUTH_TYPE_FINGER,
    AUTH_TYPE_FACE
};

class QProcess;
class QScreen;
class QMenu;
class AuthProxy;
class GreeterLoginWindow : public QWidget
{
    Q_OBJECT

public:
    explicit GreeterLoginWindow(QWidget *parent = nullptr);
    virtual ~GreeterLoginWindow() override;
    void setEditPromptFocus(int ms = 0);

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
    void setTips(Kiran::MessageType type, const QString &text);
    //认证用户名
    void startAuthUser(const QString &username, QString userIcon = QString(""));
    //用户列表模式重设UI
    void resetUIForUserListLogin();
    //手动登录模式重设UI
    void resetUIForManualLogin();
    Q_INVOKABLE void startUpdateTimeTimer();
    Q_INVOKABLE void updateTimeLabel();
    QString getCurrentDateTime();
    static void capsLockStatusChanged(bool on, void *user_data);

    ///切换到输入框输入
    void switchToPromptEdit();
    ///当前用户为自动登录用户时,切换到自动登录按钮显示,
    void switchToAutoLogin();
    void switchToReAuthentication();

    void setCurrentAuthType(AuthType type);

private slots:
    void slotTextConfirmed(const QString &text);
    void slotButtonClicked();
    /*用户列表切换，重新开始认证 */
    void slotUserActivated(const UserInfo &userInfo);

    void slotShowMessage(QString text, Kiran::MessageType type);
    void slotShowprompt(QString text, Kiran::PromptType type);
    void slotAuthenticationComplete(bool sucess);

protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::GreeterLoginWindow *ui;

    FilterUserProxyModel     m_filterModel;

    QLightDM::Greeter m_greeter;
    QLightDM::UsersModel m_userModel;
    QLightDM::PowerInterface m_powerIface;

    AuthProxy* m_authProxy = nullptr;

    QMenu *m_powerMenu = nullptr;
    QMenu *m_sessionMenu = nullptr;
    QString m_session; /** < 用户选中session的名称 **/

    CapsLockSnoop m_snoop;
    bool m_noListButotnVisiable;            /** < 配置项: 是否允许手动登录 **/
    bool m_showUserList;                    /** < 配置项: 显示用户列表 **/
    LoginMode m_loginMode;                  /** < 标志当前登录的模式,当前是手动输入用户名或选择用户进行登录 **/
    ButtonType m_buttonType;                /** < 标志'btn_notListAndCancel'按钮当前的作用 **/
    EditInputMode m_inputMode;              /** < 标志输入框输入模式,输入用户名/还是回复prompt消息 **/
    AuthType m_authType = AUTH_TYPE_PASSWD; /** < 标志当前的认证模式 **/
    bool m_havePrompted = false;            /** < 该次认证是否有过PAM的prompt消息，用于判断是否重新开始认证，避免死循环 **/
};

#endif  // GREETERLOGINWINDOW_H
