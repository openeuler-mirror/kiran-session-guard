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

bool getIsLoggedIn(const QString &userName)
{
    bool res = false;

    QDBusInterface dmInterface("org.freedesktop.DisplayManager",
                               "/org/freedesktop/DisplayManager",
                               "org.freedesktop.DisplayManager",
                               QDBusConnection::systemBus());
    QVariant sessionsVar = dmInterface.property("Sessions");
    if( !sessionsVar.isValid() )
    {
        KLOG_ERROR("can't get display manager property 'sessions'");
        return res;
    }

    QList<QDBusObjectPath> sessions = sessionsVar.value<QList<QDBusObjectPath>>();
    KLOG_DEBUG() << "sessions:" << sessions.count();
    for(const auto &session:sessions)
    {
        KLOG_DEBUG() << "\t-" << session.path();
    }
    foreach (const auto &session, sessions)
    {
        QDBusInterface sessionInterface("org.freedesktop.DisplayManager",
                                        session.path(),
                                        "org.freedesktop.DisplayManager.Session",
                                        QDBusConnection::systemBus());
        QVariant userNameVar = sessionInterface.property("UserName");
        if( !userNameVar.isValid() )
        {
            KLOG_ERROR("can't get display manager session property 'UserName'");
            continue;
        }

        QString sessionUser = userNameVar.toString();
        if (sessionUser.compare(userName) == 0)
        {
            res = true;
            break;
        }
    }

    return res;
}

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

    ///??????CapsLock??????
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
    ///FIXME:?????????????????????????????????????????????????????????????????????????????????????????????
    ///?????????????????????????????????????????????????????????????????????????????????
    qApp->installEventFilter(this);

    ///??????ToopTip
    ui->btn_session->setToolTip(tr("session menu"));
    ui->btn_keyboard->setToolTip(tr("virtual keyboard"));
    ui->btn_power->setToolTip(tr("power menu"));

    connect(m_sessionMenu, &QMenu::triggered, [this]() {
        m_sessionMenu->hide();
    });

    ///????????????????????????
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
    ///??????????????????
    connect(ui->btn_power, &QToolButton::pressed, [this] {
        if (m_powerMenu->isVisible())
        {
            m_powerMenu->hide();
            return;
        }
        //??????????????????
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
        //????????????????????????
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

    if (!KiranGreeterPrefs::instance()->canHibernate() &&
        !KiranGreeterPrefs::instance()->canSuspend() &&
        !KiranGreeterPrefs::instance()->canPowerOff() &&
        !KiranGreeterPrefs::instance()->canReboot())
    {
        ui->btn_power->setVisible(false);
    }

    ///??????????????????
    connect(ui->userlist, &UserListWidget::userActivated,
            this, &GreeterLoginWindow::slotUserActivated);
    ///????????????????????????
    connect(ui->btn_autologin, &LoginButton::sigClicked, [this]() {
        m_authProxy->authenticate(m_greeter.autologinUserHint());
    });
    connect(&m_greeter, &QLightDM::Greeter::autologinTimerExpired, [this]() {
        //NOTE:?????????????????????autologin-timeout,????????????autologin-user?????????
        if( !m_greeter.autologinUserHint().isEmpty() )
            m_authProxy->authenticate(m_greeter.autologinUserHint());
    });
    ///????????????????????????
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
    ///??????????????????????????????????????????
    connect(ui->promptEdit, &PromptEdit::textConfirmed,
            this, &GreeterLoginWindow::slotTextConfirmed);
    ///?????????????????????????????????
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
    ///????????????????????????????????????????????????
    connect(ui->userlist, &UserListWidget::sigRequestResetUI, [this] {
        Q_ASSERT(m_loginMode == LOGIN_MODE_USER_LIST);
        resetUIForUserListLogin();
    });
    startUpdateTimeTimer();
}

void GreeterLoginWindow::initMenu()
{
    ///?????????????????????
    m_powerMenu = new QMenu(this);                            //??????????????????????????????
    m_powerMenu->setAttribute(Qt::WA_TranslucentBackground);  //????????????
    ///FIXME:QMenu???????????????????????????????????????????????????????????????????????????????????????????????????
    ///????????????QMenu??????????????????????????????????????????????????????????????????????????????
    m_powerMenu->setWindowFlags(Qt::FramelessWindowHint | Qt::Widget);  //????????????
    m_powerMenu->setContentsMargins(0, 0, 0, 0);
    m_powerMenu->setFixedWidth(92);
    m_powerMenu->hide();

    ///session???????????????
    m_sessionMenu = new QMenu(this);
    m_sessionMenu->setMinimumWidth(92);
    m_sessionMenu->setMaximumWidth(184);
    m_sessionMenu->setAttribute(Qt::WA_TranslucentBackground);
    ///FIXME:QMenu???????????????????????????????????????????????????????????????????????????????????????????????????
    ///????????????QMenu??????????????????????????????????????????????????????????????????????????????
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
        {  //???????????????
            itemWidget->setChecked(true);
        }
        widgetAction->setDefaultWidget(itemWidget);
        m_sessionMenu->addAction(widgetAction);
    }
}

