#include "greeter-setting-window.h"
#include "tabitem.h"
#include "dbusapi.h"
#include "kiran-greeter-prefs.h"
#include "logingreeterpreviewwidget.h"
#include "hover-tips.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QDebug>
#include <QFileDialog>
#include <QStackedWidget>
#include <QPushButton>
#include <QComboBox>
#include <QApplication>
#include <QLineEdit>
#include <widget-property-helper.h>

#include <kiran-switch-button.h>
#include <kiran-sidebar-widget.h>
#include <kiran-message-box.h>

#define BACKGROUND_SAVE_LOCATION  "/usr/share/lightdm-kiran-greeter/background"
using namespace DBusApi;

enum ScaleModeEnum {
    ScaleMode_Auto = 0,
    ScaleMode_Manual,
    ScaleMode_Disable
};

enum GreeterSettingsPageEnum{
    GreeterSettings_Appearance,
    GreeterSettings_Autologin
};


GreeterSettingWindow::GreeterSettingWindow() :
        KiranTitlebarWindow()
{
    initUI();
}

GreeterSettingWindow::~GreeterSettingWindow()
{

}

void GreeterSettingWindow::initUI()
{
    setTitle(tr("greeter settings"));
    setIcon(QIcon::fromTheme("preferences-system-login"));
    resize(840, 770);

    /* 内容区域主布局 */
    QWidget *contentWidget = getWindowContentWidget();
    auto mainLayout = new QHBoxLayout(contentWidget);
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

    m_sidebarWidget = new KiranSidebarWidget(contentWidget);
    m_sidebarWidget->setObjectName("SidebarTabListWidget");
    m_sidebarWidget->setIconSize(QSize(16, 16));
    layoutSideWidget->addWidget(m_sidebarWidget);

    QListWidgetItem *item;
    item = new QListWidgetItem(tr("appearance"), m_sidebarWidget);
    item->setIcon(QIcon(":/images/appearance_setting.png"));
    m_sidebarWidget->addItem(item);

    item = new QListWidgetItem(tr("user login"), m_sidebarWidget);
    item->setIcon(QIcon(":/images/user_login_setting.png"));
    m_sidebarWidget->addItem(item);

    /* 堆叠区域控件 */
    m_stackedWidget = new QStackedWidget(contentWidget);
    m_stackedWidget->setObjectName("GreeterSettingsStacked");
    mainLayout->addWidget(m_stackedWidget);

    auto widgetAppearance = initPageAppearance();
    m_stackedWidget->addWidget(widgetAppearance);

    auto widgetAutoLogin = initPageAutoLogin();
    m_stackedWidget->addWidget(widgetAutoLogin);

    /* 悬浮提示框 */
    m_hoverTips = new HoverTips(m_stackedWidget);
    m_hoverTips->setTimeout(2000);

    /* 处理相关控件信号和初始化 */
    connect(m_sidebarWidget, &KiranSidebarWidget::itemSelectionChanged, [this] {
        QList<QListWidgetItem *> selecteds = m_sidebarWidget->selectedItems();
        if (selecteds.size() != 1) {
            qFatal("tabList: selecteds size != 1");
        }
        int page = m_sidebarWidget->row(selecteds.at(0));
        m_stackedWidget->setCurrentIndex(page);

        /* 重置页面 */
        if( page==GreeterSettings_Appearance ){
            resetAppearanceSettings();
        }else if(page==GreeterSettings_Autologin){
            resetAutoLoginSettings();
        }

        /* 隐藏悬浮提示 */
        m_hoverTips->hide();
    });
    m_sidebarWidget->setCurrentRow(0);
    resetAppearanceSettings();
    resetAutoLoginSettings();

    /* 更新特殊字体要求的控件 */
    updateSpecialFont();
}

