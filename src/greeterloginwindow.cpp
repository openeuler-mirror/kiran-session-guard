
#include <QPainter>
#include <QGraphicsBlurEffect>
#include <QDebug>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QMenu>
#include <QScreen>
#include <QSessionManager>
#include <QLightDM/SessionsModel>
#include <QWidgetAction>
#include <QDateTime>
#include <QTimer>
#include <QButtonGroup>

#include "greeterkeyboard.h"
#include "greetersetting.h"
#include "greetermenuitem.h"
#include "greeterloginwindow.h"
#include "ui_greeterloginwindow.h"

Q_DECLARE_METATYPE(UserInfo);
using namespace QLightDM;
GreeterLoginWindow::GreeterLoginWindow(QWidget *parent) :
    QWidget(parent)
  , ui(new Ui::GreeterLoginWindow)
  , m_greeter(this)
  , m_authQueue(this)
  , m_powerMenu(nullptr)
  , m_sessionMenu(nullptr)
  , m_noListButotnVisiable(true)
  , m_showUserList(false)
  , m_loginMode(LOGIN_MODE_USER_LIST)
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
#ifdef VIRTUAL_KEYBOARD
    GreeterKeyboard::instance()->keyboardProcessExit();
#endif
    delete ui;
}

void GreeterLoginWindow::setEditPromptFocus(int ms)
{
    if(!ms){
        ui->promptEdit->setFocus();
    }else{
        QTimer::singleShot(ms,ui->promptEdit,SLOT(setFocus()));
    }
}

