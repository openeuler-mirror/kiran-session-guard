#include "greeterloginwindow.h"
#include "ui_greeterloginwindow.h"
#include <QPainter>
#include <QGraphicsBlurEffect>
#include <QGraphicsDropShadowEffect>
#include <QProcess>
#include <QDebug>
#include <QDesktopWidget>
#include <QDesktopWidget>
#include <QVector>
#include <QMouseEvent>
#include <QMenu>
#include "greeterkeyboard.h"
#include "greetersetting.h"
#include "greetermenuitem.h"
#include <QScreen>
#include <QAction>
#include <QSessionManager>
#include <QLightDM/SessionsModel>
#include <QWidgetAction>
#include <QDateTime>
#include <QTimer>
#include <QButtonGroup>

#define DEFAULT_MENU_QSS ":/themes/QMenu.qss"

Q_DECLARE_METATYPE(UserInfo);
GreeterLoginWindow::GreeterLoginWindow(QWidget *parent) :
    QWidget(parent)
  , ui(new Ui::GreeterLoginWindow)
  , m_greeter(this)
  , m_powerMenu(nullptr)
  , m_sessionMenu(nullptr)
  , m_noListButotnVisiable(true)
  , m_showUserList(false)
  , m_havePAMError(false)
  , m_havePrompted(false)
  , m_loginMode(LOGIN_BY_USER_LIST)
  , m_buttonType(BUTTON_SWITCH_TO_MANUAL_LOGIN)
{
    qRegisterMetaType<UserInfo>("UserInfo");
    ui->setupUi(this);
    ///启动CapsLock监控
    std::string error;
    if(!m_snoop.start(capsLockStatusChanged,this,error)){
        qWarning() << "capslock snoop start failed: " << error.c_str();
    }
    initMenu();
    initUI();
    initLightdmGreeter();
    initSettings();
    getCurrentDateTime();
}

GreeterLoginWindow::~GreeterLoginWindow()
{
    m_snoop.stop();
    delete ui;
}

void GreeterLoginWindow::setEditPromptFocus()
{
    ui->promptEdit->setFocus();
}

void GreeterLoginWindow::initUI()
{
    ///FIXME:因弹出窗口不是作为新的窗口，而是作为一个控件，需要我们去做隐藏
    ///开始监听整个应用程序事件，在窗口点击事件中判断隐藏菜单
    qApp->installEventFilter(this);

    ///设置字体阴影
    QGraphicsDropShadowEffect* labelTextShadowEffect = new QGraphicsDropShadowEffect(this);
    labelTextShadowEffect->setColor(QColor(0,0,0,255*0.1));
    labelTextShadowEffect->setBlurRadius(0);
    labelTextShadowEffect->setOffset(2, 2);
    ui->label_userName->setGraphicsEffect(labelTextShadowEffect);

    QGraphicsDropShadowEffect* labelTipsShadowEffect = new QGraphicsDropShadowEffect(this);
    labelTipsShadowEffect->setColor(QColor(255,255,255,255*0.1));
    labelTipsShadowEffect->setBlurRadius(0);
    labelTipsShadowEffect->setOffset(0, 0);
    ui->label_tips->setGraphicsEffect(labelTipsShadowEffect);

    connect(m_sessionMenu,&QMenu::triggered,[this](){
        m_sessionMenu->hide();
    });

    ///会话选择按钮点击
    connect(ui->btn_session,&QToolButton::pressed,this,[=]{
        QPoint menuLeftTop;
        QPoint btnRightTopPos;
        QSize  menuSize;

        btnRightTopPos = ui->btn_session->mapTo(this,QPoint(ui->btn_session->width(),0));
        menuSize = m_sessionMenu->sizeHint();

        menuLeftTop.setX(btnRightTopPos.x()-menuSize.width());
        menuLeftTop.setY(btnRightTopPos.y()-4-menuSize.height());

        qInfo() << "btn_session clicked,popup menu " << menuLeftTop;
        m_sessionMenu->popup(menuLeftTop);
    });

    connect(m_powerMenu,&QMenu::triggered,[this](){
        m_powerMenu->hide();
    });
    ///电源按钮点击
    connect(ui->btn_power,&QToolButton::pressed,this,[this]{
        //重新设置选项
        m_powerMenu->clear();
        if( m_powerIface.canHibernate()){
            m_powerMenu->addAction(tr("hibernate"),this,[this]{
                this->m_powerIface.hibernate();
            });
        }
        if( m_powerIface.canRestart() ){
            m_powerMenu->addAction(tr("restart"),this,[this]{
                this->m_powerIface.restart();
            });
        }
        if( m_powerIface.canShutdown() ){
            m_powerMenu->addAction(tr("shutdown"),this,[this]{
                this->m_powerIface.shutdown();
            });
        }
        if( m_powerIface.canSuspend() ){
            m_powerMenu->addAction(tr("suspend"),this,[this]{
                this->m_powerIface.suspend();
            });
        }
        //计算菜单显示坐标
        QPoint btnRightTopPos = ui->btn_power->mapTo(this,QPoint(ui->btn_power->width(),0));
        QSize menuSize = m_powerMenu->sizeHint();

        QPoint menuLeftTop;
        menuLeftTop.setX(btnRightTopPos.x()-menuSize.width());
        menuLeftTop.setY(btnRightTopPos.y()-4-menuSize.height());

        qInfo() << "btn_power clicked,popup menu " << menuLeftTop;
        m_powerMenu->popup(menuLeftTop);
    });

    ///用户列表点击
    connect(ui->userlist,SIGNAL(userActivated(const UserInfo&)),this,SLOT(slotUserActivated(const UserInfo&)));
    ///连接输入框回车和按钮点击信号
    connect(ui->promptEdit,SIGNAL(textConfirmed(const QString&)),this,SLOT(slotTextConfirmed(const QString&)));
    ///切换模式按钮和返回按钮
    connect(ui->button,SIGNAL(pressed()),
            this,SLOT(slotButtonClicked()));
    connect(ui->btn_keyboard,&QToolButton::pressed,this,[this]{
        GreeterKeyboard& keyboard = GreeterKeyboard::instance();
        if( keyboard.isVisible() ){
            keyboard.hide();
        } else {
            keyboard.showAdjustSize(this);
        }
        this->window()->windowHandle()->setKeyboardGrabEnabled(true);
    });
    ///用户列表请求重置用户选择登录界面
    connect(ui->userlist,&UserListWidget::sigRequestResetUI,this,[this]{
        Q_ASSERT(m_loginMode==LOGIN_BY_USER_LIST);
        resetUIForUserListLogin();
    });
    startUpdateTimeTimer();
}