QWidget *GreeterSettingWindow::initPageAutoLogin()
{
    auto pageAutoLogin = new QWidget(this);

    auto mainLayout = new QVBoxLayout(pageAutoLogin);
    mainLayout->setContentsMargins(12, 0, 0, 0);
    mainLayout->setSpacing(0);

    /* 自动登录设置标题 */
    m_labelAutomaticLogon = new QLabel(tr("Automatic Logon"), this);
    m_labelAutomaticLogon->setObjectName("label_automaticLogon");
    m_labelAutomaticLogon->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_labelAutomaticLogon->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_labelAutomaticLogon->setStyleSheet("QLabel{margin-bottom:18px;}");
    m_labelAutomaticLogon->installEventFilter(this);
    mainLayout->addWidget(m_labelAutomaticLogon, 0, Qt::AlignLeft | Qt::AlignTop);

    /* 自动登录账户 */
    auto labelAutoLogonAccount = new QLabel(tr("auto login account(take effect after restart)"), this);
    labelAutoLogonAccount->setObjectName("label_autoLogonAccount");
    labelAutoLogonAccount->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    labelAutoLogonAccount->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    labelAutoLogonAccount->setStyleSheet("QLabel{margin-bottom:8px; margin-top:15px;}");
    mainLayout->addWidget(labelAutoLogonAccount);

    m_comboAutoLoginAccount = new QComboBox(this);
    m_comboAutoLoginAccount->setFixedSize(280, 40);
    m_comboAutoLoginAccount->setIconSize(QSize(24, 24));
    initUserComboBox(m_comboAutoLoginAccount);
    mainLayout->addWidget(m_comboAutoLoginAccount, 0, Qt::AlignLeft);

    /* 自动登录延时 */
    auto labelAutoLogonDealy = new QLabel(tr("auto login dealy(seconds)(take effect after restart)"), this);
    labelAutoLogonDealy->setObjectName("label_autoLogonDealy");
    labelAutoLogonDealy->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    labelAutoLogonDealy->setStyleSheet("QLabel{margin-bottom:8px; margin-top:15px;}");
    mainLayout->addWidget(labelAutoLogonDealy);

    m_editAutoLoginDelay = new QLineEdit(this);
    m_editAutoLoginDelay->setFixedSize(280, 40);
    QValidator *validator = new QIntValidator(0, INT_MAX, this);
    m_editAutoLoginDelay->setValidator(validator);
    mainLayout->addWidget(m_editAutoLoginDelay, 0, Qt::AlignLeft);

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
    btn_reset->setText("Reset");
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

QWidget *GreeterSettingWindow::initPageAppearance()
{
    auto pageAppearance = new QWidget(this);
    auto mainLayout = new QVBoxLayout(pageAppearance);
    mainLayout->setContentsMargins(12, 0, 0, 0);
    mainLayout->setSpacing(0);

    /* 外观设置标题 */
    m_labelAppearance = new QLabel(tr("Appearance Settings"), this);
    m_labelAppearance->setObjectName("label_appearance");
    m_labelAppearance->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_labelAppearance->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_labelAppearance->setStyleSheet("QLabel{margin-bottom:18px;}");
    m_labelAppearance->installEventFilter(this);
    mainLayout->addWidget(m_labelAppearance, 0, Qt::AlignLeft | Qt::AlignTop);

    /* 登录外观预览 */
    m_greeterPreview = new LoginGreeterPreviewWidget(this);
    m_greeterPreview->setObjectName("loginGreeterPreviewWidget");
    m_greeterPreview->updatePreviewBackground(KiranGreeterPrefs::instance()->backgroundFile());
    mainLayout->addWidget(m_greeterPreview, 0, Qt::AlignLeft);

    /* 更换背景 */
    auto labelSelectImage = new QLabel(tr("Select Image"), this);
    labelSelectImage->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    labelSelectImage->setObjectName("label_selectImage");
    labelSelectImage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    labelSelectImage->setStyleSheet("QLabel{margin-top:15px;margin-bottom:8px;}");
    mainLayout->addWidget(labelSelectImage);

    m_btnBrowse = new QPushButton(tr("Browse"), this);
    m_btnBrowse->setFixedSize(70, 30);
    m_btnBrowse->setObjectName("btn_Browse");
    Kiran::WidgetPropertyHelper::setButtonType(m_btnBrowse, Kiran::BUTTON_Default);
    mainLayout->addWidget(m_btnBrowse);

    connect(m_btnBrowse, &QPushButton::clicked, [this] {
        QFileDialog selectImageDialog;
        QString fileName = QFileDialog::getOpenFileName(this, tr("select greeter background"),
                                                        "/usr/share/backgrounds/",
                                                        tr("image files(*.bmp *.jpg *.png *.tif *.gif *.pcx *.tga *.exif *.fpx *.svg *.psd *.cdr *.pcd *.dxf *.ufo *.eps *.ai *.raw *.WMF *.webp)"));
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            QFileInfo fileInfo(fileName);
            /* TODO:判断文件是否在用户目录,提示是否需要选择,可能登录界面加载不到 */
            m_greeterPreview->updatePreviewBackground(fileName);
        }
    });

    /* 缩放模式 */
    auto labelScaleMode = new QLabel(tr("Scale Mode"), this);
    labelScaleMode->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    labelScaleMode->setObjectName("label_scaleMode");
    labelScaleMode->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    labelScaleMode->setStyleSheet("QLabel{margin-top:15px;margin-bottom:8px;}");
    mainLayout->addWidget(labelScaleMode);

    m_comboScaleMode = new QComboBox(this);
    m_comboScaleMode->setObjectName("combo_scaleMode");
    m_comboScaleMode->setFixedSize(280, 40);
    m_comboScaleMode->addItem(tr("auto"), ScaleMode_Auto);
    m_comboScaleMode->addItem(tr("manual"), ScaleMode_Manual);
    m_comboScaleMode->addItem(tr("disable"), ScaleMode_Disable);
    mainLayout->addWidget(m_comboScaleMode, 0, Qt::AlignLeft);

    connect(m_comboScaleMode, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int idx) {
        bool enableScaleFactor = false;

        QVariant modeData = m_comboScaleMode->itemData(idx);
        int itemScaleMode = modeData.toUInt();

        m_comboScaleFactor->setEnabled(itemScaleMode == ScaleMode_Manual);
    });

    /* 缩放倍率 */
    auto labelScaleFactor = new QLabel(tr("Scale Factor"), this);
    labelScaleFactor->setObjectName("label_ScaleFactor");
    labelScaleFactor->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    labelScaleFactor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    labelScaleFactor->setStyleSheet("QLabel{margin-top:15px;margin-bottom:8px;}");
    mainLayout->addWidget(labelScaleFactor, 0, Qt::AlignLeft);

    m_comboScaleFactor = new QComboBox(this);
    m_comboScaleFactor->setObjectName("combo_scaleFactor");
    m_comboScaleFactor->setFixedSize(280, 40);
    m_comboScaleFactor->addItem("100%", 1);
    m_comboScaleFactor->addItem("200%", 2);
    mainLayout->addWidget(m_comboScaleFactor, 0, Qt::AlignLeft);

    /* 手动登录 */
    auto widgetManualLogin = new QWidget(this);
    widgetManualLogin->setObjectName("widget_manualLogin");
    widgetManualLogin->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    widgetManualLogin->setFixedWidth(280);
    mainLayout->addWidget(widgetManualLogin);

    auto layoutManualLogin = new QHBoxLayout(widgetManualLogin);
    layoutManualLogin->setObjectName("layout_manualLogin");
    layoutManualLogin->setContentsMargins(0, 15, 0, 0);

    auto labelManualLogin = new QLabel(tr("Enable Manual Login"), this);
    labelManualLogin->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    labelManualLogin->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    layoutManualLogin->addWidget(labelManualLogin);

    auto manualLoginSpacerItem = new QSpacerItem(20, 20,
                                                 QSizePolicy::Expanding,
                                                 QSizePolicy::Preferred);
    layoutManualLogin->addItem(manualLoginSpacerItem);

    m_enableManualSwitch = new KiranSwitchButton(this);
    m_enableManualSwitch->setObjectName("btn_enableManualLogin");
    layoutManualLogin->addWidget(m_enableManualSwitch);

    /* 用户列表登录设置 */
    auto widgetUserListLogin = new QWidget(this);
    widgetUserListLogin->setObjectName("widget_hideUserList");
    widgetUserListLogin->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    widgetUserListLogin->setFixedWidth(280);
    mainLayout->addWidget(widgetUserListLogin);

    auto layoutUserList = new QHBoxLayout(widgetUserListLogin);
    layoutUserList->setObjectName("layout_userListLogin");
    layoutUserList->setContentsMargins(0, 15, 0, 0);

    auto labelHideUserList = new QLabel(tr("Hide User List"), this);
    labelHideUserList->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    labelHideUserList->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    layoutUserList->addWidget(labelHideUserList);

    auto hideUserListSpacerItem = new QSpacerItem(20, 20,
                                                  QSizePolicy::Expanding,
                                                  QSizePolicy::Preferred);
    layoutUserList->addItem(hideUserListSpacerItem);

    m_hideUserListSwitch = new KiranSwitchButton(this);
    m_hideUserListSwitch->setObjectName("btn_hideUserList");
    layoutUserList->addWidget(m_hideUserListSwitch);

    /* 占位 */
    auto mainLayoutSpacerItem = new QSpacerItem(10, 20,
                                                QSizePolicy::Minimum,
                                                QSizePolicy::Expanding);
    mainLayout->addItem(mainLayoutSpacerItem);

    /*保存-取消*/
    auto widgetButtonBox = new QWidget(this);
    widgetButtonBox->setObjectName("widget_appearanceButton");
    widgetButtonBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    mainLayout->addWidget(widgetButtonBox);

    auto layoutButtonBox = new QHBoxLayout(widgetButtonBox);
    layoutButtonBox->setSpacing(30);
    layoutButtonBox->setObjectName("layout_appearanceButtonBox");
    layoutButtonBox->setContentsMargins(0, 10, 0, 40);

    auto buttonBoxSpacerItem1 = new QSpacerItem(10, 20,
                                                QSizePolicy::Expanding,
                                                QSizePolicy::Minimum);
    layoutButtonBox->addItem(buttonBoxSpacerItem1);

    auto btn_save = new QPushButton(this);
    btn_save->setObjectName("btn_saveAppearance");
    btn_save->setFixedSize(232, 60);
    btn_save->setText(tr("Save"));
    Kiran::WidgetPropertyHelper::setButtonType(btn_save, Kiran::BUTTON_Default);
    layoutButtonBox->addWidget(btn_save);
    connect(btn_save,&QPushButton::clicked,[this](){
        saveAppearanceSettings();
    });

    auto buttonBoxSpacerItem2 = new QSpacerItem(10, 20,
                                                QSizePolicy::Expanding,
                                                QSizePolicy::Minimum);
    layoutButtonBox->addItem(buttonBoxSpacerItem2);

    auto btn_reset = new QPushButton(this);
    btn_reset->setObjectName("btn_resetAppearance");
    btn_reset->setFixedSize(232, 60);
    btn_reset->setText(tr("Reset"));
    layoutButtonBox->addWidget(btn_reset);
    connect(btn_reset,&QPushButton::clicked,[this](){
        resetAppearanceSettings();
    });

    auto buttonBoxSpacerItem3 = new QSpacerItem(10, 20,
                                                QSizePolicy::Expanding,
                                                QSizePolicy::Minimum);
    layoutButtonBox->addItem(buttonBoxSpacerItem3);
    layoutButtonBox->setStretch(0, 5);
    layoutButtonBox->setStretch(2, 2);
    layoutButtonBox->setStretch(4, 5);

    return pageAppearance;
}


