#include "greetersetting.h"
#include "ui_greetersetting.h"
#include "tabitem.h"
#include "lightdmprefs.h"
#include "dbusapi.h"

#include <QDebug>
#include <QFileDialog>
#include <QValidator>
#include <QMessageBox>
#include <widget-property-helper.h>
#include <kiran-switch-button.h>

#define KEY_FONT_NAME "fontName"

#define BACKGROUND_SAVE_LOCATION  "/usr/share/lightdm-kiran-greeter/background"

using namespace DBusApi;

GreeterSetting::GreeterSetting() :
    KiranTitlebarWindow(),
    ui(new Ui::GreeterSetting),
    m_mateInterfaceSettings("org.mate.interface")
{
    ui->setupUi(getWindowContentWidget());
    initUI();
    connect(&m_mateInterfaceSettings,&QGSettings::changed,[this](const QString& key){
        qInfo() << "changed:" << key;
        if(key!=KEY_FONT_NAME){
            return;
        }
        updateFont();
    });
    updateFont();
}

GreeterSetting::~GreeterSetting()
{
    delete ui;
}

void GreeterSetting::initUI()
{
    setTitle(tr("greeter settings"));
    setIcon(QIcon::fromTheme("preferences-system-login"));

    resize(790,580);

    ///初始化左侧选择页列表
    connect(ui->tabList,&QListWidget::itemSelectionChanged,[this](){
        QList<QListWidgetItem*> selecteds = ui->tabList->selectedItems();
        if(selecteds.size()!=1){
            qFatal("tabList: selecteds size != 1");
        }
        ui->stackedWidget->setCurrentIndex(ui->tabList->row(selecteds.at(0)));
    });
    QListWidgetItem* item;
    item = new QListWidgetItem(tr("appearance"),ui->tabList);
    item->setIcon(QIcon(":/images/appearance_setting.png"));
    ui->tabList->addItem(item);

    item = new QListWidgetItem(tr("user login"),ui->tabList);
    item->setIcon(QIcon(":/images/user_login_setting.png"));
    ui->tabList->addItem(item);
    ui->tabList->setCurrentRow(0);

    ///初始化下拉栏样式
    ui->combo_mode->setView(new QListView);
    ui->combo_mode->addItem(tr("auto"),"auto");
    ui->combo_mode->addItem(tr("manual"),"manual");
    ui->combo_mode->addItem(tr("disable"),"disable");
    ui->combo_mode->view()->window()->setAttribute(Qt::WA_TranslucentBackground);

    ui->combo_scaleFactor->setView(new QListView);
    ui->combo_scaleFactor->addItem("100%","1");
    ui->combo_scaleFactor->addItem("200%","2");
    ui->combo_scaleFactor->view()->window()->setAttribute(Qt::WA_TranslucentBackground);
    ///自动登录延时设置输入限制
    QValidator* validator = new QIntValidator(0,INT_MAX,this);
    ui->edit_autologinDelay->setValidator(validator);

    ///设置配置项到界面，并单向绑定界面到配置文件
    QString str;
    //背景图片
    ui->preview->updatePreviewBackground(LightdmPrefs::instance()->greeterBackground());
    connect(LightdmPrefs::instance(),&LightdmPrefs::greeterBackgroundChanged,
            [this](QString background){
        ui->preview->updatePreviewBackground(background);
    });
    Kiran::WidgetPropertyHelper::setButtonType(ui->btn_browse,Kiran::ButtonType::BUTTON_Default);
    //选择图片触发
    connect(ui->btn_browse,&QToolButton::clicked,[this]{
        QFileDialog selectImageDialog;
        QString fileName = QFileDialog::getOpenFileName(this,tr("select greeter background"),
                                                        "/usr/share/backgrounds/",
                                                        tr("image files(*.bmp *.jpg *.png *.tif *.gif *.pcx *.tga *.exif *.fpx *.svg *.psd *.cdr *.pcd *.dxf *.ufo *.eps *.ai *.raw *.WMF *.webp)"));
        if(!fileName.isEmpty()){
            QFile file(fileName);
            QFileInfo fileInfo(fileName);

            QString destPath = BACKGROUND_SAVE_LOCATION;
            QFileInfo destPathFile(destPath);

            if(destPathFile.exists()){
                QFile::remove(destPath);
            }

            if( !QFile::copy(fileName,destPath) ){
                qWarning() << "copy" << fileName << "-->" << destPath;
                QMessageBox::warning(this,tr("warning"),tr("Failed to set background image."));
                return;
            }
            QFile backgroudnFile(destPath);
            backgroudnFile.setPermissions(backgroudnFile.permissions()|QFile::ReadOther|QFile::ReadUser|QFile::ReadGroup|QFile::ReadOwner);
            LightdmPrefs::instance()->setGreeterBackground(destPath);
        }
    });
    //缩放模式
    QString str;
    str = LightdmPrefs::instance()->scaleMode();
    int idx = ui->combo_mode->findData(str);
    if(idx==-1)
        qFatal("can't find scale mode combo box option,%s",str.toStdString().c_str()) ;
    if(str!="manual"){
        ui->combo_scaleFactor->setDisabled(true);
    }
    ui->combo_mode->setCurrentIndex(idx);
    connect(ui->combo_mode,QOverload<int>::of(&QComboBox::currentIndexChanged),[this](int idx){
        bool enableScaleFactor = false;
        QVariant userData = ui->combo_mode->itemData(idx);
        if(userData.toString()=="manual"){
            enableScaleFactor = true;
        }
        ui->combo_scaleFactor->setEnabled(enableScaleFactor);
        LightdmPrefs::instance()->setScaleMode(userData.toString());
    });

    //缩放比例
    str = LightdmPrefs::instance()->scaleFactor();
    idx = ui->combo_scaleFactor->findData(str);
    if(idx==-1)
        qFatal("can't find scale factor combo box option,%s",str.toStdString().c_str()) ;
    ui->combo_scaleFactor->setCurrentIndex(idx);
    connect(ui->combo_scaleFactor,QOverload<int>::of(&QComboBox::currentIndexChanged),[this](int idx){
        QVariant userData = ui->combo_scaleFactor->itemData(idx);
        LightdmPrefs::instance()->setScaleFactor(userData.toString());
    });

    //允许手动登录
    ui->check_enableManual->setChecked(LightdmPrefs::instance()->enableManualLogin());
    connect(ui->check_enableManual,&QCheckBox::stateChanged,[this]{
        LightdmPrefs::instance()->setEnableManualLogin(ui->check_enableManual->isChecked());
    });


    //隐藏用户列表
    ui->check_hideUserList->setChecked(LightdmPrefs::instance()->hideUserList());
    connect(ui->check_hideUserList,&QCheckBox::stateChanged,this,[this](int state){
        LightdmPrefs::instance()->setHideUserList(ui->check_hideUserList->isChecked());
    });


    //自动登录用户名
    ui->combo_autoLoginUser->setView(new QListView);
    ui->combo_autoLoginUser->view()->window()->setAttribute(Qt::WA_TranslucentBackground);
    initUserComboBox();
    connect(ui->combo_autoLoginUser,
            QOverload<const QString&>::of(&QComboBox::currentTextChanged),
            [this](const QString& user){
        LightdmPrefs::instance()->setAutoLoginUser(user);
    });

    //自动登录延时
    ui->edit_autologinDelay->setText(LightdmPrefs::instance()->autoLoginDelay());
    connect(ui->edit_autologinDelay,static_cast<void (QLineEdit::*)(const QString&)>(&QLineEdit::textChanged),
            this,[this](const QString& text){
        LightdmPrefs::instance()->setAutoLoginDelay(text);
    });
}

