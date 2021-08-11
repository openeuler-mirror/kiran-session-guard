#include "screensaver-dialog.h"

#include <pwd.h>
#include <qt5-log-i.h>
#include <sys/types.h>
#include <unistd.h>
#include <QDebug>
#include <QFile>
#include <QGraphicsDropShadowEffect>
#include <QMenu>
#include <QMetaObject>
#include <QMouseEvent>
#include <QPainter>
#include <QWindow>
#include <QtDBus>
#include <iostream>
#include "auth-msg-queue.h"
#include "auth-pam.h"
#include "auth-proxy.h"
#include "dbus-api-wrapper/dbusapihelper.h"
#include "gsettings-helper.h"
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

ScreenSaverDialog::ScreenSaverDialog(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::ScreenSaverDialog)
{
    ui->setupUi(this);
    printWindowID();
    init();
}

ScreenSaverDialog::~ScreenSaverDialog()
{
    delete ui;
}

void ScreenSaverDialog::setSwitchUserEnabled(bool enable)
{
    ui->btn_switchuser->setVisible(enable);
}

void ScreenSaverDialog::init()
{
    initAuth();
    initUI();
    startUpdateTimeTimer();
    startAuth();
}

void ScreenSaverDialog::initAuth()
{
    AuthBase* authPam = new AuthPam(this);
    AuthMsgQueueBase* msgQueue = new AuthMsgQueue(this);
    m_authProxy = new AuthProxy(authPam,this);
    if( !m_authProxy->init() )
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
    connect(ui->btn_cancel, &QToolButton::pressed, this, [=] {
        responseCancelAndQuit();
    });

    ///输入框回车或点击解锁按钮时，回复给认证接口
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

    ///切换用户按钮
    connect(ui->btn_switchuser, &QToolButton::pressed, this, [=] {
        QTimer::singleShot(2000, this, SLOT(responseCancelAndQuit()));
        if (!DBusApi::DisplayManager::switchToGreeter())
        {
            KLOG_WARNING() << "call SwitchToGreeter failed.";
        }
    });

    ///设置阴影
    QGraphicsDropShadowEffect *labelTextShadowEffect2 = new QGraphicsDropShadowEffect(this);
    labelTextShadowEffect2->setColor(QColor(0, 0, 0, 255 * 0.1));
    labelTextShadowEffect2->setBlurRadius(0);
    labelTextShadowEffect2->setOffset(2, 2);
    ui->btn_cancel->setGraphicsEffect(labelTextShadowEffect2);

    ///背景图
    QString backgroundPath = GSettingsHelper::getBackgrountPath();
    KLOG_DEBUG() << "screensaver-dialog background: " << backgroundPath;

    QStringList backgrounds = {backgroundPath, SYSTEM_DEFAULT_BACKGROUND, DEFAULT_BACKGROUND};
    foreach (const QString &background, backgrounds)
    {
        if (!m_background.load(background))
        {
            KLOG_WARNING() << "load background:" << background << "failed!";
            continue;
        }
        KLOG_DEBUG() << "load background:" << background;
        break;
    }

    ///用户
    m_userName = get_current_user();
    ui->label_userName->setText(m_userName);

    ///电源菜单
    m_powerMenu = new QMenu(this);
    m_powerMenu->setAttribute(Qt::WA_TranslucentBackground);  ///透明必需
    ///NOTE:QMenu不能为窗口，只能为子控件，不然透明效果依赖于窗口管理器混成特效与显卡
    ///控件的话QMenu显示出来的话，不能点击其他区域隐藏窗口，需要手动隐藏
    m_powerMenu->setWindowFlags(Qt::FramelessWindowHint | Qt::Widget);  ///透明必需
    m_powerMenu->setFixedWidth(92);
    m_powerMenu->hide();
    m_powerMenu->addAction(tr("reboot"), this, [=] {
        if (!DBusApi::SessionManager::reboot())
        {
            KLOG_WARNING() << "call reboot failed";
        }
    });
    m_powerMenu->addAction(tr("shutdown"), this, [=] {
        if (!DBusApi::SessionManager::shutdown())
        {
            KLOG_WARNING() << "call shutdown failed";
        }
    });
    m_powerMenu->addAction(tr("suspend"), this, [=] {
        if (!DBusApi::SessionManager::suspend())
        {
            KLOG_WARNING() << "call suspend failed";
        }
    });
    connect(m_powerMenu, &QMenu::triggered, this, [=] {
        m_powerMenu->hide();
    });

    ///电源按钮
    connect(ui->btn_power, &QToolButton::pressed, this, [=] {
        if (m_powerMenu->isVisible())
        {
            m_powerMenu->hide();
            return;
        }
        QPoint btnRightTopPos = ui->btn_power->mapTo(this, QPoint(ui->btn_power->width(), 0));
        QSize menuSize = m_powerMenu->sizeHint();
        QPoint menuLeftTop;
        menuLeftTop.setX(btnRightTopPos.x() - menuSize.width());
        menuLeftTop.setY(btnRightTopPos.y() - 4 - menuSize.height());

        m_powerMenu->popup(menuLeftTop);
    });

    ///重新认证按钮
    connect(ui->btn_reAuth, &QPushButton::clicked, this, [=] {
        startAuth();
    });

    //NOTE:暂时解决方案单独禁用输入框，等待pam的prompt消息会启用输入框
    ui->promptEdit->setEnabled(false);
    ui->btn_switchuser->setVisible(false);
    ui->loginAvatar->setImage(DBusApi::AccountService::getUserIconFilePath(m_userName));

    ///安装事件过滤器，来实现菜单的自动隐藏
    qApp->installEventFilter(this);
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

void ScreenSaverDialog::printWindowID()
{
    std::cout << "WINDOW ID=" << winId() << std::endl;
}

void ScreenSaverDialog::responseOkAndQuit()
{
    static const char *response = "RESPONSE=OK";
    std::cout << response << std::endl;
    KLOG_INFO() << response;
    this->close();
}

void ScreenSaverDialog::responseCancelAndQuit()
{
    static const char *response = "RESPONSE=CANCEL";
    std::cout << response << std::endl;
    KLOG_INFO() << response;
    this->close();
}

void ScreenSaverDialog::responseNoticeAuthFailed()
{
    static const char *response = "NOTICE=AUTH FAILED";
    std::cout << response << std::endl;
    KLOG_DEBUG() << response;
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

void ScreenSaverDialog::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    if (!m_scaledBackground.isNull())
    {
        QSize pixbufSize = m_scaledBackground.size();
        QSize windowSize = size();
        QRect drawTargetRect((pixbufSize.width() - windowSize.width()) / -2,
                             (pixbufSize.height() - windowSize.height()) / -2,
                             pixbufSize.width(),
                             pixbufSize.height());
        painter.drawPixmap(drawTargetRect, m_scaledBackground, m_scaledBackground.rect());
    }
    QWidget::paintEvent(event);
}

void ScreenSaverDialog::resizeEvent(QResizeEvent *event)
{
    if (!m_background.isNull())
    {
        QRect rect(0, 0, event->size().width(), event->size().height());
        QPixmap dest(event->size());

        QSize minSize = rect.size();
        QSize pixbufSize = m_background.size();
        double factor;
        QSize newPixbufSize;
        factor = qMax(minSize.width() / (double)pixbufSize.width(),
                      minSize.height() / (double)pixbufSize.height());

        newPixbufSize.setWidth(floor(pixbufSize.width() * factor + 0.5));
        newPixbufSize.setHeight(floor(pixbufSize.height() * factor + 0.5));

        QPixmap scaledPixmap = m_background.scaled(newPixbufSize, Qt::KeepAspectRatio, Qt::FastTransformation);
        QImage tmp = scaledPixmap.toImage();
        qt_blurImage(tmp, 10, true);
        m_scaledBackground = QPixmap::fromImage(tmp);
    }
    QWidget::resizeEvent(event);
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
        responseOkAndQuit();
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
