#include "screensaverdialog.h"
#include "ui_screensaverdialog.h"
#include <iostream>
#include <QDebug>
#include <QPainter>
#include <QMenu>
#include <QFile>
#include <QMouseEvent>
#include <QWindow>
#include <QGraphicsDropShadowEffect>
#include <QtDBus>
#include <QMetaObject>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <error.h>

#include "gsettingshelper.h"
#include "greeterkeyboard.h"
#include "dbus-api-wrapper/dbusapihelper.h"
#include <kiran-pam-msg.h>

#define DEFAULT_BACKGROUND ":/images/default_background.jpg"

QT_BEGIN_NAMESPACE
Q_WIDGETS_EXPORT void qt_blurImage (QImage &blurImage, qreal radius, bool quality, int transposed = 0);

QT_END_NAMESPACE

ScreenSaverDialog::ScreenSaverDialog (QWidget *parent) :
        QWidget(parent),
        ui(new Ui::ScreenSaverDialog),
        m_authProxy(this)
{
    ui->setupUi(this);
    printWindowID();
    init();
}

ScreenSaverDialog::~ScreenSaverDialog ()
{
#ifdef VIRTUAL_KEYBOARD
    GreeterKeyboard::instance()->keyboardProcessExit();
#endif
    delete ui;
}

void ScreenSaverDialog::setSwitchUserEnabled (bool enable)
{
    ui->btn_switchuser->setVisible(enable);
}

void ScreenSaverDialog::init ()
{
    ///初始化PAM认证代理
    initPamAuthProxy();
    ///初始化UI
    initUI();
    ///开始更新时间Timer
    startUpdateTimeTimer();
    ///开始认证
    startAuth();
}

void ScreenSaverDialog::initPamAuthProxy ()
{
    connect(&m_authProxy, &PamAuthProxy::showMessage, this, &ScreenSaverDialog::slotShowMessage);
    connect(&m_authProxy, &PamAuthProxy::showPrompt, this, &ScreenSaverDialog::slotShowPrompt);
    connect(&m_authProxy, &PamAuthProxy::authenticationComplete, this, &ScreenSaverDialog::slotAuthenticationComplete);
}

