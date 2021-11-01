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

#include "screensaver-dialog.h"

#include <qt5-log-i.h>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>

#include <QDebug>
#include <QFile>
//#include <QGraphicsDropShadowEffect>
#include <QMenu>
#include <QMetaObject>
#include <QMouseEvent>
#include <QPainter>
#include <QWindow>
#include <QtDBus>

#include <kiran-screensaver/interface.h>

#include "auth-msg-queue.h"
#include "auth-pam.h"
#include "auth-proxy.h"
#include "dbus-api-wrapper/dbusapihelper.h"
#include "gsettings-helper.h"
#include "prefs.h"
#include "ui_screensaver-dialog.h"
#include "virtual-keyboard.h"

#define SYSTEM_DEFAULT_BACKGROUND "/usr/share/backgrounds/default.jpg"
#define DEFAULT_BACKGROUND ":/images/default_background.jpg"

QT_BEGIN_NAMESPACE
Q_WIDGETS_EXPORT void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed = 0);
QT_END_NAMESPACE

QString get_current_user()
{
    uid_t uid = getuid();
    KLOG_INFO() << "current uid:" << uid;

    long bufSize = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (bufSize == -1)
    {
        KLOG_WARNING() << "autodetect getpw_r bufsize failed.";
        return QString("");
    }

    std::vector<char> buf(bufSize);
    struct passwd pwd;
    struct passwd *pResult = nullptr;
    int iRet = 0;

    do
    {
        iRet = getpwuid_r(uid, &pwd, &buf[0], bufSize, &pResult);
        if (iRet == ERANGE)
        {
            bufSize *= 2;
            buf.resize(bufSize);
        }
    } while ((iRet == EINTR) || (iRet == ERANGE));

    if (iRet != 0)
    {
        KLOG_ERROR() << "getpwuid_r failed,error: [" << iRet << "]" << strerror(iRet);
        return QString("");
    }

    if (pResult == nullptr)
    {
        KLOG_ERROR() << "getpwuid_r no matching password record was found";
        return QString("");
    }

    return pResult->pw_name;
}

ScreenSaverDialog::ScreenSaverDialog(Kiran::ScreenSaver::Interface* ksInterface,QWidget *parent)
    : QWidget(parent),
      m_ksInterface(ksInterface),
      ui(new Ui::ScreenSaverDialog)
{
    ui->setupUi(this);
    init();
}

ScreenSaverDialog::~ScreenSaverDialog()
{
    delete ui;
}

QWidget *ScreenSaverDialog::get_widget_ptr()
{
    return this;
}

void ScreenSaverDialog::setAnimationEnabled(bool enabled)
{
    if (enabled == m_animationEnabled)
    {
        return;
    }

    if (!enabled)
    {
        if (m_animation.state() == QPropertyAnimation::Running)
        {
            QVariant var = m_animation.endValue();
            m_animation.stop();
            qreal opacity = var.toReal();
            m_opacityEffect->setOpacity(opacity);
        }
    }

    m_animationEnabled = enabled;
}

void ScreenSaverDialog::setAnimationDelay(int fadeInDelay, int fadeOutDelay)
{
    m_fadeInDelayMs = fadeInDelay;
    m_fadeOutDelayMs = fadeOutDelay;
}

void ScreenSaverDialog::setAnimationDuration(int fadeInMs, int fadeOutMs)
{
    if (m_fadeInDurationMs != fadeInMs)
    {
        if (m_animation.state() == QPropertyAnimation::Running &&
            m_animation.direction() == QPropertyAnimation::Forward)
        {
            //QVariantAnimation会自动计算更新当前剩余时间
            m_animation.setDuration(fadeInMs);
        }
        m_fadeInDurationMs = fadeInMs;
    }

    if (m_fadeOutDurationMs != fadeOutMs)
    {
        if ((m_animation.state() == QPropertyAnimation::Running) &&
            (m_animation.direction() == QPropertyAnimation::Backward))
        {
            m_animation.setDuration(fadeOutMs);
        }
        m_fadeOutDurationMs = fadeOutMs;
    }
}

bool ScreenSaverDialog::fadeVisible()
{
    return m_fadeVisible;
}

bool ScreenSaverDialog::fadeIn()
{
    if(m_fadeVisible)
    {
        return true;
    }

    m_fadeVisible = true;

    if(m_fadeDelayTimer!=0)
    {
        killTimer(m_fadeDelayTimer);
        m_fadeDelayTimer = 0;
    }


    if(m_animationEnabled)
    {
        m_fadeDelayTimer = startTimer(m_fadeInDelayMs);
    }
    else
    {
        m_opacityEffect->setOpacity(1);
    }

    return true;
}