bool GreeterSetting::initUserComboBox()
{
    struct UserInfo{
        QString name;
        QString iconFile;
        void clean(){
            name.clear();
            iconFile.clear();
        }
    };
    QVector<UserInfo> userInfoVector;
    QDBusObjectPathVector objVector;

    ///通过AccountService加载用户信息
    if( !DBusApi::AccountsService::listCachedUsers(objVector) ){
        qWarning() << "init user list failed,error: listCachedUsers failed";
        return false;
    }
    UserInfo userInfo;
    for(auto iter=objVector.begin();
        iter!=objVector.end();
        iter++){
        if(!AccountsService::getUserObjectUserNameProperty(*iter,userInfo.name)||
                !AccountsService::getUserObjectIconFileProperty(*iter,userInfo.iconFile)){
            qWarning() << "get " << iter->path() << "UserName,IconFile failed";
            continue;
        }
        userInfoVector.push_back(userInfo);
    }
    userInfo.clean();
    userInfo.name = "root";
    if(!AccountsService::getRootIconFileProperty(userInfo.iconFile)){
        qWarning() << "init user list failed,error: getRootIconFileProperty failed";
        return false;
    }
    userInfoVector.push_back(userInfo);

    ///加入ComboBox
    ///TODO:show user iconFile
    for(auto iter=userInfoVector.begin();
        iter!=userInfoVector.end();
        iter++){
        ui->combo_autoLoginUser->addItem(iter->name);
    }
    ui->combo_autoLoginUser->addItem("");

    ///设置默认选中
    int idx = ui->combo_autoLoginUser->findText(LightdmPrefs::instance()->autoLoginUser());
    if(idx==-1){
        ui->combo_autoLoginUser->setCurrentText("");
    }else{
        ui->combo_autoLoginUser->setCurrentIndex(idx);
    }
    return true;
}

void GreeterSetting::updateFont()
{
    QVariant fontNameVar = m_mateInterfaceSettings.get(KEY_FONT_NAME);
    QString fontNameString = fontNameVar.toString();
    QStringList splitRes = fontNameString.split(" ",QString::SkipEmptyParts);
    QString fontPxSize = splitRes.takeLast();
    QString fontFamily = splitRes.join(" ");

    QFont font = qApp->font();
    font.setFamily(fontFamily);
    font.setPointSize(fontPxSize.toInt());
    qInfo() << fontFamily << fontPxSize;
    qApp->setFont(font,"QWidget");

    font.setPixelSize(fontPxSize.toInt()*2);
    ui->label_appearence->setFont(font);
    ui->label_userlogin->setFont(font);
}
