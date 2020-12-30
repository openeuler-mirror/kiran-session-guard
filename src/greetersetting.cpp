#include "greetersetting.h"
#include <QSettings>
#include <QtDebug>
#include <QFile>
#include <QVector>
#include <QMap>
#include "greeter-setting-define.h"

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

    QVariant messageInterval = setting.value(KEY_MESSAGE_INTERVAL,DEFAULT_MESSAGE_INTERVAL);
    bool isOK;
    m_messageDisplayInterval = messageInterval.toInt(&isOK);
    if(!isOK){
        m_messageDisplayInterval = DEFAULT_MESSAGE_INTERVAL;
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

int GreeterSetting::messageDisplayInterval()
{
    return m_messageDisplayInterval;
}

void GreeterSetting::dumpGreeterSetting()
{
    qInfo() << "load greeter setting result:";
    qInfo() << "    BackgroundURI:      " << m_backgroundPath;
    qInfo() << "    UserListHiding:     " << m_userlistHiding;
    qInfo() << "    EnableManualLogin:  " << m_enableManualLogin;
    qInfo() << "    EnableScaling:      " << m_enableScaling;
    qInfo() << "    ScaleFactor:        " << m_scaleFactor;
    qInfo() << "    MessageInterval:    " << m_messageDisplayInterval;
}

GreeterSetting::GreeterSetting()
    :m_backgroundPath(DEFAULT_BACKGROUND_URI)
    ,m_userlistHiding(DEFAULT_USER_LIST_HIDING)
    ,m_enableManualLogin(DEFAULT_ENABLE_MANUAL_LOGIN)
    ,m_enableScaling(SCALING_AUTO)
    ,m_scaleFactor(DEFAULT_SCALE_FACTOR)
    ,m_messageDisplayInterval(DEFAULT_MESSAGE_INTERVAL)
{
    load();
}
