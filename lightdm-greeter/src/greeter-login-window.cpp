
#include <qt5-log-i.h>
#include <QButtonGroup>
#include <QDateTime>
#include <QDesktopWidget>
#include <QGraphicsBlurEffect>
#include <QLightDM/SessionsModel>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>
#include <QSessionManager>
#include <QTimer>
#include <QWidgetAction>

#include "auth-lightdm.h"
#include "auth-msg-queue.h"
#include "auth-proxy.h"
#include "greeter-login-window.h"
#include "greeter-menu-item.h"
#include "kiran-greeter-prefs.h"
#include "ui_greeterloginwindow.h"
#include "virtual-keyboard.h"

Q_DECLARE_METATYPE(UserInfo);
using namespace QLightDM;

GreeterLoginWindow::GreeterLoginWindow(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::GreeterLoginWindow),
      m_greeter(this),
      m_powerMenu(nullptr),
      m_sessionMenu(nullptr),
      m_noListButotnVisiable(true),
      m_showUserList(false),
      m_loginMode(LOGIN_MODE_USER_LIST),
      m_buttonType(BUTTON_SWITCH_TO_MANUAL_LOGIN)
{
    qRegisterMetaType<UserInfo>("UserInfo");
    ui->setupUi(this);

    ///启动CapsLock监控
    std::string error;
    if (!m_snoop.start(capsLockStatusChanged, this, error))
    {
        KLOG_WARNING() << "capslock snoop start failed: " << error.c_str();
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

void GreeterLoginWindow::setEditPromptFocus(int ms)
{
    if (!ms)
    {
        ui->promptEdit->setFocus();
    }
    else
    {
        QTimer::singleShot(ms, ui->promptEdit, SLOT(setFocus()));
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

    connect(m_sessionMenu, &QMenu::triggered, [this]() {
        m_sessionMenu->hide();
    });

    ///会话选择按钮点击
    connect(ui->btn_session, &QToolButton::pressed, [this] {
        QPoint menuLeftTop;
        QPoint btnRightTopPos;
        QSize menuSize;

        if (m_sessionMenu->isVisible())
        {
            m_sessionMenu->hide();
            return;
        }

        btnRightTopPos = ui->btn_session->mapTo(this, QPoint(ui->btn_session->width(), 0));
        if (m_sessionMenu->actions().count() == 0)
        {
            menuSize = QSize(92, 10);
        }
        else
        {
            menuSize = m_sessionMenu->sizeHint();
        }

        menuLeftTop.setX(btnRightTopPos.x() - menuSize.width());
        menuLeftTop.setY(btnRightTopPos.y() - 4 - menuSize.height());

        KLOG_DEBUG() << "btn_session clicked,popup menu " << menuLeftTop;
        m_sessionMenu->popup(menuLeftTop);
    });

    connect(m_powerMenu, &QMenu::triggered, [this]() {
        m_powerMenu->hide();
    });
    ///电源按钮点击
    connect(ui->btn_power, &QToolButton::pressed, [this] {
        if (m_powerMenu->isVisible())
        {
            m_powerMenu->hide();
            return;
        }
        //重新设置选项
        m_powerMenu->clear();
        if (m_powerIface.canHibernate() && KiranGreeterPrefs::instance()->canHibernate())
        {
            m_powerMenu->addAction(tr("hibernate"), [this] {
                this->m_powerIface.hibernate();
            });
        }
        if (m_powerIface.canSuspend() && KiranGreeterPrefs::instance()->canSuspend())
        {
            m_powerMenu->addAction(tr("suspend"), [this] {
                this->m_powerIface.suspend();
            });
        }
        if (m_powerIface.canRestart() && KiranGreeterPrefs::instance()->canReboot())
        {
            m_powerMenu->addAction(tr("restart"), [this] {
                this->m_powerIface.restart();
            });
        }
        if (m_powerIface.canShutdown() && KiranGreeterPrefs::instance()->canPowerOff())
        {
            m_powerMenu->addAction(tr("shutdown"), [this] {
                this->m_powerIface.shutdown();
            });
        }
        //计算菜单显示坐标
        QPoint btnRightTopPos = ui->btn_power->mapTo(this, QPoint(ui->btn_power->width(), 0));
        QSize menuSize;
        if (m_powerMenu->actions().count() == 0)
        {
            menuSize = QSize(92, 10);
        }
        else
        {
            menuSize = m_powerMenu->sizeHint();
        }
        QPoint menuLeftTop;
        menuLeftTop.setX(btnRightTopPos.x() - menuSize.width());
        menuLeftTop.setY(btnRightTopPos.y() - 4 - menuSize.height());

        m_powerMenu->popup(menuLeftTop);
    });

    if( !KiranGreeterPrefs::instance()->canHibernate() &&
        !KiranGreeterPrefs::instance()->canSuspend() &&
        !KiranGreeterPrefs::instance()->canPowerOff() &&
        !KiranGreeterPrefs::instance()->canReboot())
    {
        ui->btn_power->setVisible(false);
    }

    ///用户列表点击
    connect(ui->userlist, &UserListWidget::userActivated,
            this, &GreeterLoginWindow::slotUserActivated);
    ///自动登录按钮点击
    connect(ui->btn_autologin, &LoginButton::sigClicked, [this]() {
        m_greeter.authenticateAutologin();
    });
    connect(&m_greeter, &QLightDM::Greeter::autologinTimerExpired, [this]() {
        m_greeter.authenticateAutologin();
    });
    ///重新认证按钮点击
    connect(ui->btn_reAuth, &QPushButton::clicked, [this]() {
        if (m_loginMode == LOGIN_MODE_USER_LIST)
        {
            resetUIForUserListLogin();
        }
        else
        {
            resetUIForManualLogin();
        }
    });
    ///连接输入框回车和按钮点击信号
    connect(ui->promptEdit, &PromptEdit::textConfirmed,
            this, &GreeterLoginWindow::slotTextConfirmed);
    ///切换模式按钮和返回按钮
    connect(ui->btn_notListAndCancel, &QToolButton::pressed,
            this, &GreeterLoginWindow::slotButtonClicked);
#ifdef VIRTUAL_KEYBOARD
    connect(ui->btn_keyboard, &QToolButton::pressed, [this] {
        VirtualKeyboard *keyboard = VirtualKeyboard::instance();
        if (keyboard->isVisible())
        {
            keyboard->hide();
        }
        else
        {
            keyboard->showAdjustSize(this);
        }
        this->window()->windowHandle()->setKeyboardGrabEnabled(true);
    });
#else
    ui->btn_keyboard->setVisible(false);
#endif
    ///用户列表请求重置用户选择登录界面
    connect(ui->userlist, &UserListWidget::sigRequestResetUI, [this] {
        Q_ASSERT(m_loginMode == LOGIN_MODE_USER_LIST);
        resetUIForUserListLogin();
    });
    startUpdateTimeTimer();
}

void GreeterLoginWindow::initMenu()
{
    ///电源菜单初始化
    m_powerMenu = new QMenu(this);                            //透明化需要设置父控件
    m_powerMenu->setAttribute(Qt::WA_TranslucentBackground);  //透明必需
    ///FIXME:QMenu不能为窗口，只能为控件，不然透明效果依赖于窗口管理器混成特效与显卡
    ///控件的话QMenu显示出来的话，不能点击其他区域隐藏窗口，需要手动隐藏
    m_powerMenu->setWindowFlags(Qt::FramelessWindowHint | Qt::Widget);  //透明必需
    m_powerMenu->setContentsMargins(0, 0, 0, 0);
    m_powerMenu->setFixedWidth(92);
    m_powerMenu->hide();

    ///session菜单初始化
    m_sessionMenu = new QMenu(this);
    m_sessionMenu->setMinimumWidth(92);
    m_sessionMenu->setMaximumWidth(184);
    m_sessionMenu->setAttribute(Qt::WA_TranslucentBackground);
    ///FIXME:QMenu不能为窗口，只能为控件，不然透明效果依赖于窗口管理器混成特效与显卡
    ///控件的话QMenu显示出来的话，不能点击其他区域隐藏窗口，需要手动隐藏
    m_sessionMenu->setWindowFlags(Qt::FramelessWindowHint | Qt::Widget);
    m_sessionMenu->setContentsMargins(0, 0, 0, 0);
    m_sessionMenu->hide();

    QButtonGroup *buttonGroup = new QButtonGroup(m_sessionMenu);
    buttonGroup->setExclusive(true);
    QLightDM::SessionsModel sessionModel;
    QStringList hiddenSessions = KiranGreeterPrefs::instance()->hiddenSessions();
    for (int i = 0; i < sessionModel.rowCount(QModelIndex()); i++)
    {
        QVariant key, id;
        QWidgetAction *widgetAction = nullptr;
        GreeterMenuItem *itemWidget = nullptr;
        key = sessionModel.data(sessionModel.index(i, 0), QLightDM::SessionsModel::KeyRole);
        id = sessionModel.data(sessionModel.index(i, 0), QLightDM::SessionsModel::IdRole);
        if (hiddenSessions.contains(key.toString()))
        {
            continue;
        }
        widgetAction = new QWidgetAction(m_sessionMenu);
        itemWidget = new GreeterMenuItem(key.toString(), true);
        itemWidget->setFixedHeight(28);
        itemWidget->setMinimumWidth(90);
        itemWidget->setMaximumWidth(120);
        itemWidget->setExclusiveGroup(buttonGroup);
        connect(itemWidget, &GreeterMenuItem::sigChecked, [this](QString action) {
            KLOG_DEBUG() << "select session:" << action;
            m_session = action;
            m_sessionMenu->hide();
        });
        itemWidget->setObjectName("GreeterMenuItem");
        if (m_session.isEmpty())
        {  //设置默认值
            itemWidget->setChecked(true);
        }
        widgetAction->setDefaultWidget(itemWidget);
        m_sessionMenu->addAction(widgetAction);
    }
}

void GreeterLoginWindow::initLightdmGreeter()
{
    AuthBase *authInterface = new AuthLightdm(&m_greeter, this);
    AuthMsgQueue *msgQueue = new AuthMsgQueue(m_authProxy);
    m_authProxy = new AuthProxy(authInterface, this);
    m_authProxy->setMsgQueue(msgQueue);
    m_authProxy->setSessionAuthType(SESSION_AUTH_TYPE_TOGETHER);
    if (!m_authProxy->init())
    {
        KLOG_ERROR("can not init auth proxy!");
        return;
    }

    if (!connect(m_authProxy, &AuthProxy::showMessage, this, &GreeterLoginWindow::slotShowMessage) ||
        !connect(m_authProxy, &AuthProxy::showPrompt, this, &GreeterLoginWindow::slotShowprompt) ||
        !connect(m_authProxy, &AuthProxy::authenticationComplete, this, &GreeterLoginWindow::slotAuthenticationComplete))
    {
        KLOG_FATAL("can not connect to auth proxy signals");
    }

    ///处理用户个数从0到1个和1到0的情况
    ///TODO:加入金风那边的需求，加入禁止显示的用户列表配置功能
    bool bRes;
    bRes = connect(&m_userModel, &QLightDM::UsersModel::rowsInserted,
                   [this](const QModelIndex &parent, int first, int last) {
                       ///用户0->1 且 配置允许显示用户链表 且 当前登录模式为输入用户登录 且 手动登录还未输入用户名并点击确定
                       ///显示返回按钮
                       qInfo() << "rowInserted:" << m_userModel.rowCount(QModelIndex());
                       if ((m_userModel.rowCount(QModelIndex()) == 1) && m_showUserList && m_loginMode == LOGIN_MODE_MANUAL && !m_authProxy->isAuthenticated())
                       {
                           qInfo() << "setReturn visible true";
                           ui->btn_notListAndCancel->setVisible(true);
                       }
                   });
    if (!bRes)
    {
        KLOG_WARNING("connect rowsInserted failed!");
    }

    bRes = connect(&m_userModel, &QLightDM::UsersModel::rowsRemoved,
                   [this](const QModelIndex &parent, int first, int last) {
                       qInfo() << "rowRemoved:" << m_userModel.rowCount(QModelIndex());
                       if ((m_userModel.rowCount(QModelIndex()) == 0))
                       {
                           ///TODO:是否需要判断配置文件中能否手动登录
                           resetUIForManualLogin();
                       }
                   });
    if (!bRes)
    {
        KLOG_INFO("connect rowsRemoved failed!");
    }

    ui->userlist->loadUserList();
}

void GreeterLoginWindow::initSettings()
{
    if (KiranGreeterPrefs::instance()->isValid())
    {
        m_noListButotnVisiable = KiranGreeterPrefs::instance()->allow_manual_login();
        //不允许手动输入用户名，必须显示用户列表
        if (!m_noListButotnVisiable)
        {
            m_showUserList = true;
        }
        else
        {
            m_showUserList = !KiranGreeterPrefs::instance()->hide_user_list();
        }
    }
    else
    {
        KLOG_ERROR() << "can't connect greeter settings backend!";
        m_noListButotnVisiable = true;
        m_showUserList = true;
    }

    ///输出所有的配置项
    KLOG_DEBUG() << "greeter settings:"
                 << "\n"
                 << "\tmanual login:            " << m_noListButotnVisiable << "\n"
                 << "\tshow user list:          " << m_showUserList;

    if (m_showUserList && m_userModel.rowCount(QModelIndex()) > 0)
    {
        resetUIForUserListLogin();
    }
    else
    {
        resetUIForManualLogin();
    }
}

void GreeterLoginWindow::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
#ifdef VIRTUAL_KEYBOARD
    if (!event->isAccepted())
    {
        if (VirtualKeyboard::instance()->getKeyboard() != nullptr &&
            VirtualKeyboard::instance()->getKeyboard()->isVisible())
        {
            VirtualKeyboard::instance()->getKeyboard()->hide();
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
    bool needFilter = false;
    QMouseEvent *mouseEvent = nullptr;

    if (event->type() != QEvent::MouseButtonPress)
    {
        return false;
    }

    mouseEvent = dynamic_cast<QMouseEvent *>(event);

    QPoint mapedPoint = this->mapFromGlobal(mouseEvent->globalPos());

    QRect m_sessionMenuGemometry = m_sessionMenu->geometry();
    QRect m_powerMenuGeometry = m_powerMenu->geometry();

    if ((!m_sessionMenuGemometry.contains(mapedPoint)) && m_sessionMenu->isVisible())
    {
        m_sessionMenu->hide();
        needFilter = true;
        KLOG_DEBUG() << "session menu filter: " << obj->objectName() << event->type() << mouseEvent->buttons() << ",session menu hide!";
    }
    if ((!m_powerMenuGeometry.contains(mapedPoint)) && m_powerMenu->isVisible())
    {
        m_powerMenu->hide();
        needFilter = true;
        KLOG_DEBUG() << "power menu filter: " << obj->objectName() << event->type() << mouseEvent->buttons() << ",power menu hide";
    }
    if (needFilter)
    {
        KLOG_DEBUG() << "session: " << m_sessionMenuGemometry;
        KLOG_DEBUG() << "menu:    " << m_powerMenuGeometry;
        KLOG_DEBUG() << "pos:     " << mapedPoint;
    }
    return needFilter;
}

void GreeterLoginWindow::setTips(Kiran::MessageType type, const QString &text)
{
    QString colorText = QString("<font color=%1>%2</font>")
                            .arg("white")
                            .arg(text);
    ui->label_tips->setText(colorText);
}

void GreeterLoginWindow::startAuthUser(const QString &username, QString userIcon)
{
    KLOG_DEBUG() << "start authproxy:"
                 << "\n"
                 << "\tname:" << username << "\n"
                 << "\ticon:" << userIcon;

    if (m_authProxy->inAuthentication())
    {
        m_authProxy->cancelAuthentication();
    }

    m_havePrompted = false;

    ui->label_userName->setText(username);
    ui->loginAvatar->setImage(userIcon);
    if (username == m_greeter.autologinUserHint())
    {
        KLOG_DEBUG() << "authproxy user" << username << "is auto login user,switch to auto login";
        switchToAutoLogin();
        return;
    }
    else
    {
        switchToPromptEdit();
    }
    ui->promptEdit->reset();

    ///NOTE:为了解决在某些环境启动过快，导致的lightdm的认证回复prompt过慢几秒，
    ///     登录界面输入框未切换到密码模式,用户直接输入明文密码
    ///     暂时解决方案单独禁用输入框，等待lightdm的prompt消息会启用输入框
    ui->promptEdit->setEnabled(false);

    ///FIXME:鼠标点击认证用户列表时，需要延时设置输入焦点到输入框，不然又会被置回UserItem
    setEditPromptFocus(200);

    m_authProxy->authenticate(username);
}

void GreeterLoginWindow::resetUIForUserListLogin()
{
    KLOG_DEBUG("set ui for user list login");
    if (m_authProxy->inAuthentication())
    {
        m_authProxy->cancelAuthentication();
    }

    //NotList按钮
    m_buttonType = BUTTON_SWITCH_TO_MANUAL_LOGIN;
    ui->btn_notListAndCancel->setText(tr("login other user"));
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

    setCurrentAuthType(AUTH_TYPE_PASSWD);

    UserInfo userinfo;
    if (ui->userlist->getCurrentSelected(userinfo))
    {
        slotUserActivated(userinfo);
    }
    else
    {
        ui->userlist->setRow0();
    }
}

void GreeterLoginWindow::resetUIForManualLogin()
{
    KLOG_DEBUG("set ui for manual login");
    if (m_authProxy->inAuthentication())
    {
        m_authProxy->cancelAuthentication();
    }

    //返回使用用户列表登录模式
    m_buttonType = BUTTON_RETURN;
    ui->btn_notListAndCancel->setText(tr("Return"));
    ui->btn_notListAndCancel->setVisible(m_showUserList && ui->userlist->userCount() > 0);
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

    setCurrentAuthType(AUTH_TYPE_PASSWD);

    //用户列表隐藏
    ui->userlist->setEnabled(false);
    ui->userlist->setVisible(false);

    m_loginMode = LOGIN_MODE_MANUAL;
}

void GreeterLoginWindow::startUpdateTimeTimer()
{
    QMetaObject::invokeMethod(this, "updateTimeLabel", Qt::AutoConnection);
    QTime curTime = QTime::currentTime();
    int nextUpdateSecond = 60 - curTime.second();
    QTimer::singleShot(nextUpdateSecond * 1000, this, SLOT(startUpdateTimeTimer()));
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
    KLOG_INFO() << "caps lock status changed: " << on;
    GreeterLoginWindow *This = static_cast<GreeterLoginWindow *>(user_data);
    QPixmap pixmap;
    if (on)
    {
        pixmap.load(":/images/caps_lock.png");
        pixmap = pixmap.scaledToWidth(This->ui->label_capsLock->width());
        pixmap = pixmap.scaledToHeight(This->ui->label_capsLock->height());
        This->ui->label_capsLock->setPixmap(pixmap);
    }
    else
    {
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
    if (m_inputMode == EDIT_INPUT_PROMPT_RESPOND)
    {
        m_authProxy->respond(ui->promptEdit->getText());
    }
    else
    {
        startAuthUser(ui->promptEdit->getText(),
                      ui->userlist->getIconByUserName(ui->promptEdit->getText()));
    }
}

void GreeterLoginWindow::slotUserActivated(const UserInfo &userInfo)
{
    ui->label_tips->clear();
    startAuthUser(userInfo.name, userInfo.imagePath);
}

void GreeterLoginWindow::slotButtonClicked()
{
    KLOG_DEBUG() << "button clicked:";
    KLOG_DEBUG() << "    button type[" << m_buttonType << "]";
    KLOG_DEBUG() << "    login  mode[" << m_loginMode << "]";
    KLOG_DEBUG() << "    intput mode[" << m_inputMode << "]";
    if (m_buttonType == BUTTON_SWITCH_TO_MANUAL_LOGIN)
    {
        Q_ASSERT(m_loginMode == LOGIN_MODE_USER_LIST);
        resetUIForManualLogin();
    }
    else if (m_buttonType == BUTTON_RETURN)
    {
        Q_ASSERT(m_loginMode == LOGIN_MODE_MANUAL);
        //输入用户名返回则返回至用户列表选择
        if (m_inputMode == EDIT_INPUT_USER_NAME)
        {
            //用户列表不显示不应该执行到这
            Q_ASSERT(m_showUserList);
            resetUIForUserListLogin();
        }
        else if (m_inputMode == EDIT_INPUT_PROMPT_RESPOND)
        {
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

void GreeterLoginWindow::slotShowMessage(QString text, Kiran::MessageType type)
{
    std::string stdText = text.toStdString();
    setTips(type, stdText.c_str());
}

void GreeterLoginWindow::slotShowprompt(QString text, Kiran::PromptType type)
{
    //用户手动登录，需要设置用户名
    if (m_loginMode == LOGIN_MODE_MANUAL)
    {
        if (m_authProxy->authenticationUser() != ui->label_userName->text())
        {
            ui->label_userName->setText(m_authProxy->authenticationUser());
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
    ui->promptEdit->setEchoMode(
        type == Kiran::PromptTypeSecret ? QLineEdit::Password : QLineEdit::Normal);
    ///FIXME:需要延时设置输入焦点到输入框，不然又会被置回UserItem
    m_havePrompted = true;
    setEditPromptFocus(200);
}

void GreeterLoginWindow::slotAuthenticationComplete(bool success)
{
    if (success)
    {
#ifdef VIRTUAL_KEYBOARD
        //在关闭时若虚拟键盘的副窗口设置为当前窗口的话，则更改父窗口,避免释放相关X资源导致onboard释放出错，导致onboard崩溃
        if (VirtualKeyboard::instance()->getKeyboard())
        {
            if (VirtualKeyboard::instance()->getKeyboard()->parentWidget() == this)
            {
                VirtualKeyboard::instance()->getKeyboard()->setParent(nullptr);
            }
        }
#endif
        if (!m_greeter.startSessionSync(m_session))
        {
            KLOG_WARNING() << "start session failed,session:" << m_session;
        }
    }
    else
    {
        if (m_havePrompted)
        {
            startAuthUser(m_authProxy->authenticationUser(),
                          ui->userlist->getIconByUserName(m_authProxy->authenticationUser()));
        }
        else
        {
            switchToReAuthentication();
        }
    }
}

void GreeterLoginWindow::setCurrentAuthType(AuthType type)
{
    ui->promptEdit->setVisible(type == AUTH_TYPE_PASSWD);
    ui->loginAvatar->setVisible(type == AUTH_TYPE_PASSWD);

    ui->faceAvatar->setVisible(type == AUTH_TYPE_FACE);
    if (type == AUTH_TYPE_FACE)
    {
        slotShowMessage(tr("Start face authentication"), Kiran::MessageTypeInfo);
        ui->faceAvatar->startAnimation();
    }
    else
    {
        ui->faceAvatar->stopAnimation();
    }

    ui->fingerAvatar->setVisible(type == AUTH_TYPE_FINGER);
    if (type == AUTH_TYPE_FINGER)
    {
        slotShowMessage(tr("Start fingerprint authentication"), Kiran::MessageTypeInfo);
        ui->fingerAvatar->startAnimation();
    }
    else
    {
        ui->fingerAvatar->stopAnimation();
    }

    m_authType = type;
}
