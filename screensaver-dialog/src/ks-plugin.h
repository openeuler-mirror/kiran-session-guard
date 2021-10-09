//
// Created by lxh on 2021/9/17.
//

#ifndef KIRAN_SESSION_GUARD_SCREENSAVER_DIALOG_SRC_KS_PLUGIN_H_
#define KIRAN_SESSION_GUARD_SCREENSAVER_DIALOG_SRC_KS_PLUGIN_H_

#include <kiran-screensaver/ks-interface.h>
#include <kiran-screensaver/ks-plugin-interface.h>

class QTranslator;
class KSPlugin : public QObject,public KSPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID KSPluginInterface_iid)
    Q_INTERFACES(KSPluginInterface)
public:
    KSPlugin();
    ~KSPlugin() override;

    int init(KSInterface* ksInterface) override;
    void uninit() override;

    KSLockerInterface* createLocker() override;

private:
    KSInterface* m_ksInterface = nullptr;
    QTranslator* m_translator = nullptr;
};

#endif  //KIRAN_SESSION_GUARD_SCREENSAVER_DIALOG_SRC_KS_PLUGIN_H_
