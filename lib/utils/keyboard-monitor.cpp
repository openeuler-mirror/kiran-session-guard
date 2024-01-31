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

#include "keyboard-monitor.h"

#include <QDebug>
#include <QX11Info>

#include <X11/X.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XI2proto.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>
#include <xkbcommon/xkbcommon-keysyms.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace Kiran
{
namespace SessionGuard
{
int KeyboardMonitor::getXiMajorVersion(Display *display)
{
    XExtensionVersion *version;
    static int vers = -1;

    if (vers != -1)
    {
        return vers;
    }

    version = XGetExtensionVersion(display, INAME);
    if (!version || (version ==  reinterpret_cast<XExtensionVersion *>(NoSuchExtension)))
    {
        vers = 0;
        return vers;
    }

    vers = version->major_version;
    XFree(version);
    return vers;
}

void KeyboardMonitor::selectEvents(Display *display)
{
    XIEventMask evmask;
    unsigned char mask[XIMaskLen(XI_LASTEVENT)] = {0};

    memset(&evmask, 0, sizeof(evmask));
    memset(mask, 0, sizeof(mask));

    XISetMask(mask, XI_RawKeyRelease);

    evmask.deviceid = XIAllMasterDevices;
    evmask.mask_len = sizeof(mask);
    evmask.mask = mask;

    XISelectEvents(display, DefaultRootWindow(display), &evmask, 1);
    XFlush(display);
}

int KeyboardMonitor::listenXiEvent(Display *display)
{
    // Window root = DefaultRootWindow(display);

    while (1)
    {
        XEvent ev;
        XGenericEventCookie *cookie = (XGenericEventCookie *)&ev.xcookie;
        XNextEvent(display, (XEvent *)&ev);

        if (XGetEventData(display, cookie) && cookie->type == GenericEvent &&
            cookie->extension == m_xi2Opcode)
        {
            XIRawEvent *event = static_cast<XIRawEvent *>(cookie->data);
            switch (cookie->evtype)
            {
            case XI_RawKeyRelease:
            {
                KeySym sym = XkbKeycodeToKeysym(display, event->detail, 0, 0);
                if (sym == XKB_KEY_Caps_Lock)
                {
                    emit capslockStatusChanged(isCapslockOn());
                }
                else if (sym == XKB_KEY_Num_Lock)
                {
                    emit this->numlockStatusChanged(isNumlockOn());
                }
                break;
            }
            default:
                break;
            }
        }

        XFreeEventData(display, cookie);
    }
    return EXIT_SUCCESS;
}

KeyboardMonitor::KeyboardMonitor() : QThread()
{
}

KeyboardMonitor *KeyboardMonitor::instance()
{
    static KeyboardMonitor *KeyboardMonitorInstance = nullptr;

    if (!KeyboardMonitorInstance)
    {
        KeyboardMonitorInstance = new KeyboardMonitor;
    }

    return KeyboardMonitorInstance;
}

bool KeyboardMonitor::isCapslockOn()
{
    bool result;
    unsigned int n = 0;
    static Display *d = QX11Info::display();

    XkbGetIndicatorState(d, XkbUseCoreKbd, &n);
    result = (n & 0x01) != 0;

    return result;
}

bool KeyboardMonitor::isNumlockOn()
{
    bool result;
    unsigned int n = 0;
    static Display *d = QX11Info::display();

    XkbGetIndicatorState(d, XkbUseCoreKbd, &n);
    result = (n & 0x02) != 0;

    return result;
}

bool KeyboardMonitor::setNumlockStatus(const bool &on)
{
    Display *d = QX11Info::display();

    bool result =
        XkbLockModifiers(d, XkbUseCoreKbd, Mod2Mask, on ? Mod2Mask : 0);
    XFlush(d);

    return result;
}

void KeyboardMonitor::run()
{
    Display *display = XOpenDisplay(NULL);
    int event, error;

    if (!XQueryExtension(display, "XInputExtension", &m_xi2Opcode, &event,
                         &error))
    {
        fprintf(stderr, "XInput2 not available.\n");
        return;
    }

    if (!getXiMajorVersion(display))
    {
        fprintf(stderr, "XInput2 extension not available\n");
        return;
    }

    selectEvents(display);
    listenXiEvent(display);
}

}  // namespace SessionGuard
}  // namespace Kiran