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
#include <QThread>
#include <QX11Info>
#include "guard-global.h"

GUARD_BEGIN_NAMESPACE
class KeyboardMonitor : public QThread {
    Q_OBJECT

public:
    static KeyboardMonitor *instance();

    bool isCapslockOn();
    bool isNumlockOn();

    bool setNumlockStatus(const bool &on);
    
signals:
    void capslockStatusChanged(bool on);
    void numlockStatusChanged(bool on);

protected:
    void run() override;

private:
    KeyboardMonitor();
    int getXiMajorVersion(Display* display);
    void selectEvents(Display *display);
    int listenXiEvent(Display *display);

private:
    int m_xi2Opcode = 0;
};
GUARD_END_NAMESPACE