void GreeterLoginWindow::initMenu()
{
    ///菜单样式
    QFile menuStyleFile(DEFAULT_MENU_QSS);
    QString menuQss;
    if(menuStyleFile.open(QIODevice::ReadOnly)){
        menuQss = menuStyleFile.readAll();
    }else{
        qWarning("open %s failed",DEFAULT_MENU_QSS);
    }
    ///电源菜单初始化
    m_powerMenu = new QMenu(this);//透明化需要设置父控件
    m_powerMenu->setStyleSheet(menuQss);
    m_powerMenu->setAttribute(Qt::WA_TranslucentBackground);//透明必需
    ///FIXME:QMenu不能为窗口，只能为控件，不然透明效果依赖于窗口管理器混成特效与显卡
    ///控件的话QMenu显示出来的话，不能点击其他区域隐藏窗口，需要手动隐藏
    m_powerMenu->setWindowFlags(Qt::FramelessWindowHint|Qt::Widget);//透明必需
    m_powerMenu->setContentsMargins(0,0,0,0);
    m_powerMenu->setFixedWidth(92);
    m_powerMenu->hide();

    ///session菜单初始化
    m_sessionMenu = new QMenu(this);
    m_sessionMenu->setMinimumWidth(92);
    m_sessionMenu->setMaximumWidth(184);
    m_sessionMenu->setStyleSheet(menuQss);
    m_sessionMenu->setAttribute(Qt::WA_TranslucentBackground);
    ///FIXME:QMenu不能为窗口，只能为控件，不然透明效果依赖于窗口管理器混成特效与显卡
    ///控件的话QMenu显示出来的话，不能点击其他区域隐藏窗口，需要手动隐藏
    m_sessionMenu->setWindowFlags(Qt::FramelessWindowHint|Qt::Widget);
    m_sessionMenu->setContentsMargins(0,0,0,0);
    m_sessionMenu->hide();

    QButtonGroup* buttonGroup = new QButtonGroup(m_sessionMenu);
    buttonGroup->setExclusive(true);
    QLightDM::SessionsModel sessionModel;
    for(int i=0;i<sessionModel.rowCount(QModelIndex());i++){
        QVariant key,id;
        QWidgetAction* widgetAction = nullptr;
        GreeterMenuItem* itemWidget = nullptr;
        key = sessionModel.data(sessionModel.index(i,0),QLightDM::SessionsModel::KeyRole);
        id  = sessionModel.data(sessionModel.index(i,0),QLightDM::SessionsModel::IdRole);
        widgetAction = new QWidgetAction(m_sessionMenu);
        itemWidget = new GreeterMenuItem(key.toString(),true);
        itemWidget->setFixedHeight(28);
        itemWidget->setMinimumWidth(90);
        itemWidget->setMaximumWidth(120);
        itemWidget->setExclusiveGroup(buttonGroup);
        connect(itemWidget,static_cast<void(GreeterMenuItem::*)(QString)>(&GreeterMenuItem::sigChecked),this,[this](QString action){
            qInfo() << "select session:" << action;
            m_session = action;
            m_sessionMenu->hide();
        });
        itemWidget->setObjectName("GreeterMenuItem");
        if( m_session.isEmpty() ){//设置默认值
            itemWidget->setChecked(true);
        }
        widgetAction->setDefaultWidget(itemWidget);
        m_sessionMenu->addAction(widgetAction);
    }

}


