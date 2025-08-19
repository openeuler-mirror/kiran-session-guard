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

#include "setting-window.h"
#include "../common/greeter-define.h"
#include "../common/prefs.h"
#include "hover-tips.h"
#include "user-manager.h"

#include <style-property.h>
#include <QApplication>
#include <QComboBox>
#include <QDir>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStackedWidget>
#include <QStandardPaths>

#include <kiran-color-block.h>
#include <kiran-image-selector.h>
#include <kiran-message-box.h>
#include <kiran-sidebar-widget.h>
#include <kiran-switch-button.h>
#include <kiran-system-daemon/greeter-i.h>
#include <qt5-log-i.h>

#define BACKGROUND_SAVE_LOCATION "/usr/share/lightdm-kiran-greeter/background"

#define ITEM_GENERAL_SETTINGS    QT_TRANSLATE_NOOP("Kiran::SessionGuard::Greeter::SettingWindow","General")
#define ITEM_AUTO_LOGIN_SETTINGS QT_TRANSLATE_NOOP("Kiran::SessionGuard::Greeter::SettingWindow","Autologin")

enum GreeterSettingsPageEnum
{
    GreeterSettings_Appearance,
    GreeterSettings_Autologin
};

namespace Kiran
{
namespace SessionGuard
{
namespace Greeter
{
SettingWindow::SettingWindow(QWidget *parent) : QWidget(parent)
{
    Prefs::globalInit();
    m_prefs = Prefs::getInstance();

    initUI();
}

SettingWindow::~SettingWindow()
{
    Prefs::globalDeinit();
}

void SettingWindow::initUI()
{
    /* 内容区域主布局 */
    auto mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(4);

    /* 左侧侧边栏 */
    auto sideWidget = new KiranColorBlock(this);
    sideWidget->setObjectName("widget_side");
    sideWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    sideWidget->setFixedWidth(272);
    mainLayout->addWidget(sideWidget);

    auto layoutSideWidget = new QHBoxLayout(sideWidget);
    layoutSideWidget->setSpacing(0);
    layoutSideWidget->setMargin(0);

    m_sidebarWidget = new KiranSidebarWidget(this);
    m_sidebarWidget->setInvertIconPixelsEnable(true);
    m_sidebarWidget->setFrameShape(QFrame::NoFrame);
    m_sidebarWidget->setObjectName("SidebarTabListWidget");
    m_sidebarWidget->setIconSize(QSize(16, 16));
    m_sidebarWidget->viewport()->setAutoFillBackground(false);
    layoutSideWidget->addWidget(m_sidebarWidget);

    QListWidgetItem *item;
    item = new QListWidgetItem(tr(ITEM_GENERAL_SETTINGS), m_sidebarWidget);
    item->setIcon(QIcon(":/kcp-greeter/images/appearance_setting.png"));
    m_sidebarWidget->addItem(item);

    item = new QListWidgetItem(tr(ITEM_AUTO_LOGIN_SETTINGS), m_sidebarWidget);
    item->setIcon(QIcon(":/kcp-greeter/images/user_login_setting.png"));
    m_sidebarWidget->addItem(item);

    /* 堆叠区域控件 */
    KiranColorBlock *stackedColorBlock = new KiranColorBlock(this);
    mainLayout->addWidget(stackedColorBlock);
    auto colorBlockLayout = new QHBoxLayout(stackedColorBlock);
    colorBlockLayout->setSpacing(0);
    colorBlockLayout->setMargin(0);

    m_stackedWidget = new QStackedWidget(this);
    m_stackedWidget->setObjectName("GreeterSettingsStacked");
    colorBlockLayout->addWidget(m_stackedWidget);

    auto widgetGeneralSettings = initPageGeneralSettings();
    m_stackedWidget->addWidget(widgetGeneralSettings);

    auto widgetAutoLogin = initPageAutoLogin();
    m_stackedWidget->addWidget(widgetAutoLogin);

    /* 悬浮提示框 */
    m_hoverTips = new HoverTips(m_stackedWidget);
    m_hoverTips->setTimeout(2000);

    /* 处理相关控件信号和初始化 */
    connect(m_sidebarWidget, &KiranSidebarWidget::itemSelectionChanged, [this]
            {
                QList<QListWidgetItem *> selecteds = m_sidebarWidget->selectedItems();
                if (selecteds.size() != 1)
                {
                    KLOG_FATAL("tabList: selecteds size != 1");
                }
                int page = m_sidebarWidget->row(selecteds.at(0));
                m_stackedWidget->setCurrentIndex(page);

                /* 重置页面 */
                if (page == GreeterSettings_Appearance)
                {
                    resetGeneralSettings();
                }
                else if (page == GreeterSettings_Autologin)
                {
                    resetAutoLoginSettings();
                }

                /* 隐藏悬浮提示 */
                m_hoverTips->hide();
            });
    m_sidebarWidget->setCurrentRow(0);
    resetGeneralSettings();
    resetAutoLoginSettings();
}

QWidget *SettingWindow::initPageAutoLogin()
{
    auto pageAutoLogin = new QWidget(this);

    auto mainLayout = new QVBoxLayout(pageAutoLogin);
    mainLayout->setContentsMargins(16, 16, 16, 0);
    mainLayout->setSpacing(0);

    /* 自动登录用户总开关 */
    auto *autologinSwitchLayout = new QHBoxLayout();
    autologinSwitchLayout->setSpacing(0);
    autologinSwitchLayout->setMargin(0);

    auto labelAutoLogonUser = new QLabel(tr("auto login user(take effect after restart)"), pageAutoLogin);
    labelAutoLogonUser->setObjectName("label_autoLogonUser");
    labelAutoLogonUser->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    labelAutoLogonUser->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    autologinSwitchLayout->addWidget(labelAutoLogonUser);

    auto spaceItem = new QSpacerItem(20, 8, QSizePolicy::Expanding, QSizePolicy::Minimum);
    autologinSwitchLayout->addItem(spaceItem);

    m_autologinSwitch = new KiranSwitchButton(pageAutoLogin);
    connect(m_autologinSwitch, &QAbstractButton::toggled, [this](bool checked)
            { m_autologinComboWidget->setVisible(checked); });
    autologinSwitchLayout->addWidget(m_autologinSwitch);

    mainLayout->addItem(autologinSwitchLayout);

    /* 自动登录下拉框 */
    m_autologinComboWidget = new QWidget(pageAutoLogin);
    auto autologinHBoxLayout = new QHBoxLayout(m_autologinComboWidget);
    autologinHBoxLayout->setSpacing(0);
    autologinHBoxLayout->setContentsMargins(0, 10, 0, 0);

    m_comboAutoLoginUser = new QComboBox(this);
    m_comboAutoLoginUser->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_comboAutoLoginUser->setFixedHeight(40);
    m_comboAutoLoginUser->setIconSize(QSize(24, 24));
    initUserComboBox(m_comboAutoLoginUser);
    autologinHBoxLayout->addWidget(m_comboAutoLoginUser, 0);
    mainLayout->addWidget(m_autologinComboWidget);

    /* 自动登录延时 */
    auto spaceItem_2 = new QSpacerItem(20, 16, QSizePolicy::Minimum, QSizePolicy::Fixed);
    mainLayout->addItem(spaceItem_2);

    auto labelAutoLogonDelay = new QLabel(tr("auto login delay(seconds)(take effect after restart)"), this);
    labelAutoLogonDelay->setObjectName("label_autoLogonDealy");
    labelAutoLogonDelay->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    mainLayout->addWidget(labelAutoLogonDelay);

    auto spaceItem_3 = new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Fixed);
    mainLayout->addItem(spaceItem_3);

