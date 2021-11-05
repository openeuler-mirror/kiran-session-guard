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

#include "greeter-setting-window.h"
#include "dbusapi.h"
#include "define.h"
#include "hover-tips.h"
#include "kiran-greeter-prefs.h"

#include <widget-property-helper.h>
#include <QApplication>
#include <QComboBox>
#include <QDir>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStackedWidget>
#include <QStandardPaths>

#include <kiran-image-selector.h>
#include <kiran-message-box.h>
#include <kiran-sidebar-widget.h>
#include <kiran-switch-button.h>
#include <kiran-system-daemon/greeter-i.h>
#include <qt5-log-i.h>

#define BACKGROUND_SAVE_LOCATION "/usr/share/lightdm-kiran-greeter/background"
using namespace DBusApi;

enum GreeterSettingsPageEnum
{
    GreeterSettings_Appearance,
    GreeterSettings_Autologin
};

GreeterSettingWindow::GreeterSettingWindow(QWidget *parent) : QWidget(parent)
{
    initUI();
}

GreeterSettingWindow::~GreeterSettingWindow()
{
}

void GreeterSettingWindow::initUI()
{
    /* 内容区域主布局 */
    auto mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(9, 0, 9, 9);

    /* 左侧侧边栏 */
    auto sideWidget = new QWidget(this);
    sideWidget->setObjectName("widget_side");
    sideWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    sideWidget->setFixedWidth(286);
    sideWidget->setStyleSheet("QWidget{border-right:1px solid rgba(255,255,255,25);}");
    mainLayout->addWidget(sideWidget);

    auto layoutSideWidget = new QHBoxLayout(sideWidget);
    layoutSideWidget->setSpacing(0);
    layoutSideWidget->setMargin(0);

    m_sidebarWidget = new KiranSidebarWidget(this);
    m_sidebarWidget->setObjectName("SidebarTabListWidget");
    m_sidebarWidget->setIconSize(QSize(16, 16));
    layoutSideWidget->addWidget(m_sidebarWidget);

    QListWidgetItem *item;
    item = new QListWidgetItem(tr("general settings"), m_sidebarWidget);
    item->setIcon(QIcon(":/kcp-greeter-images/appearance_setting.png"));
    m_sidebarWidget->addItem(item);

    item = new QListWidgetItem(tr("autologin"), m_sidebarWidget);
    item->setIcon(QIcon(":/kcp-greeter-images/user_login_setting.png"));
    m_sidebarWidget->addItem(item);

    /* 堆叠区域控件 */
    m_stackedWidget = new QStackedWidget(this);
    m_stackedWidget->setObjectName("GreeterSettingsStacked");
    mainLayout->addWidget(m_stackedWidget);

    auto widgetGeneralSettings = initPageGeneralSettings();
    m_stackedWidget->addWidget(widgetGeneralSettings);

    auto widgetAutoLogin = initPageAutoLogin();
    m_stackedWidget->addWidget(widgetAutoLogin);

    /* 悬浮提示框 */
    m_hoverTips = new HoverTips(m_stackedWidget);
    m_hoverTips->setTimeout(2000);

    /* 处理相关控件信号和初始化 */
    connect(m_sidebarWidget, &KiranSidebarWidget::itemSelectionChanged, [this] {
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

QWidget *GreeterSettingWindow::initPageAutoLogin()
{
    auto pageAutoLogin = new QWidget(this);

    auto mainLayout = new QVBoxLayout(pageAutoLogin);
    mainLayout->setContentsMargins(12, 24, 0, 0);
    mainLayout->setSpacing(0);

    /* 自动登录用户 */
    auto labelAutoLogonUser = new QLabel(tr("auto login user(take effect after restart)"), this);
    labelAutoLogonUser->setObjectName("label_autoLogonUser");
    labelAutoLogonUser->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    labelAutoLogonUser->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    labelAutoLogonUser->setStyleSheet("QLabel{margin-bottom:8px;}");
    mainLayout->addWidget(labelAutoLogonUser);

    m_comboAutoLoginUser = new QComboBox(this);
    m_comboAutoLoginUser->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_comboAutoLoginUser->setFixedHeight(40);
    m_comboAutoLoginUser->setIconSize(QSize(24, 24));
    initUserComboBox(m_comboAutoLoginUser);
    mainLayout->addWidget(m_comboAutoLoginUser, 0);

    /* 自动登录延时 */
    auto labelAutoLogonDelay = new QLabel(tr("auto login delay(seconds)(take effect after restart)"), this);
    labelAutoLogonDelay->setObjectName("label_autoLogonDealy");
    labelAutoLogonDelay->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    labelAutoLogonDelay->setStyleSheet("QLabel{margin-bottom:8px; margin-top:15px;}");
    mainLayout->addWidget(labelAutoLogonDelay);

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
    layoutButtonBox->setSpacing(30);
    layoutButtonBox->setObjectName("layout_autoLoginButtonBox");
    layoutButtonBox->setContentsMargins(0, 10, 0, 40);
    mainLayout->addLayout(layoutButtonBox);

    auto buttonBoxSpacerItem1 = new QSpacerItem(10, 20,
                                                QSizePolicy::Expanding,
                                                QSizePolicy::Minimum);
    layoutButtonBox->addItem(buttonBoxSpacerItem1);

    auto btn_save = new QPushButton(this);
    btn_save->setObjectName("btn_saveAutoLogin");
    btn_save->setFixedSize(232, 60);
    btn_save->setText(tr("Save"));
    Kiran::WidgetPropertyHelper::setButtonType(btn_save, Kiran::BUTTON_Default);
    layoutButtonBox->addWidget(btn_save);
    connect(btn_save, &QPushButton::clicked, [this]() {
        saveAutoLoginSettings();
    });

    auto buttonBoxSpacerItem2 = new QSpacerItem(10, 20,
                                                QSizePolicy::Expanding,
                                                QSizePolicy::Minimum);
    layoutButtonBox->addItem(buttonBoxSpacerItem2);

    auto btn_reset = new QPushButton(this);
    btn_reset->setObjectName("btn_resetAutoLogin");
    btn_reset->setFixedSize(232, 60);
    btn_reset->setText(tr("Reset"));
    layoutButtonBox->addWidget(btn_reset);

    connect(btn_reset, &QPushButton::clicked, [this]() {
        resetAutoLoginSettings();
    });

    auto buttonBoxSpacerItem3 = new QSpacerItem(10, 20,
                                                QSizePolicy::Expanding,
                                                QSizePolicy::Minimum);
    layoutButtonBox->addItem(buttonBoxSpacerItem3);
    layoutButtonBox->setStretch(0, 5);
    layoutButtonBox->setStretch(2, 2);
    layoutButtonBox->setStretch(4, 5);

    return pageAutoLogin;
}

QWidget *GreeterSettingWindow::initPageGeneralSettings()
{
    auto pageGeneralSettings = new QWidget(this);
    auto mainLayout = new QVBoxLayout(pageGeneralSettings);
    mainLayout->setContentsMargins(12, 24, 0, 0);
    mainLayout->setSpacing(0);

    /* 外观设置 */
    /* 图片选择控件 */
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
    labelScaleMode->setStyleSheet("QLabel{margin-top:15px;margin-bottom:8px;}");
    mainLayout->addWidget(labelScaleMode);

    m_comboScaleMode = new QComboBox(this);
    m_comboScaleMode->setObjectName("combo_scaleMode");
    m_comboScaleMode->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_comboScaleMode->setFixedHeight(40);
    m_comboScaleMode->addItem(tr("auto"), GreeterScalingMode::GREETER_SCALING_MODE_AUTO);
    m_comboScaleMode->addItem(tr("manual"), GreeterScalingMode::GREETER_SCALING_MODE_MANUAL);
    m_comboScaleMode->addItem(tr("disable"), GreeterScalingMode::GREETER_SCALING_MODE_DISABLE);
    mainLayout->addWidget(m_comboScaleMode, 0);

    connect(m_comboScaleMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this,&GreeterSettingWindow::onScaleModeChanged);

    /* 缩放倍率 */
    auto labelScaleFactor = new QLabel(tr("Scale Factor"), this);
    labelScaleFactor->setObjectName("label_ScaleFactor");
    labelScaleFactor->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    labelScaleFactor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    labelScaleFactor->setStyleSheet("QLabel{margin-top:15px;margin-bottom:8px;}");
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
    layoutManualLogin->setContentsMargins(0, 15, 0, 0);

    auto labelManualLogin = new QLabel(tr("Enable manual input user login"), this);
    labelManualLogin->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    labelManualLogin->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    layoutManualLogin->addWidget(labelManualLogin);

    auto manualLoginSpacerItem = new QSpacerItem(20, 20,
                                                 QSizePolicy::Expanding,
                                                 QSizePolicy::Preferred);
    layoutManualLogin->addItem(manualLoginSpacerItem);

    m_enableManualSwitch = new KiranSwitchButton(this);
    connect(m_enableManualSwitch, &KiranSwitchButton::toggled, this, &GreeterSettingWindow::onLoginOptionsChanged);
    m_enableManualSwitch->setObjectName("btn_enableManualLogin");
    layoutManualLogin->addWidget(m_enableManualSwitch);

    /* 用户列表登录设置 */
    auto widgetUserListLogin = new QWidget(this);
    widgetUserListLogin->setObjectName("widget_showUserList");
    widgetUserListLogin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    mainLayout->addWidget(widgetUserListLogin);

    auto layoutUserList = new QHBoxLayout(widgetUserListLogin);
    layoutUserList->setObjectName("layout_userListLogin");
    layoutUserList->setContentsMargins(0, 15, 0, 0);

    auto labelShowUserList = new QLabel(tr("Show User List"), this);
    labelShowUserList->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    labelShowUserList->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    layoutUserList->addWidget(labelShowUserList);

    auto showUserListSpacerItem = new QSpacerItem(20, 20,
                                                  QSizePolicy::Expanding,
                                                  QSizePolicy::Preferred);
    layoutUserList->addItem(showUserListSpacerItem);

    m_showUserListSwitch = new KiranSwitchButton(this);
    connect(m_showUserListSwitch, &KiranSwitchButton::toggled, this, &GreeterSettingWindow::onLoginOptionsChanged);
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
    layoutButtonBox->setSpacing(30);
    layoutButtonBox->setObjectName("layout_generalSettingsButtonBox");
    layoutButtonBox->setContentsMargins(0, 10, 0, 40);

    auto buttonBoxSpacerItem1 = new QSpacerItem(10, 20,
                                                QSizePolicy::Expanding,
                                                QSizePolicy::Minimum);
    layoutButtonBox->addItem(buttonBoxSpacerItem1);

    auto btn_save = new QPushButton(this);
    btn_save->setObjectName("btn_saveGeneralSettings");
    btn_save->setFixedSize(232, 60);
    btn_save->setText(tr("Save"));
    Kiran::WidgetPropertyHelper::setButtonType(btn_save, Kiran::BUTTON_Default);
    layoutButtonBox->addWidget(btn_save);
    connect(btn_save, &QPushButton::clicked, [this]() {
        saveGeneralSettings();
    });

    auto buttonBoxSpacerItem2 = new QSpacerItem(10, 20,
                                                QSizePolicy::Expanding,
                                                QSizePolicy::Minimum);
    layoutButtonBox->addItem(buttonBoxSpacerItem2);

    auto btn_reset = new QPushButton(this);
    btn_reset->setObjectName("btn_resetGeneralSettings");
    btn_reset->setFixedSize(232, 60);
    btn_reset->setText(tr("Reset"));
    layoutButtonBox->addWidget(btn_reset);
    connect(btn_reset, &QPushButton::clicked, [this]() {
        resetGeneralSettings();
    });

    auto buttonBoxSpacerItem3 = new QSpacerItem(10, 20,
                                                QSizePolicy::Expanding,
                                                QSizePolicy::Minimum);
    layoutButtonBox->addItem(buttonBoxSpacerItem3);
    layoutButtonBox->setStretch(0, 5);
    layoutButtonBox->setStretch(2, 2);
    layoutButtonBox->setStretch(4, 5);

    return pageGeneralSettings;
}

void GreeterSettingWindow::initUserComboBox(QComboBox *combo)
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
    QVector<UserInfo> userInfoVector;
    QDBusObjectPathVector objVector;

    ///通过AccountService加载用户信息
    if (!DBusApi::AccountsService::listCachedUsers(objVector))
    {
        KLOG_WARNING() << "init user list failed,error: listCachedUsers failed";
        return;
    }
    UserInfo userInfo;
    for (auto &iter : objVector)
    {
        if (!AccountsService::getUserObjectUserNameProperty(iter, userInfo.name) ||
            !AccountsService::getUserObjectIconFileProperty(iter, userInfo.iconFile))
        {
            KLOG_WARNING() << "get " << iter.path() << "UserName,IconFile failed";
            continue;
        }
        userInfoVector.push_back(userInfo);
    }
    userInfo.clean();
    userInfo.name = "root";
    if (!AccountsService::getRootIconFileProperty(userInfo.iconFile))
    {
        KLOG_WARNING() << "init user list failed,error: getRootIconFileProperty failed";
        return;
    }
    userInfoVector.push_back(userInfo);

    ///加入ComboBox
    for (auto &iter : userInfoVector)
    {
        combo->addItem(QIcon(iter.iconFile), iter.name);
    }
    combo->addItem("");
}

void GreeterSettingWindow::saveGeneralSettings()
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

    reply = KiranGreeterPrefs::instance()->SetBackground(m_imageSelector->selectedImage());
    reply.waitForFinished();
    if (reply.isError())
    {
        KLOG_ERROR() << "SetBackgroundFile failed," << reply.error();
        hasError = true;
        goto failed;
    }

    reply = KiranGreeterPrefs::instance()->SetHideUserList(!m_showUserListSwitch->isChecked());
    reply.waitForFinished();
    if (reply.isError())
    {
        KLOG_ERROR() << "SetHideUserList failed," << reply.error();
        hasError = true;
    }

    reply = KiranGreeterPrefs::instance()->SetAllowManualLogin(m_enableManualSwitch->isChecked());
    reply.waitForFinished();
    if (reply.isError())
    {
        KLOG_ERROR() << "SetAllowManualLogin failed," << reply.error();
        hasError = true;
    }

    reply = KiranGreeterPrefs::instance()->SetScaleMode(m_comboScaleMode->currentData().toUInt(),
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

void GreeterSettingWindow::saveAutoLoginSettings()
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
    bool hasError = false;

    reply = KiranGreeterPrefs::instance()->SetAutologinUser(m_comboAutoLoginUser->currentText());
    reply.waitForFinished();
    if (reply.isError())
    {
        KLOG_ERROR() << "SetAutologinUser failed," << reply.error();
        hasError = true;
        goto failed;
    }

    reply = KiranGreeterPrefs::instance()->SetAutologinTimeout(m_editAutoLoginDelay->text().toUInt());
    reply.waitForFinished();
    if (reply.isError())
    {
        KLOG_ERROR() << "SetAutologinTimeout" << reply.error();
        hasError = true;
    }

failed:
    /* 提示保存是否成功 */
    if (hasError)
    {
        m_hoverTips->show(HoverTips::HOVE_TIPS_ERR, tr("Save failed, reload"));
    }
    else
    {
        m_hoverTips->show(HoverTips::HOVE_TIPS_SUC, tr("Saved successfully"));
    }

    /* 重新加载配置，更新界面，重新缓存加载的配置文件 */
    resetAutoLoginSettings();
}

/* 重新加载登录外观设置,更新到界面,并缓存 */
void GreeterSettingWindow::resetGeneralSettings()
{
    GreeterSettingInfo::GeneralSetting appearanceSetting = getGeneralSettingInfoFromBackend();

    QString background = appearanceSetting.background;

    ///背景设置为空使用默认背景图片,如果背景图片是链接的话，读出指向文件位置
    if (background.isEmpty())
    {
        QFileInfo backgroundFileInfo(DEFAULT_BACKGROUND);
        if (backgroundFileInfo.isSymLink())
        {
            background = backgroundFileInfo.readLink();
        }
        else
        {
            background = DEFAULT_BACKGROUND;
        }
    }

    ///如果图片选择控件不存在选中的图片的话则添加进入图片选择列表
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
void GreeterSettingWindow::resetAutoLoginSettings()
{
    GreeterSettingInfo::AutoLoginSetting autoLoginSetting = getAutologinSettingInfoFromBackend();

    m_comboAutoLoginUser->setCurrentText(autoLoginSetting.autoLoginUser);
    m_editAutoLoginDelay->setText(QString::number(autoLoginSetting.autoLoginTimeout));

    m_origSettingInfo.autoLoginInfo = autoLoginSetting;
}

GreeterSettingInfo::GeneralSetting GreeterSettingWindow::getGeneralSettingInfoFromBackend()
{
    GreeterSettingInfo::GeneralSetting appearanceSetting;

    appearanceSetting.background = KiranGreeterPrefs::instance()->background();
    appearanceSetting.hideUserList = KiranGreeterPrefs::instance()->hide_user_list();
    appearanceSetting.allowManualLogin = KiranGreeterPrefs::instance()->allow_manual_login();
    appearanceSetting.scaleMode = KiranGreeterPrefs::instance()->scale_mode();

    if (m_comboScaleMode->findData(appearanceSetting.scaleMode) == -1)
    {
        KLOG_WARNING() << "no such scale mode <" << appearanceSetting.scaleMode << ">,using default scale mode:auto";
        appearanceSetting.scaleMode = 0;
    }

    appearanceSetting.scaleFactor = KiranGreeterPrefs::instance()->scale_factor();
    if (m_comboScaleFactor->findData(appearanceSetting.scaleFactor) == -1)
    {
        KLOG_WARNING() << "no such scale factor <" << appearanceSetting.scaleFactor << ">,using default scale factor:1";
        appearanceSetting.scaleFactor = 1;
    }

    return appearanceSetting;
}

GreeterSettingInfo::AutoLoginSetting GreeterSettingWindow::getAutologinSettingInfoFromBackend()
{
    GreeterSettingInfo::AutoLoginSetting autoLoginSetting;

    autoLoginSetting.autoLoginTimeout = KiranGreeterPrefs::instance()->autologin_timeout();
    autoLoginSetting.autoLoginUser = KiranGreeterPrefs::instance()->autologin_user();
    if (m_comboAutoLoginUser->findText(autoLoginSetting.autoLoginUser) == -1)
    {
        KLOG_WARNING() << "no such user," << autoLoginSetting.autoLoginUser;
        autoLoginSetting.autoLoginUser = "";
    }

    return autoLoginSetting;
}

QSize GreeterSettingWindow::sizeHint() const
{
    return {940, 653};
}

void GreeterSettingWindow::onLoginOptionsChanged()
{
    auto changedSwitch = qobject_cast<KiranSwitchButton *>(sender());
    if (changedSwitch->isChecked())
    {
        return;
    }

    //不允许手动登录以及不显示用户列表
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
void GreeterSettingWindow::onScaleModeChanged(int idx)
{
    QVariant modeData = m_comboScaleMode->itemData(idx);
    int itemScaleMode = modeData.toUInt();

    m_comboScaleFactor->setEnabled(itemScaleMode == GreeterScalingMode::GREETER_SCALING_MODE_MANUAL);
}