void GreeterLoginWindow::initLightdmGreeter()
{
#ifdef TEST
    ui->userlist->JustForTest(10);
#endif
    //连接到Lightdm
    if( !m_greeter.connectSync() ){
        qWarning("connect to lightdm greeter failed.");
        return;
    }

    //连接Lightdm信号槽
    connect(&m_greeter,SIGNAL(showMessage(QString,QLightDM::Greeter::MessageType)),
            this,SLOT(slotShowMessage(QString,QLightDM::Greeter::MessageType)));
    connect(&m_greeter,SIGNAL(showPrompt(QString,QLightDM::Greeter::PromptType)),
            this,SLOT(slotShowprompt(QString,QLightDM::Greeter::PromptType)));
    connect(&m_greeter,SIGNAL(authenticationComplete()),
            this,SLOT(slotAuthenticationComplete()));

    ui->userlist->loadUserList();
}

void GreeterLoginWindow::initSettings()
{
    m_noListButotnVisiable = GreeterSetting::instance()->getEnableManualLogin();
    if( !m_noListButotnVisiable ){
        //不允许输入用户名,必须显示用户列表
        m_showUserList = true;
    }else{
        m_showUserList = !GreeterSetting::instance()->getUserListHiding();
    }

    if(m_showUserList){
        resetUIForUserListLogin();
    }else{
        resetUIForManualLogin();
    }
}

void GreeterLoginWindow::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    if( !event->isAccepted() ){
        if(GreeterKeyboard::instance().getKeyboard()!=nullptr&&
           GreeterKeyboard::instance().getKeyboard()->isVisible()){
            GreeterKeyboard::instance().getKeyboard()->hide();
        }
    }
}

/**
 * @brief 事件监听，该事件过滤器是对QApplication使用，获取应用程序中所有的鼠标点击事件
 *        当点击的不是菜单区域直接收起菜单
 * @param obj   事件对象
 * @param  event 事件
 * @return 是否过滤
 */
bool GreeterLoginWindow::eventFilter(QObject *obj, QEvent *event)
{
    bool needFilter =  false;
    QMouseEvent* mouseEvent = nullptr;

    if(event->type()!=QEvent::MouseButtonPress){
        return false;
    }

    mouseEvent = dynamic_cast<QMouseEvent*>(event);

    QPoint mousePressGlobal = mouseEvent->globalPos();
    QRect m_sessionMenuGemometry = m_sessionMenu->geometry();
    QRect m_powerMenuGeometry = m_powerMenu->geometry();

    if( (!m_sessionMenuGemometry.contains(mousePressGlobal)) && m_sessionMenu->isVisible() ){
        m_sessionMenu->hide();
        needFilter = true;
        qInfo() << " session menu filter : " << obj->objectName() << event->type() << mouseEvent->buttons();
    }
    if( (!m_powerMenuGeometry.contains(mousePressGlobal)) && m_powerMenu->isVisible() ){
        m_powerMenu->hide();
        needFilter = true;
        qInfo() << "power menu filter : " << obj->objectName() << event->type() << mouseEvent->buttons();
    }
    if(needFilter){
        qInfo() << "session: " << m_sessionMenuGemometry;
        qInfo() << "menu:    " << m_powerMenuGeometry;
        qInfo() << "pos:     " << mousePressGlobal;
    }
    return needFilter;
}

void GreeterLoginWindow::setTips(QLightDM::Greeter::MessageType type, const QString &text)
{
    QString colorText = QString("<font color=%1>%2</font>")
                        .arg("white")
                        .arg(text);
    ui->label_tips->setText(colorText);
}

