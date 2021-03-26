#include "cursorhelper.h"
#include <QDebug>
#include <QApplication>
#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <X11/Xcursor/Xcursor.h>
#include <X11/extensions/Xfixes.h>
#include <QX11Info>
#include <QtMath>

static unsigned long loadCursorHandle (Display *dpy, const char *name, int size)
{
    if (size == -1)
    {
        size = XcursorGetDefaultSize(dpy);
        qInfo() << "loadCursorHandle GetDefaultSize" << size;
    }
    XcursorImages *images = nullptr;
    images = XcursorLibraryLoadImages(name, "Adwaita", size);
    if (!images)
    {
        return 0;
    }
    unsigned long handle = (unsigned long) XcursorImagesLoadCursor(dpy, images);
    XcursorImagesDestroy(images);
    return handle;
}

bool CursorHelper::setDefaultCursorSize (double scaleFactor)
{
    bool bRet = false;
    Display *dpy = QX11Info::display();
    if (dpy == nullptr)
    {
        qWarning() << "QX11Info::display == nullptr";
        return false;
    }
    ///FIXME:因为特定情况下，获取默认大小会过大,暂时使用默认大小为24进行放大
    int defaultCursorSize = 24;
    double tmpSize = defaultCursorSize * scaleFactor;
    int scaledCursorSize = floor(tmpSize);
    if (XcursorSetDefaultSize(dpy, scaledCursorSize) == XcursorTrue)
    {
        qInfo() << "XcursorSetDefaultSize" << scaledCursorSize << "success";
        bRet = true;
    }
    else
    {
        qWarning() << "XcursorSetDefaultSize" << scaledCursorSize << "failed";
    }
    return bRet;
}

bool CursorHelper::setRootWindowWatchCursor ()
{
    Display *display = XOpenDisplay(NULL);
    if (!display)
    {
        qDebug() << "Open display failed";
        return false;
    }
    Cursor cursor = (Cursor) loadCursorHandle(display, "watch", -1);
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
