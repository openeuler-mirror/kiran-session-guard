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
#include "face-preview-widget.h"
#include "prefs.h"
#include "user-manager.h"
#include "virtual-keyboard.h"

#include <kiran-screensaver/interface.h>
#include <qt5-log-i.h>
#include <QApplication>
#include <QBoxLayout>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDateTime>
#include <QFile>
#include <QGraphicsOpacityEffect>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPropertyAnimation>
#include <QTimer>
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
    // 锁屏环境下授权码申请走 --auto CLI 模式，通过 tips 反馈结果
    setAutoAuthCodeRequest(true);
    KLOG_DEBUG() << "locker create for" << m_userName;
    initMenus();
    initUI();
    initAnimation();
    initAuth();
}

Frame::~Frame()
{
    qApp->removeEventFilter(this);
    m_passiveAuthStartPending = false;
    if (m_passiveAuthActive)
    {
        for (int i = 0; i < 3; ++i)
        {
            if (notifyPassiveAuthReportFilter(false))
            {
                break;
            }
        }
        m_passiveAuthActive = false;
    }
    if (m_passiveAuthTimer)
    {
        m_passiveAuthTimer->stop();
        delete m_passiveAuthTimer;
        m_passiveAuthTimer = nullptr;
    }
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
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
        // 在Qt5.9+版本中，支持setTabletTracking，触控笔相关功能
        // FIXME:后续如果遇到需要在低版本qt中支持，需要解析XInput事件
        button->setTabletTracking(true);
#endif
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

    // 初始化虚拟键盘
    m_keyboard = new VirtualKeyboard(this);
    m_keyboard->init();
    if (m_keyboard->isSupported())
    {
        rbBtnLayout->addWidget(m_btnKeyboard, 1);
    }
    else
    {
        m_btnKeyboard->setVisible(false);
    }

    rbBtnLayout->addWidget(m_btnPower, 1);
    setRightBottomWidget(rbBtnWidget);

    if (Prefs::getInstance()->facePreviewEnabled())
    {
        auto* facePreview = new FacePreviewWidget(this);
        setLeftTopWidget(facePreview);
        KLOG_INFO() << "locker: face preview mounted (SHM; system bus)";
    }
    else
    {
        KLOG_INFO() << "locker: face preview disabled (set face-preview-enabled=true in screensaver-dialog.ini)";
    }

    m_btnSwitchToGreeter->setVisible(false);
    if (m_powerMenu->actions().size() == 0)
    {
        m_btnPower->setVisible(false);
    }

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

    setSpecifyUser(m_userName);
    setAuthUserInfo(m_userName);
    switchControlPage(LoginFrame::CONTROL_PAGE_REAUTH);
    setTips(MessageTypeInfo, QString());

    /* 注册全局事件过滤器，用于被动认证循环检测用户交互 */
    qApp->installEventFilter(this);

    /* 被动认证循环（异步 D-Bus 查询不阻塞UI）。 */
    QTimer::singleShot(0, this, [this](){ initPassiveAuthLoop(); });

    /* 人脸预览。 */
    QTimer::singleShot(0, this, [this]()
                       {
        auto* preview = findChild<FacePreviewWidget*>();
        if (preview != nullptr)
        {
            preview->hide();
            preview->show();
        } });
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
        /* 解锁成功即结束被动循环，勿等析构 */
        stopPassiveAuthLoop("AuthSuccess");
        m_ksInterface->authenticationPassed();
        return;
    }

    /* 被动认证循环：仅软人脸失败后 2 秒再重试。
     * priming 阶段可能尚无 timer，软人脸就位后在此补建，避免只试探一次就僵死。
     * 非软人脸不建循环，保持「至多 priming 一次」的通用行为。 */
    if (!m_passiveAuthActive || lastAuthType() != KAD_AUTH_TYPE_SOFT_FACE)
    {
        return;
    }
    if (m_passiveAuthTimer == nullptr)
    {
        initPassiveAuthLoop();
    }
    if (m_passiveAuthTimer != nullptr)
    {
        m_passiveAuthTimer->setInterval(2000);
        m_passiveAuthTimer->start();
        KLOG_INFO() << "passiveAuth: retry scheduled in 2000ms"
                    << "epochMs=" << QDateTime::currentMSecsSinceEpoch();
    }
}

