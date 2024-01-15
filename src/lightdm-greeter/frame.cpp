/**
 * Copyright (c) 2020 ~ 2023 KylinSec Co., Ltd.
 * kiran-session-guard is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     liuxinhao <liuxinhao@kylinsec.com.cn>
 */
#include "frame.h"
#include "auth-lightdm.h"
#include "auxiliary.h"
#include "prefs.h"
#include "qt5-log-i.h"
#include "user-manager.h"
#include "virtual-keyboard.h"
#include "widgets/greeter-menu-item.h"
#include "widgets/login-button.h"
#include "widgets/user-list.h"

#include <QBoxLayout>
#include <QButtonGroup>
#include <QMenu>
#include <QMouseEvent>
#include <QSpacerItem>
#include <QStateMachine>
#include <QToolButton>
#include <QWidgetAction>
#include <QWindow>

namespace Kiran
{
namespace SessionGuard
{
namespace Greeter
{
Frame::Frame(Prefs* prefs, QWidget* parent)
    : LoginFrame(parent),
      m_prefs(prefs),
      m_greeter(new QLightDM::Greeter())
{
    setObjectName("Frame");

    initSettings();
    initMenus();
    initUI();
    initAuth();
    initConnection();

    if (m_showUserList && m_userList->userCount() > 0)
    {
        reset(STATE_USER_LIST_LOGIN);
    }
    else
    {
        reset(STATE_INTPUT_USER_NAME);
    }
}

Frame::~Frame()
{
}

bool Frame::eventFilter(QObject* watched, QEvent* event)
{
    RETURN_VAL_IF_TRUE(event->type() != QEvent::MouseButtonPress, false);

    bool filtered = false;
    auto mouseEvent = dynamic_cast<QMouseEvent*>(event);
    auto pressedPos = this->mapFromGlobal(mouseEvent->globalPos());
    auto sessionMenuGeo = m_sessionMenu->geometry();
    auto powerMenuGeo = m_powerMenu->geometry();

    if ((!sessionMenuGeo.contains(pressedPos)) && m_sessionMenu->isVisible())
    {
        m_sessionMenu->hide();
        filtered = true;
    }

    if ((!powerMenuGeo.contains(pressedPos)) && m_powerMenu->isVisible())
    {
        m_powerMenu->hide();
        filtered = true;
    }

    return filtered;
}

void Frame::initSettings()
{
    m_allowManualLogin = true;
    m_showUserList = true;

    if (!m_prefs->allow_manual_login())
    {
        m_allowManualLogin = false;
    }
    else if (m_prefs->hide_user_list())
    {
        m_showUserList = false;
    }
}

void Frame::initMenus()
{
    auto setMenuAttributes = [](QMenu* menu)
    {
        menu->setAttribute(Qt::WA_TranslucentBackground);  // 透明必需
        // FIXME:QMenu不能为窗口，只能为控件，不然透明效果依赖于窗口管理器混成特效与显卡
        // 控件的话QMenu显示出来的话，不能点击其他区域隐藏窗口，需要手动隐藏
        menu->setWindowFlags(Qt::FramelessWindowHint | Qt::Widget);  // 透明必需
        menu->setContentsMargins(0, 0, 0, 0);
        menu->hide();
    };

    m_powerMenu = new QMenu(this);
    m_powerMenu->setFixedWidth(92);
    setMenuAttributes(m_powerMenu);

    m_sessionMenu = new QMenu(this);
    m_sessionMenu->setMinimumWidth(92);
    m_sessionMenu->setMaximumWidth(184);
    setMenuAttributes(m_sessionMenu);

    QLightDM::SessionsModel sessionModel;
    sessionModel.sort(QLightDM::SessionsModel::KeyRole);

    QButtonGroup* buttonGroup = new QButtonGroup(m_sessionMenu);
    buttonGroup->setExclusive(true);

    QStringList hiddenSessions = m_prefs->hiddenSessions();
    for (int i = 0; i < sessionModel.rowCount(QModelIndex()); i++)
    {
        auto key = sessionModel.data(sessionModel.index(i, 0), QLightDM::SessionsModel::KeyRole);
        auto id = sessionModel.data(sessionModel.index(i, 0), QLightDM::SessionsModel::IdRole);

        CONTINUE_IF_TRUE(hiddenSessions.contains(key.toString()));

        auto itemWidget = new GreeterMenuItem(key.toString(), true);
        itemWidget->setObjectName("GreeterMenuItem");
        itemWidget->setFixedHeight(28);
        itemWidget->setMinimumWidth(90);
        itemWidget->setMaximumWidth(120);
        itemWidget->setExclusiveGroup(buttonGroup);

        auto widgetAction = new QWidgetAction(m_sessionMenu);
        widgetAction->setDefaultWidget(itemWidget);
        m_sessionMenu->addAction(widgetAction);
        m_sessionItemMap[key.toString()] = itemWidget;

        // clang-format off
        connect(itemWidget, &GreeterMenuItem::sigChecked, [this](QString action){
            m_specifiedSession = action;
            m_sessionMenu->hide();
        });
        // clang-format on
    }
}

void Frame::initUI()
{
    resize(1024, 768);

    /// FIXME:因弹出窗口不是作为新的窗口，而是作为一个控件，需要我们去做隐藏
    /// 开始监听整个应用程序事件，在窗口点击事件中判断隐藏菜单
    qApp->installEventFilter(this);

    // 添加左下方用户列表
    m_userList = new UserList(this);
    m_userList->setObjectName("userList");
    m_userList->setMinimumSize(QSize(114, 62));
    m_userList->setMaximumSize(QSize(228, 310));
    m_userList->loadUserList();
    m_userList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_userList->setEnabled(m_showUserList);
    setLeftBottomWidget(m_userList);

    // 添加中下部登录其他用户按钮
    QFont font;
    font.setFamily("Noto Sans CJK SC");
    font.setUnderline(true);
    m_btnLoginOther = new QToolButton(this);
    m_btnLoginOther->setCursor(QCursor(Qt::PointingHandCursor));
    m_btnLoginOther->setObjectName("LoginOtherUser");
    m_btnLoginOther->setText(tr("login other user"));
    m_btnLoginOther->setFont(font);
    addWidgetToCenterBottomWidget(0, m_btnLoginOther);
    addLayoutItemToCenterBottomWidget(0, new QSpacerItem(10, 16, QSizePolicy::Minimum, QSizePolicy::Fixed));

    // 添加中部控制页面,用于添加登录按钮用于触发登录自动登录用户
    auto controlPage = new QWidget(this);
    auto controlLayout = new QHBoxLayout(controlPage);
    controlLayout->setContentsMargins(0, 0, 0, 0);
    controlLayout->setSpacing(0);

    m_btnAutoLogin = new LoginButton(this);
    m_btnAutoLogin->setMinimumSize(QSize(150, 30));
    m_btnAutoLogin->setMaximumSize(QSize(180, 36));
    controlLayout->addWidget(m_btnAutoLogin, 0, Qt::AlignCenter);
    m_autloginIdx = appendControlPage(controlPage);

    // 初始化添加右下控件
    // 获取菜单弹出坐标,按钮和触发菜单右对齐
    // FIXME: Qt在特定虚拟机环境下QMenu::popup传入正确的pos时,QMenu通过Pos找到screen,但screen的大小错误(为调整分辨率之前的分辨率)
    // 导致popup pos被修改成在错误的屏幕范围内
    auto getMenuPopupPos = [this](QMenu* menu, const QToolButton* triggerBtn) -> QPoint
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
        // button->setMinimumSize(QSize(24, 20));
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
    m_btnSession = createActionButton("btn_session", tr("session menu"), [this,getMenuPopupPos] () {
        if( m_sessionMenu->isVisible() )
        {
            m_sessionMenu->hide();
            return;
        }

        auto pos = getMenuPopupPos(m_sessionMenu,m_btnSession);
        m_powerMenu->close();
        m_sessionMenu->popup(pos);
    });
    m_btnPower = createActionButton("btn_power",tr("power menu"),[this,getMenuPopupPos]{
        if( m_powerMenu->isVisible() )
        {
            m_powerMenu->hide();
            return;
        }
        m_powerMenu->clear();
        if (m_powerIface.canHibernate() && m_prefs->canHibernate())
        {
            m_powerMenu->addAction(tr("hibernate"), [this] {
                this->m_powerIface.hibernate();
            });
        }
        if (m_powerIface.canSuspend() && m_prefs->canSuspend())
        {
            m_powerMenu->addAction(tr("suspend"), [this] {
                this->m_powerIface.suspend();
            });
        }
        if (m_powerIface.canRestart() && m_prefs->canReboot())
        {
            m_powerMenu->addAction(tr("restart"), [this] {
                this->m_powerIface.restart();
            });
        }
        if (m_powerIface.canShutdown() && m_prefs->canPowerOff())
        {
            m_powerMenu->addAction(tr("shutdown"), [this] {
                this->m_powerIface.shutdown();
            });
        }
        auto pos = getMenuPopupPos(m_powerMenu,m_btnPower);
        m_powerMenu->popup(pos);
    });
    m_btnKeyboard = createActionButton("btn_keyboard",tr("virtual keyboard"),[this]{
        auto keyboard = VirtualKeyboard::instance();
        if( keyboard->isVisible() )
            keyboard->hide();
        else
            keyboard->showAdjustSize(this);
        this->window()->windowHandle()->setKeyboardGrabEnabled(true);
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
    rbBtnLayout->addWidget(m_btnSession, 1);
    rbBtnLayout->addWidget(m_btnKeyboard, 1);
    rbBtnLayout->addWidget(m_btnPower, 1);

    setRightBottomWidget(rbBtnWidget);
}

void Frame::initAuth()
{
    AuthLightdm* auth = new AuthLightdm(m_greeter);
    LoginFrame::initAuth(auth);
    connect(m_greeter.data(),&QLightDM::Greeter::autologinTimerExpired,this,&Frame::onAutoLoginTimeout);
}

void Frame::initConnection()
{
    // clang-format off
    connect(m_userList, &UserList::userActivated, [this](const UserInfo& info){ onUserSelected(info.name); });
    connect(m_userList, &UserList::userCountChanged, this, &Frame::onUserListUserCountChanged);
    connect(m_userList, &UserList::userRemoved,this,&Frame::onUserListUserRemoved);
    connect(m_btnLoginOther, &QToolButton::clicked, this, &Frame::onLoginOtherClicked);
    connect(m_btnAutoLogin, &LoginButton::sigClicked, [this](){
        startAuthUser(m_userName);
    });
    // clang-format on
}

void Frame::reset(State state)
{
    RETURN_IF_FALSE(state != m_state);

    switch (state)
    {
    case STATE_USER_LIST_LOGIN:
    {
        LoginFrame::reset();
        m_userName.clear();

        m_userList->setEnabled(true);
        m_userList->setVisible(true);

        m_btnLoginOther->setVisible(m_allowManualLogin);
        m_btnLoginOther->setText(tr("login other user"));

        UserInfo info;
        m_userList->setRow0();
        m_userList->getCurrentSelected(info);
        onUserSelected(info.name);
        break;
    }
    case STATE_INTPUT_USER_NAME:
    {
        LoginFrame::reset();
        m_userName.clear();

        m_userList->setEnabled(false);
        m_userList->setVisible(false);

        m_btnLoginOther->setVisible(m_showUserList && m_userList->userCount() > 0);
        m_btnLoginOther->setText(tr("return"));

        break;
    }
    case STATE_MANUAL_LOGIN_AUTH:
    {
        m_userList->setEnabled(false);
        m_userList->setVisible(false);

        m_btnLoginOther->setVisible(true);
        m_btnLoginOther->setText(tr("return"));
        break;
    }
    default:
        break;
    }

    m_state = state;
}

void Frame::authUserInputed(const QString& userName)
{
    // 手动输入用户名完成,切换状态,以及匹配的界面
    reset(STATE_MANUAL_LOGIN_AUTH);
    // 触发该用户登录
    onUserSelected(userName);
}

void Frame::authenticateComplete(bool authRes, const QString& userName)
{
    if (authRes)
    {
        m_greeter->startSessionSync(m_specifiedSession);
    }
}

void Frame::onUserSelected(const QString& name)
{
    RETURN_IF_FALSE(name != m_userName);

    m_userName = name;
    m_specifiedSession.clear();

    QString userSession = UserManager::getUserLastSession(name);
    KLOG_DEBUG() << "user session:" << name << userSession;

    GreeterMenuItem* sessionMenuItem = nullptr;
    if (!userSession.isEmpty() && m_sessionItemMap.contains(userSession))
    {
        // 默认选择用户上次进入的桌面环境
        auto lastSessionItem = m_sessionItemMap[userSession];
        sessionMenuItem = lastSessionItem;
    }
    else if (!m_sessionItemMap.isEmpty())
    {
        // 该用户未登录过桌面环境
        auto kiranSession = m_sessionItemMap.find("kiran");
        if (kiranSession != m_sessionItemMap.end())
        {
            sessionMenuItem = *kiranSession;
        }
        else
        {
            sessionMenuItem = m_sessionItemMap.isEmpty() ? nullptr : m_sessionItemMap.first();
        }
    }

    if (sessionMenuItem != nullptr)
    {
        sessionMenuItem->setChecked(true);
        // 手动发出信号，避免之前已经是勾选的情况不能触发相应的信号槽
        sessionMenuItem->sigChecked(sessionMenuItem->getActionName());
    }

    if (name == m_greeter->autologinUserHint())
    {
        // 当用户选用自动登录用户时,不马上开启认证,可是切换至自动登录按钮,按钮点击时再触发自动登录
        setAuthUserInfo(m_userName);
        switchControlPage(m_autloginIdx);
    }
    else
    {
        startAuthUser(m_userName);
    }
}

// 处理用户列表用户变化信号,切换到合适的模式
void Frame::onUserListUserCountChanged(int oldCount, int newCount)
{
    // 处于用户手动输入用户名阶段,且支持用户列表,切换至用户列表登录模式
    if ((oldCount == 0 && newCount > 0) && m_state == STATE_INTPUT_USER_NAME && m_showUserList)
    {
        reset(STATE_USER_LIST_LOGIN);
        return;
    }

    // 用户列表从有到无,处于用户列表模式,切换至手动输入用户名登录方式
    if ((oldCount > 0 && newCount == 0) && m_state == STATE_USER_LIST_LOGIN)
    {
        reset(STATE_INTPUT_USER_NAME);
        return;
    }
}

// 处理用户列表用户删除信号,切换认证用户
void Frame::onUserListUserRemoved(const QString& name)
{
    if (m_userName == name && m_state == STATE_USER_LIST_LOGIN)
    {
        m_userList->setRow0();
    }
}

void Frame::onLoginOtherClicked()
{
    // 根据不同的界面状态,LoginOther所代表的按钮不同,执行不同的操作
    switch (m_state)
    {
    case STATE_USER_LIST_LOGIN:
        // 当前是用户列表登录模式,切换至手动输入用户名模式
        reset(STATE_INTPUT_USER_NAME);
        break;
    case STATE_INTPUT_USER_NAME:
        // 当前是手动输入用户名模式,切换至用户列表登录模式
        reset(STATE_USER_LIST_LOGIN);
        break;
    case STATE_MANUAL_LOGIN_AUTH:
        // 当前是手动输入名已完成开始认证阶段,切换至手动输入用户名模式
        reset(STATE_INTPUT_USER_NAME);
        break;
    default:
        break;
    }
}

static bool getIsLoggedIn(const QString& userName)
{
    bool res = false;

    QDBusInterface dmInterface("org.freedesktop.DisplayManager",
                               "/org/freedesktop/DisplayManager",
                               "org.freedesktop.DisplayManager",
                               QDBusConnection::systemBus());
    QVariant sessionsVar = dmInterface.property("Sessions");
    if (!sessionsVar.isValid())
    {
        KLOG_ERROR("can't get display manager property 'sessions'");
        return res;
    }

    QList<QDBusObjectPath> sessions = sessionsVar.value<QList<QDBusObjectPath>>();
    KLOG_DEBUG() << "sessions:" << sessions.count();
    for (const auto& session : sessions)
    {
        KLOG_DEBUG() << "\t-" << session.path();
    }
    foreach (const auto& session, sessions)
    {
        QDBusInterface sessionInterface("org.freedesktop.DisplayManager",
                                        session.path(),
                                        "org.freedesktop.DisplayManager.Session",
                                        QDBusConnection::systemBus());
        QVariant userNameVar = sessionInterface.property("UserName");
        if (!userNameVar.isValid())
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

void Frame::onAutoLoginTimeout()
{
    if (m_greeter->autologinUserHint().isEmpty())
    {
        return;
    }

    // NOTE:修复#52982问题，若自动登录用户已存在不自动触发延时自动登录
    bool isLogged = getIsLoggedIn(m_greeter->autologinUserHint());
    if (isLogged)
    {
        return;
    }

    startAuthUser(m_greeter->autologinUserHint());
}

}  // namespace Greeter
}  // namespace SessionGuard
}  // namespace Kiran