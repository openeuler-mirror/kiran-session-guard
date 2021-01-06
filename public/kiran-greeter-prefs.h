/*
 * @file   kiran-greeter-prefs.h
 * @brief  对DBus后端GreeterSettings设置的一层封装
 * @author liuxinhao <liuxinhao@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved.
 */
#ifndef LIGHTDM_KIRAN_GREETER_KIRAN_GREETER_PREFS_H
#define LIGHTDM_KIRAN_GREETER_KIRAN_GREETER_PREFS_H

#include "greeter-settings-dbus-proxy.h"

class KiranGreeterPrefs : public ComKylinsecKiranSystemDaemonGreeterSettingsInterface{
    Q_OBJECT
public:
    static KiranGreeterPrefs* instance();
    ~KiranGreeterPrefs();

private:
    KiranGreeterPrefs();

signals:
    void propertyChanged(QString propertyName,QVariant property);

private Q_SLOTS:
    void handlePropertiesChanged(QDBusMessage msg);

};


#endif //LIGHTDM_KIRAN_GREETER_KIRAN_GREETER_PREFS_H
