/*
 * @file   kiran-greeter-prefs.h
 * @brief  获取登录相关配置项的信息
 * @author liuxinhao <liuxinhao@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved.
 */
#ifndef LIGHTDM_KIRAN_GREETER_KIRAN_GREETER_PREFS_H
#define LIGHTDM_KIRAN_GREETER_KIRAN_GREETER_PREFS_H

#include "greeter-dbus-interface.h"

class KiranGreeterPrefs : public GreeterDBusInterface
{
    Q_OBJECT
public:
    static KiranGreeterPrefs* instance();
    ~KiranGreeterPrefs();

    QStringList hiddenSessions();
    QStringList hiddenUsers();
    bool canPowerOff();
    bool canReboot();
    bool canSuspend();
    bool canHibernate();

private:
    KiranGreeterPrefs();

signals:
    void propertyChanged(QString propertyName, QVariant property);

private Q_SLOTS:
    void handlePropertiesChanged(QDBusMessage msg);

private:
    QStringList m_hiddenSessions;
    QStringList m_hiddenUsers;
    bool m_canPowerOff = true;
    bool m_canReboot = true;
    bool m_canSuspend = true;
    bool m_canHibernate = true;
};

#endif  //LIGHTDM_KIRAN_GREETER_KIRAN_GREETER_PREFS_H