bool ScreenSaverDialog::fadeOut()
{
    if(!m_fadeVisible)
    {
        return true;
    }

    m_fadeVisible = false;

    if(m_fadeDelayTimer!=0)
    {
        killTimer(m_fadeDelayTimer);
        m_fadeDelayTimer = 0;
    }

    if(m_animationEnabled)
    {
        m_fadeDelayTimer = startTimer(m_fadeOutDelayMs);
    }
    else
    {
        m_opacityEffect->setOpacity(0);
    }

    return true;
}
#define DEFAULT_STYLE_PATH ":/styles/kiran-screensaver-dialog-normal.qss"
void ScreenSaverDialog::init()
{
    initAuth();
    initUI();
    initAnimation();

    QString stylesheet;

    QFile file(DEFAULT_STYLE_PATH);
    if (file.open(QIODevice::ReadOnly))
    {
        stylesheet = file.readAll();
    }
    else
    {
        KLOG_WARNING() << "load style sheet failed";
    }
    this->setStyleSheet(stylesheet);

    startUpdateTimeTimer();
    startAuth();
}

void ScreenSaverDialog::initAuth()
{
    AuthBase *authPam = new AuthPam(this);
    AuthMsgQueueBase *msgQueue = new AuthMsgQueue(this);
    m_authProxy = new AuthProxy(authPam, this);

    if (!m_authProxy->init())
    {
        KLOG_ERROR() << "auth proxy can't init";
    }
    m_authProxy->setSessionAuthType(SESSION_AUTH_TYPE_TOGETHER_WITH_USER);
    m_authProxy->setMsgQueue(msgQueue);

    if (!connect(m_authProxy, &AuthProxy::showMessage,
                 this, &ScreenSaverDialog::slotShowMessage) ||
        !connect(m_authProxy, &AuthProxy::showPrompt,
                 this, &ScreenSaverDialog::slotShowPrompt) ||
        !connect(m_authProxy, &AuthProxy::authenticationComplete,
                 this, &ScreenSaverDialog::slotAuthenticationComplete))
    {
        KLOG_ERROR("connect to auth proxy signal failed!");
    }
}

