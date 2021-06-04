//
// Created by lxh on 2021/6/1.
//

#ifndef LIGHTDM_KIRAN_GREETER_KIRAN_CPANEL_GREETER_SRC_KCP_INTERFACE_H_
#define LIGHTDM_KIRAN_GREETER_KIRAN_CPANEL_GREETER_SRC_KCP_INTERFACE_H_

#include <kcp-plugin-interface.h>

class QTranslator;
class KcpInterface : public QObject,public KcpPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID KcpPluginInterface_iid)
    Q_INTERFACES(KcpPluginInterface)
public:
    KcpInterface();
    ~KcpInterface() override;

    int init() override;
    void uninit() override;

    QWidget* getSubItemWidget(QString subItemName) override;
    bool haveUnsavedOptions() override;
    QStringList visibleSubItems() override;

private:
    QWidget* m_currentWidget = nullptr;
    QTranslator* m_translator = nullptr;
};

#endif  //LIGHTDM_KIRAN_GREETER_KIRAN_CPANEL_GREETER_SRC_KCP_INTERFACE_H_
