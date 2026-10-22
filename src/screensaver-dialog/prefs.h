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
#pragma once


namespace Kiran
{
namespace SessionGuard
{
namespace Locker
{
class Prefs
{
private:
    Prefs();
    void init();

public:
    ~Prefs();

    static void globalInit();
    static void globalDeinit() { delete m_instance; };
    static Prefs* getInstance() { return m_instance; };

    bool canPowerOff();
    bool canReboot();
    bool canSuspend();
    bool showFullName();
    bool facePreviewEnabled() const;
    /**
     * @brief 锁屏是否自动启动认证
     * @return true 锁屏后自动 startAuthUser；false 仅显示「重新认证」按钮
     */
    bool autoStartAuth() const;

private:
    static Prefs* m_instance;
    bool m_canPowerOff = true;
    bool m_canReboot = true;
    bool m_canSuspend = true;
    bool m_showFullName = false;
    bool m_facePreviewEnabled = false;
    bool m_autoStartAuth = true;
};
}  // namespace Locker
}  // namespace SessionGuard
}  // namespace Kiran