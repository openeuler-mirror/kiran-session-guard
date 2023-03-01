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
#pragma once
#include <kiran-screensaver/locker-interface.h>
#include "auth-base.h"
#include "guard-global.h"
#include "login-frame.h"

// clang-format off
namespace Kiran{ namespace ScreenSaver { class Interface; } }
// clang-format on

QT_BEGIN_NAMESPACE
class QMenu;
class QWidget;
class QPropertyAnimation;
class QGraphicsOpacityEffect;
class QToolButton;
QT_END_NAMESPACE

GUARD_BEGIN_NAMESPACE
class AuthProxy;
class VirtualKeyboard;
GUARD_END_NAMESPACE

class GnomeSMProxy;

GUARD_LOCKER_BEGIN_NAMESPACE
class Prefs;
class Power;
class Frame : public LoginFrame, public Kiran::ScreenSaver::LockerInterface
{
    Q_OBJECT
public:
    explicit Frame(Kiran::ScreenSaver::Interface *ksinterface, Power *power, QWidget *parent = nullptr);
    virtual ~Frame();

private:
    void initMenus();
    void initUI();
    void initAnimation();
    void initAuth();
    // override LoginFrame
    // 需认证用的户名输入完成,子类对认证用户名进行判断,是否调用开始认证
    virtual void authUserInputed(const QString &userName){};
    // 认证完成,子类根据认证结果以及用户名执行各自操作
    virtual void authenticateComplete(bool authRes, const QString &userName);

public:
    // override LockerInterface
    QWidget *get_widget_ptr() override;
    void setAnimationEnabled(bool enabled) override;
    void setAnimationDelay(int fadeInDelay, int fadeOutDelay) override;
    void setAnimationDuration(int fadeInMs, int fadeOutMs) override;
    bool fadeVisible() override;
    bool fadeIn() override;
    bool fadeOut() override;
    void setEnableSwitch(bool enable) override;
    bool enableSwitch() override;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

private:
    Power *m_power;
    VirtualKeyboard *m_keyboard;
    Kiran::ScreenSaver::Interface *m_ksInterface;
    QString m_userName;
    AuthProxy *m_authProxy;
    QMenu *m_powerMenu;
    QToolButton *m_btnSwitchToGreeter;
    QToolButton *m_btnPower;
    QToolButton *m_btnKeyboard;

    // Forward为淡入，Backward为淡出
    QPropertyAnimation *m_animation;
    QGraphicsOpacityEffect *m_opacityEffect;

    bool m_fadeVisible = false;
    bool m_animationEnabled = false;
    int m_fadeDelayTimer = 0;
    int m_fadeInDelayMs = 0;
    int m_fadeOutDelayMs = 0;
    int m_fadeInDurationMs = 0;
    int m_fadeOutDurationMs = 0;
};
GUARD_LOCKER_END_NAMESPACE