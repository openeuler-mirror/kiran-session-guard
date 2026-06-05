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

#include "login-frame.h"
#include "auth-controller.h"
#include "auth-type-switcher.h"
#include "auxiliary.h"
#include "face-daemon-signal-listener.h"
#include "face-preview-widget.h"
#include "ui_login-frame.h"
#include "user-manager.h"

#include <qt5-log-i.h>
#include <QBoxLayout>
#include <QDebug>
#include <QDateTime>
#include <QFile>
#include <QTime>
#include <QTimer>
#include <QToolButton>
#include <QDBusConnection>
#include <QJsonDocument>
#include <QJsonObject>

namespace Kiran
{
namespace SessionGuard
{
LoginFrame::LoginFrame(QWidget* parent)
    : QWidget(parent),
      ui(new Ui::LoginFrame),
      m_authController(new AuthController(this))
{
    ui->setupUi(this);
    m_faceDaemonSignals = new FaceDaemonSignalListener(this);
    connect(m_faceDaemonSignals, &FaceDaemonSignalListener::leaveDetected, this, &LoginFrame::onFaceLeaveDetected);
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

    QString displayName = userName;
    if( shouldShowFullName() )
    {
        QString fullName = UserManager::getUserRealName(userName);
        if( !fullName.isEmpty() )
        {
            KLOG_DEBUG() << userName << "show full name:" << fullName;
            displayName = fullName;
        }
        else
        {
            KLOG_DEBUG() << userName << "full name empty";
        }
    }
    ui->userName->setText(displayName);
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
    KLOG_INFO() << "LoginFrame: setLeftTopWidget"
                << "widget=" << (m_leftTopWidget ? m_leftTopWidget->metaObject()->className() : "null");
    updateFacePreviewVisibility();
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
        KLOG_INFO() << "LoginFrame: textConfirmed"
                    << "editMode=" << (int)m_editMode
                    << "prompted=" << m_prompted
                    << "inAuth=" << (m_authController ? m_authController->inAuthentication() : false)
                    << "len=" << text.size()
                    << "epochMs=" << QDateTime::currentMSecsSinceEpoch();
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

    // 连接至AccountServce, 处理用户属性变更信号,用于更新正在登录用户头像
    auto connected = QDBusConnection::systemBus().connect("", "", "org.freedesktop.Accounts.User",
                                                          "Changed", this,
                                                          SLOT(onAuthUserPropertyChanged()));
    if (!connected)
    {
        KLOG_WARNING() << "login frame: can not connect to user property changed!";
    }
    // clang-format on

    QBoxLayout* centerBottomLayout = qobject_cast<QBoxLayout*>(ui->center_bottom->layout());
    m_switcher = new AuthTypeSwitcher(EXPAND_DIRECTION_RIGHT, 18, this);
    m_switcher->setFixedSize(36, 36);
    centerBottomLayout->insertWidget(2, m_switcher, 0, Qt::AlignHCenter);

    auto spacer = new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Fixed);
    centerBottomLayout->insertSpacerItem(3, spacer);

