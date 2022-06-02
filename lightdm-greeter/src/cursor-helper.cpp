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

#include <QApplication>
#include <QDebug>
#include <QX11Info>
#include <QtMath>
#include <qt5-log-i.h>

#include <X11/Xcursor/Xcursor.h>
#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <X11/extensions/Xfixes.h>

#include "cursor-helper.h"

static const char* const CURSOR_THEME = "Adwaita";
///FIXME:因为特定情况下，登陆器获取默认光标大小会过大,暂时使用默认大小为24进行放大
static const int GREETER_DEFAULT_CURSOR_SIZE = 24;

static unsigned long loadCursorHandle(Display *dpy, const char *name, int size)
{
    if (size == -1)
    {
        size = XcursorGetDefaultSize(dpy);
        KLOG_DEBUG("load default cursor size(%d)",size);
    }
    XcursorImages *images = nullptr;
    images = XcursorLibraryLoadImages(name, CURSOR_THEME, size);
    if (!images)
    {
        KLOG_ERROR("load cursor image %s(size:%d) from theme(%s) failed!",name,size,CURSOR_THEME);
        return 0;
    }
    unsigned long handle = (unsigned long)XcursorImagesLoadCursor(dpy, images);
    XcursorImagesDestroy(images);
    return handle;
}

bool CursorHelper::setDefaultCursorSize(double scaleFactor)
{
    bool bRet = false;
    Display *dpy = QX11Info::display();
    if (dpy == nullptr)
    {
        KLOG_ERROR("can't open display!");
        return false;
    }

    double tmpSize = GREETER_DEFAULT_CURSOR_SIZE * scaleFactor;
    int scaledCursorSize = floor(tmpSize);
    if (XcursorSetDefaultSize(dpy, scaledCursorSize) == XcursorTrue)
    {
        KLOG_DEBUG("set greeter default cursor size(%d) success!",scaledCursorSize);
        bRet = true;
    }
    else
    {
        KLOG_ERROR("set greeter default cursor size(%d) failed!",scaledCursorSize);
    }

    return bRet;
}

bool CursorHelper::setRootWindowWatchCursor()
{
    Display *display = XOpenDisplay(NULL);
    if (!display)
    {
        KLOG_ERROR("can't open display!");
        return false;
    }
    Cursor cursor = (Cursor)loadCursorHandle(display, "watch", -1);
    if (!cursor)
    {
        KLOG_ERROR() << "can load 'watch' cursor!";
        XCloseDisplay(display);
        return false;
    }
    XDefineCursor(display, XDefaultRootWindow(display), cursor);
    XFixesChangeCursorByName(display, cursor, "watch");
    XFreeCursor(display, cursor);
    XCloseDisplay(display);
    return true;
}
