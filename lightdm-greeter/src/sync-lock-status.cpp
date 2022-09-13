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

#include <QX11Info>
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <string.h>

#include "sync-lock-status.h"

unsigned int xkb_mask_modifier(XkbDescPtr xkb, const char *name)
{
    int i;
    if (!xkb || !xkb->names)
        return 0;
    for (i = 0;
         i < XkbNumVirtualMods;
         i++)
    {
        char *modStr = XGetAtomName(xkb->dpy, xkb->names->vmods[i]);
        if (modStr != NULL && strcmp(name, modStr) == 0)
        {
            unsigned int mask;
            XkbVirtualModsToReal(xkb, 1 << i, &mask);
            return mask;
        }
    }
    return 0;
}

unsigned int xkb_numlock_mask(Display *dpy)
{
    XkbDescPtr xkb;
    if ((xkb = XkbGetKeyboard(dpy, XkbAllComponentsMask, XkbUseCoreKbd)) != NULL)
    {
        unsigned int mask = xkb_mask_modifier(xkb, "NumLock");
        XkbFreeKeyboard(xkb, 0, True);
        return mask;
    }
    return 0;
}

unsigned int xkb_capslock_mask(Display *dpy)
{
    XkbDescPtr xkb;
    if ((xkb = XkbGetKeyboard(dpy, XkbAllComponentsMask, XkbUseCoreKbd)) != NULL)
    {
        unsigned int mask = xkb_mask_modifier(xkb, "CapsLock");
        XkbFreeKeyboard(xkb, 0, True);
        return mask;
    }
    return 0;
}

bool initLockStatus(bool numLockOn)
{
    Display *   display = QX11Info::display();
    XkbStateRec xkbState;

    if (!display)
    {
        return false;
    }

    //numlock
    int mask = xkb_numlock_mask(display);
    XkbGetState(display, XkbUseCoreKbd, &xkbState);
    unsigned int numlockState = xkbState.locked_mods & mask;
    if( numLockOn )//强制打开NumLock
    {
        XkbLockModifiers(display, XkbUseCoreKbd, mask, mask);
    }
    else
    {
        XkbLockModifiers(display, XkbUseCoreKbd, mask, numlockState);
    }

    //capslock
    mask = xkb_capslock_mask(display);
    XkbGetState(display, XkbUseCoreKbd, &xkbState);
    unsigned int capslockState = xkbState.locked_mods & mask;
    XkbLockModifiers(display, XkbUseCoreKbd, mask, 0);

    return true;
}