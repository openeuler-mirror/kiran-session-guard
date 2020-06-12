#include "greetersetting.h"
#include <QSettings>
#include <QtDebug>
#include <QFile>
#include <QVector>
#include <QMap>

#define  GREETER_SETTING_PATH                   "/etc/lightdm/lightdm-kiran-greeter.conf"
#define  GREETER_SETTING_GROUP                  "Greeter"

///背景配置
#define  KEY_BACKGROUND_URI                     "background-picture-uri"
#define  DEFAULT_BACKGROUND_URI                 ":/images/default_background.png"

///用户列表是否隐藏
#define  KEY_USER_LIST_HIDING                   "user-list-hiding"
#define  DEFAULT_USER_LIST_HIDING               false

///是否允许手动登录
#define  KEY_ENABLE_MANUAL_LOGIN                "enable-manual-login"
#define  DEFAULT_ENABLE_MANUAL_LOGIN            true

///是否开启缩放,支持三项配置 auto,enable,disable
#define  KEY_ENABLE_SCALING                     "enable-scaling"
#define  DEFAULT_ENABLE_SCALING                 "auto"

///缩放比例
#define  KEY_SCALE_FACTOR                       "scale-factor"
#define  DEFAULT_SCALE_FACTOR                   0.0

static const QMap<QString,GreeterSetting::EnableScalingEnum> ScalingEnumMap = {
    {"auto",GreeterSetting::SCALING_AUTO},
    {"manual",GreeterSetting::SCALING_ENABLE},
    {"disable",GreeterSetting::SCALING_DISABLE}
};

GreeterSetting *GreeterSetting::instance()
{
    static GreeterSetting setting;
    return &setting;
}

void GreeterSetting::load()
{
    QFile settingsFile(GREETER_SETTING_PATH);

    if( !settingsFile.exists() ){
        qInfo() << GREETER_SETTING_PATH << " is not exists";
        return;
    }

    if( !settingsFile.open(QIODevice::ReadOnly) ){
        qInfo() << "cant open " << GREETER_SETTING_PATH;
    }

    if( !settingsFile.isReadable() ){
        qInfo() << "can't read" << GREETER_SETTING_PATH;
        return;
    }

    QSettings setting(GREETER_SETTING_PATH,QSettings::IniFormat);
    QStringList groups = setting.childGroups();
    if( !groups.contains(GREETER_SETTING_GROUP) ){
        qInfo("%s don't have <%s> group.",GREETER_SETTING_PATH,GREETER_SETTING_GROUP);
        return;
    }
    setting.beginGroup(GREETER_SETTING_GROUP);
    QVariant backgroundURI = setting.value(KEY_BACKGROUND_URI,DEFAULT_BACKGROUND_URI);
    m_backgroundPath = backgroundURI.toString();

    QVariant userListHiding = setting.value(KEY_USER_LIST_HIDING,DEFAULT_USER_LIST_HIDING);
    m_userlistHiding = userListHiding.toBool();

    QVariant enableManualLogin = setting.value(KEY_ENABLE_MANUAL_LOGIN,DEFAULT_ENABLE_MANUAL_LOGIN);
    m_enableManualLogin = enableManualLogin.toBool();

    QVariant enableScaling = setting.value(KEY_ENABLE_SCALING,DEFAULT_ENABLE_SCALING);
    QString enableScalingString = enableScaling.toString();
    auto iter = ScalingEnumMap.find(enableScalingString);
    if( iter==ScalingEnumMap.end() ){
        m_enableScaling = SCALING_AUTO;
    }else{
        m_enableScaling = iter.value();
    }

    QVariant scaleFactor = setting.value(KEY_SCALE_FACTOR,DEFAULT_SCALE_FACTOR);
    m_scaleFactor = scaleFactor.toDouble();
    if( m_scaleFactor<1 ){
        m_scaleFactor = 1;
    }else if(m_scaleFactor>2){
        m_scaleFactor = 2;
    }
}

QString GreeterSetting::getBackgroundPath()
{
    return m_backgroundPath;
}

bool GreeterSetting::getUserListHiding()
{
    return m_userlistHiding;
}

bool GreeterSetting::getEnableManualLogin()
{
    return m_enableManualLogin;
}

GreeterSetting::EnableScalingEnum GreeterSetting::getEnableScaling()
{
    return m_enableScaling;
}

double GreeterSetting::getScaleFactor()
{
    return m_scaleFactor;
}

void GreeterSetting::dumpGreeterSetting()
{
    qInfo() << "load greeter setting result:";
    qInfo() << "    BackgroundURI:      " << m_backgroundPath;
    qInfo() << "    UserListHiding:     " << m_userlistHiding;
    qInfo() << "    EnableManualLogin:  " << m_enableManualLogin;
    qInfo() << "    EnableScaling:      " << m_enableScaling;
    qInfo() << "    ScaleFactor:        " << m_scaleFactor;
}

GreeterSetting::GreeterSetting()
    :m_backgroundPath(DEFAULT_BACKGROUND_URI)
    ,m_userlistHiding(DEFAULT_USER_LIST_HIDING)
    ,m_enableManualLogin(DEFAULT_ENABLE_MANUAL_LOGIN)
    ,m_enableScaling(SCALING_AUTO)
    ,m_scaleFactor(DEFAULT_SCALE_FACTOR)
{
    load();
}
