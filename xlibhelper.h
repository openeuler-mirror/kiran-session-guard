#ifndef XLIBHELPER_H
#define XLIBHELPER_H

#include <X11/Xdefs.h>
#include <QSize>

namespace XLibHelper {
    XID     getTopLevelWindowByXID(XID winID);
    QSize   getWindowSizeByXID(XID winID);
    bool    selectInputEvent(XID winID);
    void    mapWindow(XID winID);
    void    unMapWindow(XID winID);
}

#endif // XLIBHELPER_H