    m_editAutoLoginDelay = new QLineEdit(this);
    m_editAutoLoginDelay->setFixedHeight(40);
    m_editAutoLoginDelay->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QValidator *validator = new QIntValidator(0, INT_MAX, this);
    m_editAutoLoginDelay->setValidator(validator);
    mainLayout->addWidget(m_editAutoLoginDelay, 0);

    /* 占位 */
    auto mainLayoutSpacerItem = new QSpacerItem(10, 20,
                                                QSizePolicy::Minimum,
                                                QSizePolicy::Expanding);
    mainLayout->addItem(mainLayoutSpacerItem);

    /* 保存-取消 */
    auto layoutButtonBox = new QHBoxLayout();
    layoutButtonBox->setSpacing(0);
    layoutButtonBox->setObjectName("layout_autoLoginButtonBox");
    layoutButtonBox->setContentsMargins(0, 10, 0, 40);
    mainLayout->addLayout(layoutButtonBox);

    auto buttonBoxSpacerItem1 = new QSpacerItem(10, 20,
                                                QSizePolicy::Expanding,
                                                QSizePolicy::Minimum);
    layoutButtonBox->addItem(buttonBoxSpacerItem1);

    auto btn_save = new QPushButton(this);
    btn_save->setObjectName("btn_saveAutoLogin");
    btn_save->setFixedSize(110, 40);
    btn_save->setText(tr("Save"));
    Kiran::StylePropertyHelper::setButtonType(btn_save, Kiran::BUTTON_Default);
    layoutButtonBox->addWidget(btn_save);
    connect(btn_save, &QPushButton::clicked, [this]()
            { saveAutoLoginSettings(); });

