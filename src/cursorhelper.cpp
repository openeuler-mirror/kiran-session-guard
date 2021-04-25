#include <QApplication>
#include <QDebug>
#include <QX11Info>
#include <QtMath>

#include <X11/Xcursor/Xcursor.h>
#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <X11/extensions/Xfixes.h>

#include "cursorhelper.h"
#include "log.h"

static unsigned long loadCursorHandle(Display *dpy, const char *name, int size)
{
    if (size == -1)
    {
        size = XcursorGetDefaultSize(dpy);
        LOG_DEBUG("load default cursor size(%d)",size);
    }
    XcursorImages *images = nullptr;
    images = XcursorLibraryLoadImages(name, "Adwaita", size);
    if (!images)
    {
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
        LOG_ERROR("can't open display!");
        return false;
    }
    ///FIXME:因为特定情况下，获取默认大小会过大,暂时使用默认大小为24进行放大
    int defaultCursorSize = 24;
    double tmpSize = defaultCursorSize * scaleFactor;
    int scaledCursorSize = floor(tmpSize);
    if (XcursorSetDefaultSize(dpy, scaledCursorSize) == XcursorTrue)
    {
        LOG_DEBUG("set default cursor size(%d) success!",scaledCursorSize);
        bRet = true;
    }
    else
    {
        LOG_WARNING("set default cursor size(%d) failed!",scaledCursorSize);
    }
    return bRet;
}

bool CursorHelper::setRootWindowWatchCursor()
{
    Display *display = XOpenDisplay(NULL);
    if (!display)
    {
        LOG_WARNING("can't open display!");
        return false;
    }
    Cursor cursor = (Cursor)loadCursorHandle(display, "watch", -1);
    if (!cursor)
    {
        XCloseDisplay(display);
        return false;
    }
    XDefineCursor(display, XDefaultRootWindow(display), cursor);
    XFixesChangeCursorByName(display, cursor, "watch");
    XFreeCursor(display, cursor);
    XCloseDisplay(display);
    return true;
}
