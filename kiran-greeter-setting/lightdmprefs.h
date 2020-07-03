#ifndef LIGHTDMPREFS_H
#define LIGHTDMPREFS_H

#include <QObject>
#include <glib.h>
#include <QSettings>

class LightdmPrefs : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString autoLoginUser READ autoLoginUser WRITE setAutoLoginUser NOTIFY autoLoginUserChanged)
    Q_PROPERTY(QString autoLoginDelay READ autoLoginDelay WRITE setAutoLoginDelay NOTIFY autoLoginDelayChanged)
    Q_PROPERTY(bool enableManualLogin READ enableManualLogin WRITE setEnableManualLogin NOTIFY enableManualLoginChanged)
    Q_PROPERTY(bool hideUserList READ hideUserList WRITE setHideUserList NOTIFY hideUserListChanged)
    Q_PROPERTY(QString scaleMode READ scaleMode WRITE setScaleMode NOTIFY scaleModeChanged)
    Q_PROPERTY(QString scaleFactor READ scaleFactor WRITE setScaleFactor NOTIFY scaleFactorChanged)
    Q_PROPERTY(QString greeterBackground READ greeterBackground WRITE setGreeterBackground NOTIFY greeterBackgroundChanged)
public:
    enum EnableScalingEnum{
        SCALING_AUTO = 0,
        SCALING_ENABLE,
        SCALING_DISABLE,
        SCALING_LAST,
    };
    static LightdmPrefs* instance();
    ~LightdmPrefs();
    QString autoLoginUser() const;
    QString autoLoginDelay() const;
    bool enableManualLogin() const;
    bool hideUserList() const;
    QString scaleMode() const;
    QString scaleFactor() const;
    QString greeterBackground() const;
private:
    explicit LightdmPrefs(QObject *parent = nullptr);
    void load();
    void save();
    static  QVariant getLightdmConf(const char* key,QVariant defaultValue);
signals:
    void autoLoginUserChanged(QString autoLoginUser);
    void autoLoginDelayChanged(QString autoLoginDelay);
    void enableManualLoginChanged(bool enableManualLogin);
    void hideUserListChanged(bool hideUserList);
    void scaleModeChanged(QString scaleMode);
    void scaleFactorChanged(QString scaleFactor);
    void greeterBackgroundChanged(QString greeterBackground);
public slots:
    void setAutoLoginUser(QString autoLoginUser);
    void setAutoLoginDelay(QString autoLoginDelay);
    void setEnableManualLogin(bool enableManualLogin);
    void setHideUserList(bool hideUserList);
    void setScaleMode(QString scaleMode);
    void setScaleFactor(QString scaleFactor);
    void setGreeterBackground(QString greeterBackground);
private:
    GKeyFile* m_lightdmConf;
    QSettings m_greeterConf;
    QString m_autoLoginUser;
    QString m_autoLoginDelay;
    bool m_enableManualLogin;
    bool m_hideUserList;
    QString m_scaleMode;
    QString m_scaleFactor;
    QString m_greeterBackground;
};

#endif // LIGHTDMPREFS_H
