#include "screensaverdialog.h"
#include "ui_screensaverdialog.h"
#include <iostream>
#include <QDebug>
#include <QPainter>
#include <unistd.h>
#include <QMenu>
#include <QFile>
#include <QMouseEvent>
#include <QWindow>
#include <QGraphicsDropShadowEffect>
#include <QtDBus>
#include <QMetaObject>

#include "gsettingshelper.h"
#include "greeterkeyboard.h"
#include "dbusapihelper.h"
#include "tool.h"

#define DEFAULT_BACKGROUND ":/images/default_background.jpg"

ScreenSaverDialog::ScreenSaverDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScreenSaverDialog),
    m_authProxy(this)
{
    ui->setupUi(this);
    printWindowID();
    InitUI();
}

ScreenSaverDialog::~ScreenSaverDialog()
{
    m_authProxy.cancelAuthenticate();
    delete ui;
}

void ScreenSaverDialog::setSwitchUserEnabled(bool enable)
{
    ui->btn_switchuser->setVisible(enable);
}

void ScreenSaverDialog::InitUI()
{

    connect(ui->btn_cancel,&QToolButton::pressed,this,[=]{
        responseCancelAndQuit();
    });

    connect(ui->promptEdit,&GreeterLineEdit::textConfirmed,this,[=]{
        m_authProxy.response(true,ui->promptEdit->getText());
    });

    connect(ui->btn_keyboard,&QToolButton::pressed,this,[this]{
        GreeterKeyboard& keyboard = GreeterKeyboard::instance();
        if( keyboard.isVisible() ){
            keyboard.hide();
        } else {
            keyboard.showAdjustSize(this);
        }
        this->window()->windowHandle()->setKeyboardGrabEnabled(true);
    });

    connect(ui->btn_switchuser,&QToolButton::pressed,this,[=]{
        if( !DBusApi::DisplayManager::switchToGreeter() ){
            qWarning() << "call SwitchToGreeter failed.";
        }
    });

    connect(&m_authProxy,SIGNAL(authenticateComplete(bool)),this,SLOT(slotAuthenticateComplete(bool)));

    connect(&m_authProxy,&PamAuthProxy::authenticateError,this,[=]{
        ui->promptEdit->setHasError(true);
    });

    ///设置阴影
    QGraphicsDropShadowEffect* labelTextShadowEffect = new QGraphicsDropShadowEffect(this);
    labelTextShadowEffect->setColor(QColor(0,0,0,255*0.1));
    labelTextShadowEffect->setBlurRadius(0);
    labelTextShadowEffect->setOffset(2, 2);

    QGraphicsDropShadowEffect* labelTextShadowEffect2 = new QGraphicsDropShadowEffect(this);
    labelTextShadowEffect2->setColor(QColor(0,0,0,255*0.1));
    labelTextShadowEffect2->setBlurRadius(0);
    labelTextShadowEffect2->setOffset(2, 2);

    ui->label_userName->setGraphicsEffect(labelTextShadowEffect);
    ui->btn_cancel->setGraphicsEffect(labelTextShadowEffect2);

    ///背景图
    QString backgroundPath = GSettingsHelper::getBackgrountPath();
    if( !m_background.load(backgroundPath) ){
        qWarning() << "load background" << backgroundPath << "failed";
        m_background.load(DEFAULT_BACKGROUND);
    }

    m_scaledBackground = m_background.scaled(this->size(),Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation);
    m_scaledBackground = QPixmap::fromImage(Tool::blurredImage(m_scaledBackground.toImage(),m_scaledBackground.rect(),8,false));

    ///用户
    m_userName = getUser();
    ui->label_userName->setText(m_userName);

    ///电源菜单
    QString menuStyle;
    QFile menuStyleFile(":/styles/menu_style.qss");
    if( menuStyleFile.open(QIODevice::ReadOnly) ){
        menuStyle = menuStyleFile.readAll();
        menuStyleFile.close();
    }else{
        qWarning("can't open menu style file");
    }

    m_powerMenu = new QMenu(this);
    m_powerMenu->setStyleSheet(menuStyle);
    m_powerMenu->setAttribute(Qt::WA_TranslucentBackground);///透明必需
    ///FIXME:QMenu不能为窗口，只能为控件，不然透明效果依赖于窗口管理器混成特效与显卡
    ///控件的话QMenu显示出来的话，不能点击其他区域隐藏窗口，需要手动隐藏
    m_powerMenu->setWindowFlags(Qt::FramelessWindowHint|Qt::Widget);///透明必需
    m_powerMenu->setFixedWidth(92);
    m_powerMenu->hide();
    m_powerMenu->addAction(tr("reboot"),this,[=]{
        if( !DBusApi::SessionManager::reboot() ){
            qWarning() << "call reboot failed";
        }
    });
    m_powerMenu->addAction(tr("shutdown"),this,[=]{
        if( !DBusApi::SessionManager::shutdown() ){
            qWarning() << "call shutdown failed";
        }
    });
    m_powerMenu->addAction(tr("suspend"),this,[=]{
        if( !DBusApi::SessionManager::suspend() ){
            qWarning() << "call suspend failed";
        }
    });
    connect(m_powerMenu,&QMenu::triggered,this,[=]{
        m_powerMenu->hide();
    });

    connect(ui->btn_power,&QToolButton::pressed,this,[=]{
        QPoint btnRightTopPos = ui->btn_power->mapTo(this,QPoint(ui->btn_power->width(),0));
        QSize menuSize = m_powerMenu->sizeHint();

        QPoint menuLeftTop;
        menuLeftTop.setX(btnRightTopPos.x()-menuSize.width());
        menuLeftTop.setY(btnRightTopPos.y()-4-menuSize.height());

        m_powerMenu->popup(menuLeftTop);
    });

    if( !m_authProxy.startAuthenticate(m_userName.toStdString().c_str()) ){
        qWarning() << "PamAuthProxy::startAuthenticate failed";
    }

    ui->btn_switchuser->setVisible(false);

    ui->loginAvatar->setImage(DBusApi::AccountService::getUserIconFilePath(m_userName));

    ///安装事件过滤器，来实现菜单的自动隐藏
    qApp->installEventFilter(this);

    startUpdateTimeTimer();
}

