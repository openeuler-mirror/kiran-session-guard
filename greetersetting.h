#ifndef GREETERSETTING_H
#define GREETERSETTING_H

#include <QMap>
#include <QVariant>

class GreeterSetting
{
public:
    static GreeterSetting* instance();
    ~GreeterSetting() = default;
public:
    void load();
    QString getBackgroundPath();
    bool getUserListHiding();
    bool getEnableManualLogin();
    void dumpGreeterSetting();
private:
    GreeterSetting();
private:
    QString m_backgroundPath;
    bool m_userlistHiding;
    bool m_enableManualLogin;
};

#endif // GREETERSETTING_H