void GreeterLoginWindow::initUI()
{
    ///FIXME:因弹出窗口不是作为新的窗口，而是作为一个控件，需要我们去做隐藏
    ///开始监听整个应用程序事件，在窗口点击事件中判断隐藏菜单
    qApp->installEventFilter(this);

    ///按钮ToopTip
    ui->btn_session->setToolTip(tr("session menu"));
    ui->btn_keyboard->setToolTip(tr("virtual keyboard"));
    ui->btn_power->setToolTip(tr("power menu"));

    connect(m_sessionMenu,&QMenu::triggered,[this](){
        m_sessionMenu->hide();
    });

    ///会话选择按钮点击
    connect(ui->btn_session,&QToolButton::pressed,[this]{
        QPoint menuLeftTop;
        QPoint btnRightTopPos;
        QSize  menuSize;

        if( m_sessionMenu->isVisible() ){
            m_sessionMenu->hide();
            return;
        }

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
    connect(ui->btn_power,&QToolButton::pressed,[this]{
        if( m_powerMenu->isVisible() ){
            m_powerMenu->hide();
            return;
        }
        //重新设置选项
        m_powerMenu->clear();
        if( m_powerIface.canHibernate()){
            m_powerMenu->addAction(tr("hibernate"),[this]{
                this->m_powerIface.hibernate();
            });
        }
        if( m_powerIface.canSuspend() ){
            m_powerMenu->addAction(tr("suspend"),[this]{
                this->m_powerIface.suspend();
            });
        }
        if( m_powerIface.canRestart() ){
            m_powerMenu->addAction(tr("restart"),[this]{
                this->m_powerIface.restart();
            });
        }
        if( m_powerIface.canShutdown() ){
            m_powerMenu->addAction(tr("shutdown"),[this]{
                this->m_powerIface.shutdown();
            });
        }
        //计算菜单显示坐标
        QPoint btnRightTopPos = ui->btn_power->mapTo(this,QPoint(ui->btn_power->width(),0));
        QSize menuSize = m_powerMenu->sizeHint();

        QPoint menuLeftTop;
        menuLeftTop.setX(btnRightTopPos.x()-menuSize.width());
        menuLeftTop.setY(btnRightTopPos.y()-4-menuSize.height());

        m_powerMenu->popup(menuLeftTop);
    });

    ///用户列表点击
    connect(ui->userlist,&UserListWidget::userActivated,
            this,&GreeterLoginWindow::slotUserActivated);
    ///自动登录按钮点击
    connect(ui->btn_autologin,&LoginButton::sigClicked,[this](){
        m_greeter.authenticateAutologin();
    });
    connect(&m_greeter,&QLightDM::Greeter::autologinTimerExpired,[this](){
        m_greeter.authenticateAutologin();
    });
    ///重新认证按钮点击
    connect(ui->btn_reAuth,&QPushButton::clicked,[this](){
        if( m_loginMode==LOGIN_MODE_USER_LIST ){
            resetUIForUserListLogin();
        }else{
            resetUIForManualLogin();
        }
    });
    ///连接输入框回车和按钮点击信号
    connect(ui->promptEdit,&GreeterLineEdit::textConfirmed,
            this,&GreeterLoginWindow::slotTextConfirmed);
    ///切换模式按钮和返回按钮
    connect(ui->btn_notListAndCancel,&QToolButton::pressed,
            this,&GreeterLoginWindow::slotButtonClicked);
#ifdef VIRTUAL_KEYBOARD
    connect(ui->btn_keyboard,&QToolButton::pressed,[this]{
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
    ///用户列表请求重置用户选择登录界面
    connect(ui->userlist,&UserListWidget::sigRequestResetUI,[this]{
        if( m_loginMode==LOGIN_MODE_USER_LIST ){
            resetUIForUserListLogin();
        }
    });
    startUpdateTimeTimer();
}

void GreeterLoginWindow::initMenu()
{
    ///电源菜单初始化
    m_powerMenu = new QMenu(this);//透明化需要设置父控件
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
        connect(itemWidget,&GreeterMenuItem::sigChecked,[this](QString action){
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
    //连接到Lightdm
    if( !m_greeter.connectSync() ){
        qWarning("connect to lightdm greeter failed.");
        return;
    }

    ///将lightdm的认证消息发往队列，从队列中获取事件，而不是直接从lightdm的认证接口
    connect(&m_greeter,&Greeter::showMessage,[this](QString text, QLightDM::Greeter::MessageType type){
        if( type==Greeter::MessageTypeError ){
            m_havePAMError = true;
        }
        AuthMsgQueue::PamMessage msg;
        msg.text = text;
        msg.type = AuthMsgQueue::PMT_MSG;
        msg.extra.msgType = type==Greeter::MessageTypeInfo?AuthMsgQueue::MESSAGE_TYPE_INFO:AuthMsgQueue::MESSAGE_TYPE_ERROR;
        m_authQueue.append(msg);
    });
    connect(&m_greeter,&Greeter::showPrompt,[this](QString text, QLightDM::Greeter::PromptType type){
        if( !m_havePrompted ){
            m_havePrompted = true;
        }
        AuthMsgQueue::PamMessage msg;
        msg.text = text;
        msg.type = AuthMsgQueue::PMT_PROMPT;
        msg.extra.promptType = type==Greeter::PromptTypeQuestion?AuthMsgQueue::PROMPT_TYPE_QUESTION:AuthMsgQueue::PROMPT_TYPE_SECRET;
        m_authQueue.append(msg);
    });
    connect(&m_greeter,&Greeter::authenticationComplete,[this](){
        AuthMsgQueue::PamMessage authCompleteMsg;
        authCompleteMsg.type = AuthMsgQueue::PMT_AUTHENTICATION_COMPLETE;
        authCompleteMsg.extra.completeResult.isSuccess = m_greeter.isAuthenticated();
        bool reAuth = false;

        if( !m_greeter.isAuthenticated() ){
            AuthMsgQueue::PamMessage errorMsg;
            errorMsg.type = AuthMsgQueue::PMT_MSG;
            errorMsg.extra.msgType = AuthMsgQueue::MESSAGE_TYPE_ERROR;
            ///如果存在过prompt消息但是没有error消息，伪造一个错误消息
            ///存在过prompt消息才会开始重新认证，避免没有prompt反复调用pam开始认证，陷入死循环
            if(m_havePrompted){
                if( !m_havePAMError ){
                    switch (m_loginMode) {
                        case LOGIN_MODE_USER_LIST:
                            errorMsg.text = tr("Incorrect password, please try again");
                            break;
                        case LOGIN_MODE_MANUAL:
                            errorMsg.text = tr("Incorrect username or password");
                            break;
                    }
                }
                reAuth = true;
            }else{
                ///没存在过prompt消息，也没有error消息，抽象点的提示给用户
                if( !m_havePAMError ){
                    errorMsg.text = tr("Failed to authenticate");
                }
            }
            if( !errorMsg.text.isEmpty() ){
                m_authQueue.append(errorMsg);
            }
        }
        authCompleteMsg.extra.completeResult.reAuthentication = reAuth;
        m_authQueue.append(authCompleteMsg);
    });
    m_authQueue.startDispatcher();

    ///连接到队列发出的事件，进行处理
    qInfo() << "connect showMessage:" << connect(&m_authQueue,&AuthMsgQueue::showMessage,this,&GreeterLoginWindow::slotShowMessage);
    qInfo() << "connect showPrompt:" << connect(&m_authQueue,&AuthMsgQueue::showPrompt,this,&GreeterLoginWindow::slotShowprompt);
    qInfo() << "connect Authticate Complete:" << connect(&m_authQueue,&AuthMsgQueue::authenticateComplete,this,&GreeterLoginWindow::slotAuthenticationComplete);

    ///处理用户个数从0到1个和1到0的情况
    bool bres;
    bres = connect(&m_userModel,&QLightDM::UsersModel::rowsInserted,[this](const QModelIndex &parent, int first, int last){
        ///用户0->1 且 配置允许显示用户链表 且 当前登录模式为输入用户登录 且 手动登录还未输入用户名并点击确定
        ///显示返回按钮
        qInfo() << "rowInserted:" << m_userModel.rowCount(QModelIndex());
        if((m_userModel.rowCount(QModelIndex())==1)&&m_showUserList
                &&m_loginMode==LOGIN_MODE_MANUAL&&!m_greeter.isAuthenticated()){
            qInfo() << "setReturn visible true";
            ui->btn_notListAndCancel->setVisible(true);
        }
    });
    if( !bres ){
        qWarning() << "connect rowsInserted failed!";
    }

    bres = connect(&m_userModel,&QLightDM::UsersModel::rowsRemoved,[this](const QModelIndex &parent, int first, int last){
        qInfo() << "rowRemoved:" << m_userModel.rowCount(QModelIndex());
        if((m_userModel.rowCount(QModelIndex())==0)){
            ///TODO:是否需要判断配置文件中能否手动登录
            resetUIForManualLogin();
        }
    });
    if( !bres ){
        qInfo() << "connect rowsRemoved failed!";
    }

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


    m_authQueue.setMsgInterval(GreeterSetting::instance()->messageDisplayInterval());

    if(m_showUserList && m_userModel.rowCount(QModelIndex())>0 ){
        resetUIForUserListLogin();
    }else{
        resetUIForManualLogin();
    }
}

void GreeterLoginWindow::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
#ifdef VIRTUAL_KEYBOARD
    if( !event->isAccepted() ){
        if(GreeterKeyboard::instance()->getKeyboard()!=nullptr&&
           GreeterKeyboard::instance()->getKeyboard()->isVisible()){
            GreeterKeyboard::instance()->getKeyboard()->hide();
        }
    }
#endif
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

void GreeterLoginWindow::setTips(AuthMsgQueue::MessageType type, const QString &text)
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

    if( m_greeter.inAuthentication() ){
        m_greeter.cancelAuthentication();
    }

    m_authQueue.clean();

    m_havePrompted = false;
    m_havePAMError = false;

    ui->label_userName->setText(username);
    ui->loginAvatar->setImage(userIcon);
    if(username==m_greeter.autologinUserHint()){
        switchToAutoLogin();
        return;
    }else{
        switchToPromptEdit();
    }
    ui->promptEdit->reset();

    ///NOTE:为了解决在某些环境启动过快，导致的lightdm的认证回复prompt过慢几秒，
    ///     登录界面输入框未切换到密码模式,用户直接输入明文密码
    ///     暂时解决方案单独禁用输入框，等待lightdm的prompt消息会启用输入框
    ui->promptEdit->setEnabled(false);

    ///FIXME:鼠标点击认证用户列表时，需要延时设置输入焦点到输入框，不然又会被置回UserItem
    setEditPromptFocus(200);

    m_greeter.authenticate(username);
}

void GreeterLoginWindow::resetUIForUserListLogin()
{
    qInfo() << "set ui for user list login";
    if( m_greeter.inAuthentication() ){
        m_greeter.cancelAuthentication();
    }

    m_authQueue.clean();

    //NotList按钮
    m_buttonType = BUTTON_SWITCH_TO_MANUAL_LOGIN;
    ui->btn_notListAndCancel->setText(tr("Not Listed?"));
    ui->btn_notListAndCancel->setVisible(m_noListButotnVisiable);
    ui->btn_notListAndCancel->setEnabled(true);

    //头像设置成默认
    ui->loginAvatar->setDefaultImage();

    //用户名清空
    ui->label_userName->clear();

    //输入框复位
    ui->promptEdit->reset();
    switchToPromptEdit();
    setEditPromptFocus();

    //tips清空
    ui->label_tips->clear();

    //显示用户列表
    ui->userlist->setVisible(true);
    ui->userlist->setEnabled(true);

    m_loginMode = LOGIN_MODE_USER_LIST;

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

    m_authQueue.clean();

    //返回使用用户列表登录模式
    m_buttonType = BUTTON_RETURN;
    ui->btn_notListAndCancel->setText(tr("Return"));
    ui->btn_notListAndCancel->setVisible(m_showUserList&&ui->userlist->userCount()>0);
    ui->btn_notListAndCancel->setEnabled(true);

    //头像设置成默认
    ui->loginAvatar->setDefaultImage();

    //用户名清空
    ui->label_userName->clear();

    //输入框复位
    ui->promptEdit->reset();
    ui->promptEdit->setPlaceHolderText(tr("Entry your name"));
    m_inputMode = EDIT_INPUT_USER_NAME;

    switchToPromptEdit();
    setEditPromptFocus();

    //tips清空
    ui->label_tips->clear();

    //用户列表隐藏
    ui->userlist->setEnabled(false);
    ui->userlist->setVisible(false);

    m_loginMode = LOGIN_MODE_MANUAL;
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
    //tr : MM月MM日 dddd HH:mm
    QString res = dateTime.toString(tr("dddd MMM HH:mm"));
    return res;
}

void GreeterLoginWindow::capsLockStatusChanged(bool on, void *user_data)
{
    qInfo() << "caps lock status changed: " << on;
    GreeterLoginWindow* This = static_cast<GreeterLoginWindow*>(user_data);
    QPixmap pixmap;
    if(on){
        pixmap.load(":/images/caps_lock.png");
        pixmap = pixmap.scaledToWidth(This->ui->label_capsLock->width());
        pixmap = pixmap.scaledToHeight(This->ui->label_capsLock->height());
        This->ui->label_capsLock->setPixmap(pixmap);
    }else{
        This->ui->label_capsLock->setPixmap(pixmap);
    }
}

void GreeterLoginWindow::switchToPromptEdit()
{
    ui->promptEdit->setVisible(true);
    ui->btn_autologin->setVisible(false);
    ui->btn_reAuth->setVisible(false);
}

void GreeterLoginWindow::switchToAutoLogin()
{
    ui->promptEdit->setVisible(false);
    ui->btn_autologin->setVisible(true);
    ui->btn_reAuth->setVisible(false);
}

void GreeterLoginWindow::switchToReAuthentication()
{
    ui->promptEdit->setVisible(false);
    ui->btn_autologin->setVisible(false);
    ui->btn_reAuth->setVisible(true);
}

void GreeterLoginWindow::slotTextConfirmed(const QString &text)
{
    ///如果输入框当前是输入
    if( m_inputMode==EDIT_INPUT_PROMPT_RESPOND ){
        m_greeter.respond(ui->promptEdit->getText());
    }else{
        startAuthUser(ui->promptEdit->getText(),
                      ui->userlist->getIconByAccount(ui->promptEdit->getText()));
    }
}

void GreeterLoginWindow::slotUserActivated(const UserInfo &userInfo)
{
    ui->label_tips->clear();
    startAuthUser(userInfo.name,userInfo.imagePath);
}

void GreeterLoginWindow::slotButtonClicked()
{
    qInfo() << "button clicked:";
    qInfo() << "    button type[" << m_buttonType << "]";
    qInfo() << "    login  mode[" << m_loginMode << "]";
    qInfo() << "    intput mode[" << m_inputMode << "]";

    if(m_buttonType == BUTTON_SWITCH_TO_MANUAL_LOGIN){
        Q_ASSERT(m_loginMode==LOGIN_MODE_USER_LIST);
        resetUIForManualLogin();
    }else if(m_buttonType == BUTTON_RETURN){
        Q_ASSERT(m_loginMode==LOGIN_MODE_MANUAL);
        //输入用户名返回则返回至用户列表选择
        if( m_inputMode == EDIT_INPUT_USER_NAME ){
            //用户列表不显示不应该执行到这
            Q_ASSERT(m_showUserList);
            resetUIForUserListLogin();
        }else if( m_inputMode == EDIT_INPUT_PROMPT_RESPOND ){
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

void GreeterLoginWindow::slotShowMessage(QString text, AuthMsgQueue::MessageType type) {
    std::string stdText = text.toStdString();
    setTips(type,stdText.c_str());
}

void GreeterLoginWindow::slotShowprompt(QString text, AuthMsgQueue::PromptType type) {
    //用户手动登录，需要设置用户名
    if( m_loginMode==LOGIN_MODE_MANUAL ){
        if( m_greeter.authenticationUser()!=ui->label_userName->text() ){
            ui->label_userName->setText(m_greeter.authenticationUser());
        }
        //显示返回按钮
        m_buttonType = BUTTON_RETURN;
        ui->btn_notListAndCancel->setText(tr("Return"));
        ui->btn_notListAndCancel->setVisible(true);
        ui->btn_notListAndCancel->setEnabled(true);
    }
    ui->promptEdit->reset();
    ui->promptEdit->setPlaceHolderText(text);
    m_inputMode = EDIT_INPUT_PROMPT_RESPOND;
    ui->promptEdit->setEchoMode(type==AuthMsgQueue::PromptType::PROMPT_TYPE_SECRET?QLineEdit::Password:QLineEdit::Normal);
    ///FIXME:需要延时设置输入焦点到输入框，不然又会被置回UserItem
    setEditPromptFocus(200);
}

void GreeterLoginWindow::slotAuthenticationComplete(bool success, bool reAuthentication) {
    if(success){
        if(!m_greeter.startSessionSync(m_session)){
            qInfo("start session %s failed",m_session.toStdString().c_str());
        }
    }else{
        if(reAuthentication){
            startAuthUser(m_greeter.authenticationUser(),
                          ui->userlist->getIconByAccount(m_greeter.authenticationUser()));
        }else{
            switchToReAuthentication();
        }
    }
}

