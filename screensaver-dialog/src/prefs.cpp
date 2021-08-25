//
// Created by lxh on 2021/8/4.
//

#include "prefs.h"
#include <QMutex>
#include <QScopedPointer>
#include <QSettings>

Prefs* Prefs::instance()
{
    static QMutex mutex;
    static QScopedPointer<Prefs> pInst;

    if (Q_UNLIKELY(!pInst))
    {
        QMutexLocker locker(&mutex);
        if (pInst.isNull())
        {
            pInst.reset(new Prefs);
        }
    }
    return pInst.data();
}

Prefs::~Prefs()
{
}

Prefs::Prefs()
{
    QSettings settings("/usr/share/kiran-screensaver-dialog/screensaver-dialog.ini",QSettings::IniFormat);
    ///power
    settings.beginGroup("Power");

    auto canPowerOff = settings.value("can-poweroff");
    m_canPowerOff = canPowerOff.toBool();

    auto canReboot = settings.value("can-reboot");
    m_canReboot = canReboot.toBool();

    auto canSuspend = settings.value("can-suspend");
    m_canSuspend = canSuspend.toBool();
}

bool Prefs::canPowerOff()
{
    return m_canPowerOff;
}

bool Prefs::canReboot()
{
    return m_canReboot;
}

bool Prefs::canSuspend()
{
    return m_canSuspend;
}
