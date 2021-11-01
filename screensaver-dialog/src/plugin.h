#ifndef __KIRAN_SCREENSAVER_PLUGIN_H__
#define __KIRAN_SCREENSAVER_PLUGIN_H__

#include <QObject>
#include <kiran-screensaver/plugin-interface.h>
#include <QTranslator>

namespace Kiran
{
namespace ScreenSaver
{
class Interface;
class LockerInterface;
}  // namespace ScreenSaver
}  // namespace Kiran

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

#endif