bool GreeterSettingWindow::eventFilter(QObject *watched, QEvent *event)
{
    if ((watched == m_labelAutomaticLogon || watched == m_labelAutomaticLogon) &&
        (event->type() == QEvent::ApplicationFontChange)) {
        updateSpecialFont();
        return true;
    }
    return KiranTitlebarWindow::eventFilter(watched, event);
}

void GreeterSettingWindow::updateSpecialFont()
{
    QFont font = QApplication::font();
    font.setPointSize(font.pointSize() * 2);


    m_labelAppearance->setFont(font);
    m_labelAutomaticLogon->setFont(font);
}

void GreeterSettingWindow::initUserComboBox(QComboBox *combo)
{
    struct UserInfo {
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
    if (!DBusApi::AccountsService::listCachedUsers(objVector)) {
        qWarning() << "init user list failed,error: listCachedUsers failed";
        return;
    }
    UserInfo userInfo;
    for (auto iter = objVector.begin();
         iter != objVector.end();
         iter++) {
        if (!AccountsService::getUserObjectUserNameProperty(*iter, userInfo.name) ||
            !AccountsService::getUserObjectIconFileProperty(*iter, userInfo.iconFile)) {
            qWarning() << "get " << iter->path() << "UserName,IconFile failed";
            continue;
        }
        userInfoVector.push_back(userInfo);
    }
    userInfo.clean();
    userInfo.name = "root";
    if (!AccountsService::getRootIconFileProperty(userInfo.iconFile)) {
        qWarning() << "init user list failed,error: getRootIconFileProperty failed";
        return;
    }
    userInfoVector.push_back(userInfo);

    ///加入ComboBox
    ///TODO:show user iconFile
    for (auto iter = userInfoVector.begin();
         iter != userInfoVector.end(); iter++) {
        combo->addItem(QIcon(iter->iconFile), iter->name);
    }
    combo->addItem("");
}

void GreeterSettingWindow::saveAppearanceSettings()
{
    GreeterSettingInfo::AppearanceSetting backendInfo = getAppearanceSettingInfoFromBackend();

    /* UI修改之前所加载的配置信息 */
    GreeterSettingInfo::AppearanceSetting origUIInfo = m_origSettingInfo.appearanceInfo;

    /* 如果后端不是在界面端所做修改,提示用户,是覆盖还是重新加载 */
    if (!(backendInfo == origUIInfo)) {
        auto clickedRole = KiranMessageBox::message(this,
                                                    tr("Configuration changed"),
                                                    tr("The external configuration file has changed\n"
                                                       "If you choose to save, all external changes will be overwritten\n"
                                                       "Select discard to discard the modification and reload the new configuration"),
                                                    KiranMessageBox::Save | KiranMessageBox::Discard);
        /* 丢弃所有的修改,重新加载 */
        if( clickedRole == KiranMessageBox::Discard ){
            resetAppearanceSettings();
            return;
        }
    }

    /* 保存 */
    QDBusPendingReply<> reply;
    bool hasError = false;

    reply = KiranGreeterPrefs::instance()->SetBackgroundFile(m_greeterPreview->backgroundPath());
    reply.waitForFinished();
    if(reply.isError()){
        qInfo() << "SetBackgroundFile failed," << reply.error();
        hasError = true;
    }

    reply = KiranGreeterPrefs::instance()->SetHideUserList(m_hideUserListSwitch->isChecked());
    reply.waitForFinished();
    if(reply.isError()){
        qInfo() << "SetHideUserList failed," << reply.error();
        hasError = true;
    }

    reply = KiranGreeterPrefs::instance()->SetAllowManualLogin(m_enableManualSwitch->isChecked());
    reply.waitForFinished();
    if(reply.isError()){
        qInfo() << "SetAllowManualLogin failed," << reply.error();
        hasError = true;
    }

    reply = KiranGreeterPrefs::instance()->SetScaleMode(m_comboScaleMode->currentData().toUInt(),
                                                m_comboScaleFactor->currentData().toUInt());
    reply.waitForFinished();
    if(reply.isError()){
        qInfo() << "SetScaleMode failed," << reply.error();
        hasError = true;
    }

    /* 提示保存相关配置失败 */
    if(hasError){
        m_hoverTips->show(HoverTips::HOVE_TIPS_ERR,tr("Save failed, reload"));
    }else{
        m_hoverTips->show(HoverTips::HOVE_TIPS_SUC,tr("Saved successfully"));
    }

    /* 重新加载配置，更新界面，重新缓存加载的配置文件 */
    resetAppearanceSettings();
}

void GreeterSettingWindow::saveAutoLoginSettings()
{
    GreeterSettingInfo::AutoLoginSetting backendInfo = getAutologinSettingInfoFromBackend();

    /* UI修改之前所加载的配置信息 */
    GreeterSettingInfo::AutoLoginSetting origUIInfo = m_origSettingInfo.autoLoginInfo;

    /* 如果后端不是在界面端所做修改,提示用户,是覆盖还是重新加载 */
    if (!(backendInfo == origUIInfo)) {
        auto clickedRole = KiranMessageBox::message(this,
                                                    tr("Configuration changed"),
                                                    tr("The external configuration file has changed\n"
                                                       "If you choose to save, all external changes will be overwritten\n"
                                                       "Select discard to discard the modification and reload the new configuration"),
                                                    KiranMessageBox::Save | KiranMessageBox::Discard);
        /* 丢弃所有的修改,重新加载 */
        if( clickedRole == KiranMessageBox::Discard ){
            resetAutoLoginSettings();
            return;
        }
    }

    /* 保存 */
    QDBusPendingReply<> reply;
    bool hasError = false;

    reply = KiranGreeterPrefs::instance()->SetAutologinUser(m_comboAutoLoginAccount->currentText());
    reply.waitForFinished();
    if(reply.isError()){
        qInfo() << "SetAutologinUser failed," << reply.error();
        hasError = true;
    }

    reply = KiranGreeterPrefs::instance()->SetAutologinTimeout(m_editAutoLoginDelay->text().toUInt());
    reply.waitForFinished();
    if(reply.isError()){
        qInfo() << "SetAutologinTimeout" << reply.error();
        hasError = true;
    }

    /* 提示保存是否成功 */
    if(hasError){
        m_hoverTips->show(HoverTips::HOVE_TIPS_ERR,tr("Save failed, reload"));
    }else{
        m_hoverTips->show(HoverTips::HOVE_TIPS_SUC,tr("Saved successfully"));
    }

    /* 重新加载配置，更新界面，重新缓存加载的配置文件 */
    resetAutoLoginSettings();
}

/* 重新加载登录外观设置,更新到界面,并缓存 */
void GreeterSettingWindow::resetAppearanceSettings()
{
    GreeterSettingInfo::AppearanceSetting appearanceSetting = getAppearanceSettingInfoFromBackend();

    m_greeterPreview->updatePreviewBackground(appearanceSetting.background);
    m_hideUserListSwitch->setChecked(appearanceSetting.hideUserList);
    m_enableManualSwitch->setChecked(appearanceSetting.allowManualLogin);
    int idx = m_comboScaleMode->findData(appearanceSetting.scaleMode);
    m_comboScaleMode->setCurrentIndex(idx);
    idx = m_comboScaleFactor->findData(appearanceSetting.scaleFactor);
    m_comboScaleFactor->setCurrentIndex(idx);

    m_origSettingInfo.appearanceInfo = appearanceSetting;
}

/* 重新加载登录自动登录设置,更新到界面,并缓存 */
void GreeterSettingWindow::resetAutoLoginSettings()
{
    GreeterSettingInfo::AutoLoginSetting autoLoginSetting = getAutologinSettingInfoFromBackend();

    m_comboAutoLoginAccount->setCurrentText(autoLoginSetting.autoLoginUser);
    m_editAutoLoginDelay->setText(QString::number(autoLoginSetting.autoLoginTimeout));

    m_origSettingInfo.autoLoginInfo = autoLoginSetting;
}

GreeterSettingInfo::AppearanceSetting GreeterSettingWindow::getAppearanceSettingInfoFromBackend()
{
    GreeterSettingInfo::AppearanceSetting appearanceSetting;

    appearanceSetting.background = KiranGreeterPrefs::instance()->backgroundFile();
    appearanceSetting.hideUserList = KiranGreeterPrefs::instance()->hideUserList();
    appearanceSetting.allowManualLogin = KiranGreeterPrefs::instance()->allowManualLogin();
    appearanceSetting.scaleMode = KiranGreeterPrefs::instance()->scaleMode();
    if (m_comboScaleMode->findData(appearanceSetting.scaleMode) == -1) {
        qWarning() << "no such scale mode" << appearanceSetting.scaleMode;
        appearanceSetting.scaleMode = 0;
    }
    appearanceSetting.scaleFactor = KiranGreeterPrefs::instance()->scaleFactor();
    if (m_comboScaleFactor->findData(appearanceSetting.scaleFactor) == -1) {
        qWarning() << "no such scale factor" << appearanceSetting.scaleFactor;
        appearanceSetting.scaleFactor = 1;
    }

    return appearanceSetting;
}

GreeterSettingInfo::AutoLoginSetting GreeterSettingWindow::getAutologinSettingInfoFromBackend()
{
    GreeterSettingInfo::AutoLoginSetting autoLoginSetting;

    autoLoginSetting.autoLoginTimeout = KiranGreeterPrefs::instance()->autologinTimeout();
    autoLoginSetting.autoLoginUser = KiranGreeterPrefs::instance()->autologinUser();
    if (m_comboAutoLoginAccount->findText(autoLoginSetting.autoLoginUser) == -1) {
        qWarning() << "no such user," << autoLoginSetting.autoLoginUser;
        autoLoginSetting.autoLoginUser = "";
    }

    return autoLoginSetting;
}

GreeterSettingInfo::AppearanceSetting GreeterSettingWindow::getAppearanceSettingInfoFromUI()
{
    GreeterSettingInfo::AppearanceSetting appearanceSetting;

    appearanceSetting.background = m_greeterPreview->backgroundPath();
    appearanceSetting.scaleFactor = m_comboScaleFactor->currentData().toUInt();
    appearanceSetting.scaleMode = m_comboScaleMode->currentData().toUInt();
    appearanceSetting.allowManualLogin = m_enableManualSwitch->isChecked();
    appearanceSetting.hideUserList = m_hideUserListSwitch->isChecked();

    return appearanceSetting;
}

GreeterSettingInfo::AutoLoginSetting GreeterSettingWindow::getAutologinSettingInfoFromUI()
{
    GreeterSettingInfo::AutoLoginSetting autoLoginSetting;

    autoLoginSetting.autoLoginUser = m_comboAutoLoginAccount->currentText();
    autoLoginSetting.autoLoginTimeout = m_editAutoLoginDelay->text().toUInt();

    return autoLoginSetting;
}
