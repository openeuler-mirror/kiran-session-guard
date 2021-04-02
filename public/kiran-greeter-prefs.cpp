//
// Created by lxh on 2020/12/30.
//

#include "kiran-greeter-prefs.h"

#include <QDBusConnection>

KiranGreeterPrefs *KiranGreeterPrefs::instance()
{
    static QMutex                            mutex;
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
    : ComKylinsecKiranSystemDaemonGreeterSettingsInterface(ComKylinsecKiranSystemDaemonGreeterSettingsInterface::staticInterfaceName(),
                                                           "/com/kylinsec/Kiran/SystemDaemon/GreeterSettings",
                                                           QDBusConnection::systemBus())
{
    QDBusConnection::systemBus().connect("com.kylinsec.Kiran.SystemDaemon",
                                         "/com/kylinsec/Kiran/SystemDaemon/GreeterSettings",
                                         "org.freedesktop.DBus.Properties",
                                         "PropertiesChanged",
                                         this, SLOT(handlePropertiesChanged(QDBusMessage)));
}

KiranGreeterPrefs::~KiranGreeterPrefs()
{
}

void KiranGreeterPrefs::handlePropertiesChanged(QDBusMessage msg)
{
    QList<QVariant> arguments    = msg.arguments();
    QVariantMap     changedProps = qdbus_cast<QVariantMap>(arguments.at(1).value<QDBusArgument>());
    for (auto iter = changedProps.begin(); iter != changedProps.end(); iter++)
    {
        emit propertyChanged(iter.key(), iter.value());
    }
}