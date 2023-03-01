#include "listener.h"
#include "dialog.h"

#include <kiran-log/qt5-log-i.h>
#include <PolkitQt1/Details>
#include <QApplication>
#include <QDBusConnection>
#include <QDBusError>
#include <QScreen>
#include <QTimer>

GUARD_POLKIT_AGENT_BEGIN_NAMESPACE
Listener::Listener(QObject *parent)
    : PolkitQt1::Agent::Listener(parent)
{
}

Listener::~Listener()
{
}

void Listener::initiateAuthentication(const QString &actionId,
                                      const QString &message,
                                      const QString &iconName,
                                      const PolkitQt1::Details &details,
                                      const QString &cookie,
                                      const PolkitQt1::Identity::List &identities,
                                      PolkitQt1::Agent::AsyncResult *result)
{
    if (m_inProcess)
    {
        result->setError(tr("Existing authentication is in progress, please try again later"));
        result->setCompleted();
        return;
    }

    m_inProcess = true;
    m_result = result;

    AuthInfo info;
    info.actionID = actionId;
    info.message = message;
    info.iconName = iconName;
    info.details = details;
    info.cookie = cookie;
    info.identities = identities;
    info.result = result;
    // info.dump();

    m_authDialog = new Dialog();
    if (!m_authDialog->init(info))
    {
        result->setError(tr("Existing authentication is in progress, please try again later"));
        result->setCompleted();
        m_result = nullptr;
        m_inProcess = false;
        return;
    }

    connect(m_authDialog, &Dialog::completed, this, &Listener::onAuthDialogCompleted);
    connect(m_authDialog, &Dialog::cancelled, this, &Listener::onAuthDialogCancelled);

    auto screen = QApplication::screenAt(QCursor::pos());
    QRect screenGeometry = screen->geometry();
    m_authDialog->move(screenGeometry.x() + (screenGeometry.width() - m_authDialog->width()) / 2,
                       screenGeometry.y() + (screenGeometry.height() - m_authDialog->height()) / 2);

    QTimer::singleShot(200, [this]()
                       {
        m_authDialog->show();
        m_authDialog->activateWindow(); });

    return;
}

bool Listener::initiateAuthenticationFinish()
{
    return true;
}

void Listener::cancelAuthentication()
{
    if (m_inProcess)
    {
        m_inProcess = false;
        m_authDialog->hide();
        m_authDialog->deleteLater();

        m_result->setError("Authentication Cancelled");
        m_result->setCompleted();
        m_result = nullptr;
    }
}

void Listener::onAuthDialogCompleted(bool isSuccess)
{
    if (!isSuccess)
    {
        m_result->setError("Authentication Error");
    }
    m_inProcess = false;

    m_result->setCompleted();
    m_result = nullptr;

    m_authDialog->deleteLater();
    m_authDialog = nullptr;
}

void Listener::onAuthDialogCancelled()
{
    KLOG_DEBUG() << "dialog cancelled,cancel authentication";
    cancelAuthentication();
}
GUARD_POLKIT_AGENT_END_NAMESPACE