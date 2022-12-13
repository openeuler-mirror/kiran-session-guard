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

#ifndef WIDGET_H
#define WIDGET_H

#include <kiran-screensaver/locker-interface.h>
#include <QAbstractNativeEventFilter>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QWidget>
#include "auth-base.h"

namespace Ui
{
class ScreenSaverDialog;
}

namespace Kiran
{
namespace ScreenSaver
{
class Interface;
}
}  // namespace Kiran

class QMenu;
class AuthProxy;
class VirtualKeyboard;
class ScreenSaverDialog : public QWidget, public Kiran::ScreenSaver::LockerInterface
{
    Q_OBJECT
public:
    explicit ScreenSaverDialog(Kiran::ScreenSaver::Interface *ksinterface, QWidget *parent = nullptr);
    virtual ~ScreenSaverDialog();

public:
    QWidget *get_widget_ptr() override;

    void setAnimationEnabled(bool enabled) override;
    void setAnimationDelay(int fadeInDelay, int fadeOutDelay) override;
    void setAnimationDuration(int fadeInMs, int fadeOutMs) override;

    bool fadeVisible() override;
    bool fadeIn() override;
    bool fadeOut() override;

    // 切换用户按钮
    void setEnableSwitch(bool enable) override;
    bool enableSwitch() override;

private:
    void init();
    void initAuth();
    void initUI();
    void initAnimation();

    Q_INVOKABLE void startUpdateTimeTimer();
    Q_INVOKABLE void updateTimeLabel();
    QString getCurrentDateTime();
    void updateCurrentAuthType(Kiran::AuthType type);

private:
    ///开始进行PAM认证
    void startAuth();

    ///切换输入框到重新认证按钮
    void switchToReauthentication();

    ///显示输入框
    void switchToPromptEdit();

private slots:
    void slotShowMessage(QString text, Kiran::MessageType messageType);
    void slotShowPrompt(QString text, Kiran::PromptType promptType);
    void slotAuthenticationComplete(bool authRes);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    virtual void closeEvent(QCloseEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

private:
    Ui::ScreenSaverDialog *ui;
    QMenu *m_powerMenu = nullptr;

    Kiran::ScreenSaver::Interface *m_ksInterface;

    bool m_fadeVisible = false;
    bool m_animationEnabled = false;
    int m_fadeDelayTimer = 0;
    int m_fadeInDelayMs = 0;
    int m_fadeOutDelayMs = 0;
    int m_fadeInDurationMs = 0;
    int m_fadeOutDurationMs = 0;

    //Forward为淡入，Backward为淡出
    QPropertyAnimation m_animation;
    QGraphicsOpacityEffect *m_opacityEffect = nullptr;

    AuthProxy *m_authProxy = nullptr;
    Kiran::AuthType m_authType = Kiran::AUTH_TYPE_PASSWD;
    bool m_havePrompt = false;
    QString m_userName;
#ifdef VIRTUAL_KEYBOARD
    VirtualKeyboard *m_keyboard = nullptr;
#endif
};

#endif  // WIDGET_H
