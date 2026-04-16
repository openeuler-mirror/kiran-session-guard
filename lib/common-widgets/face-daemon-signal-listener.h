/**
 * Copyright (c) 2026.
 * kiran-session-guard is licensed under Mulan PSL v2.
 */
#pragma once

#include <QObject>

namespace Kiran
{
namespace SessionGuard
{

class FaceDaemonSignalListener : public QObject
{
    Q_OBJECT
public:
    explicit FaceDaemonSignalListener(QObject* parent = nullptr);
    ~FaceDaemonSignalListener() override;

    bool connectLeaveDetected();
    void disconnectLeaveDetected();
    bool isLeaveDetectedConnected() const;

signals:
    void leaveDetected(QString json);

private slots:
    void onLeaveDetected(const QString& json);

private:
    bool m_leaveDetectedConnected = false;
};

}  // namespace SessionGuard
}  // namespace Kiran

