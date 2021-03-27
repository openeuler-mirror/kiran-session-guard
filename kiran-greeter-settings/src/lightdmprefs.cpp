#include "lightdmprefs.h"
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include "greeter-setting-define.h"
#include <iostream>

#define LIGHTDM_CONF_PATH       "/etc/lightdm/lightdm.conf"
#define LIGHTDM_GROUP_NAME      "Seat:*"
#define KEY_AUTOLOGIN_USER      "autologin-user"
#define KEY_AUTOLOGIN_DELAY     "autologin-user-timeout"

static const QMap<QString, LightdmPrefs::EnableScalingEnum> ScalingEnumMap = {
        {"auto",    LightdmPrefs::SCALING_AUTO},
        {"manual",  LightdmPrefs::SCALING_ENABLE},
        {"disable", LightdmPrefs::SCALING_DISABLE}
};

LightdmPrefs::LightdmPrefs (QObject *parent)
        : QObject(parent),
          m_lightdmConf(nullptr),
          m_greeterConf(GREETER_SETTING_PATH, QSettings::IniFormat)
{
    load();
}

void LightdmPrefs::load ()
{
    GError *error = nullptr;
    QVariant var;

    //lightdm.conf
    m_lightdmConf = g_key_file_new();
    if (!g_key_file_load_from_file(m_lightdmConf, LIGHTDM_CONF_PATH, G_KEY_FILE_KEEP_COMMENTS, &error))
    {
        qFatal("load key file %s failed,%s", LIGHTDM_CONF_PATH, error->message);
        g_error_free(error);
        error = nullptr;
        g_key_file_unref(m_lightdmConf);
        m_lightdmConf = nullptr;
        return;
    }
    var = getLightdmConf(KEY_AUTOLOGIN_USER, "");
    m_autoLoginUser = var.toString();

    var = getLightdmConf(KEY_AUTOLOGIN_DELAY, "");
    m_autoLoginDelay = var.toString();

    //ligthdm-kiran-greeter.conf
    m_greeterConf.beginGroup(GREETER_SETTING_GROUP);

    var = m_greeterConf.value(KEY_BACKGROUND_URI, "");
    m_greeterBackground = var.toString();

    var = m_greeterConf.value(KEY_USER_LIST_HIDING, DEFAULT_USER_LIST_HIDING);
    m_hideUserList = var.toBool();

    var = m_greeterConf.value(KEY_ENABLE_MANUAL_LOGIN, DEFAULT_ENABLE_MANUAL_LOGIN);
    m_enableManualLogin = var.toBool();

    var = m_greeterConf.value(KEY_ENABLE_SCALING, DEFAULT_ENABLE_SCALING);
    auto iter = ScalingEnumMap.find(var.toString());
    if (iter == ScalingEnumMap.end())
    {
        m_scaleMode = "auto";
    }
    else
    {
        m_scaleMode = var.toString();
    }

    var = m_greeterConf.value(KEY_SCALE_FACTOR, DEFAULT_SCALE_FACTOR);
    if (var.toInt() <= 1)
    {
        m_scaleFactor = "1";
    }
    else
    {
        m_scaleFactor = "2";
    }
}

