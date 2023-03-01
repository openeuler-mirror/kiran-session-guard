#include "login-frame.h"
#include "auth-controller.h"
#include "auth-type-switcher.h"
#include "auxiliary.h"
#include "ui_login-frame.h"
#include "user-manager.h"

#include <qt5-log-i.h>
#include <QBoxLayout>
#include <QDebug>
#include <QFile>
#include <QTime>
#include <QTimer>
#include <QToolButton>

GUARD_BEGIN_NAMESPACE
LoginFrame::LoginFrame(QWidget* parent)
    : QWidget(parent),
      ui(new Ui::LoginFrame),
      m_authController(new AuthController(this))
{
    ui->setupUi(this);
    initUI();
}

LoginFrame::~LoginFrame()
{
    delete ui;
}

void LoginFrame::initAuth(AuthBase* auth)
{
    m_authController->init(auth);

    connect(m_authController, &AuthController::showMessage, this, &LoginFrame::onShowMessage);
    connect(m_authController, &AuthController::showPrompt, this, &LoginFrame::onShowPrmpt);
    connect(m_authController, &AuthController::authenticationComplete, this, &LoginFrame::onAuthComplete);

    connect(m_authController, &AuthController::notifyAuthMode, this, &LoginFrame::onNotifyAuthMode);
    connect(m_authController, &AuthController::supportedAuthTypeChanged, this, &LoginFrame::onSupportedAuthTypeChanged);
    connect(m_authController, &AuthController::authTypeChanged, this, &LoginFrame::onAuthTypeChanged);
}

void LoginFrame::reset()
{
    // 取消认证
    if (m_authController->inAuthentication())
    {
        m_authController->cancelAuthentication();
    }

    // 清空内容
    switchControlPage(LoginFrame::CONTROL_PAGE_PROMPT_EDIT);

    ui->avatar->setDefaultImage();
    // NOTE:QLabel::clear会清空QLabelPrivate::isTextLabel标志,会影响QLabel大小,导致重新布局,界面闪烁
    // 使用setText()重置文本
    ui->userName->setText("");
    ui->edit->reset();
    ui->tips->clear();
    m_switcher->setVisible(false);

    m_editMode = EDIT_MODE_USER_NAME;
    m_prompted = false;
    m_specifyUser.clear();

    ui->edit->setPlaceHolderText(tr("Entry your name"));
    setEditFocus();
}

void LoginFrame::setAuthUserInfo(const QString& userName)
{
    QString icon = UserManager::getUserIcon(userName);
    ui->avatar->setImage(icon);
    ui->userName->setText(userName);
}

void LoginFrame::startAuthUser(const QString& userName)
{
    KLOG_DEBUG() << "start auth:" << userName;
    setAuthUserInfo(userName);

    m_editMode = EDIT_MODE_PROMPT_RESPOSE;
    m_prompted = false;
    m_specifyUser = userName;

    /// NOTE:为了解决在某些环境启动过快，导致的lightdm的认证回复prompt过慢几秒，
    ///      登录界面输入框未切换到密码模式,用户直接输入明文密码
    ///      暂时解决方案单独禁用输入框，等待lightdm的prompt消息会启用输入框
    ui->edit->reset();
    ui->edit->setEnabled(false);
    ui->tips->clear();
    m_switcher->setVisible(false);

    switchControlPage(CONTROL_PAGE_PROMPT_EDIT);
    m_authController->authenticate(userName);
}

void LoginFrame::setTips(MessageType type, const QString& text)
{
    QString colorText = QString("<font color=%1>%2</font>")
                            .arg("white")
                            .arg(text);
    ui->tips->setText(colorText);
}

void LoginFrame::setLeftTopWidget(QWidget* w)
{
    if (m_leftTopWidget)
    {
        ui->left_top->layout()->removeWidget(m_leftTopWidget);
    }
    m_leftTopWidget = w;
    ui->left_top->layout()->addWidget(m_leftTopWidget);
}

void LoginFrame::setLeftBottomWidget(QWidget* w)
{
    if (m_leftBottomWidget)
    {
        ui->left_bottom->layout()->removeWidget(m_leftBottomWidget);
    }
    m_leftBottomWidget = w;
    ui->left_bottom->layout()->addWidget(m_leftBottomWidget);
}

void LoginFrame::setRightTopWidget(QWidget* w)
{
    if (m_rightTopWidget)
    {
        ui->right_top->layout()->removeWidget(m_rightTopWidget);
    }
    m_rightTopWidget = w;
    ui->right_top->layout()->addWidget(m_rightTopWidget);
}

void LoginFrame::setRightBottomWidget(QWidget* w)
{
    if (m_rightBottomWidget)
    {
        ui->right_bottom->layout()->removeWidget(m_rightBottomWidget);
    }
    m_rightBottomWidget = w;
    ui->right_bottom->layout()->addWidget(m_rightBottomWidget);
}

void LoginFrame::addWidgetToCenterBottomWidget(int index, QWidget* w, Qt::Alignment align)
{
    qobject_cast<QBoxLayout*>(ui->center_bottom->layout())->insertWidget(index, w, 0, align);
}

