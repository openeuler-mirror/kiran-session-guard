//
// Created by lxh on 2020/12/30.
//

#include "kiran-greeter-prefs.h"

#include <QDBusConnection>
#include <QSettings>

KiranGreeterPrefs *KiranGreeterPrefs::instance()
{
    static QMutex mutex;
    static QScopedPointer<KiranGreeterPrefs> pInst;

    if (Q_UNLIKELY(!pInst))
    {
        QMutexLocker locker(&mutex);
        if (pInst.isNull())
        {
            pInst.reset(new KiranGreeterPrefs);
        }
    }

    return pInst.data();
}

KiranGreeterPrefs::KiranGreeterPrefs()
    : GreeterDBusInterface(GreeterDBusInterface::staticInterfaceName(),
                           GreeterDBusInterface::staticInterfacePath(),
                           QDBusConnection::systemBus())
{
    QDBusConnection::systemBus().connect(GreeterDBusInterface::staticInterfaceName(),
                                         GreeterDBusInterface::staticInterfacePath(),
                                         "org.freedesktop.DBus.Properties",
                                         "PropertiesChanged",
                                         this, SLOT(handlePropertiesChanged(QDBusMessage)));

    QSettings settings("/usr/share/lightdm-kiran-greeter/greeter.ini",QSettings::IniFormat);

    ///common
    settings.beginGroup("Common");

    auto hiddenSession = settings.value("hidden-sessions");
    m_hiddenSessions = hiddenSession.toStringList();

    auto hiddenUsers = settings.value("hide-users");
    m_hiddenUsers = hiddenUsers.toStringList();

    settings.endGroup();

    ///power
    settings.beginGroup("Power");

    auto canPowerOff = settings.value("can-poweroff");
    m_canPowerOff = canPowerOff.toBool();

    auto canReboot = settings.value("can-reboot");
    m_canReboot = canReboot.toBool();

    auto canSuspend = settings.value("can-suspend");
    m_canSuspend = canSuspend.toBool();

    auto canHibernate = settings.value("can-hibernate");
    m_canHibernate = canHibernate.toBool();
}

KiranGreeterPrefs::~KiranGreeterPrefs()
{
}

void KiranGreeterPrefs::handlePropertiesChanged(QDBusMessage msg)
{
    QList<QVariant> arguments = msg.arguments();
    QVariantMap changedProps = qdbus_cast<QVariantMap>(arguments.at(1).value<QDBusArgument>());
    for (auto iter = changedProps.begin(); iter != changedProps.end(); iter++)
    {
        emit propertyChanged(iter.key(), iter.value());
    }
}

QStringList KiranGreeterPrefs::hiddenSessions()
{
    return m_hiddenSessions;
}

QStringList KiranGreeterPrefs::hiddenUsers()
{
    return m_hiddenUsers;
}

bool KiranGreeterPrefs::canPowerOff()
{
    return m_canPowerOff;
}

bool KiranGreeterPrefs::canReboot()
{
    return m_canReboot;
}

bool KiranGreeterPrefs::canSuspend()
{
    return m_canSuspend;
}

bool KiranGreeterPrefs::canHibernate()
{
    return m_canHibernate;
}