void ScreenSaverDialog::initUI()
{
    // 输入框回车或点击解锁按钮时，回复给认证接口
    connect(ui->promptEdit, &PromptEdit::textConfirmed, this, [=] {
        m_authProxy->respond(ui->promptEdit->getText());
    });

#ifdef VIRTUAL_KEYBOARD
    connect(ui->btn_keyboard, &QToolButton::pressed, this, [this] {
        VirtualKeyboard *keyboard = VirtualKeyboard::instance();
        if (keyboard->isVisible())
        {
            keyboard->hide();
        }
        else
        {
            //虚拟键盘通过传入的父窗口调整大小并进行显示
            keyboard->showAdjustSize(this);
        }
        this->window()->windowHandle()->setKeyboardGrabEnabled(true);
    });
#else
    ui->btn_keyboard->setVisible(false);
#endif

    // 切换用户按钮
    connect(ui->btn_switchuser, &QToolButton::pressed, this, [=] {
        QTimer::singleShot(2000, this, SLOT(responseCancelAndQuit()));
        if (!DBusApi::DisplayManager::switchToGreeter())
        {
            KLOG_WARNING() << "call SwitchToGreeter failed.";
        }
    });

    // 设置阴影
//    QGraphicsDropShadowEffect *labelTextShadowEffect2 = new QGraphicsDropShadowEffect(ui->btn_cancel);
//    labelTextShadowEffect2->setColor(QColor(0, 0, 0, 255 * 0.1));
//    labelTextShadowEffect2->setBlurRadius(0);
//    labelTextShadowEffect2->setOffset(2, 2);
//    ui->btn_cancel->setGraphicsEffect(labelTextShadowEffect2);

    // 用户
    m_userName = get_current_user();
    ui->label_userName->setText(m_userName);

    // 电源菜单
    m_powerMenu = new QMenu(this);
    m_powerMenu->setAttribute(Qt::WA_TranslucentBackground);  // 透明必需
    //NOTE:QMenu不能为窗口，只能为子控件，不然透明效果依赖于窗口管理器混成特效与显卡
    //控件的话QMenu显示出来的话，不能点击其他区域隐藏窗口，需要手动隐藏
    m_powerMenu->setWindowFlags(Qt::FramelessWindowHint | Qt::Widget);  // 透明必需
    m_powerMenu->setFixedWidth(92);
    m_powerMenu->hide();

    if (Prefs::instance()->canReboot())
    {
        m_powerMenu->addAction(tr("reboot"), this, [=] {
            if (!DBusApi::SessionManager::reboot())
            {
                KLOG_WARNING() << "call reboot failed";
            }
        });
    }

    if (Prefs::instance()->canPowerOff())
    {
        m_powerMenu->addAction(tr("shutdown"), this, [=] {
            if (!DBusApi::SessionManager::shutdown())
            {
                KLOG_WARNING() << "call shutdown failed";
            }
        });
    }

    if (Prefs::instance()->canSuspend())
    {
        m_powerMenu->addAction(tr("suspend"), this, [=] {
            if (!DBusApi::SessionManager::suspend())
            {
                KLOG_WARNING() << "call suspend failed";
            }
        });
    }

    connect(m_powerMenu, &QMenu::triggered, this, [=] {
        m_powerMenu->hide();
    });

    // 电源按钮
    connect(ui->btn_power, &QToolButton::pressed, this, [=] {
        if (m_powerMenu->isVisible())
        {
            m_powerMenu->hide();
            return;
        }
        QPoint btnRightTopPos = ui->btn_power->mapTo(this, QPoint(ui->btn_power->width(), 0));
        QSize menuSize = m_powerMenu->sizeHint();
        if (m_powerMenu->actions().count() == 0)
        {
            menuSize = QSize(92, 10);
        }

        QPoint menuLeftTop;
        menuLeftTop.setX(btnRightTopPos.x() - menuSize.width());
        menuLeftTop.setY(btnRightTopPos.y() - 4 - menuSize.height());

        m_powerMenu->popup(menuLeftTop);
    });
    if (m_powerMenu->isEmpty())
    {
        ui->btn_power->setVisible(false);
    }

    // 重新认证按钮
    connect(ui->btn_reAuth, &QPushButton::clicked, this, [=] {
        startAuth();
    });

    //　默认禁用输入框，等待prompt消息再启用
    ui->promptEdit->setEnabled(false);
    ui->btn_switchuser->setVisible(false);
    ui->loginAvatar->setImage(DBusApi::AccountService::getUserIconFilePath(m_userName));

    // 安装事件过滤器，来实现菜单的自动隐藏
    qApp->installEventFilter(this);
}

void ScreenSaverDialog::initAnimation()
{
    m_opacityEffect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(m_opacityEffect);
    m_opacityEffect->setOpacity(0);

    m_animation.setTargetObject(m_opacityEffect);
    m_animation.setPropertyName("opacity");
    m_animation.setStartValue(0);
    m_animation.setEndValue(1);
    m_animation.setDuration(m_fadeInDurationMs);
}

void ScreenSaverDialog::startUpdateTimeTimer()
{
    QMetaObject::invokeMethod(this, "updateTimeLabel", Qt::AutoConnection);
    QTime curTime = QTime::currentTime();
    int nextUpdateSecond = 60 - curTime.second();
    QTimer::singleShot(nextUpdateSecond * 1000, this, SLOT(startUpdateTimeTimer()));
}

void ScreenSaverDialog::updateTimeLabel()
{
    ui->label_dataAndTime->setText(getCurrentDateTime());
}

QString ScreenSaverDialog::getCurrentDateTime()
{
    QDateTime dateTime = QDateTime::currentDateTime();
    QLocale locale;
    QString dateString;

    return dateTime.toString(tr("ddd MMM dd HH:mm"));
}

void ScreenSaverDialog::updateCurrentAuthType(Kiran::AuthType type)
{
    ui->loginAvatar->setVisible(type == Kiran::AUTH_TYPE_PASSWD);
    ui->promptEdit->setVisible(type == Kiran::AUTH_TYPE_PASSWD);

    ui->fingerAvatar->setVisible(type == Kiran::AUTH_TYPE_FINGER);
    if (type == Kiran::AUTH_TYPE_FINGER)
    {
        slotShowMessage(tr("Start fingerprint authentication"), Kiran::MessageTypeInfo);
        ui->fingerAvatar->startAnimation();
    }
    else
    {
        ui->fingerAvatar->stopAnimation();
    }

    ui->faceAvatar->setVisible(type == Kiran::AUTH_TYPE_FACE);
    if (type == Kiran::AUTH_TYPE_FACE)
    {
        slotShowMessage(tr("Start face authentication"), Kiran::MessageTypeInfo);
        ui->faceAvatar->startAnimation();
    }
    else
    {
        ui->faceAvatar->stopAnimation();
    }

    m_authType = type;
}