    m_switcher->setVisible(false);
    connect(m_switcher, &AuthTypeSwitcher::authTypeChanged, [this](KADAuthType authType) {
        KLOG_INFO() << "LoginFrame: switcher authTypeChanged"
                    << "authType=" << (int)authType
                    << "epochMs=" << QDateTime::currentMSecsSinceEpoch();
        // UI 侧先行更新控件页与输入框状态，避免等待后端通知导致“切换后无输入框/不可输入”。
        updateControlPageForAuthType(authType);

        // UI 侧立即更新预览显隐，避免等待认证服务通知导致残留。
        m_lastAuthType = authType;
        updateFacePreviewVisibility();
        this->m_authController->switchAuthType(authType);
    });

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

// 切换控制页面(界面中间认证流程控件，包含输入框，重新认证按钮，空页面等)
void LoginFrame::switchControlPage(int pageIdx)
{
    RETURN_IF_TRUE(pageIdx < 0 || pageIdx >= ui->stackedWidget->count());
    ui->stackedWidget->setCurrentIndex(pageIdx);

    // 切换控件时，将焦点给需要焦点的控件，便于用户操作
    auto widget = ui->stackedWidget->widget(pageIdx);
    auto edit = widget->findChild<QLineEdit*>();
    auto button = widget->findChild<QPushButton*>();
    if( edit )
    {
        edit->setFocus();
    }
    else if (button)
    {
        button->setFocus();
    }
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
    QString res = dateTime.toString(tr("ddd MMM dd HH:mm"));
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
    KLOG_INFO() << "LoginFrame: onShowPrompt"
                << "type=" << (int)type
                << "promptedBefore=" << m_prompted
                << "inAuth=" << (m_authController ? m_authController->inAuthentication() : false)
                << "epochMs=" << QDateTime::currentMSecsSinceEpoch();
    ui->edit->reset();
    ui->edit->setPlaceHolderText(text);
    m_editMode = EDIT_MODE_PROMPT_RESPOSE;
    // 授权码输入框需明文便于核对
    const bool fromSwitcher = m_switcher && isVirtualCodeAuthType((KADAuthType)m_switcher->getCurrentAuthType());
    const bool virtualAuthCode = isVirtualCodeAuthType(m_lastAuthType) || fromSwitcher;
    const bool useSecretEcho = (type == PromptTypeSecret) && !virtualAuthCode;
    ui->edit->setEchoMode(useSecretEcho ? QLineEdit::Password : QLineEdit::Normal);
    m_prompted = true;
    /// NOTE:需要延时设置输入焦点到输入框，不然又会被置回UserItem
    setEditFocus(200);
}

void LoginFrame::onAuthComplete(bool authRes)
{
    KLOG_DEBUG() << "auth complete" << authRes;
    authenticateComplete(authRes, m_authController->authenticationUser());

    if (!authRes)
    {
#if 0
        if (m_prompted)
        {
            startAuthUser(m_authController->authenticationUser());
        }
        else
#endif
        {
            // 未存在prompt消息,应切换至显示重新认真按钮,点击重新认证按钮再开始认证
            switchControlPage(CONTROL_PAGE_REAUTH);
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

//  检查当前认证的用户头像变更
void LoginFrame::onAuthUserPropertyChanged()
{
    if (m_specifyUser.isEmpty())
    {
        return;
    }

    QString imagePath = UserManager::getUserIcon(m_specifyUser);
    QString currentImagePath = ui->avatar->getCurrentImage();
    if (imagePath == currentImagePath)
    {
        return;
    }

    ui->avatar->setImage(imagePath);
    return;
}

void LoginFrame::onAuthTypeChanged(KADAuthType type)
{
    KLOG_INFO() << "LoginFrame: onAuthTypeChanged authType=" << (int)type;
    m_lastAuthType = type;
    if (m_switcher->getCurrentAuthType() != type)
    {
        m_switcher->setCurrentAuthType(type);
    }

    updateControlPageForAuthType(type);
    updateFacePreviewVisibility();
}

bool LoginFrame::isEmptyControlAuthType(KADAuthType type) const
{
    static const QSet<int> emptyControlAuthType = {
        KAD_AUTH_TYPE_FINGERPRINT,
        KAD_AUTH_TYPE_FINGERVEIN,
        KAD_AUTH_TYPE_IRIS,
        KAD_AUTH_TYPE_FACE,
        KAD_AUTH_TYPE_VIRTUAL_FACE};
    return emptyControlAuthType.contains(type);
}

void LoginFrame::updateControlPageForAuthType(KADAuthType authType)
{
    if (isEmptyControlAuthType(authType))
    {
        switchControlPage(CONTROL_PAGE_EMPTY);
        ui->tips->clear();
        return;
    }

    switchControlPage(CONTROL_PAGE_PROMPT_EDIT);
    ui->edit->reset();
    ui->edit->setEnabled(true);
    ui->edit->setEchoMode(authType == KAD_AUTH_TYPE_PASSWORD ? QLineEdit::Password : QLineEdit::Normal);
    setEditFocus(0);
    m_prompted = false;
    m_editMode = EDIT_MODE_PROMPT_RESPOSE;
    ui->tips->clear();
}

bool LoginFrame::isVirtualCodeAuthType(KADAuthType type) const
{
    return type == KAD_AUTH_TYPE_VIRTUAL_CODE || type == KAD_AUTH_TYPE_VIRTUAL_CODE_NO_CAMERA;
}

bool LoginFrame::isFaceAuthType(KADAuthType type) const
{
    return type == KAD_AUTH_TYPE_VIRTUAL_FACE || type == KAD_AUTH_TYPE_VIRTUAL_CODE;
}

void LoginFrame::updateFacePreviewVisibility()
{
    auto* facePreview = qobject_cast<FacePreviewWidget*>(m_leftTopWidget);
    if (!facePreview)
    {
        m_facePreviewSuppressedByLeave = false;
        if (m_faceDaemonSignals)
        {
            m_faceDaemonSignals->disconnectLeaveDetected();
        }
        return;
    }

    // 仅对虚拟人脸/虚拟授权码显示；同时要求 D-Bus 服务存在。
    const bool isVirtual = isFaceAuthType(m_lastAuthType);
    const bool daemonOk = FacePreviewWidget::isFaceDaemonAvailable();
    const bool shouldShow = isVirtual && daemonOk;
    KLOG_INFO() << "LoginFrame: face preview visibility decision"
                << "authType=" << (int)m_lastAuthType
                << "isVirtual=" << isVirtual
                << "faceDaemonAvailable=" << daemonOk
                << "visible=" << shouldShow;
    facePreview->setVisible(shouldShow);

    if (!m_faceDaemonSignals)
    {
        return;
    }

    if (shouldShow)
    {
        m_faceDaemonSignals->connectLeaveDetected();
    }
    else
    {
        m_faceDaemonSignals->disconnectLeaveDetected();
    }
}

void LoginFrame::onFaceLeaveDetected(QString json)
{
    const QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    const QJsonObject obj = doc.object();
    const int code = obj.value(QStringLiteral("code")).toInt(-1);
    const QString businessId = obj.value(QStringLiteral("business_id")).toString();
    const int personId = obj.value(QStringLiteral("person_id")).toInt(-1);
    const int leaveTime = obj.value(QStringLiteral("leave_time")).toInt(-1);
    const int presenceTime = obj.value(QStringLiteral("presence_time")).toInt(-1);
    const int faceCount = obj.value(QStringLiteral("face_count")).toInt(-1);

    KLOG_INFO() << "LoginFrame: LeaveDetected received"
                << "code=" << code
                << "business_id=" << businessId
                << "person_id=" << personId
                << "leave_time=" << leaveTime
                << "presence_time=" << presenceTime
                << "face_count=" << faceCount
                << "json=" << json;

    // 容错：仅当服务端明确成功，且“离开时人脸数量为 0”时，才触发隐藏。
    if (code != 0)
    {
        return;
    }
    if (faceCount != 0)
    {
        return;
    }

    auto* facePreview = qobject_cast<FacePreviewWidget*>(m_leftTopWidget);
    if (!facePreview)
    {
        return;
    }

    m_facePreviewSuppressedByLeave = true;
    facePreview->setVisible(false);
    if (m_faceDaemonSignals)
    {
        m_faceDaemonSignals->disconnectLeaveDetected();
    }
}

}  // namespace SessionGuard
}  // namespace Kiran
