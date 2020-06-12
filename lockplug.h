#ifndef LOCKPLUG_H
#define LOCKPLUG_H

#include <QObject>
#include <QWidget>
#include <QAbstractNativeEventFilter>
#include <X11/Xdefs.h>

class LockPlug : public QWidget
{
    Q_OBJECT
public:
    explicit LockPlug(QWidget *parent = nullptr);
public:
    void printID();
    void responseOkAndQuit();
    void responseCancelAndQuit();
    void responseNoticeAuthFailed();
private:
    XID m_topLevelWindowID;
};

#endif // LOCKPLUG_H
