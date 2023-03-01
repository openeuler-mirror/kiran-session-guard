#pragma once
#include <QObject>
#include <QTranslator>
#include <kiran-screensaver/plugin-interface.h>
#include "guard-global.h"

// clang-format off
namespace Kiran { namespace ScreenSaver {class Interface;class LockerInterface;}}
// clang-format on

GUARD_LOCKER_BEGIN_NAMESPACE
class Prefs;
class Power;
class KSPlugin : public QObject, public Kiran::ScreenSaver::PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PluginInterface_iid)
    Q_INTERFACES(Kiran::ScreenSaver::PluginInterface)

public:
    KSPlugin() = default;
    ~KSPlugin() = default;

    int init(Kiran::ScreenSaver::Interface* ksInterface) override;
    void uninit() override;
    Kiran::ScreenSaver::LockerInterface* createLocker() override;

private:
    Kiran::ScreenSaver::Interface* m_ksInterface = nullptr;
    QTranslator* m_translator = nullptr;
    
};
GUARD_LOCKER_END_NAMESPACE