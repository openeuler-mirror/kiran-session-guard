/**
 * Copyright (c) 2026.
 * kiran-session-guard is licensed under Mulan PSL v2.
 */

#include "face-daemon-signal-listener.h"

#include <qt5-log-i.h>
#include <QDBusConnection>
#include <QDBusError>

namespace
{
const QString kFaceService = QStringLiteral("com.czht.face.daemon");
const QString kFacePath = QStringLiteral("/com/czht/face/daemon");
const QString kFaceInterface = QStringLiteral("com.czht.face.daemon");
}  // namespace

namespace Kiran
{
namespace SessionGuard
{

FaceDaemonSignalListener::FaceDaemonSignalListener(QObject* parent)
    : QObject(parent)
{
}

FaceDaemonSignalListener::~FaceDaemonSignalListener()
{
    disconnectLeaveDetected();
}

bool FaceDaemonSignalListener::connectLeaveDetected()
{
    if (m_leaveDetectedConnected)
    {
        return true;
    }

    auto bus = QDBusConnection::systemBus();
    const bool ok = bus.connect(kFaceService,
                                kFacePath,
                                kFaceInterface,
                                QStringLiteral("LeaveDetected"),
                                this,
                                SLOT(onLeaveDetected(QString)));
    if (!ok)
    {
        KLOG_WARNING() << "FaceDaemonSignalListener: connect LeaveDetected failed, bus name=" << bus.name()
                       << "error=" << bus.lastError().message();
        return false;
    }

    m_leaveDetectedConnected = true;
    KLOG_INFO() << "FaceDaemonSignalListener: LeaveDetected signal connected, bus name=" << bus.name();
    return true;
}

void FaceDaemonSignalListener::disconnectLeaveDetected()
{
    if (!m_leaveDetectedConnected)
    {
        return;
    }
    auto bus = QDBusConnection::systemBus();
    bus.disconnect(kFaceService,
                   kFacePath,
                   kFaceInterface,
                   QStringLiteral("LeaveDetected"),
                   this,
                   SLOT(onLeaveDetected(QString)));
    m_leaveDetectedConnected = false;
    KLOG_INFO() << "FaceDaemonSignalListener: LeaveDetected disconnected";
}

bool FaceDaemonSignalListener::isLeaveDetectedConnected() const
{
    return m_leaveDetectedConnected;
}

void FaceDaemonSignalListener::onLeaveDetected(const QString& json)
{
    emit leaveDetected(json);
}

}  // namespace SessionGuard
}  // namespace Kiran