void GreeterLoginWindow::startAuthUser(const QString &username,QString userIcon)
{
    qInfo() << "start auth:";
    qInfo() << "    name[" << username << "]";
    qInfo() << "    icon["<< userIcon << "]";

    m_havePAMError = false;
    m_havePrompted = false;
    if( m_greeter.inAuthentication() ){
        m_greeter.cancelAuthentication();
    }
    ui->label_userName->setText(username);
    ui->loginAvatar->setImage(userIcon);
    ui->promptEdit->reset();
    m_greeter.authenticate(username);
}

void GreeterLoginWindow::resetUIForUserListLogin()
{
    qInfo() << "set ui for user list login";
    if( m_greeter.inAuthentication() ){
        m_greeter.cancelAuthentication();
    }
    //NotList按钮
    m_buttonType = BUTTON_SWITCH_TO_MANUAL_LOGIN;
    ui->button->setText(tr("Not Listed?"));
    ui->button->setVisible(m_noListButotnVisiable);
    ui->button->setEnabled(true);

    //头像设置成默认
    ui->loginAvatar->setDefaultImage();

    //用户名清空
    ui->label_userName->clear();

    //输入框复位
    ui->promptEdit->reset();
    setEditPromptFocus();

    //tips清空
    ui->label_tips->clear();

    //显示用户列表
    ui->userlist->setVisible(true);
    ui->userlist->setEnabled(true);

    m_loginMode = LOGIN_BY_USER_LIST;

    UserInfo userinfo;
    if( ui->userlist->getCurrentSelected(userinfo) ){
        slotUserActivated(userinfo);
    }else{
        ui->userlist->setRow0();
    }
}

void GreeterLoginWindow::resetUIForManualLogin()
{
    qInfo() << "set ui for manual login";
    if( m_greeter.inAuthentication() ){
        m_greeter.cancelAuthentication();
    }
    //返回使用用户列表登录模式
    m_buttonType = BUTTON_RETURN;
    ui->button->setText(tr("Return"));
    ui->button->setVisible(m_showUserList);
    ui->button->setEnabled(true);

    //头像设置成默认
    ui->loginAvatar->setDefaultImage();

    //用户名清空
    ui->label_userName->clear();

    //输入框复位
    ui->promptEdit->reset();
    ui->promptEdit->setPlaceHolderText(tr("Entry your name"));
    ui->promptEdit->setInputMode(GreeterLineEdit::INPUT_USERNAME);
    setEditPromptFocus();

    //tips清空
    ui->label_tips->clear();

    //用户列表隐藏
    ui->userlist->setEnabled(false);
    ui->userlist->setVisible(false);

    m_loginMode = LOGIN_BY_INPUT_USER;
}

void GreeterLoginWindow::startUpdateTimeTimer()
{
    QMetaObject::invokeMethod(this,"updateTimeLabel",Qt::AutoConnection);
    QTime curTime = QTime::currentTime();
    int nextUpdateSecond = 60-curTime.second();
    QTimer::singleShot(nextUpdateSecond*1000,this,SLOT(startUpdateTimeTimer()));
}

void GreeterLoginWindow::updateTimeLabel()
{
    ui->label_dataAndTime->setText(getCurrentDateTime());
}