void ScreenSaverDialog::initUI ()
{
    ///取消按钮
    connect(ui->btn_cancel, &QToolButton::pressed, this, [=] {
        responseCancelAndQuit();
    });

    ///输入框封装
    connect(ui->promptEdit, &GreeterLineEdit::textConfirmed, this, [=] {
        m_authProxy.response(ui->promptEdit->getText());
    });

#ifdef VIRTUAL_KEYBOARD
    connect(ui->btn_keyboard,&QToolButton::pressed,this,[this]{
        GreeterKeyboard* keyboard = GreeterKeyboard::instance();
        if( keyboard->isVisible() ){
            keyboard->hide();
        } else {
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
            qWarning() << "call SwitchToGreeter failed.";
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
    qDebug() << "screensaver-dialog background: " << backgroundPath;
    if (!m_background.load(backgroundPath))
    {
        qWarning() << "load background" << backgroundPath << "failed";
        m_background.load(DEFAULT_BACKGROUND);
    }

    ///用户
    m_userName = getUser();
    qDebug() << "screensaver-dialog login user: " << m_userName;
    ui->label_userName->setText(m_userName);

    ///电源菜单
    m_powerMenu = new QMenu(this);
    m_powerMenu->setAttribute(Qt::WA_TranslucentBackground);///透明必需
    ///FIXME:QMenu不能为窗口，只能为控件，不然透明效果依赖于窗口管理器混成特效与显卡
    ///控件的话QMenu显示出来的话，不能点击其他区域隐藏窗口，需要手动隐藏
    m_powerMenu->setWindowFlags(Qt::FramelessWindowHint | Qt::Widget);///透明必需
    m_powerMenu->setFixedWidth(92);
    m_powerMenu->hide();
    m_powerMenu->addAction(tr("reboot"), this, [=] {
        if (!DBusApi::SessionManager::reboot())
        {
            qWarning() << "call reboot failed";
        }
    });
    m_powerMenu->addAction(tr("shutdown"), this, [=] {
        if (!DBusApi::SessionManager::shutdown())
        {
            qWarning() << "call shutdown failed";
        }
    });
    m_powerMenu->addAction(tr("suspend"), this, [=] {
        if (!DBusApi::SessionManager::suspend())
        {
            qWarning() << "call suspend failed";
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

QString ScreenSaverDialog::getUser ()
{

    uid_t uid = getuid();
    qInfo() << "current uid:" << uid;

    long bufSize = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (bufSize == -1)
    {
        qWarning() << "autodetect getpw_r bufsize failed.";
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
        qWarning() << "getpwuid_r failed,error: [" << iRet << "]" << strerror(iRet);
        return QString("");
    }

    if (pResult == nullptr)
    {
        qWarning() << "getpwuid_r no matching password record was found";
        return QString("");
    }

    qInfo() << "getpwuid_r: " << pResult->pw_name;
    return pResult->pw_name;
}

void ScreenSaverDialog::startUpdateTimeTimer ()
{
    QMetaObject::invokeMethod(this, "updateTimeLabel", Qt::AutoConnection);
    QTime curTime = QTime::currentTime();
    int nextUpdateSecond = 60 - curTime.second();
    QTimer::singleShot(nextUpdateSecond * 1000, this, SLOT(startUpdateTimeTimer()));
}

void ScreenSaverDialog::updateTimeLabel ()
{
    ui->label_dataAndTime->setText(getCurrentDateTime());
}

QString ScreenSaverDialog::getCurrentDateTime ()
{
    QDateTime dateTime = QDateTime::currentDateTime();
    QLocale locale;
    QString dateString;
    //TODO:修改
    if (locale.language() == QLocale::Chinese)
    {
        ///5月21日 星期四 09:52
        static const char *dayOfWeekArray[] = {"星期一", "星期二", "星期三", "星期四", "星期五", "星期六", "星期日"};
        QString dayOfWeekString = dayOfWeekArray[dateTime.date().dayOfWeek() - 1];
        dateString = QString("%1 %2 %3").arg(dateTime.toString("MM月dd日"))
                .arg(dayOfWeekString)
                .arg(dateTime.toString("HH:mm"));
    }
    else
    {
        ///Thu May 21 09:52
        static const char *dayOfWeekArray[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
        static const char *monthOfYearArray[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sept", "Oct",
                                                 "Nov", "Dec"};
        dateString = QString("%1 %2 %3").arg(dayOfWeekArray[dateTime.date().dayOfWeek() - 1])
                .arg(monthOfYearArray[dateTime.date().month() - 1])
                .arg(dateTime.toString("HH:mm"));
    }
    return dateString;
}

void ScreenSaverDialog::updateCurrentAuthType (ScreenSaverDialog::AuthType type)
{
    ui->loginAvatar->setVisible(type == AUTH_TYPE_PASSWD);
    ui->promptEdit->setVisible(type == AUTH_TYPE_PASSWD);

    ui->fingerAvatar->setVisible(type == AUTH_TYPE_FINGER);
    if (type == AUTH_TYPE_FINGER)
    {
        slotShowMessage(tr("Start fingerprint authentication"), PamAuthProxy::MessageTypeInfo);
        ui->fingerAvatar->startAnimation();
    }
    else
    {
        ui->fingerAvatar->stopAnimation();
    }

    ui->faceAvatar->setVisible(type == AUTH_TYPE_FACE);
    if (type == AUTH_TYPE_FACE)
    {
        slotShowMessage(tr("Start face authentication"), PamAuthProxy::MessageTypeInfo);
        ui->faceAvatar->startAnimation();
    }
    else
    {
        ui->faceAvatar->stopAnimation();
    }

    m_authType = type;
}

void ScreenSaverDialog::printWindowID ()
{
    std::cout << "WINDOW ID=" << winId() << std::endl;
}

void ScreenSaverDialog::responseOkAndQuit ()
{
    static const char *response = "RESPONSE=OK";
    std::cout << response << std::endl;
    qInfo() << response;
    this->close();
}

void ScreenSaverDialog::responseCancelAndQuit ()
{
    static const char *response = "RESPONSE=CANCEL";
    this->hide();
    std::cout << response << std::endl;
    qInfo() << response;
    this->close();
}

void ScreenSaverDialog::responseNoticeAuthFailed ()
{
    static const char *response = "NOTICE=AUTH FAILED";
    std::cout << response << std::endl;
    qInfo() << response;
}

bool ScreenSaverDialog::eventFilter (QObject *obj, QEvent *event)
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
        qInfo() << "power menu filter : " << obj->objectName() << event->type() << mouseEvent->buttons();
    }

    return needFilter;
}

void ScreenSaverDialog::paintEvent (QPaintEvent *event)
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

void ScreenSaverDialog::resizeEvent (QResizeEvent *event)
{
    if (!m_background.isNull())
    {
        QRect rect(0, 0, event->size().width(), event->size().height());
        QPixmap dest(event->size());

        QSize minSize = rect.size();
        QSize pixbufSize = m_background.size();
        double factor;
        QSize newPixbufSize;
        factor = qMax(minSize.width() / (double) pixbufSize.width(),
                      minSize.height() / (double) pixbufSize.height());

        newPixbufSize.setWidth(floor(pixbufSize.width() * factor + 0.5));
        newPixbufSize.setHeight(floor(pixbufSize.height() * factor + 0.5));

        QPixmap scaledPixmap = m_background.scaled(newPixbufSize, Qt::KeepAspectRatio, Qt::FastTransformation);
        QImage tmp = scaledPixmap.toImage();
        qt_blurImage(tmp, 10, true);
        m_scaledBackground = QPixmap::fromImage(tmp);
    }
    QWidget::resizeEvent(event);
}

void ScreenSaverDialog::slotAuthenticationComplete ()
{
    bool isSuccess = m_authProxy.isAuthenticated();
    if (!isSuccess)
    {
        //responseCancelAndQuit();
        switchToReauthentication();
    }
    else
    {
        ui->promptEdit->reset();
        ui->promptEdit->setHasError(false);
        responseOkAndQuit();
    }
}

void ScreenSaverDialog::slotShowPrompt (QString text, PamAuthProxy::PromptType promptType)
{
    if (text == ASK_FPINT)
    {
        updateCurrentAuthType(AUTH_TYPE_FINGER);
        m_authProxy.response(REP_FPINT);
    }
    else if (text == ASK_FACE)
    {
        updateCurrentAuthType(AUTH_TYPE_FACE);
        m_authProxy.response(REP_FACE);
    }
    else
    {
        updateCurrentAuthType(AUTH_TYPE_PASSWD);
    }
    ui->promptEdit->reset();
    ui->promptEdit->setPlaceHolderText(text);
    ui->promptEdit->setEchoMode(
            promptType == PamAuthProxy::PromptTypeQuestion ? QLineEdit::Normal : QLineEdit::Password);
    ui->promptEdit->setFocus();
}

void ScreenSaverDialog::slotShowMessage (QString text, PamAuthProxy::MessageType messageType)
{
    QString colorText = QString("<font color=%1>%2</font>")
            .arg(messageType == PamAuthProxy::MessageTypeInfo ? "white" : "red")
            .arg(text);
    ui->label_tips->setText(colorText);
}

void ScreenSaverDialog::switchToReauthentication ()
{
    ui->promptEdit->setVisible(false);
    ui->label_capsLock->setVisible(false);
    ui->label_JustForSpace->setVisible(false);

    ui->btn_reAuth->setVisible(true);
}

void ScreenSaverDialog::switchToPromptEdit ()
{
    ui->btn_reAuth->setVisible(false);

    ui->promptEdit->setVisible(true);
    ui->label_capsLock->setVisible(true);
    ui->label_JustForSpace->setVisible(true);
}

void ScreenSaverDialog::startAuth ()
{
    updateCurrentAuthType(AUTH_TYPE_PASSWD);
    if (m_authProxy.isRunning())
    {
        m_authProxy.reAuthenticate();
    }
    else
    {
        m_authProxy.startAuthenticate(m_userName);
    }
    switchToPromptEdit();
}