bool Frame::shouldShowFullName()
{
    return Prefs::getInstance()->showFullName();
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

    // 屏保切换解锁框，淡入时输入框丢失焦点。将焦点置回输入框
    setEditFocus(m_fadeInDelayMs);
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
    /* 被动认证循环：仅明确按键/点击才停止（含 btn_reAuth）。
     * 忽略 MouseMove/Wheel，避免光标微动或虚拟机同步误停。 */
    if (m_passiveAuthActive)
    {
        switch (event->type())
        {
        case QEvent::MouseButtonPress:
        {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            const Qt::MouseButton button = mouseEvent->button();
            if (button == Qt::LeftButton || button == Qt::RightButton)
            {
                stopPassiveAuthLoop("MouseButtonPress");
            }
            break;
        }
        case QEvent::KeyPress:
            stopPassiveAuthLoop("KeyPress");
            break;
        default:
            break;
        }
    }

    /* 保留：菜单外部点击隐藏 */
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

        /* 淡入完成后启动被动认证计时器（若激活且未运行） */
        if (m_fadeVisible && m_passiveAuthActive && m_passiveAuthTimer != nullptr
            && !m_passiveAuthTimer->isActive())
        {
            m_passiveAuthTimer->start();
            KLOG_INFO() << "passiveAuth: timer started after fadeIn"
                        << "epochMs=" << QDateTime::currentMSecsSinceEpoch();
        }
    }

    killTimer(m_fadeDelayTimer);
    m_fadeDelayTimer = 0;

    QObject::timerEvent(event);
}

/* ---- 被动认证循环 ---- */

void Frame::initPassiveAuthLoop()
{
    /* D-Bus 配置进程内只查一次 */
    static int s_cachedPassiveLoopSec = -1;
    static bool s_dbusQueried = false;

    if (!s_dbusQueried)
    {
        /* 异步查询 D-Bus GetAssistDetectConfig，不阻塞锁屏 UI */
        QDBusInterface iface(QStringLiteral("com.ks.auth.dbus.service"),
                              QStringLiteral("/com/ks/auth/dbus/service"),
                              QStringLiteral("com.ks.auth.dbus.service"),
                              QDBusConnection::systemBus());
        QDBusReply<QString> reply = iface.call(QStringLiteral("GetAssistDetectConfig"));

        int passiveLoopSec = 2;
        bool ok = false;
        if (reply.isValid())
        {
            const QJsonDocument doc = QJsonDocument::fromJson(reply.value().toUtf8());
            if (doc.isObject())
            {
                const int v = doc.object().value(QStringLiteral("passive_loop_sec")).toInt(-1);
                if (v >= 0)
                {
                    passiveLoopSec = v;
                    ok = true;
                }
            }
        }

        s_cachedPassiveLoopSec = ok ? passiveLoopSec : 0;
        s_dbusQueried = true;

        if (!ok || passiveLoopSec <= 0)
        {
            KLOG_INFO() << "passiveAuth: loop NOT started"
                        << "passive_loop_sec=" << passiveLoopSec
                        << "dbus_ok=" << ok
                        << "epochMs=" << QDateTime::currentMSecsSinceEpoch();
            return;
        }
    }

    if (s_cachedPassiveLoopSec <= 0)
    {
        return;
    }

    /* 已正式开启：仅在软人脸下补建 timer */
    if (m_passiveAuthActive)
    {
        if (lastAuthType() == KAD_AUTH_TYPE_SOFT_FACE)
        {
            ensurePassiveAuthTimer();
        }
        return;
    }

    /* 前 5 秒：不置 active、不发 enable、不监听键鼠；到点再一并开启 */
    if (m_passiveAuthStartPending)
    {
        return;
    }

    m_passiveLoopSec = s_cachedPassiveLoopSec;
    m_passiveAuthStartPending = true;
    QTimer::singleShot(5000, this, [this]() {
        if (!m_passiveAuthStartPending)
        {
            KLOG_INFO() << "passiveAuth: delayed start skipped"
                        << "epochMs=" << QDateTime::currentMSecsSinceEpoch();
            return;
        }
        m_passiveAuthStartPending = false;
        beginPassiveAuthLoop();
    });
    KLOG_INFO() << "passiveAuth: delayed start scheduled in 5000ms"
                << "authType=" << static_cast<int>(lastAuthType())
                << "passive_loop_sec=" << m_passiveLoopSec
                << "epochMs=" << QDateTime::currentMSecsSinceEpoch();
}

void Frame::beginPassiveAuthLoop()
{
    m_passiveAuthActive = true;
    notifyPassiveAuthReportFilter(true);
    ensurePassiveAuthTimer();
    KLOG_INFO() << "passiveAuth: loop started"
                << "passive_loop_sec=" << m_passiveLoopSec
                << "authType=" << static_cast<int>(lastAuthType())
                << "epochMs=" << QDateTime::currentMSecsSinceEpoch();
    triggerReAuthButton();
}

