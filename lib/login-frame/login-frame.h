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
#include <QLabel>
#include <QWidget>
#include "auth-define.h"

namespace Ui
{
class LoginFrame;
}

QT_BEGIN_NAMESPACE
class QLayoutItem;
QT_END_NAMESPACE

// TODO:对于AuthController的修改未改完
namespace Kiran
{
namespace SessionGuard
{
// 提供基础的认证界面给登录以及解锁框
// 负责基础的认证流程
class AuthController;
class UserListWidget;
class AuthBase;
class AuthTypeSwitcher;
class LoginFrame : public QWidget
{
    Q_OBJECT
    enum ControlPageEnum
    {
        CONTROL_PAGE_PROMPT_EDIT,  // 输入框模式
        CONTROL_PAGE_REAUTH,       // 重新认证,认证失败且过程未存在Prompt消息,则不应自动重新开始认证,而是应该显示重新认证按钮,等待用户手动触发认证
        CONTROL_PAGE_EMPTY         // 控制界面为空,用于进行不需要用户操作的生物认证
    };
    enum EditMode
    {
        EDIT_MODE_USER_NAME,      // 输入框输入用户名
        EDIT_MODE_PROMPT_RESPOSE  // 输入框输入PAM询问的prompt回复
    };

public:
    LoginFrame(QWidget* parent = nullptr);
    ~LoginFrame();

    void initAuth(AuthBase* auth);
    void reset();

    void startAuthUser(const QString& userName);
    void setAuthUserInfo(const QString& userName);
    void setTips(MessageType type, const QString& text);

    void setLeftTopWidget(QWidget* w);
    void setLeftBottomWidget(QWidget* w);
    void setRightTopWidget(QWidget* w);
    void setRightBottomWidget(QWidget* w);
    void addWidgetToCenterBottomWidget(int index, QWidget* w, Qt::Alignment align = Qt::AlignCenter);
    void addLayoutItemToCenterBottomWidget(int index, QLayoutItem* item);
    int appendControlPage(QWidget* controlWidget);
    void switchControlPage(int pageIdx);

private:
    void initUI();
    void respond(const QString& response);

    virtual QString getCurrentDateTime();
    Q_INVOKABLE void startUpdateTimeTimer();
    Q_INVOKABLE void updateTimeLabel();

    void capsLockStatusChanged(bool on);
    void setEditFocus(int delayMs = 0);

    // 需认证用的户名输入完成,子类对认证用户名进行判断,是否调用开始认证
    virtual void authUserInputed(const QString& userName) = 0;
    // 认证完成,子类根据认证结果以及用户名执行各自操作
    virtual void authenticateComplete(bool authRes, const QString& userName) = 0;
    // 认证头像下方是否显示FullName
    virtual bool shouldShowFullName() = 0;

private slots:
    void onShowMessage(const QString& text, MessageType type);
    void onShowPrmpt(const QString& text, PromptType type);
    void onAuthComplete(bool authRes);
    void onNotifyAuthMode(KADAuthMode mode);
    void onSupportedAuthTypeChanged(QList<KADAuthType> supportedTypes);
    void onAuthTypeChanged(KADAuthType type);
    void onAuthUserPropertyChanged();

private:
    Ui::LoginFrame* ui;
    QWidget* m_leftTopWidget = nullptr;
    QWidget* m_leftBottomWidget = nullptr;
    QWidget* m_rightTopWidget = nullptr;
    QWidget* m_rightBottomWidget = nullptr;
    AuthController* m_authController = nullptr;
    EditMode m_editMode;
    bool m_prompted = false;
    QString m_specifyUser;
    AuthTypeSwitcher* m_switcher = nullptr;
};
}  // namespace SessionGuard
}  // namespace Kiran
