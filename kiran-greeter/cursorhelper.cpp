#include "cursorhelper.h"
#include <QDebug>
#include <QApplication>
#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <X11/Xcursor/Xcursor.h>
#include <X11/extensions/Xfixes.h>

static unsigned long loadCursorHandle(Display *dpy, const char *name, int size)
{
    if (size == -1) {
        size = XcursorGetDefaultSize(dpy);
    }
    XcursorImages *images = nullptr;
    images = XcursorLibraryLoadImages("watch","Adwaita",size);
    if( !images ){
        return 0;
    }
    unsigned long handle = (unsigned long)XcursorImagesLoadCursor(dpy,images);
    XcursorImagesDestroy(images);
    return handle;
}

bool CursorHelper::setRootWindowCursor()
{
    Display* display = XOpenDisplay(NULL);
    if (!display) {
        qDebug() << "Open display failed";
        return false;
    }
    Cursor cursor =  (Cursor)loadCursorHandle(display, "watch", 24);
    if(!cursor){
        return false;
    }
    XDefineCursor(display, XDefaultRootWindow(display),cursor);
    // XFixesChangeCursorByName is the key to change the cursor
    // and the XFreeCursor and XCloseDisplay is also essential.
    XFixesChangeCursorByName(display, cursor, "watch");
    XFreeCursor(display, cursor);
    XCloseDisplay(display);
    return true;
}
