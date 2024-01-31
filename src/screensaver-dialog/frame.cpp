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

#include "frame.h"
#include "auth-controller.h"
#include "auth-pam.h"
#include "auxiliary.h"
#include "power.h"
#include "prefs.h"
#include "user-manager.h"
#include "virtual-keyboard.h"

#include <kiran-screensaver/interface.h>
#include <qt5-log-i.h>
#include <QApplication>
#include <QBoxLayout>
#include <QFile>
#include <QGraphicsOpacityEffect>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPropertyAnimation>
#include <QToolButton>

#define DEFAULT_STYLE_PATH ":/locker/stylesheets/kiran-screensaver-dialog-normal.qss"

QT_BEGIN_NAMESPACE
Q_WIDGETS_EXPORT void qt_blurImage(QPainter* p, QImage& blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0);
QT_END_NAMESPACE

namespace Kiran
{
namespace SessionGuard
{
namespace Locker
{
Frame::Frame(Kiran::ScreenSaver::Interface* ksinterface, Power* power, QWidget* parent)
    : LoginFrame(parent),
      m_power(power),
      m_keyboard(nullptr),
      m_ksInterface(ksinterface),
      m_userName(UserManager::getCurrentUser()),
      m_authProxy(nullptr)
{
    KLOG_DEBUG() << "locker create for" << m_userName;
    initMenus();
    initUI();
    initAnimation();
    initAuth();
}

Frame::~Frame()
{
    if (m_keyboard)
    {
        if (m_keyboard->getKeyboard() && m_keyboard->getKeyboard()->parentWidget() == this)
        {
            m_keyboard->getKeyboard()->setParent(nullptr);
        }
    }
}

void Frame::initMenus()
{
    m_powerMenu = new QMenu(this);
    m_powerMenu->setAttribute(Qt::WA_TranslucentBackground);  // 透明必需
    // NOTE:QMenu不能为窗口，只能为子控件，不然透明效果依赖于窗口管理器混成特效与显卡
    // 控件的话QMenu显示出来的话，不能点击其他区域隐藏窗口，需要手动隐藏
    m_powerMenu->setWindowFlags(Qt::FramelessWindowHint | Qt::Widget);  // 透明必需
    m_powerMenu->setFixedWidth(92);
    m_powerMenu->hide();

    // clang-format off
    if (m_power->canReboot())
    {
        m_powerMenu->addAction(tr("reboot"), [this]{ m_power->reboot(); });
    }
    if (m_power->canPoweroff())
    {
        m_powerMenu->addAction(tr("shutdown"), this, [this]{ m_power->poweroff(); });
    }
    if (m_power->canSuspend())
    {
        m_powerMenu->addAction(tr("suspend"), this, [this]{ m_power->suspend(); });
    }
    connect(m_powerMenu, &QMenu::triggered, this, [this]{ m_powerMenu->hide(); });
    // clang-format on
}

void Frame::initUI()
{
    // 初始化添加右下控件
    // 获取菜单弹出坐标,按钮和触发菜单右对齐
    // NOTE: Qt在特定虚拟机环境下QMenu::popup传入正确的pos时,QMenu通过Pos找到screen,但screen的大小错误(为调整分辨率之前的分辨率)
    // 导致popup pos被修改成在错误的屏幕范围内
    auto getMenuPopupPos = [](QMenu* menu, const QToolButton* triggerBtn) -> QPoint
    {
        QSize menuSize = menu->actions().count() == 0 ? QSize(92, 10) : menu->sizeHint();
        QPoint btnRightTopPos = triggerBtn->mapToGlobal(QPoint(triggerBtn->width(), 0));
        QPoint menuLeftTopPos(btnRightTopPos.x() - menuSize.width(), btnRightTopPos.y() - 4 - menuSize.height());
        return menuLeftTopPos;
    };

    // 创建初始化右下角按钮
    auto createActionButton = [this](const QString& objName,
                                     const QString& tooltip,
                                     std::function<void()> triggerSlot) -> QToolButton*
    {
        auto button = new QToolButton(this);
        button->setObjectName(objName);
        button->setToolTip(tooltip);
        button->setFixedSize(QSize(48, 40));
        QSizePolicy sizePolicy = button->sizePolicy();
        sizePolicy.setHeightForWidth(true);
        sizePolicy.setHorizontalStretch(1);
        sizePolicy.setVerticalStretch(1);
        sizePolicy.setHorizontalPolicy(QSizePolicy::Preferred);
        sizePolicy.setVerticalPolicy(QSizePolicy::Preferred);

        button->setSizePolicy(sizePolicy);
        button->setTabletTracking(true);
        button->setSizeIncrement(QSize(0, 0));
        button->setCursor(QCursor(Qt::PointingHandCursor));
        connect(button, &QToolButton::pressed, triggerSlot);
        return button;
    };

    // clang-format off
    m_btnSwitchToGreeter = createActionButton("btn_switchuser", tr("switch to greeter"), [](){
        KLOG_DEBUG() << "switch to greeter clicked";
        UserManager::switchToGreeter();
    });
    m_btnKeyboard = createActionButton("btn_keyboard",tr("virtual keyboard"),[this](){
        if( m_keyboard->isVisible() )
        {
            m_keyboard->hide();
        }
        else
        {
            m_keyboard->showAdjustSize(this);
        }
    });
    m_btnPower = createActionButton("btn_power", tr("power menu"), [this,getMenuPopupPos](){
        KLOG_DEBUG() << "power button clicked";
        if (m_powerMenu->isVisible())
        {
            m_powerMenu->hide();
            return;
        }
        QPoint pos = getMenuPopupPos(m_powerMenu,m_btnPower);
        m_powerMenu->popup(pos);
    });
    // clang-format on

    auto rbBtnWidget = new QWidget(this);
    rbBtnWidget->setMinimumSize(QSize(114, 40));
    rbBtnWidget->setMaximumSize(QSize(228, 40));

    auto rbBtnLayout = new QHBoxLayout(rbBtnWidget);
    rbBtnLayout->setSpacing(20);
    rbBtnLayout->setMargin(0);

    auto rbBtnLayoutItem = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    rbBtnLayout->addItem(rbBtnLayoutItem);
    rbBtnLayout->addWidget(m_btnSwitchToGreeter, 1);
    rbBtnLayout->addWidget(m_btnKeyboard, 1);
    rbBtnLayout->addWidget(m_btnPower, 1);
    setRightBottomWidget(rbBtnWidget);

    m_btnSwitchToGreeter->setVisible(false);
    if (m_powerMenu->actions().size() == 0)
    {
        m_btnPower->setVisible(false);
    }

    // 初始化虚拟键盘
    m_keyboard = VirtualKeyboard::instance();
    m_keyboard->init();

    // 加载样式表
    QFile file(DEFAULT_STYLE_PATH);
    if (file.open(QIODevice::ReadOnly))
    {
        auto stylesheet = file.readAll();
        this->setStyleSheet(stylesheet);
    }
}

void Frame::initAnimation()
{
    m_opacityEffect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(m_opacityEffect);
    m_opacityEffect->setOpacity(0);

    m_animation = new QPropertyAnimation(this);
    m_animation->setTargetObject(m_opacityEffect);
    m_animation->setPropertyName("opacity");
    m_animation->setStartValue(0);
    m_animation->setEndValue(1);
    m_animation->setDuration(m_fadeInDurationMs);
}

void Frame::initAuth()
{
    AuthPam* auth = new AuthPam();
    LoginFrame::initAuth(auth);

    startAuthUser(m_userName);
}

// TODO: 解锁框或模式下按压其他用户的指纹,应为错误
void Frame::authenticateComplete(bool authRes, const QString& userName)
{
    if (userName != m_userName)
    {
        return;
    }

    if (authRes)
    {
        m_ksInterface->authenticationPassed();
    }
}

QWidget* Frame::get_widget_ptr()
{
    return this;
}

void Frame::setAnimationEnabled(bool enabled)
{
    RETURN_IF_FALSE(enabled != m_animationEnabled);

    if (!enabled)
    {
        if (m_animation->state() == QPropertyAnimation::Running)
        {
            QVariant var = m_animation->endValue();
            m_animation->stop();
            qreal opacity = var.toReal();
            m_opacityEffect->setOpacity(opacity);
        }
    }

    m_animationEnabled = enabled;
}

void Frame::setAnimationDelay(int fadeInDelay, int fadeOutDelay)
{
    m_fadeInDelayMs = fadeInDelay;
    m_fadeOutDelayMs = fadeOutDelay;
}

void Frame::setAnimationDuration(int fadeInMs, int fadeOutMs)
{
    if (m_fadeInDurationMs != fadeInMs)
    {
        if (m_animation->state() == QPropertyAnimation::Running &&
            m_animation->direction() == QPropertyAnimation::Forward)
        {
            // QVariantAnimation会自动计算更新当前剩余时间
            m_animation->setDuration(fadeInMs);
        }
        m_fadeInDurationMs = fadeInMs;
    }

    if (m_fadeOutDurationMs != fadeOutMs)
    {
        if ((m_animation->state() == QPropertyAnimation::Running) &&
            (m_animation->direction() == QPropertyAnimation::Backward))
        {
            m_animation->setDuration(fadeOutMs);
        }
        m_fadeOutDurationMs = fadeOutMs;
    }
}

bool Frame::fadeVisible()
{
    return m_fadeVisible;
}

bool Frame::fadeIn()
{
    RETURN_VAL_IF_FALSE(!m_fadeVisible, true);

    m_fadeVisible = true;

    if (m_fadeDelayTimer != 0)
    {
        killTimer(m_fadeDelayTimer);
        m_fadeDelayTimer = 0;
    }

    if (m_animationEnabled)
    {
        m_fadeDelayTimer = startTimer(m_fadeInDelayMs);
    }
    else
    {
        m_opacityEffect->setOpacity(1);
    }

    return true;
}

bool Frame::fadeOut()
{
    RETURN_VAL_IF_FALSE(m_fadeVisible, true);

    m_fadeVisible = false;

    if (m_fadeDelayTimer != 0)
    {
        killTimer(m_fadeDelayTimer);
        m_fadeDelayTimer = 0;
    }

    if (m_animationEnabled)
    {
        m_fadeDelayTimer = startTimer(m_fadeOutDelayMs);
    }
    else
    {
        m_opacityEffect->setOpacity(0);
    }

    return true;
}

void Frame::setEnableSwitch(bool enable)
{
    m_btnSwitchToGreeter->setVisible(enable);
}

bool Frame::enableSwitch()
{
    return m_btnSwitchToGreeter->isVisible();
}

bool Frame::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() != QEvent::MouseButtonPress)
    {
        return false;
    }

    auto mouseEvent = dynamic_cast<QMouseEvent*>(event);

    QPoint mousePressGlobal = mouseEvent->globalPos();
    QRect m_powerMenuGeometry = m_powerMenu->geometry();

    if ((!m_powerMenuGeometry.contains(mousePressGlobal)) && m_powerMenu->isVisible())
    {
        m_powerMenu->hide();
        KLOG_DEBUG() << "power menu filter : " << obj->objectName() << event->type() << mouseEvent->buttons();
        return true;
    }

    return false;
}

void Frame::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == m_fadeDelayTimer)
    {
        m_animation->setDirection(m_fadeVisible ? QPropertyAnimation::Forward : QPropertyAnimation::Backward);
        m_animation->setDuration(m_fadeVisible ? m_fadeInDurationMs : m_fadeOutDurationMs);
        m_animation->start();
    }

    killTimer(m_fadeDelayTimer);
    m_fadeDelayTimer = 0;

    QObject::timerEvent(event);
}

}  // namespace Locker
}  // namespace SessionGuard
}  // namespace Kiran