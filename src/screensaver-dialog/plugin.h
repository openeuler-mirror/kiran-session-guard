/**
 * Copyright (c) 2020 ~ 2023 KylinSec Co., Ltd.
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
#include <kiran-screensaver/plugin-interface.h>
#include <QObject>
#include <QTranslator>


// clang-format off
namespace Kiran { namespace ScreenSaver {class Interface;class LockerInterface;}}
// clang-format on

namespace Kiran
{
namespace SessionGuard
{
namespace Locker
{
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
}  // namespace Locker
}  // namespace SessionGuard
}  // namespace Kiran