void GreeterLoginWindow::initLightdmGreeter()
{
    AuthBase *authInterface = new AuthLightdm(&m_greeter);
    AuthMsgQueue *msgQueue = new AuthMsgQueue();

    m_authProxy = new AuthProxy(authInterface, this);
    authInterface->setParent(m_authProxy);
    msgQueue->setParent(m_authProxy);

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

    ///?????????????????????0???1??????1???0?????????
    QStringList hiddenUsers = KiranGreeterPrefs::instance()->hiddenUsers();
    m_filterModel.setSourceModel(&m_userModel);
    m_filterModel.setFilterRole(UsersModel::NameRole);
    m_filterModel.setFilterUsers(hiddenUsers);

    bool bRes;
    bRes = connect(&m_filterModel, &QLightDM::UsersModel::rowsInserted,
                   [this](const QModelIndex &parent, int first, int last) {
                       ///??????0->1 ??? ?????????????????????????????? ??? ??????????????????????????????????????? ??? ????????????????????????????????????????????????
                       ///??????????????????
                       qInfo() << "rowInserted:" << m_filterModel.rowCount(QModelIndex());
                       if ((m_filterModel.rowCount(QModelIndex()) == 1) && m_showUserList && m_loginMode == LOGIN_MODE_MANUAL && !m_greeter.isAuthenticated())
                       {
                           qInfo() << "setReturn visible true";
                           ui->btn_notListAndCancel->setVisible(true);
                       }
                   });
    if (!bRes)
    {
        KLOG_WARNING("connect rowsInserted failed!");
    }

    bRes = connect(&m_filterModel, &QLightDM::UsersModel::rowsRemoved,
                   [this](const QModelIndex &parent, int first, int last) {
                       qInfo() << "rowRemoved:" << m_filterModel.rowCount(QModelIndex());
                       if ((m_filterModel.rowCount(QModelIndex()) == 0))
                       {
                           ///TODO:???????????????????????????????????????????????????
                           resetUIForManualLogin();
                       }
                   });
    if (!bRes)
    {
        KLOG_INFO("connect rowsRemoved failed!");
    }

    ui->userlist->loadUserList();

    //NOTE:??????#52982????????????????????????????????????????????????????????????????????????
    if ( !m_greeter.autologinUserHint().isEmpty() )
    {
        bool isLogged = getIsLoggedIn(m_greeter.autologinUserHint());
        if( isLogged )
        {
            m_greeter.cancelAutologin();
        }
#if 0
        //WARNING:????????????????????????root?????????????????????
        UserInfo autologinUserInfo = ui->userlist->getUserInfoByUserName(m_greeter.autologinUserHint());
        if (autologinUserInfo.loggedIn)
        {
            m_greeter.cancelAutologin();
        }
#endif
    }
}