QVariant LightdmPrefs::getLightdmConf (const char *key, QVariant defaultValue)
{
    QVariant var = defaultValue;
    static QVector<QString> pathVec = {
            "/usr/share/lightdm/lightdm.conf.d/",
            "/etc/lightdm/lightdm.conf.d/",
            "/etc/lightdm/lightdm.conf"
    };
    static QVector<QString> groupVec = {
            "Seat:*",
            "SeatDefaults"
    };
    QList<QString> confFiles;


    Q_FOREACH(QString path, pathVec)
        {
            QFileInfo confInfo(path);
            if (!confInfo.exists())
            {
                continue;
            }
            if (confInfo.isDir())
            {
                QDir dir = confInfo.dir();
                QFileInfoList files = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
                for (QFileInfo fileInfo:files)
                {
                    if (fileInfo.fileName().endsWith(".conf"))
                        confFiles.insert(0, fileInfo.absoluteFilePath());
                }
            }
            else
            {
                confFiles.insert(0, confInfo.absoluteFilePath());
            }
        }

    Q_FOREACH(QString file, confFiles)
        {
            GKeyFile *keyFile = nullptr;
            GError *error = nullptr;
            std::string filePath = file.toStdString();
            bool needBreak = false;

            keyFile = g_key_file_new();
            if (!g_key_file_load_from_file(keyFile, filePath.c_str(), G_KEY_FILE_KEEP_COMMENTS, &error))
            {
                qWarning() << "load key file" << filePath.c_str() << "failed," << error->message;
                g_error_free(error);
                g_key_file_unref(keyFile);
                continue;
            }

            Q_FOREACH(QString group, groupVec)
                {
                    std::string groupName = group.toStdString();
                    if (!g_key_file_has_group(keyFile, groupName.c_str()) ||
                        !g_key_file_has_key(keyFile, groupName.c_str(), key, nullptr))
                    {
                        continue;
                    }
                    if (defaultValue.type() == QVariant::Bool)
                    {
                        gboolean res = g_key_file_get_boolean(keyFile, groupName.c_str(), key, &error);
                        if (error)
                        {
                            qWarning() << "file:" << filePath.c_str()
                                       << "group:" << groupName.c_str()
                                       << "is not bool," << error->message;
                            g_error_free(error);
                            error = nullptr;
                            break;
                        }
                        qDebug() << filePath.c_str() << key << "-" << (res ? "true" : "false");
                        var.setValue<bool>(res ? true : false);
                        needBreak = true;
                    }
                    else if (defaultValue.type() == QVariant::String)
                    {
                        gchar *str = g_key_file_get_string(keyFile, groupName.c_str(), key, &error);
                        if (error)
                        {
                            qWarning() << "file:" << filePath.c_str()
                                       << "group:" << groupName.c_str()
                                       << "is not string," << error->message;
                            g_error_free(error);
                            error = nullptr;
                            break;
                        }
                        qDebug() << filePath.c_str() << key << "-" << str;
                        var.setValue<QString>(str);
                        g_free(str);
                        needBreak = true;
                    }
                    break;
                }
            g_key_file_unref(keyFile);
            if (needBreak)
                break;
        }
    return var;
}

LightdmPrefs *LightdmPrefs::instance ()
{
    static LightdmPrefs prefs;
    return &prefs;
}

LightdmPrefs::~LightdmPrefs ()
{
    if (m_lightdmConf)
    {
        GError *error = nullptr;
        if (!g_key_file_save_to_file(m_lightdmConf, LIGHTDM_CONF_PATH, &error))
        {
            std::cerr << "save failed " << LIGHTDM_CONF_PATH << "," << error->message << std::endl;
            g_error_free(error);
            error = nullptr;
        }
        g_key_file_unref(m_lightdmConf);
    }
//    m_greeterConf.sync();
}

QString LightdmPrefs::autoLoginUser () const
{
    return m_autoLoginUser;
}

QString LightdmPrefs::autoLoginDelay () const
{
    return m_autoLoginDelay;
}

bool LightdmPrefs::hideUserList () const
{
    return m_hideUserList;
}

QString LightdmPrefs::scaleMode () const
{
    return m_scaleMode;
}

QString LightdmPrefs::scaleFactor () const
{
    return m_scaleFactor;
}

QString LightdmPrefs::greeterBackground () const
{
    return m_greeterBackground;
}

bool LightdmPrefs::enableManualLogin () const
{
    return m_enableManualLogin;
}

void LightdmPrefs::setAutoLoginUser (QString autoLoginUser)
{
    if (m_autoLoginUser == autoLoginUser)
        return;
    if (m_lightdmConf == nullptr)
        return;
    GError *error = nullptr;
    if (autoLoginUser.isEmpty())
    {
        if (!g_key_file_remove_key(m_lightdmConf, LIGHTDM_GROUP_NAME,
                                   KEY_AUTOLOGIN_USER, &error))
        {
            qWarning() << "keyfile" << LIGHTDM_CONF_PATH << ",remove key"
                       << KEY_AUTOLOGIN_USER" error," << error->message;
            g_error_free(error);
            error = nullptr;
        }
    }
    else
    {
        std::string stdLoginUser = autoLoginUser.toStdString();
        g_key_file_set_string(m_lightdmConf, LIGHTDM_GROUP_NAME,
                              KEY_AUTOLOGIN_USER, stdLoginUser.c_str());
    }
    m_autoLoginUser = autoLoginUser;
    emit autoLoginUserChanged(m_autoLoginUser);
}