bool ScreenSaverDialog::eventFilter(QObject *obj, QEvent *event)
{
    bool needFilter = false;
    QMouseEvent *mouseEvent = nullptr;

    if (event->type() != QEvent::MouseButtonPress)
    {
        return false;
    }
    mouseEvent = dynamic_cast<QMouseEvent *>(event);

    QPoint mousePressGlobal = mouseEvent->globalPos();
    QRect m_powerMenuGeometry = m_powerMenu->geometry();

    if ((!m_powerMenuGeometry.contains(mousePressGlobal)) && m_powerMenu->isVisible())
    {
        m_powerMenu->hide();
        needFilter = true;
        KLOG_INFO() << "power menu filter : " << obj->objectName() << event->type() << mouseEvent->buttons();
    }

    return needFilter;
}

void ScreenSaverDialog::slotAuthenticationComplete(bool authRes)
{
    KLOG_DEBUG() << "slot authentication complete!";

    if (!authRes)
    {
        if (!m_havePrompt)
        {
            /// 如果不存在过Prompt直接失败，显示重新认证按钮，避免一直认证失败，重新认证的死循环
            switchToReauthentication();
        }
        else
        {
            startAuth();
        }
    }
    else
    {
        ui->promptEdit->reset();
        ui->promptEdit->setHasError(false);
        m_ksInterface->authenticationPassed();
    }
}

void ScreenSaverDialog::slotShowPrompt(QString text, Kiran::PromptType promptType)
{
    m_havePrompt = true;
    ui->promptEdit->reset();
    ui->promptEdit->setPlaceHolderText(text);
    ui->promptEdit->setEchoMode(
        promptType == Kiran::PromptTypeQuestion ? QLineEdit::Normal : QLineEdit::Password);
    ui->promptEdit->setFocus();
}

void ScreenSaverDialog::slotShowMessage(QString text, Kiran::MessageType messageType)
{
    QString colorText = QString("<font color=%1>%2</font>")
                            //            .arg(messageType == AuthPam::MessageTypeInfo ? "white" : "red")
                            .arg("white")
                            .arg(text);
    ui->label_tips->setText(colorText);
}

void ScreenSaverDialog::switchToReauthentication()
{
    ui->promptEdit->setVisible(false);
    ui->label_capsLock->setVisible(false);
    ui->label_JustForSpace->setVisible(false);

    ui->btn_reAuth->setVisible(true);
}

void ScreenSaverDialog::switchToPromptEdit()
{
    ui->btn_reAuth->setVisible(false);

    ui->promptEdit->setVisible(true);
    ui->label_capsLock->setVisible(true);
    ui->label_JustForSpace->setVisible(true);
}

void ScreenSaverDialog::startAuth()
{
    updateCurrentAuthType(Kiran::AUTH_TYPE_PASSWD);

    m_havePrompt = false;
    if (m_authProxy->inAuthentication())
    {
        m_authProxy->cancelAuthentication();
    }

    m_authProxy->authenticate(m_userName);
    switchToPromptEdit();
}

void ScreenSaverDialog::closeEvent(QCloseEvent *event)
{
#ifdef VIRTUAL_KEYBOARD
    //在关闭时若虚拟键盘的副窗口设置为当前窗口的话，则更改父窗口,避免释放相关X资源导致onboard释放出错，导致onboard崩溃
    if (VirtualKeyboard::instance()->getKeyboard())
    {
        if (VirtualKeyboard::instance()->getKeyboard()->parentWidget() == this)
        {
            KLOG_DEBUG() << "keyboard reparent";
            VirtualKeyboard::instance()->getKeyboard()->setParent(nullptr);
        }
    }
#endif
    QWidget::closeEvent(event);
}

void ScreenSaverDialog::setEnableSwitch(bool enable)
{
    ui->btn_switchuser->setVisible(enable);
}

bool ScreenSaverDialog::enableSwitch()
{
    return ui->btn_switchuser->isVisible();
}

void ScreenSaverDialog::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == m_fadeDelayTimer)
    {
        m_animation.setDirection(m_fadeVisible?QPropertyAnimation::Forward:QPropertyAnimation::Backward);
        m_animation.setDuration(m_fadeVisible?m_fadeInDurationMs:m_fadeOutDurationMs);
        m_animation.start();
    }

    killTimer(m_fadeDelayTimer);
    m_fadeDelayTimer = 0;

    QObject::timerEvent(event);
}
