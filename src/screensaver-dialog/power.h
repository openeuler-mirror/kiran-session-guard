#pragma once
#include <QObject>
#include "guard-global.h"

QT_BEGIN_NAMESPACE
class QDBusInterface;
QT_END_NAMESPACE

GUARD_LOCKER_BEGIN_NAMESPACE
class Prefs;
class Power : public QObject
{
    Q_OBJECT
private:
    explicit Power(Prefs* prefs, QObject* parent = nullptr);
public:
    ~Power();

    static void globalInit(Prefs* prefs);
    static void globalDeinit() { delete m_instance; };
    static Power* getInstance() { return m_instance; };

    bool canPoweroff();
    bool canReboot();
    bool canSuspend();

    bool poweroff();
    bool reboot();
    bool suspend();

private:
    static Power* m_instance;
    Prefs* m_prefs;
    QDBusInterface* m_loginProxy;
    QDBusInterface* m_smProxy;
};
GUARD_LOCKER_END_NAMESPACE