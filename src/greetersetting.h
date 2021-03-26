#ifndef GREETERSETTING_H
#define GREETERSETTING_H

#include <QMap>
#include <QVariant>

class GreeterSetting
{
public:
    enum EnableScalingEnum
    {
        SCALING_AUTO = 0,
        SCALING_ENABLE,
        SCALING_DISABLE,
        SCALING_LAST,
    };
public:
    static GreeterSetting *instance ();
    ~GreeterSetting () = default;
public:
    void load ();
    QString getBackgroundPath ();
    bool getUserListHiding ();
    bool getEnableManualLogin ();
    EnableScalingEnum getEnableScaling ();
    double getScaleFactor ();
    int messageDisplayInterval ();
    void dumpGreeterSetting ();
private:
    GreeterSetting ();
private:
    QString m_backgroundPath;
    bool m_userlistHiding;
    bool m_enableManualLogin;
    EnableScalingEnum m_enableScaling;
    double m_scaleFactor;
    int m_messageDisplayInterval = 3;
};

#endif // GREETERSETTING_H
