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
#include <kiran-titlebar-window.h>
#include <PolkitQt1/Agent/Listener>
#include <PolkitQt1/Details>
#include <QObject>
#include <QString>
#include "auth-base.h"

namespace Ui
{
class Dialog;
}

class KiranPasswdEdit;

QT_BEGIN_NAMESPACE
class QGridLayout;
class QLabel;
class QComboBox;
QT_END_NAMESPACE

namespace Kiran
{
namespace SessionGuard
{
class AuthController;
class AuthTypeSwitcher;
}  // namespace SessionGuard
}  // namespace Kiran

namespace Kiran
{
namespace SessionGuard
{
namespace PolkitAgent
{
class AuthPolkit;
class AuthInfo
{
public:
    QString actionID;
    QString message;
    QString iconName;
    PolkitQt1::Details details;
    QString cookie;
    PolkitQt1::Identity::List identities;
    PolkitQt1::Agent::AsyncResult* result;
    void dump();
};

class Dialog : public KiranTitlebarWindow
{
    Q_OBJECT
    enum ButtonLayout
    {
        BUTTON_LAYOUT_NORMAL,
        BUTTON_LAYOUT_REAUTH
    };
public:
    explicit Dialog(QWidget* parent = nullptr);
    ~Dialog();

public:
    bool init(const AuthInfo& authInfo);

signals:
    void cancelled();
    void completed(bool isSuccess);

private:
    void initUI();
    void switchButtonLayout(ButtonLayout layout);
    bool setAuthInfo(const AuthInfo& authInfo);
    void startAuth(const QString& userName);

private slots:
    void onCancelClicked();
    void onOkClicked();
    void onReauthClicked();
    void onCurrentUserChanged(int idx);
    void onSwitcherAuthTypeChanged(KADAuthType authType);

    void onNotifyAuthMode(KADAuthMode mode);
    void onSupportedAuthTypeChanged(QList<KADAuthType> authTypes);
    void onNotifyAuthTypeChanged(KADAuthType authType);

    void onAuthComplete(bool success);
    void onAuthShowPrompt(const QString& text, PromptType promptType);
    void onAuthShowMessage(const QString& text, MessageType msgType);

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    Ui::Dialog* ui;
    AuthController* m_authController;
    AuthTypeSwitcher* m_switcher;
    bool m_havePrompt = false;
    ButtonLayout m_buttonLayout = BUTTON_LAYOUT_NORMAL;
};
}  // namespace PolkitAgent
}  // namespace SessionGuard
}  // namespace Kiran