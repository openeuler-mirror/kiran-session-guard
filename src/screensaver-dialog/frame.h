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

namespace Kiran
{
namespace SessionGuard
{
class AuthProxy;
class VirtualKeyboard;
}  // namespace SessionGuard
}  // namespace Kiran

class GnomeSMProxy;

namespace Kiran
{
namespace SessionGuard
{
namespace Locker
{
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
    void initPassiveAuthLoop();
    /**
     * @brief 延迟到期后正式开启被动循环（gate + 键鼠监听 + 首次认证）
     */
    void beginPassiveAuthLoop();
    /**
     * @brief 确保正式循环 QTimer 已创建并运行（仅被动已激活时）
     */
    void ensurePassiveAuthTimer();
    void stopPassiveAuthLoop(const char* reason);
    /**
     * @brief 通知后端开关锁屏被动认证上报过滤
     * @param[in] enable true=被动循环启动；false=循环结束
     * @return true=通知成功；失败仅记日志，不影响锁屏主流程
     */
    bool notifyPassiveAuthReportFilter(bool enable);
    // override LoginFrame
    // 需认证用的户名输入完成,子类对认证用户名进行判断,是否调用开始认证
    virtual void authUserInputed(const QString &userName){};
    // 认证完成,子类根据认证结果以及用户名执行各自操作
    virtual void authenticateComplete(bool authRes, const QString &userName);
    // 认证类型变化：非软人脸则停被动；SOFT_FACE 且已激活时补建循环 timer
    void onAuthTypeChanged(KADAuthType type) override;
    virtual bool shouldShowFullName() override;
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

    // 被动认证循环
    QTimer* m_passiveAuthTimer = nullptr;
    bool m_passiveAuthActive = false;       /**< 已正式开启（含键鼠停循环监听） */
    bool m_passiveAuthStartPending = false; /**< 已预约延迟启动，尚未到点 */
    int m_passiveLoopSec = 2;
};
}  // namespace Locker
}  // namespace SessionGuard
}  // namespace Kiran