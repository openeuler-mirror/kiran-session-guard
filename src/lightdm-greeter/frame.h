/**
 * Copyright (c) 2020 ~ 2023 KylinSec Co., Ltd.
 * kiran-session-guard is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     liuxinhao <liuxinhao@kylinsec.com.cn>
 */
#pragma once
#include <QLightDM/Greeter>
#include <QLightDM/Power>
#include <QLightDM/SessionsModel>
#include <QLightDM/UsersModel>
#include <QMap>
#include <QSharedPointer>
#include <QWidget>
#include "login-frame.h"

QT_BEGIN_NAMESPACE
class QMenu;
class QToolButton;
QT_END_NAMESPACE

namespace Kiran
{
namespace SessionGuard
{
namespace Greeter
{
class Prefs;
class GreeterMenuItem;
class UserList;
class LoginButton;
class Frame : public LoginFrame
{
    Q_OBJECT
    enum State
    {
        STATE_NONE,
        STATE_USER_LIST_LOGIN,
        STATE_INTPUT_USER_NAME,
        STATE_MANUAL_LOGIN_AUTH
    };

public:
    Frame(Prefs* prefs, QWidget* parent = nullptr);
    ~Frame();

    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void initSettings();
    void initMenus();
    void initUI();
    void initAuth();
    void initConnection();

    // 更新界面
    void reset(State state);
    // 认证用户输入完成,判断是否执行开始认证
    void authUserInputed(const QString& userName) override;
    // 认证用户认证完成
    void authenticateComplete(bool authRes, const QString& userName) override;
    bool shouldShowFullName() override;
private slots:
    void onUserSelected(const QString& name);
    void onUserListUserCountChanged(int oldCount, int newCount);
    void onUserListUserRemoved(const QString& name);
    void onLoginOtherClicked();
    void onAutoLoginTimeout();

private:
    Prefs* m_prefs;

    State m_state = STATE_NONE;
    QString m_userName;
    QString m_specifiedSession;
    bool m_allowManualLogin = false;
    bool m_showUserList = true;

    int m_autloginIdx = 0;
    LoginButton* m_btnAutoLogin;
    QMenu* m_sessionMenu;
    QMenu* m_powerMenu;
    QToolButton* m_btnSession;
    QToolButton* m_btnPower;
    QToolButton* m_btnKeyboard;
    QToolButton* m_btnLoginOther;
    UserList* m_userList;

    QLightDM::PowerInterface m_powerIface;
    QSharedPointer<QLightDM::Greeter> m_greeter;
    QMap<QString, GreeterMenuItem*> m_sessionItemMap;
};
}  // namespace Greeter
}  // namespace SessionGuard
}  // namespace Kiran