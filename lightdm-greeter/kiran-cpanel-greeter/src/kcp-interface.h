/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd.
 * kiran-session-guard is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     liuxinhao <liuxinhao@kylinos.com.cn>
 */

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
