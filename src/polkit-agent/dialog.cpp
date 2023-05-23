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
#include "dialog.h"
#include "auth-controller.h"
#include "auth-polkit.h"
#include "auth-type-switcher.h"
#include "auxiliary.h"
#include "ui_dialog.h"

#include <kiran-passwd-edit.h>
#include <qt5-log-i.h>
#include <style-property.h>
#include <QComboBox>
#include <QDebug>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTimer>

#define MAX_ERROR_COUNT 3

namespace Kiran
{
namespace SessionGuard
{
namespace PolkitAgent
{
void AuthInfo::dump()
{
    qDebug() << "action:   " << actionID;
    qDebug() << "messsage: " << message;
    qDebug() << "icon:     " << iconName;
    QStringList detailKeys = details.keys();
    for (const QString& key : detailKeys)
    {
        QString value = details.lookup(key);
        qDebug() << "detail key:" << key << "value:" << value;
    }
    qDebug() << "cookie:    " << cookie;
    for (const PolkitQt1::Identity identity : identities)
    {
        qDebug() << "identity:  " << identity.toString();
    }
};

Dialog::Dialog(QWidget* parent)
    : KiranTitlebarWindow(parent),
      ui(new Ui::Dialog)
{
ui->setupUi(getWindowContentWidget());
    resize(408, 290);
    initUI();
}

Dialog::~Dialog()
{
    m_authController->cancelAuthentication();
}

bool Dialog::setAuthInfo(const AuthInfo& authInfo)
{
    ui->label_msg->setText(authInfo.message);

    m_authController = new AuthController(this);
    auto auth = new AuthPolkit(authInfo.cookie);
    RETURN_VAL_IF_FALSE(m_authController->init(auth), false);

    connect(m_authController, &AuthController::notifyAuthMode, this, &Dialog::onNotifyAuthMode);
    connect(m_authController, &AuthController::supportedAuthTypeChanged, this, &Dialog::onSupportedAuthTypeChanged);
    connect(m_authController, &AuthController::authTypeChanged, this, &Dialog::onNotifyAuthTypeChanged);
    connect(m_authController, &AuthController::authenticationComplete, this, &Dialog::onAuthComplete);
    connect(m_authController, &AuthController::showMessage, this, &Dialog::onAuthShowMessage);
    connect(m_authController, &AuthController::showPrompt, this, &Dialog::onAuthShowPrompt);

    PolkitQt1::Identity::List identitieList = authInfo.identities;
    for (const PolkitQt1::Identity& identity : identitieList)
    {
        QString tmp = identity.toString();
        QString name = tmp.remove("unix-user:");
        if (name == qgetenv("USER"))
            ui->combobox_user->insertItem(0, name, identity.toString());
        else
            ui->combobox_user->addItem(name, identity.toString());
    }

    ui->combobox_user->setCurrentIndex(0);
    return true;
}

bool Dialog::init(const AuthInfo& info)
{
    QSignalBlocker signalBlocker(ui->combobox_user);
    RETURN_VAL_IF_FALSE(setAuthInfo(info), false);
    onCurrentUserChanged(0);
    return true;
}

void Dialog::initUI()
{
    setTitle(tr("authorization"));

    setAttribute(Qt::WA_DeleteOnClose, false);
    setWindowFlags(Qt::Dialog | windowFlags());
    setWindowModality(Qt::WindowModal);
    setResizeable(false);
    setButtonHints(KiranTitlebarWindow::TitlebarCloseButtonHint);
    setTitlebarColorBlockEnable(true);
    layout()->setSizeConstraint(QLayout::SetFixedSize);
    ui->label_tips->setWordWrap(true);
    Kiran::StylePropertyHelper::setButtonType(ui->btn_ok, Kiran::BUTTON_Default);

    m_switcher = new AuthTypeSwitcher(EXPAND_DIRECTION_BOTTOM, 4, this);
    m_switcher->setAdjustColorToTheme(true);
    m_switcher->setFixedSize(QSize(42, 36));
    m_switcher->setVisible(false);
    ui->layout_edit->addWidget(m_switcher);
    connect(m_switcher, &AuthTypeSwitcher::authTypeChanged, this, &Dialog::onCurrentAuthTypeChanged);

    connect(ui->btn_cancel, &QPushButton::clicked, this, &Dialog::onCancelClicked);
    connect(ui->btn_ok, &QPushButton::clicked, this, &Dialog::onOkClicked);
    connect(ui->edit->lineEdit(), &QLineEdit::returnPressed, this, &Dialog::onOkClicked);
    connect(ui->combobox_user, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Dialog::onCurrentUserChanged);
}

void Dialog::startAuth(const QString& userName)
{
    m_havePrompt = false;
    m_triesCount++;

    if (m_authController->inAuthentication())
    {
        m_authController->cancelAuthentication();
    }

    ui->btn_ok->setEnabled(false);
    ui->edit->setEnabled(false);
    ui->edit->lineEdit()->setPlaceholderText("");
    ui->edit->lineEdit()->clear();
    ui->label_tips->setText("");
    m_switcher->setVisible(false);

    m_authController->authenticate(userName);
}

void Dialog::closeEvent(QCloseEvent* event)
{
    emit cancelled();
    KiranTitlebarWindow::closeEvent(event);
}

void Dialog::onCancelClicked()
{
    emit cancelled();
}

void Dialog::onOkClicked()
{
    m_authController->respond(ui->edit->lineEdit()->text());
    ui->edit->setEnabled(false);
    ui->btn_ok->setEnabled(false);
}

void Dialog::onCurrentUserChanged(int idx)
{
    startAuth(ui->combobox_user->currentText());
}

void Dialog::onCurrentAuthTypeChanged(KADAuthType authType)
{
    QMap<KADAuthType, QString> authTypeDesc = {
        {KAD_AUTH_TYPE_FINGERPRINT, tr("fingerprint auth")},
        {KAD_AUTH_TYPE_FACE, tr("face auth")},
        {KAD_AUTH_TYPE_FINGERVEIN, tr("fingervein auth")}};

    ui->label_tips->setText("");
    m_authController->switchAuthType(authType);
    ui->edit->setEnabled(false);
    ui->btn_ok->setEnabled(false);
    ui->edit->lineEdit()->clear();
    ui->edit->lineEdit()->setPlaceholderText("");

    if (authTypeDesc.contains(authType))
    {
        ui->edit->lineEdit()->setPlaceholderText(authTypeDesc[authType]);
    }
}

void Dialog::onNotifyAuthMode(KADAuthMode mode)
{
    m_switcher->setVisible(mode == KAD_AUTH_MODE_OR);
}

void Dialog::onSupportedAuthTypeChanged(QList<KADAuthType> authTypes)
{
    m_switcher->setAuthTypes(authTypes);
}

void Dialog::onNotifyAuthTypeChanged(KADAuthType authType)
{
    m_switcher->setCurrentAuthType(authType);
}

void Dialog::onAuthComplete(bool success)
{
    if (m_authController->isAuthenticated())
    {
        emit completed(true);
        this->close();
        return;
    }

    if (m_havePrompt && (m_triesCount < MAX_ERROR_COUNT))
    {
        onAuthShowMessage("Authentication error, please authenticate again.", MessageTypeInfo);
        startAuth(ui->combobox_user->currentText());
    }
    else
    {
        if (m_triesCount == MAX_ERROR_COUNT)
        {
            onAuthShowMessage("Authentication error", MessageTypeError);
        }
        ui->btn_ok->setEnabled(false);
        ui->edit->lineEdit()->clear();
        ui->edit->setEnabled(false);
    }
}

void Dialog::onAuthShowPrompt(const QString& text, PromptType promptType)
{
    m_havePrompt = true;
    ui->btn_ok->setEnabled(true);
    ui->edit->lineEdit()->setPlaceholderText(text);
    ui->edit->setEnabled(true);
    ui->edit->lineEdit()->setFocus();
}

void Dialog::onAuthShowMessage(const QString& text, MessageType msgType)
{
    QString tips = text;
    if (msgType == MessageTypeError)
    {
        tips = QString("<font style = 'color:red;'>%1</font>").arg(text);
    }

    ui->label_tips->setText(tips);
}

}  // namespace PolkitAgent
}  // namespace SessionGuard
}  // namespace Kiran