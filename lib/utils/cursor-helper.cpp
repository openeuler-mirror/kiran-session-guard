#include "cursor-helper.h"
#include <QDebug>
#include <QX11Info>
#include <QtMath>
#include <X11/Xcursor/Xcursor.h>
#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <X11/extensions/Xfixes.h>

static const char *const CURSOR_THEME = "Adwaita";
static const int GREETER_DEFAULT_CURSOR_SIZE = 24;
static unsigned long loadCursorHandle(Display *dpy, const char *name, int size)
{
    if (size == -1)
    {
        size = XcursorGetDefaultSize(dpy);
        qDebug("load default cursor size(%d)", size);
    }
    XcursorImages *images = nullptr;
    images = XcursorLibraryLoadImages(name, CURSOR_THEME, size);
    if (!images)
    {
        qErrnoWarning("load cursor image %s(size:%d) from theme(%s) failed!", name, size, CURSOR_THEME);
        return 0;
    }
    unsigned long handle = (unsigned long)XcursorImagesLoadCursor(dpy, images);
    XcursorImagesDestroy(images);
    return handle;
}

GUARD_BEGIN_NAMESPACE
namespace CursorHelper
{
bool setDefaultCursorSize(double scaleFactor)
{
    bool bRet = false;
    Display *dpy = QX11Info::display();
    if (dpy == nullptr)
    {
        qErrnoWarning("can't open display!");
        return false;
    }

    double tmpSize = GREETER_DEFAULT_CURSOR_SIZE * scaleFactor;
    int scaledCursorSize = floor(tmpSize);
    if (XcursorSetDefaultSize(dpy, scaledCursorSize) == XcursorTrue)
    {
        qDebug("set greeter default cursor size(%d) success!", scaledCursorSize);
        bRet = true;
    }
    else
    {
        qDebug("set greeter default cursor size(%d) failed!", scaledCursorSize);
    }

    return bRet;
}

bool setRootWindowWatchCursor()
{
    Display *display = XOpenDisplay(NULL);
    if (!display)
    {
        qErrnoWarning("can't open display!");
        return false;
    }
    Cursor cursor = (Cursor)loadCursorHandle(display, "watch", -1);
    if (!cursor)
    {
        qWarning() << "can load 'watch' cursor!";
        XCloseDisplay(display);
        return false;
    }
    XDefineCursor(display, XDefaultRootWindow(display), cursor);
    XFixesChangeCursorByName(display, cursor, "watch");
    XFreeCursor(display, cursor);
    XCloseDisplay(display);
    return true;
}
}  // namespace CursorHelper
GUARD_END_NAMESPACE