void GreeterLoginWindow::initSettings()
{
    if (KiranGreeterPrefs::instance()->isValid())
    {
        m_noListButotnVisiable = KiranGreeterPrefs::instance()->allow_manual_login();
        //?????????????????????????????????????????????????????????
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

    ///????????????????????????
    KLOG_DEBUG() << "greeter settings:"
                 << "\n"
                 << "\tmanual login:            " << m_noListButotnVisiable << "\n"
                 << "\tshow user list:          " << m_showUserList;

    if (m_showUserList && m_filterModel.rowCount(QModelIndex()) > 0)
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
 * @brief ???????????????????????????????????????QApplication?????????????????????????????????????????????????????????
 *        ????????????????????????????????????????????????
 * @param obj   ????????????
 * @param  event ??????
 * @return ????????????
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
    }
    else
    {
        switchToPromptEdit();
        ui->promptEdit->reset();
        ///NOTE:???????????????????????????????????????????????????lightdm???????????????prompt???????????????
        ///     ?????????????????????????????????????????????,??????????????????????????????
        ///     ????????????????????????????????????????????????lightdm???prompt????????????????????????
        ui->promptEdit->setEnabled(false);
        ///FIXME:??????????????????????????????????????????????????????????????????????????????????????????????????????UserItem
        setEditPromptFocus(200);

        m_authProxy->authenticate(username);
    }
}

void GreeterLoginWindow::resetUIForUserListLogin()
{
    KLOG_DEBUG("set ui for user list login");
    if (m_authProxy->inAuthentication())
    {
        m_authProxy->cancelAuthentication();
    }

    //NotList??????
    m_buttonType = BUTTON_SWITCH_TO_MANUAL_LOGIN;
    ui->btn_notListAndCancel->setText(tr("login other user"));
    ui->btn_notListAndCancel->setVisible(m_noListButotnVisiable);
    ui->btn_notListAndCancel->setEnabled(true);

    //?????????????????????
    ui->loginAvatar->setDefaultImage();

    //???????????????
    ui->label_userName->clear();

    //???????????????
    ui->promptEdit->reset();
    switchToPromptEdit();
    setEditPromptFocus();

    //tips??????
    ui->label_tips->clear();

    //??????????????????
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

    //????????????????????????????????????
    m_buttonType = BUTTON_RETURN;
    ui->btn_notListAndCancel->setText(tr("Return"));
    ui->btn_notListAndCancel->setVisible(m_showUserList && ui->userlist->userCount() > 0);
    ui->btn_notListAndCancel->setEnabled(true);

    //?????????????????????
    ui->loginAvatar->setDefaultImage();

    //???????????????
    ui->label_userName->clear();

    //???????????????
    ui->promptEdit->reset();
    ui->promptEdit->setPlaceHolderText(tr("Entry your name"));
    m_inputMode = EDIT_INPUT_USER_NAME;

    switchToPromptEdit();
    setEditPromptFocus();

    //tips??????
    ui->label_tips->clear();

    setCurrentAuthType(AUTH_TYPE_PASSWD);

    //??????????????????
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
    //tr : MM???MM??? dddd HH:mm
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
    ///??????????????????????????????
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
        //???????????????????????????????????????????????????
        if (m_inputMode == EDIT_INPUT_USER_NAME)
        {
            //??????????????????????????????????????????
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
    //??????????????????????????????????????????
    if (m_loginMode == LOGIN_MODE_MANUAL)
    {
        if (m_authProxy->authenticationUser() != ui->label_userName->text())
        {
            ui->label_userName->setText(m_authProxy->authenticationUser());
        }
        //??????????????????
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
    ///FIXME:??????????????????????????????????????????????????????????????????UserItem
    m_havePrompted = true;
    setEditPromptFocus(200);
}

void GreeterLoginWindow::slotAuthenticationComplete(bool success)
{
    if (success)
    {
#ifdef VIRTUAL_KEYBOARD
        //???????????????????????????????????????????????????????????????????????????????????????,??????????????????X????????????onboard?????????????????????onboard??????
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
