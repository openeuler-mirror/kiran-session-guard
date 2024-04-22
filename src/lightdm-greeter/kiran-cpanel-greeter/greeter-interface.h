/**
 * Copyright (c) 2020 ~ 2024 KylinSec Co., Ltd.
 * kiran-session-guard is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     liuxinhao <liuxinhao@kylinsec.com.cn>
 */
#pragma once
#include <QObject>
#include <plugin-interface-v2.h>
#include <panel-interface.h>

class QTranslator;
namespace Kiran
{
namespace SessionGuard
{
class GreeterInterface: public QObject,public KiranControlPanel::PluginInterfaceV2
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID KiranControlPanel_PluginInterfaceV2_iid)
    Q_INTERFACES(KiranControlPanel::PluginInterfaceV2)
public:
    GreeterInterface(QObject* parent = nullptr);
    ~GreeterInterface();

    virtual int init(KiranControlPanel::PanelInterface* interface) override;
    virtual void uninit() override;

    // 功能项数组，生存周期由插件维护
    // 功能项发生变更时，应调用init时传入KcpInterface接口，通知主面板相关信息变更,及时加载新的功能项信息
    virtual QVector<KiranControlPanel::SubItemPtr> getSubItems() override;

private:
    KiranControlPanel::PanelInterface* m_panelInterface = nullptr;
    QTranslator* m_translator = nullptr;
    KiranControlPanel::SubItemPtr m_subitem;
};
}
}