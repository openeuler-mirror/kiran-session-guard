#include "greetersetting.h"
#include <QSettings>
#include <QtDebug>
#include <QFile>

#define GREETER_SETTING_PATH          "/etc/lightdm/lightdm-kiran-greeter.conf"
#define GREETER_SETTING_GROUP         "Greeter"

#define  KEY_BACKGROUND_URI 		  "background-picture-uri"
#define  DEFAULT_BACKGROUND_URI       ":/images/default_background.png"

#define  KEY_USER_LIST_HIDING 		  "user-list-hiding"
#define  DEFAULT_USER_LIST_HIDING     false

#define  KEY_ENABLE_MANUAL_LOGIN 	  "enable-manual-login"
#define  DEFAULT_ENABLE_MANUAL_LOGIN   true

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
    settingsFile.open(QIODevice::ReadOnly);
    if( !settingsFile.isReadable() ){
        qInfo() << GREETER_SETTING_PATH << " can't read";
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

void GreeterSetting::dumpGreeterSetting()
{
    qInfo() << "load greeter setting result:";
    qInfo() << "    BackgroundURI:      " << m_backgroundPath;
    qInfo() << "    UserListHiding:     " << m_userlistHiding;
    qInfo() << "    EnableManualLogin:  " << m_enableManualLogin;
}

GreeterSetting::GreeterSetting()
    :m_backgroundPath(DEFAULT_BACKGROUND_URI)
    ,m_userlistHiding(DEFAULT_USER_LIST_HIDING)
    ,m_enableManualLogin(DEFAULT_ENABLE_MANUAL_LOGIN)
{
    load();
}