QString ScreenSaverDialog::getUser()
{
    char* userNmae = getlogin();
    return QString(userNmae);
}

void ScreenSaverDialog::startUpdateTimeTimer()
{
    QMetaObject::invokeMethod(this,"updateTimeLabel",Qt::AutoConnection);
    QTime curTime = QTime::currentTime();
    int nextUpdateSecond = 60-curTime.second();
    QTimer::singleShot(nextUpdateSecond*1000,this,SLOT(startUpdateTimeTimer()));
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
    if( locale.language()==QLocale::Chinese ){
        ///5月21日 星期四 09:52
        static const char* dayOfWeekArray[] = {"星期一","星期二","星期三","星期四","星期五","星期六","星期日"};
        QString  dayOfWeekString = dayOfWeekArray[dateTime.date().dayOfWeek()-1];
        dateString = QString("%1 %2 %3").arg(dateTime.toString("MM月dd日"))
                                        .arg(dayOfWeekString)
                                        .arg(dateTime.toString("HH:mm"));
    }else{
        ///Thu May 21 09:52
        static const char* dayOfWeekArray[] = {"Mon","Tue","Wed","Thu","Fri","Sat","Sun"};
        static const char* monthOfYearArray[] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sept","Oct","Nov","Dec"};
        dateString = QString("%1 %2 %3").arg(dayOfWeekArray[dateTime.date().dayOfWeek()-1])
                                        .arg(monthOfYearArray[dateTime.date().month()-1])
                                        .arg(dateTime.toString("HH:mm"));
    }
    return dateString;
}

void ScreenSaverDialog::requestResponse(const char *msg, bool visiable)
{
    QMetaObject::invokeMethod(this,"requestResponse",Qt::QueuedConnection,
                              Q_ARG(QString,msg),Q_ARG(bool,visiable));
}

void ScreenSaverDialog::requestResponse(const QString &msg, bool visiable)
{
    ui->promptEdit->reset();
    ui->promptEdit->setPlaceHolderText(msg);
    ui->promptEdit->setEchoMode(visiable?QLineEdit::Normal:QLineEdit::Password);
    ui->promptEdit->setFocus();
}

void ScreenSaverDialog::onDisplayError(const char *msg)
{
    //暂时不需要
}

void ScreenSaverDialog::onDisplayTextInfo(const char *msg)
{
    //暂时不需要
}

void ScreenSaverDialog::printWindowID()
{
    std::cout << "WINDOW ID=" << winId() << std::endl;
    qInfo() << "WINDOW ID=" << winId();
}

void ScreenSaverDialog::responseOkAndQuit()
{
    static const char* response = "RESPONSE=OK";
    std::cout << response << std::endl;
    qInfo() << response;
    this->close();
}

void ScreenSaverDialog::responseCancelAndQuit()
{
    static const char* response = "RESPONSE=CANCEL";
    std::cout << response << std::endl;
    qInfo() << response;
    this->close();
}

void ScreenSaverDialog::responseNoticeAuthFailed()
{
    static const char* response = "NOTICE=AUTH FAILED";
    std::cout << response << std::endl;
    qInfo() << response;
}

void ScreenSaverDialog::slotAuthenticateComplete(bool isSuccess)
{
    if(!isSuccess){
        responseCancelAndQuit();
    }else{
        ui->promptEdit->reset();
        ui->promptEdit->setHasError(false);
        responseOkAndQuit();
    }
}

bool ScreenSaverDialog::eventFilter(QObject *obj, QEvent *event)
{
    bool needFilter =  false;
    QMouseEvent* mouseEvent = nullptr;

    if(event->type()!=QEvent::MouseButtonPress){
        return false;
    }
    mouseEvent = dynamic_cast<QMouseEvent*>(event);

    QPoint mousePressGlobal = mouseEvent->globalPos();
    QRect  m_powerMenuGeometry = m_powerMenu->geometry();

    if( (!m_powerMenuGeometry.contains(mousePressGlobal)) && m_powerMenu->isVisible() ){
        m_powerMenu->hide();
        needFilter = true;
        qInfo() << "power menu filter : " << obj->objectName() << event->type() << mouseEvent->buttons();
    }

    return needFilter;
}

void ScreenSaverDialog::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    if(!m_scaledBackground.isNull()){
        painter.drawPixmap(this->rect(),m_scaledBackground,m_scaledBackground.rect());
    }
    QWidget::paintEvent(event);
}