void LoginFrame::addLayoutItemToCenterBottomWidget(int index, QLayoutItem* item)
{
    qobject_cast<QBoxLayout*>(ui->center_bottom->layout())->insertItem(0, item);
}

void LoginFrame::initUI()
{
    // clang-format off
    connect(ui->edit, &PromptEdit::textConfirmed, [this](const QString& text){
        if( m_editMode == EDIT_MODE_USER_NAME )
        {
            authUserInputed(text);
        }
        else
        {
            respond(text);
        }
    });
    connect(ui->btn_reAuth, &QPushButton::clicked, [this]{ 
        startAuthUser(m_specifyUser);
    });
    // clang-format on

    QBoxLayout* centerBottomLayout = qobject_cast<QBoxLayout*>(ui->center_bottom->layout());
    m_switcher = new AuthTypeSwitcher(EXPAND_DIRECTION_RIGHT, 18, this);
    m_switcher->setFixedSize(36, 36);
    centerBottomLayout->insertWidget(2, m_switcher, 0, Qt::AlignHCenter);

    auto spacer = new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Fixed);
    centerBottomLayout->insertSpacerItem(3, spacer);

    m_switcher->setVisible(false);
    connect(m_switcher, &AuthTypeSwitcher::authTypeChanged, [this](KADAuthType authType)
            { this->m_authController->switchAuthType(authType); });

    switchControlPage(CONTROL_PAGE_PROMPT_EDIT);
    startUpdateTimeTimer();
}

void LoginFrame::respond(const QString& response)
{
    RETURN_IF_FALSE(m_authController);
    m_authController->respond(response);
}

int LoginFrame::appendControlPage(QWidget* controlWidget)
{
    return ui->stackedWidget->addWidget(controlWidget);
}

void LoginFrame::switchControlPage(int pageIdx)
{
    RETURN_IF_TRUE(pageIdx < 0 || pageIdx >= ui->stackedWidget->count());
    ui->stackedWidget->setCurrentIndex(pageIdx);
}

void LoginFrame::startUpdateTimeTimer()
{
    QMetaObject::invokeMethod(this, "updateTimeLabel", Qt::AutoConnection);
    QTime curTime = QTime::currentTime();
    int nextUpdateSecond = 60 - curTime.second();
    QTimer::singleShot(nextUpdateSecond * 1000, this, SLOT(startUpdateTimeTimer()));
}

QString LoginFrame::getCurrentDateTime()
{
    QDateTime dateTime = QDateTime::currentDateTime();
    // tr : MM月MM日 dddd HH:mm
    QString res = dateTime.toString(tr("dddd MMM HH:mm"));
    return res;
}

void LoginFrame::updateTimeLabel()
{
    ui->timedate->setText(getCurrentDateTime());
}

void LoginFrame::capsLockStatusChanged(bool on)
{
}

void LoginFrame::setEditFocus(int delayMs)
{
    if (delayMs)
    {
        QTimer::singleShot(delayMs, ui->edit, SLOT(setFocus()));
    }
    else
    {
        ui->edit->setFocus();
    }
}

void LoginFrame::onShowMessage(const QString& text, MessageType type)
{
    setTips(type, text);
}

void LoginFrame::onShowPrmpt(const QString& text, PromptType type)
{
    ui->edit->reset();
    ui->edit->setPlaceHolderText(text);
    m_editMode = EDIT_MODE_PROMPT_RESPOSE;
    ui->edit->setEchoMode(type == PromptTypeSecret ? QLineEdit::Password : QLineEdit::Normal);
    /// FIXME:需要延时设置输入焦点到输入框，不然又会被置回UserItem
    m_prompted = true;
    setEditFocus(200);
}

void LoginFrame::onAuthComplete(bool authRes)
{
    KLOG_DEBUG() << "auth complete" << authRes;
    if (authRes)
    {
        authenticateComplete(authRes, m_authController->authenticationUser());
    }
    else
    {
        if (m_prompted)
        {
            startAuthUser(m_authController->authenticationUser());
        }
        else
        {
            // 未存在prompt消息,应切换至显示重新认真按钮,点击重新认证按钮再开始认证
            switchControlPage(CONTROL_PAGE_REAUTH);
            m_switcher->setVisible(false);
        }
    }
}

void LoginFrame::onNotifyAuthMode(KADAuthMode mode)
{
    m_switcher->setVisible(mode == KAD_AUTH_MODE_OR);
}

void LoginFrame::onSupportedAuthTypeChanged(QList<KADAuthType> supportedTypes)
{
    m_switcher->setAuthTypes(supportedTypes);
}

void LoginFrame::onAuthTypeChanged(KADAuthType type)
{
    if (m_switcher->getCurrentAuthType() != type)
    {
        m_switcher->setCurrentAuthType(type);
    }

    ui->tips->clear();

    if (type == KAD_AUTH_TYPE_FINGERPRINT || type == KAD_AUTH_TYPE_FINGERVEIN)
    {
        switchControlPage(CONTROL_PAGE_EMPTY);
    }
    else
    {
        switchControlPage(CONTROL_PAGE_PROMPT_EDIT);
    }
}

GUARD_END_NAMESPACE