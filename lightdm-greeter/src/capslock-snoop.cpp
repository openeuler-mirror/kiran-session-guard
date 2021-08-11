#include "capslock-snoop.h"
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/extensions/XKB.h>
#include <X11/extensions/record.h>
#include <X11/keysym.h>
#include <pthread.h>
#include <string.h>

struct _CapsLockSnoopPrivate
{
    bool                            isRunning;
    pthread_t                       thread;
    Display *                       dataDisplay;  //数据连接
    Display *                       ctrlDisplay;  //控制连接
    Display *                       keyDisplay;   //获取按键的连接
    XRecordContext                  recordContext;
    XRecordRange *                  recordRange;
    bool                            capsLockIsOn;
    capslock_status_change_callback callback;
    void *                          callback_user_data;
};

CapsLockSnoop::CapsLockSnoop()
    : m_private(new CapsLockSnoopPrivate)
{
    memset(m_private, 0, sizeof(CapsLockSnoopPrivate));
}

CapsLockSnoop::~CapsLockSnoop()
{
    stop();
    delete m_private;
}

bool CapsLockSnoop::start(capslock_status_change_callback callback, void *user_data, std::string &error)
{
    int               major, minor, iRes;
    XRecordClientSpec spec = XRecordAllClients;
    if (m_private->isRunning)
    {
        error = "CapsLockSnoop is running";
        return false;
    }
    m_private->callback           = callback;
    m_private->callback_user_data = user_data;
    m_private->ctrlDisplay        = XOpenDisplay(nullptr);
    m_private->dataDisplay        = XOpenDisplay(nullptr);
    m_private->keyDisplay         = XOpenDisplay(nullptr);
    if (!m_private->ctrlDisplay || !m_private->dataDisplay || !m_private->keyDisplay)
    {
        error = "open display failed";
        goto failed;
    }
    XSynchronize(m_private->ctrlDisplay, True);
    if (XRecordQueryVersion(m_private->ctrlDisplay, &major, &minor) == False)
    {
        error = "query xrecord version failed";
        goto failed;
    }
    m_private->recordRange = XRecordAllocRange();
    if (!m_private->recordRange)
    {
        error = "can't allocate necessary memory for xrecord range";
        goto failed;
    }
    m_private->recordRange->device_events.first = KeyPress;
    m_private->recordRange->device_events.last  = KeyPress;
    m_private->recordContext                    = XRecordCreateContext(m_private->ctrlDisplay, 0,
                                                    &spec, 1, &m_private->recordRange, 1);
    if (!m_private->recordContext)
    {
        error = "can't create record context";
        goto failed;
    }
    if (!getCapsLockCurrentState(m_private->capsLockIsOn, error))
    {
        goto failed;
    }
    iRes = pthread_create(&m_private->thread, nullptr, thread_record_func, this);
    if (iRes != 0)
    {
        error = strerror(iRes);
        goto failed;
    }
    m_private->callback(m_private->capsLockIsOn, m_private->callback_user_data);
    m_private->isRunning = true;
    return true;
failed:
    if (m_private->recordContext)
    {
        XRecordDisableContext(m_private->ctrlDisplay,
                              m_private->recordContext);
        XRecordFreeContext(m_private->ctrlDisplay,
                           m_private->recordContext);
    }
    if (m_private->recordRange)
    {
        XFree(m_private->recordRange);
    }
    if (m_private->ctrlDisplay)
    {
        XCloseDisplay(m_private->ctrlDisplay);
    }
    if (m_private->dataDisplay)
    {
        XCloseDisplay(m_private->dataDisplay);
    }
    memset(m_private, 0, sizeof(CapsLockSnoopPrivate));
    return false;
}

bool CapsLockSnoop::stop()
{
    if (!m_private->isRunning)
    {
        return false;
    }
    XRecordDisableContext(m_private->ctrlDisplay, m_private->recordContext);
    XRecordFreeContext(m_private->ctrlDisplay, m_private->recordContext);
    pthread_join(m_private->thread, NULL);
    XCloseDisplay(m_private->dataDisplay);
    XCloseDisplay(m_private->ctrlDisplay);
    XCloseDisplay(m_private->keyDisplay);
    XFree(m_private->recordRange);
    memset(m_private, 0, sizeof(CapsLockSnoopPrivate));
    return true;
}

bool CapsLockSnoop::getCapsLockCurrentState(bool &isOn, std::string &error)
{
    bool     success      = false;
    Display *display      = nullptr;
    Atom     capsLockAtom = 0;
    int      ndxRtrn;
    int      stateRtrn;

    display = XOpenDisplay(nullptr);
    if (!display)
    {
        error = "getCapsLockCurrentState: can't open display";
        goto out;
    }
    XSynchronize(display, True);
    capsLockAtom = XInternAtom(display, "Caps Lock", True);
    if (XkbGetNamedIndicator(display, capsLockAtom, &ndxRtrn, &stateRtrn, NULL, NULL) == False)
    {
        error = "getCapsLockCurrentState: can't get name indicator.";
        goto out;
    }
    isOn    = stateRtrn;
    success = true;
out:
    if (display)
    {
        XCloseDisplay(display);
    }
    return success;
}

void record_intercept_proc_callback(XPointer user_data, XRecordInterceptData *data)
{
    CapsLockSnoop *     This        = (CapsLockSnoop *)user_data;
    const xEvent *      xev         = (const xEvent *)(data->data);
    static const KeySym capsLockSym = XK_Caps_Lock;

    if ((data->category == XRecordFromServer) && (xev->u.u.type == KeyPress))
    {
        const BYTE keycode = xev->u.u.detail;
        KeySym     sym     = XKeycodeToKeysym(This->m_private->keyDisplay, keycode, 0);
        if (sym == capsLockSym)
        {
            This->m_private->capsLockIsOn = !This->m_private->capsLockIsOn;
            if (This->m_private->callback)
            {
                This->m_private->callback(This->m_private->capsLockIsOn, This->m_private->callback_user_data);
            }
        }
    }

    XRecordFreeData(data);
}

void *CapsLockSnoop::thread_record_func(void *param)
{
    CapsLockSnoop *This = static_cast<CapsLockSnoop *>(param);
    XRecordEnableContext(This->m_private->dataDisplay,
                         This->m_private->recordContext, record_intercept_proc_callback, (XPointer)This);
    return nullptr;
}
