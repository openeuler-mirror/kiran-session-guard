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

#ifndef KIRAN_SCREENSAVER_DIALOG_SRC_PREFS_H_
#define KIRAN_SCREENSAVER_DIALOG_SRC_PREFS_H_

class Prefs
{
public:
    static Prefs* instance();
    ~Prefs();

    bool canPowerOff();
    bool canReboot();
    bool canSuspend();

private:
    Prefs();

private:
    bool m_canPowerOff = true;
    bool m_canReboot = true;
    bool m_canSuspend = true;
};

#endif  //KIRAN_SCREENSAVER_DIALOG_SRC_PREFS_H_
