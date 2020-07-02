#include "greetersetting.h"
#include "ui_greetersetting.h"
#include "tabitem.h"
#include "lightdmprefs.h"

#include <QDebug>
#include <QFileDialog>
#include <QValidator>

GreeterSetting::GreeterSetting(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GreeterSetting)
{
    ui->setupUi(this);
    initUI();
}

GreeterSetting::~GreeterSetting()
{
    delete ui;
}

void GreeterSetting::initUI()
{
    setWindowTitle(tr("greeter settings"));

    setFixedSize(760,520);

    ///初始化左侧选择页列表
    connect(ui->tabList,&QListWidget::itemSelectionChanged,[this](){
        QList<QListWidgetItem*> selecteds = ui->tabList->selectedItems();
        if(selecteds.size()!=1){
            qFatal("tabList: selecteds size != 1");
        }
        ui->stackedWidget->setCurrentIndex(ui->tabList->row(selecteds.at(0)));
    });
    ui->tabList->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tabList->setSelectionMode(QAbstractItemView::SingleSelection);
    QListWidgetItem* item;
    TabItem* tabItem;
    item = new QListWidgetItem(ui->tabList);
    item->setSizeHint(QSize(274,60));
    tabItem = new TabItem(":/images/appearance_setting.png",tr("appearance"),ui->tabList);
    ui->tabList->addItem(item);
    ui->tabList->setItemWidget(item,tabItem);

    item = new QListWidgetItem(ui->tabList);
    item->setSizeHint(QSize(274,60));
    tabItem = new TabItem(":/images/user_login_setting.png",tr("user login"),ui->tabList);
    ui->tabList->addItem(item);
    ui->tabList->setItemWidget(item,tabItem);
    ui->tabList->setCurrentRow(0);

    ///初始化下拉栏样式
    ui->combo_mode->setView(new QListView);
    ui->combo_mode->view()->window()->setAttribute(Qt::WA_TranslucentBackground);
    ui->combo_mode->addItem(tr("auto"),"auto");
    ui->combo_mode->addItem(tr("manual"),"manual");
    ui->combo_mode->addItem(tr("disable"),"disable");

    ui->combo_scaleFactor->setView(new QListView);
    ui->combo_scaleFactor->view()->window()->setAttribute(Qt::WA_TranslucentBackground);
    ui->combo_scaleFactor->addItem("100%","1");
    ui->combo_scaleFactor->addItem("200%","2");

    ///自动登录延时设置输入限制
    QValidator* validator = new QIntValidator(0,INT_MAX,this);
    ui->edit_autologinDelay->setValidator(validator);

    ///设置配置项到界面，并单向绑定界面到配置文件
    QString str;
    //背景图片
    ui->preview->updatePreviewBackground(LightdmPrefs::instance()->greeterBackground());
    connect(LightdmPrefs::instance(),static_cast<void (LightdmPrefs::*) (QString)>(&LightdmPrefs::greeterBackgroundChanged),
            this,[this](QString background){
        ui->preview->updatePreviewBackground(background);
    });
    //选择图片触发
    connect(ui->btn_browse,&QToolButton::clicked,
            this,[this]{
        QFileDialog selectImageDialog;
        QString fileName = QFileDialog::getOpenFileName(this,tr("select greeter background"),
                                                        "/usr/share/backgrounds/",
                                                        tr("image files(*.bmp *.jpg *.png *.tif *.gif *.pcx *.tga *.exif *.fpx *.svg *.psd *.cdr *.pcd *.dxf *.ufo *.eps *.ai *.raw *.WMF *.webp)"));
        if(!fileName.isEmpty()){
            LightdmPrefs::instance()->setGreeterBackground(fileName);
        }
    });
    //缩放模式
    str = LightdmPrefs::instance()->scaleMode();
    int idx = ui->combo_mode->findData(str);
    if(idx==-1)
        qFatal("can't find scale mode combo box option,%s",str.toStdString().c_str()) ;
    if(str!="manual"){
        ui->label_scaleFactor->setVisible(false);
        ui->combo_scaleFactor->setVisible(false);
    }
    ui->combo_mode->setCurrentIndex(idx);
    connect(ui->combo_mode,static_cast<void (QComboBox::*) (int idx)>(&QComboBox::currentIndexChanged),
            [this](int idx){
        bool showScaleFactor = false;
        QVariant userData = ui->combo_mode->itemData(idx);
        if(userData.toString()=="manual"){
            showScaleFactor = true;
        }
        ui->label_scaleFactor->setVisible(showScaleFactor);
        ui->combo_scaleFactor->setVisible(showScaleFactor);
        LightdmPrefs::instance()->setScaleMode(userData.toString());
    });

    //缩放比例
    str = LightdmPrefs::instance()->scaleFactor();
    idx = ui->combo_scaleFactor->findData(str);
    if(idx==-1)
        qFatal("can't find scale factor combo box option,%s",str.toStdString().c_str()) ;
    ui->combo_scaleFactor->setCurrentIndex(idx);
    connect(ui->combo_scaleFactor,static_cast<void (QComboBox::*)(int idx)>(&QComboBox::currentIndexChanged),
            this,[this](int idx){
        QVariant userData = ui->combo_scaleFactor->itemData(idx);
        LightdmPrefs::instance()->setScaleFactor(userData.toString());
    });

    //允许手动登录
    ui->check_enableManual->setChecked(LightdmPrefs::instance()->enableManualLogin());
    connect(ui->check_enableManual,&QCheckBox::stateChanged,this,[this]{
        LightdmPrefs::instance()->setEnableManualLogin(ui->check_enableManual->isChecked());
    });

    //隐藏用户列表
    ui->check_hideUserList->setChecked(LightdmPrefs::instance()->hideUserList());
    connect(ui->check_hideUserList,&QCheckBox::stateChanged,this,[this](int state){
        LightdmPrefs::instance()->setHideUserList(ui->check_hideUserList->isChecked());
    });

    //自动登录用户名
    ui->edit_autologinName->setText(LightdmPrefs::instance()->autoLoginUser());
    connect(ui->edit_autologinName,static_cast<void (QLineEdit::*)(const QString&)>(&QLineEdit::textChanged),
            this,[this](const QString& text){
        LightdmPrefs::instance()->setAutoLoginUser(text);
    });

    //自动登录延时
    ui->edit_autologinDelay->setText(LightdmPrefs::instance()->autoLoginDelay());
    connect(ui->edit_autologinDelay,static_cast<void (QLineEdit::*)(const QString&)>(&QLineEdit::textChanged),
            this,[this](const QString& text){
        LightdmPrefs::instance()->setAutoLoginDelay(text);
    });
}