    auto buttonBoxSpacerItem2 = new QSpacerItem(40, 20,
                                                QSizePolicy::Fixed,
                                                QSizePolicy::Minimum);
    layoutButtonBox->addItem(buttonBoxSpacerItem2);

    auto btn_reset = new QPushButton(this);
    btn_reset->setObjectName("btn_resetAutoLogin");
    btn_reset->setFixedSize(110, 40);
    btn_reset->setText(tr("Reset"));
    layoutButtonBox->addWidget(btn_reset);

    connect(btn_reset, &QPushButton::clicked, [this]()
            { resetAutoLoginSettings(); });

    auto buttonBoxSpacerItem3 = new QSpacerItem(10, 20,
                                                QSizePolicy::Expanding,
                                                QSizePolicy::Minimum);
    layoutButtonBox->addItem(buttonBoxSpacerItem3);
    return pageAutoLogin;
}

QWidget *SettingWindow::initPageGeneralSettings()
{
    auto pageGeneralSettings = new QWidget(this);
    auto mainLayout = new QVBoxLayout(pageGeneralSettings);
    mainLayout->setContentsMargins(16, 16, 16, 0);
    mainLayout->setSpacing(0);

    /* 外观设置 */
    /* 图片选择控件 */
    auto titleLabel = new QLabel(this);
    titleLabel->setText(tr("login background setting"));
    mainLayout->addWidget(titleLabel);
    mainLayout->addSpacerItem(new QSpacerItem(10,10,QSizePolicy::Minimum,QSizePolicy::Fixed));

    m_imageSelector = new KiranImageSelector(this);
    m_imageSelector->setFixedHeight(148);
    mainLayout->addWidget(m_imageSelector);
    QDir dir("/usr/share/backgrounds/kiran/");
    QFileInfoList fileInfoList = dir.entryInfoList(QStringList() << "*.jpg"
                                                                 << "*.png",
                                                   QDir::Files);
    for (const QFileInfo &fileInfo : fileInfoList)
    {
        m_imageSelector->addImage(fileInfo.absoluteFilePath());
    }

    /* 缩放模式 */
    auto labelScaleMode = new QLabel(tr("Scale Mode"), this);
    labelScaleMode->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    labelScaleMode->setObjectName("label_scaleMode");
    labelScaleMode->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    labelScaleMode->setStyleSheet("QLabel{margin-top:16px;margin-bottom:10px;}");
    mainLayout->addWidget(labelScaleMode);

    m_comboScaleMode = new QComboBox(this);
    m_comboScaleMode->setObjectName("combo_scaleMode");
    m_comboScaleMode->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_comboScaleMode->setFixedHeight(40);
    m_comboScaleMode->addItem(tr("auto"), GreeterScalingMode::GREETER_SCALING_MODE_AUTO);
    m_comboScaleMode->addItem(tr("manual"), GreeterScalingMode::GREETER_SCALING_MODE_MANUAL);
    m_comboScaleMode->addItem(tr("disable"), GreeterScalingMode::GREETER_SCALING_MODE_DISABLE);
    mainLayout->addWidget(m_comboScaleMode, 0);

    connect(m_comboScaleMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingWindow::onScaleModeChanged);

    /* 缩放倍率 */
    auto labelScaleFactor = new QLabel(tr("Scale Factor"), this);
    labelScaleFactor->setObjectName("label_ScaleFactor");
    labelScaleFactor->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    labelScaleFactor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    labelScaleFactor->setStyleSheet("QLabel{margin-top:16px;margin-bottom:10px;}");
    mainLayout->addWidget(labelScaleFactor, 0, Qt::AlignLeft);

    m_comboScaleFactor = new QComboBox(this);
    m_comboScaleFactor->setObjectName("combo_scaleFactor");
    m_comboScaleFactor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_comboScaleFactor->setFixedHeight(40);
    m_comboScaleFactor->addItem("100%", 1);
    m_comboScaleFactor->addItem("200%", 2);
    mainLayout->addWidget(m_comboScaleFactor, 0);

    /* 手动登录 */
    auto widgetManualLogin = new QWidget(this);
    widgetManualLogin->setObjectName("widget_manualLogin");
    widgetManualLogin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    mainLayout->addWidget(widgetManualLogin);

    auto layoutManualLogin = new QHBoxLayout(widgetManualLogin);
    layoutManualLogin->setObjectName("layout_manualLogin");
    layoutManualLogin->setContentsMargins(0, 8, 0, 0);

    auto labelManualLogin = new QLabel(tr("Enable manual input user login"), this);
    labelManualLogin->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    labelManualLogin->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    layoutManualLogin->addWidget(labelManualLogin);

    auto manualLoginSpacerItem = new QSpacerItem(20, 20,
                                                 QSizePolicy::Expanding,
                                                 QSizePolicy::Preferred);
    layoutManualLogin->addItem(manualLoginSpacerItem);

    m_enableManualSwitch = new KiranSwitchButton(this);
    connect(m_enableManualSwitch, &KiranSwitchButton::toggled, this, &SettingWindow::onLoginOptionsChanged);
    m_enableManualSwitch->setObjectName("btn_enableManualLogin");
    layoutManualLogin->addWidget(m_enableManualSwitch);

    /* 用户列表登录设置 */
    auto widgetUserListLogin = new QWidget(this);
    widgetUserListLogin->setObjectName("widget_showUserList");
    widgetUserListLogin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    mainLayout->addWidget(widgetUserListLogin);

    auto layoutUserList = new QHBoxLayout(widgetUserListLogin);
    layoutUserList->setObjectName("layout_userListLogin");
    layoutUserList->setContentsMargins(0, 8, 0, 0);

    auto labelShowUserList = new QLabel(tr("Show User List"), this);
    labelShowUserList->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    labelShowUserList->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    layoutUserList->addWidget(labelShowUserList);

    auto showUserListSpacerItem = new QSpacerItem(20, 20,
                                                  QSizePolicy::Expanding,
                                                  QSizePolicy::Preferred);
    layoutUserList->addItem(showUserListSpacerItem);

    m_showUserListSwitch = new KiranSwitchButton(this);
    connect(m_showUserListSwitch, &KiranSwitchButton::toggled, this, &SettingWindow::onLoginOptionsChanged);
    m_showUserListSwitch->setObjectName("btn_showUserList");
    m_showUserListSwitch->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    layoutUserList->addWidget(m_showUserListSwitch, 0, Qt::AlignVCenter | Qt::AlignRight);

    /* 占位 */
    auto mainLayoutSpacerItem = new QSpacerItem(10, 20,
                                                QSizePolicy::Minimum,
                                                QSizePolicy::Expanding);
    mainLayout->addItem(mainLayoutSpacerItem);

    /*保存-取消*/
    auto widgetButtonBox = new QWidget(this);
    widgetButtonBox->setObjectName("widget_generalSettingsButton");
    widgetButtonBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    mainLayout->addWidget(widgetButtonBox);

    auto layoutButtonBox = new QHBoxLayout(widgetButtonBox);
    layoutButtonBox->setSpacing(0);
    layoutButtonBox->setObjectName("layout_generalSettingsButtonBox");
    layoutButtonBox->setContentsMargins(0, 10, 0, 40);

    auto buttonBoxSpacerItem1 = new QSpacerItem(10, 20,
                                                QSizePolicy::Expanding,
                                                QSizePolicy::Minimum);
    layoutButtonBox->addItem(buttonBoxSpacerItem1);

    auto btn_save = new QPushButton(this);
    btn_save->setObjectName("btn_saveGeneralSettings");
    btn_save->setFixedSize(110, 40);
    btn_save->setText(tr("Save"));
    Kiran::StylePropertyHelper::setButtonType(btn_save, Kiran::BUTTON_Default);
    layoutButtonBox->addWidget(btn_save);
    connect(btn_save, &QPushButton::clicked, [this]()
            { saveGeneralSettings(); });

    auto buttonBoxSpacerItem2 = new QSpacerItem(40, 20,
                                                QSizePolicy::Fixed,
                                                QSizePolicy::Minimum);
    layoutButtonBox->addItem(buttonBoxSpacerItem2);

    auto btn_reset = new QPushButton(this);
    btn_reset->setObjectName("btn_resetGeneralSettings");
    btn_reset->setFixedSize(110, 40);
    btn_reset->setText(tr("Reset"));
    layoutButtonBox->addWidget(btn_reset);
    connect(btn_reset, &QPushButton::clicked, [this]()
            { resetGeneralSettings(); });

    auto buttonBoxSpacerItem3 = new QSpacerItem(10, 20,
                                                QSizePolicy::Expanding,
                                                QSizePolicy::Minimum);
    layoutButtonBox->addItem(buttonBoxSpacerItem3);
    return pageGeneralSettings;
}

void SettingWindow::initUserComboBox(QComboBox *combo)
{
    struct UserInfo
    {
        QString name;
        QString iconFile;

        void clean()
        {
            name.clear();
            iconFile.clear();
        }
    };

    QStringList users = UserManager::getCachedUsers();
    users.append("root");
    for (auto user : users)
    {
        QString iconFile = UserManager::getUserIcon(user);
        QPixmap pixmap;
        if (iconFile.isEmpty() || !pixmap.load(iconFile))
        {
            iconFile = ":/kcp-greeter/images/user_180.png";
        }
        combo->addItem(QIcon(iconFile), user);
    }
}

void SettingWindow::saveGeneralSettings()
{
    GreeterSettingInfo::GeneralSetting backendInfo = getGeneralSettingInfoFromBackend();

    /* UI修改之前所加载的配置信息 */
    GreeterSettingInfo::GeneralSetting origUIInfo = m_origSettingInfo.appearanceInfo;

    /* 如果后端不是在界面端所做修改,提示用户,是覆盖还是重新加载 */
    if (!(backendInfo == origUIInfo))
    {
        auto clickedRole = KiranMessageBox::message(this,
                                                    tr("Configuration changed"),
                                                    tr("The external configuration file has changed\n"
                                                       "If you choose to save, all external changes will be overwritten\n"
                                                       "Select discard to discard the modification and reload the new configuration"),
                                                    KiranMessageBox::Save | KiranMessageBox::Discard);
        /* 丢弃所有的修改,重新加载 */
        if (clickedRole == KiranMessageBox::Discard)
        {
            resetGeneralSettings();
            return;
        }
    }

    /* 保存 */
    QDBusPendingReply<> reply;
    bool hasError = false;

    reply = m_prefs->SetBackground(m_imageSelector->selectedImage());
    reply.waitForFinished();
    if (reply.isError())
    {
        KLOG_ERROR() << "SetBackgroundFile failed," << reply.error();
        hasError = true;
        goto failed;
    }

    reply = m_prefs->SetHideUserList(!m_showUserListSwitch->isChecked());
    reply.waitForFinished();
    if (reply.isError())
    {
        KLOG_ERROR() << "SetHideUserList failed," << reply.error();
        hasError = true;
    }

    reply = m_prefs->SetAllowManualLogin(m_enableManualSwitch->isChecked());
    reply.waitForFinished();
    if (reply.isError())
    {
        KLOG_ERROR() << "SetAllowManualLogin failed," << reply.error();
        hasError = true;
    }

    reply = m_prefs->SetScaleMode(m_comboScaleMode->currentData().toUInt(),
                                  m_comboScaleFactor->currentData().toUInt());
    reply.waitForFinished();
    if (reply.isError())
    {
        KLOG_ERROR() << "SetScaleMode failed," << reply.error();
        hasError = true;
    }
failed:
    /* 提示保存相关配置失败 */
    if (hasError)
    {
        m_hoverTips->show(HoverTips::HOVE_TIPS_ERR, tr("Save failed, reload"));
    }
    else
    {
        m_hoverTips->show(HoverTips::HOVE_TIPS_SUC, tr("Saved successfully"));
    }

    /* 重新加载配置，更新界面，重新缓存加载的配置文件 */
    resetGeneralSettings();
}

void SettingWindow::saveAutoLoginSettings()
{
    GreeterSettingInfo::AutoLoginSetting backendInfo = getAutologinSettingInfoFromBackend();

    /* UI修改之前所加载的配置信息 */
    GreeterSettingInfo::AutoLoginSetting origUIInfo = m_origSettingInfo.autoLoginInfo;

    /* 如果后端不是在界面端所做修改,提示用户,是覆盖还是重新加载 */
    if (!(backendInfo == origUIInfo))
    {
        auto clickedRole = KiranMessageBox::message(this,
                                                    tr("Configuration changed"),
                                                    tr("The external configuration file has changed\n"
                                                       "If you choose to save, all external changes will be overwritten\n"
                                                       "Select discard to discard the modification and reload the new configuration"),
                                                    KiranMessageBox::Save | KiranMessageBox::Discard);
        /* 丢弃所有的修改,重新加载 */
        if (clickedRole == KiranMessageBox::Discard)
        {
            resetAutoLoginSettings();
            return;
        }
    }

    /* 保存 */
    QDBusPendingReply<> reply;
    QString errMsg;
    bool hasError = false;

    QString autologinUser;
    if (m_autologinSwitch->isChecked())
    {
        autologinUser = m_comboAutoLoginUser->currentText();
    }
    reply = m_prefs->SetAutologinUser(autologinUser);
    reply.waitForFinished();
    if (reply.isError())
    {
        KLOG_ERROR() << "SetAutologinUser failed," << reply.error();
        errMsg = reply.error().message();
        hasError = true;
        goto failed;
    }

    reply = m_prefs->SetAutologinTimeout(m_editAutoLoginDelay->text().toUInt());
    reply.waitForFinished();
    if (reply.isError())
    {
        KLOG_ERROR() << "SetAutologinTimeout" << reply.error();
        errMsg = reply.error().message();
        hasError = true;
    }

failed:
    /* 提示保存是否成功 */
    if (hasError)
    {
        if (errMsg.isEmpty())
        {
            m_hoverTips->show(HoverTips::HOVE_TIPS_ERR, tr("Save failed, reload"));
        }
        else
        {
            QString error = QString(tr("Save failed: %1").arg(errMsg));
            m_hoverTips->show(HoverTips::HOVE_TIPS_ERR, error);
        }
    }
    else
    {
        m_hoverTips->show(HoverTips::HOVE_TIPS_SUC, tr("Saved successfully"));
    }

    /* 重新加载配置，更新界面，重新缓存加载的配置文件 */
    resetAutoLoginSettings();
}

/* 重新加载登录外观设置,更新到界面,并缓存 */
void SettingWindow::resetGeneralSettings()
{
    GreeterSettingInfo::GeneralSetting appearanceSetting = getGeneralSettingInfoFromBackend();

    QString background = appearanceSetting.background;

    /// 背景设置为空使用默认背景图片,如果背景图片是链接的话，读出指向文件位置
    if (background.isEmpty())
    {
        QFileInfo backgroundFileInfo(DEFAULT_BACKGROUND);
        if (backgroundFileInfo.isSymLink())
        {
            background = backgroundFileInfo.symLinkTarget();
        }
        else
        {
            background = DEFAULT_BACKGROUND;
        }
    }

    /// 如果图片选择控件不存在选中的图片的话则添加进入图片选择列表
    if (!m_imageSelector->imageList().contains(background))
    {
        m_imageSelector->addImage(background);
    }

    m_imageSelector->setSelectedImage(background);
    m_showUserListSwitch->setChecked(!appearanceSetting.hideUserList);
    m_enableManualSwitch->setChecked(appearanceSetting.allowManualLogin);
    int idx = m_comboScaleMode->findData(appearanceSetting.scaleMode);
    m_comboScaleMode->setCurrentIndex(idx);
    onScaleModeChanged(idx);

    idx = m_comboScaleFactor->findData(appearanceSetting.scaleFactor);
    m_comboScaleFactor->setCurrentIndex(idx);

    m_origSettingInfo.appearanceInfo = appearanceSetting;
}

/* 重新加载登录自动登录设置,更新到界面,并缓存 */
void SettingWindow::resetAutoLoginSettings()
{
    GreeterSettingInfo::AutoLoginSetting autoLoginSetting = getAutologinSettingInfoFromBackend();
    m_autologinSwitch->setChecked(autoLoginSetting.autoLoginUser.size());

    m_autologinComboWidget->setVisible(autoLoginSetting.autoLoginUser.size());
    if (!autoLoginSetting.autoLoginUser.isEmpty())
    {
        m_comboAutoLoginUser->setCurrentText(autoLoginSetting.autoLoginUser);
    }

    m_editAutoLoginDelay->setText(QString::number(autoLoginSetting.autoLoginTimeout));
    m_origSettingInfo.autoLoginInfo = autoLoginSetting;
}

GreeterSettingInfo::GeneralSetting SettingWindow::getGeneralSettingInfoFromBackend()
{
    GreeterSettingInfo::GeneralSetting appearanceSetting;

    appearanceSetting.background = m_prefs->background();
    appearanceSetting.hideUserList = m_prefs->hide_user_list();
    appearanceSetting.allowManualLogin = m_prefs->allow_manual_login();
    appearanceSetting.scaleMode = m_prefs->scale_mode();

    if (m_comboScaleMode->findData(appearanceSetting.scaleMode) == -1)
    {
        KLOG_WARNING() << "no such scale mode <" << appearanceSetting.scaleMode << ">,using default scale mode:auto";
        appearanceSetting.scaleMode = 0;
    }

    appearanceSetting.scaleFactor = m_prefs->scale_factor();
    if (m_comboScaleFactor->findData(appearanceSetting.scaleFactor) == -1)
    {
        KLOG_WARNING() << "no such scale factor <" << appearanceSetting.scaleFactor << ">,using default scale factor:1";
        appearanceSetting.scaleFactor = 1;
    }

    return appearanceSetting;
}

GreeterSettingInfo::AutoLoginSetting SettingWindow::getAutologinSettingInfoFromBackend()
{
    GreeterSettingInfo::AutoLoginSetting autoLoginSetting;

    autoLoginSetting.autoLoginTimeout = m_prefs->autologin_timeout();
    autoLoginSetting.autoLoginUser = m_prefs->autologin_user();
    if (m_comboAutoLoginUser->findText(autoLoginSetting.autoLoginUser) == -1)
    {
        KLOG_WARNING() << "no such user," << autoLoginSetting.autoLoginUser;
        autoLoginSetting.autoLoginUser = "";
    }

    return autoLoginSetting;
}

QSize SettingWindow::sizeHint() const
{
    return {940, 653};
}

QVector<QPair<QString,QString>> SettingWindow::getSearchKeys()
{
    QVector<QPair<QString,QString>> searchEntries = {
        {tr(ITEM_GENERAL_SETTINGS),tr(ITEM_GENERAL_SETTINGS)},
        {tr(ITEM_AUTO_LOGIN_SETTINGS),tr(ITEM_AUTO_LOGIN_SETTINGS)}
    };

    return searchEntries;
}

void SettingWindow::jumpToSearchKey(const QString& subItem)
{
    auto resList = m_sidebarWidget->findItems(subItem,Qt::MatchFixedString);
    if( resList.isEmpty() )
    {
        KLOG_ERROR() << "greeter plugin can't find SubItem:" << subItem;
        return;
    }
    m_sidebarWidget->setCurrentItem(resList.at(0));
}

void SettingWindow::onLoginOptionsChanged()
{
    auto changedSwitch = qobject_cast<KiranSwitchButton *>(sender());
    if (changedSwitch->isChecked())
    {
        return;
    }

    // 不允许手动登录以及不显示用户列表
    if (!m_enableManualSwitch->isChecked() && !m_showUserListSwitch->isChecked())
    {
        m_hoverTips->show(HoverTips::HOVE_TIPS_WARNING, tr("Please ensure that one of the two options is turned on!"));
        if (changedSwitch == m_enableManualSwitch)
        {
            m_showUserListSwitch->setChecked(true);
        }
        else
        {
            m_enableManualSwitch->setChecked(true);
        }
    }
}
void SettingWindow::onScaleModeChanged(int idx)
{
    QVariant modeData = m_comboScaleMode->itemData(idx);
    int itemScaleMode = modeData.toUInt();

    m_comboScaleFactor->setEnabled(itemScaleMode == GreeterScalingMode::GREETER_SCALING_MODE_MANUAL);
}
}  // namespace Greeter
}  // namespace SessionGuard
}  // namespace Kiran