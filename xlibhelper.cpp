#include "xlibhelper.h"
#include <QX11Info>
#include <X11/Xlib.h>

XID XLibHelper::getTopLevelWindowByXID(XID winID)
{
    XID root=0,parent=0,*children=nullptr;
    unsigned int n_children=0;
    Display* display = QX11Info::display();

    if( !XQueryTree(display,winID,&root,&parent,&children,&n_children) ){
        return 0;
    }


    if(children!=nullptr){
        XFree(children);
        children = nullptr;
    }

    if(parent==root){
        return winID;
    }else{
        return getTopLevelWindowByXID(parent);
    }
}

QSize XLibHelper::getWindowSizeByXID(XID winID)
{
    Display* display = QX11Info::display();
    XWindowAttributes attributes;
    if(!XGetWindowAttributes(display,winID,&attributes)){
        return QSize(0,0);
    }
    return QSize(attributes.width,attributes.height);
}

void XLibHelper::mapWindow(XID winID)
{
    Display* display = QX11Info::display();
    XMapSubwindows(display,winID);
}

void XLibHelper::unMapWindow(XID winID)
{
    Display* display = QX11Info::display();
    XUnmapSubwindows(display,winID);
}

bool XLibHelper::selectInputEvent(XID winID)
{
    Display* display = QX11Info::display();
    if( !XSelectInput(display,winID,ResizeRedirectMask|SubstructureNotifyMask|StructureNotifyMask)){
        return false;
    }
    return true;
}