void LightdmPrefs::setAutoLoginDelay (QString autoLoginDelay)
{
    if (m_autoLoginDelay == autoLoginDelay)
        return;
    if (m_lightdmConf == nullptr)
        return;
    GError *error = nullptr;
    if (autoLoginDelay.isEmpty())
    {
        if (!g_key_file_remove_key(m_lightdmConf, LIGHTDM_GROUP_NAME,
                                   KEY_AUTOLOGIN_DELAY, &error))
        {
            qWarning() << "keyfile" << LIGHTDM_CONF_PATH << ",remove key"
                       << KEY_AUTOLOGIN_DELAY" error," << error->message;
            g_error_free(error);
            error = nullptr;
        }
    }
    else
    {
        std::string stdAutoLoginDelay = autoLoginDelay.toStdString();
        g_key_file_set_string(m_lightdmConf, LIGHTDM_GROUP_NAME,
                              KEY_AUTOLOGIN_DELAY, stdAutoLoginDelay.c_str());
    }
    m_autoLoginDelay = autoLoginDelay;
    emit autoLoginDelayChanged(m_autoLoginDelay);
}

void LightdmPrefs::setEnableManualLogin (bool enableManualLogin)
{
    if (m_enableManualLogin == enableManualLogin)
        return;

    if (!m_greeterConf.isWritable())
    {
        qInfo() << GREETER_SETTING_PATH << "can't write";
        return;
    }

    m_enableManualLogin = enableManualLogin;
    m_greeterConf.setValue(KEY_ENABLE_MANUAL_LOGIN, enableManualLogin);
    qInfo() << "set" << KEY_ENABLE_MANUAL_LOGIN << enableManualLogin;
    emit enableManualLoginChanged(m_enableManualLogin);
}

void LightdmPrefs::setHideUserList (bool hideUserList)
{
    if (m_hideUserList == hideUserList)
        return;

    if (!m_greeterConf.isWritable())
    {
        qInfo() << GREETER_SETTING_PATH << "can't write";
        return;
    }

    m_hideUserList = hideUserList;
    m_greeterConf.setValue(KEY_USER_LIST_HIDING, hideUserList);
    qInfo() << "set" << KEY_USER_LIST_HIDING << hideUserList;
    emit hideUserListChanged(m_hideUserList);
}

void LightdmPrefs::setScaleMode (QString scaleMode)
{
    if (m_scaleMode == scaleMode)
        return;

    if (!m_greeterConf.isWritable())
    {
        qInfo() << GREETER_SETTING_PATH << "can't write";
        return;
    }

    m_scaleMode = scaleMode;
    m_greeterConf.setValue(KEY_ENABLE_SCALING, scaleMode);
    qInfo() << "set" << KEY_ENABLE_SCALING << scaleMode;
    emit scaleModeChanged(m_scaleMode);
}

void LightdmPrefs::setScaleFactor (QString scaleFactor)
{
    if (m_scaleFactor == scaleFactor)
        return;

    if (!m_greeterConf.isWritable())
    {
        qInfo() << GREETER_SETTING_PATH << "can't write";
        return;
    }

    m_scaleFactor = scaleFactor;
    m_greeterConf.setValue(KEY_SCALE_FACTOR, scaleFactor);
    qInfo() << "set" << KEY_SCALE_FACTOR << scaleFactor;
    emit scaleFactorChanged(m_scaleFactor);
}

void LightdmPrefs::setGreeterBackground (QString greeterBackground)
{
    if (!m_greeterConf.isWritable())
    {
        qInfo() << GREETER_SETTING_PATH << "can't write";
        return;
    }

    m_greeterBackground = greeterBackground;
    m_greeterConf.setValue(KEY_BACKGROUND_URI, greeterBackground);
    qInfo() << "set" << KEY_BACKGROUND_URI << greeterBackground;
    emit greeterBackgroundChanged(m_greeterBackground);
}