void Frame::ensurePassiveAuthTimer()
{
    if (m_passiveAuthTimer != nullptr)
    {
        if (m_passiveAuthActive && !m_passiveAuthTimer->isActive())
        {
            m_passiveAuthTimer->start();
            KLOG_INFO() << "passiveAuth: timer resumed"
                        << "epochMs=" << QDateTime::currentMSecsSinceEpoch();
        }
        return;
    }

    m_passiveAuthTimer = new QTimer(this);
    /* 首次认证由 beginPassiveAuthLoop 立即触发；timer 按配置间隔续跑 */
    m_passiveAuthTimer->setInterval(m_passiveLoopSec * 1000);

    connect(m_passiveAuthTimer, &QTimer::timeout, this, [this]() {
        /* 非软人脸则跳过本次 */
        if (lastAuthType() != KAD_AUTH_TYPE_SOFT_FACE)
        {
            return;
        }

        /* timer 照跑；认证中只跳过触发，不依赖回调把循环救活 */
        if (isAuthInProgress())
        {
            KLOG_INFO() << "passiveAuth: timeout skipped (auth in progress)"
                        << "authType=" << static_cast<int>(lastAuthType())
                        << "epochMs=" << QDateTime::currentMSecsSinceEpoch();
            return;
        }

        /* 被动循环仍在跑时续发开启通知，避免漏发结束通知后过滤状态残留 */
        notifyPassiveAuthReportFilter(true);

        KLOG_INFO() << "passiveAuth: timeout firing"
                    << "authType=" << static_cast<int>(lastAuthType())
                    << "epochMs=" << QDateTime::currentMSecsSinceEpoch();

        triggerReAuthButton();

        /* 失败后重试 (2000ms) → 恢复为 normal 间隔 */
        const int curInterval = m_passiveAuthTimer->interval();
        if (curInterval != m_passiveLoopSec * 1000)
        {
            m_passiveAuthTimer->setInterval(m_passiveLoopSec * 1000);
        }
    });

    if (m_passiveAuthActive)
    {
        m_passiveAuthTimer->start();
    }
}

void Frame::stopPassiveAuthLoop(const char* reason)
{
    /* 解锁成功等：作废尚未到期的延迟启动 */
    m_passiveAuthStartPending = false;
    if (m_passiveAuthTimer != nullptr)
    {
        m_passiveAuthTimer->stop();
    }
    if (m_passiveAuthActive)
    {
        /* 结束通知失败时重试，降低过滤状态残留风险 */
        for (int i = 0; i < 3; ++i)
        {
            if (notifyPassiveAuthReportFilter(false))
            {
                break;
            }
        }
    }
    m_passiveAuthActive = false;
    KLOG_INFO() << "passiveAuth: loop stopped"
                << "reason=" << reason
                << "epochMs=" << QDateTime::currentMSecsSinceEpoch();
}

bool Frame::notifyPassiveAuthReportFilter(bool enable)
{
    QDBusInterface iface(QStringLiteral("com.ks.auth.dbus.service"),
                         QStringLiteral("/com/ks/auth/dbus/service"),
                         QStringLiteral("com.ks.auth.dbus.service"),
                         QDBusConnection::systemBus());
    QJsonObject obj;
    obj.insert(QStringLiteral("business_id"), QStringLiteral("KylinsecOS"));
    obj.insert(QStringLiteral("enable"), enable);
    const QString args =
        QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
    QDBusReply<QString> reply =
        iface.call(QStringLiteral("SetPassiveAuthReportFilter"), args);
    if (!reply.isValid())
    {
        KLOG_WARNING() << "passiveAuth: SetPassiveAuthReportFilter D-Bus failed"
                       << "enable=" << enable
                       << "error=" << reply.error().message();
        return false;
    }
    KLOG_INFO() << "passiveAuth: SetPassiveAuthReportFilter ok"
                << "enable=" << enable
                << "reply=" << reply.value();
    return true;
}

void Frame::onAuthTypeChanged(KADAuthType type)
{
    LoginFrame::onAuthTypeChanged(type);

    /*
     * 仅被动已正式开启且软人脸就位时补建 timer。
     * 延迟等待期（startPending）不提前开启，避免未满 5 秒就发 enable / 听键鼠。
     */
    if (type == KAD_AUTH_TYPE_SOFT_FACE
        && m_passiveAuthActive
        && m_passiveAuthTimer == nullptr)
    {
        ensurePassiveAuthTimer();
    }
}

}  // namespace Locker
}  // namespace SessionGuard
}  // namespace Kiran