QString GreeterLoginWindow::getCurrentDateTime()
{
    QDateTime dateTime = QDateTime::currentDateTime();
    QLocale locale;
    QString dateString;
    if( locale.language()==QLocale::Chinese ){
        ///5月21日 星期四 09:52
        static const char* dayOfWeekArray[] = {"星期日","星期一","星期二","星期三","星期四","星期五","星期六"};
        QString  dayOfWeekString = dayOfWeekArray[dateTime.date().dayOfWeek()];
        dateString = QString("%1 %2 %3").arg(dateTime.toString("MM月dd日"))
                                        .arg(dayOfWeekString)
                                        .arg(dateTime.toString("HH:mm"));
    }else{
        ///Thu May 21 09:52
        static const char* dayOfWeekArray[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
        static const char* monthOfYearArray[] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sept","Oct","Nov","Dec"};
        dateString = QString("%1 %2 %3").arg(dayOfWeekArray[dateTime.date().dayOfWeek()])
                                        .arg(monthOfYearArray[dateTime.date().month()-1])
                                        .arg(dateTime.toString("HH:mm"));
    }
    return dateString;
}

void GreeterLoginWindow::capsLockStatusChanged(bool on, void *user_data)
{
    qInfo() << "caps lock status changed: " << on;
    GreeterLoginWindow* This = static_cast<GreeterLoginWindow*>(user_data);
    QPixmap pixmap;
    if(on){
        pixmap.load(":/images/caps_lock.png");
        This->ui->label_capsLock->setPixmap(pixmap);
    }else{
        This->ui->label_capsLock->setPixmap(pixmap);
    }
}

void GreeterLoginWindow::slotShowMessage(QString text, QLightDM::Greeter::MessageType type)
{
    qInfo() << "lightdm show message: type[" << type << "] text[" << text << "]";
    if( type == QLightDM::Greeter::MessageType::MessageTypeError ){
        m_havePAMError = true;
    }
    setTips(type,text);
}

void GreeterLoginWindow::slotShowprompt(QString text, QLightDM::Greeter::PromptType type)
{
    qInfo() << "lightdm show prompt: type[" << type << "] text[" << text << "]";
    //用户手动登录，需要设置用户名
    if( m_loginMode==LOGIN_BY_INPUT_USER ){
        if( m_greeter.authenticationUser()!=ui->label_userName->text() ){
            ui->label_userName->setText(m_greeter.authenticationUser());
        }
        //显示返回按钮
        m_buttonType = BUTTON_RETURN;
        ui->button->setText(tr("Return"));
        ui->button->setVisible(true);
        ui->button->setEnabled(true);
    }
    m_havePrompted = true;
    ui->promptEdit->reset();
    ui->promptEdit->setPlaceHolderText(text);
    ui->promptEdit->setInputMode(GreeterLineEdit::INPUT_PROMPT);
    ui->promptEdit->setEchoMode(type==QLightDM::Greeter::PromptType::PromptTypeSecret?QLineEdit::Password:QLineEdit::Normal);
    setEditPromptFocus();
}

void GreeterLoginWindow::slotAuthenticationComplete()
{
    qInfo() << "lightdm authentication complete";
    if( m_greeter.isAuthenticated() ){
        if( !m_greeter.startSessionSync(m_session) ){
            qInfo("start session %s failed",m_session.toStdString().c_str());
            setTips(QLightDM::Greeter::MessageType::MessageTypeError,
                    tr("Start session failed"));
        }
    }else{
        if( m_havePrompted ){
            if( !m_havePAMError ){
                setTips(QLightDM::Greeter::MessageType::MessageTypeError,
                        tr("Incorrect password, please try again"));
            }
            startAuthUser(m_greeter.authenticationUser(),
                          ui->userlist->getIconByAccount(m_greeter.authenticationUser()));
        }else{
            if( !m_havePAMError ){
                setTips(QLightDM::Greeter::MessageType::MessageTypeError,
                        tr("Failed to authenticate"));
            }
        }
    }
}

void GreeterLoginWindow::slotTextConfirmed(const QString &text)
{
    qInfo() << "lineedit confirmed";
    if( ui->promptEdit->inputMode()==GreeterLineEdit::INPUT_PROMPT ){
        qInfo() << "respond: " << ui->promptEdit->getText();
        m_greeter.respond(ui->promptEdit->getText());
    }else{
        startAuthUser(ui->promptEdit->getText(),
                      ui->userlist->getIconByAccount(ui->promptEdit->getText()));
    }
}

void GreeterLoginWindow::slotUserActivated(const UserInfo &userInfo)
{
    startAuthUser(userInfo.name,userInfo.imagePath);
    ui->promptEdit->setFocus();
}

void GreeterLoginWindow::slotButtonClicked()
{
    qInfo() << "button clicked:";
    qInfo() << "    button type[" << m_buttonType << "]";
    qInfo() << "    login  mode[" << m_loginMode << "]";
    qInfo() << "    intput mode[" << ui->promptEdit->inputMode() << "]";

    if(m_buttonType == BUTTON_SWITCH_TO_MANUAL_LOGIN){
        Q_ASSERT(m_loginMode==LOGIN_BY_USER_LIST);
        resetUIForManualLogin();
    }else if(m_buttonType == BUTTON_RETURN){
        Q_ASSERT(m_loginMode==LOGIN_BY_INPUT_USER);
        //输入用户名返回则返回至用户列表选择
        if( ui->promptEdit->inputMode()==GreeterLineEdit::INPUT_USERNAME ){
            //用户列表不显示不应该执行到这
            Q_ASSERT(m_showUserList);
            resetUIForUserListLogin();
        }else if( ui->promptEdit->inputMode()==GreeterLineEdit::INPUT_PROMPT ){
            Q_ASSERT(m_noListButotnVisiable);
            resetUIForManualLogin();
        }
    }
}

void GreeterLoginWindow::resizeEvent(QResizeEvent *event)
{
    setEditPromptFocus();
    activateWindow();
    QWidget::resizeEvent(event);
}
