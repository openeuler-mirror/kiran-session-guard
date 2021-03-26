#include "dbusapihelper.h"
#include <QtDBus>
#include <QDBusInterface>
#include <QDebug>

#define SESSION_MANAGER_DBUS      "org.gnome.SessionManager"
#define SESSION_MANAGER_PATH      "/org/gnome/SessionManager"
#define SESSION_MANAGER_INTERFACE "org.gnome.SessionManager"

#define METHOD_SUSPEND      "Suspend"
#define METHOD_HIBERNATE    "Hibernate"
#define METHOD_SHUTDOWN     "RequestShutdown"
#define METHOD_REBOOT       "RequestReboot"

#define ACCOUNT_SERVICE_DBUS        "org.freedesktop.Accounts"
#define ACCOUNT_SERVICE_PATH        "/org/freedesktop/Accounts"
#define ACCOUNT_SERVICE_INTERFACE   "org.freedesktop.Accounts"

#define TIMEOUT_MS 300

bool DBusApi::SessionManager::suspend ()
{
    QDBusMessage methodSuspend = QDBusMessage::createMethodCall(SESSION_MANAGER_DBUS,
                                                                SESSION_MANAGER_PATH,
                                                                SESSION_MANAGER_INTERFACE,
                                                                METHOD_SUSPEND);

    QDBusMessage reply = QDBusConnection::sessionBus().call(methodSuspend, QDBus::Block, TIMEOUT_MS);

    if (reply.type() == QDBusMessage::ReplyMessage)
    {
        return true;
    }

    qWarning() << SESSION_MANAGER_DBUS << METHOD_SUSPEND
               << reply.errorName() << reply.errorMessage();
    return false;
}

bool DBusApi::SessionManager::hibernate ()
{
    QDBusMessage methodHibernate = QDBusMessage::createMethodCall(SESSION_MANAGER_DBUS,
                                                                  SESSION_MANAGER_PATH,
                                                                  SESSION_MANAGER_INTERFACE,
                                                                  METHOD_HIBERNATE);

    QDBusMessage reply = QDBusConnection::sessionBus().call(methodHibernate, QDBus::Block, TIMEOUT_MS);

    if (reply.type() == QDBusMessage::ReplyMessage)
    {
        return true;
    }

    qWarning() << SESSION_MANAGER_DBUS << METHOD_SUSPEND
               << reply.errorName() << reply.errorMessage();
    return false;
}

bool DBusApi::SessionManager::shutdown ()
{
    QDBusMessage methodShutdown = QDBusMessage::createMethodCall(SESSION_MANAGER_DBUS,
                                                                 SESSION_MANAGER_PATH,
                                                                 SESSION_MANAGER_INTERFACE,
                                                                 METHOD_SHUTDOWN);

    QDBusMessage reply = QDBusConnection::sessionBus().call(methodShutdown, QDBus::Block, TIMEOUT_MS);

    if (reply.type() == QDBusMessage::ReplyMessage)
    {
        return true;
    }

    qWarning() << SESSION_MANAGER_DBUS << METHOD_SUSPEND
               << reply.errorName() << reply.errorMessage();
    return false;
}

bool DBusApi::SessionManager::reboot ()
{
    QDBusMessage methodReboot = QDBusMessage::createMethodCall(SESSION_MANAGER_DBUS,
                                                               SESSION_MANAGER_PATH,
                                                               SESSION_MANAGER_INTERFACE,
                                                               METHOD_REBOOT);

    QDBusMessage reply = QDBusConnection::sessionBus().call(methodReboot, QDBus::Block, TIMEOUT_MS);

    if (reply.type() == QDBusMessage::ReplyMessage)
    {
        return true;
    }

    qWarning() << SESSION_MANAGER_DBUS << METHOD_SUSPEND
               << reply.errorName() << reply.errorMessage();
    return false;
}

bool DBusApi::DisplayManager::switchToGreeter ()
{
    QDBusMessage methodSwitchToGreeter = QDBusMessage::createMethodCall("org.freedesktop.DisplayManager",
                                                                        qgetenv("XDG_SEAT_PATH"),
                                                                        "org.freedesktop.DisplayManager.Seat",
                                                                        "SwitchToGreeter");

    QDBusMessage reply = QDBusConnection::systemBus().call(methodSwitchToGreeter, QDBus::Block, TIMEOUT_MS);

    if (reply.type() == QDBusMessage::ReplyMessage)
    {
        return true;
    }

    qWarning() << methodSwitchToGreeter.path() << methodSwitchToGreeter.member()
               << reply.errorName() << reply.errorMessage();

    return false;
}

QString DBusApi::AccountService::getUserIconFilePath (const QString &user)
{
    QString userObj, iconFile;
    userObj = findUserObjectByName(user);
    if (userObj.isEmpty())
    {
        return "";
    }
    iconFile = getUserObjectIconFileProperty(userObj);
    return iconFile;
}

QString DBusApi::AccountService::findUserObjectByName (const QString &user)
{
    QDBusMessage methodFindUserByName = QDBusMessage::createMethodCall(ACCOUNT_SERVICE_DBUS,
                                                                       ACCOUNT_SERVICE_PATH,
                                                                       ACCOUNT_SERVICE_INTERFACE,
                                                                       "FindUserByName");
    methodFindUserByName << user;
    QDBusMessage reply = QDBusConnection::systemBus().call(methodFindUserByName,
                                                           QDBus::Block, TIMEOUT_MS);
    if (reply.type() != QDBusMessage::ReplyMessage)
    {
        qWarning() << reply.errorMessage();
        return QString("");
    }
    QList<QVariant> args = reply.arguments();
    if (args.size() == 0)
    {
        qWarning() << "no arguments";
        return "";
    }
    QVariant firstArg = args.takeFirst();
    QDBusObjectPath objPath = firstArg.value<QDBusObjectPath>();
    return objPath.path();
}

QString DBusApi::AccountService::getUserObjectIconFileProperty (const QString &userObjPath)
{
    QDBusMessage methodGetIconFile = QDBusMessage::createMethodCall(ACCOUNT_SERVICE_DBUS,
                                                                    userObjPath,
                                                                    "org.freedesktop.DBus.Properties",
                                                                    "Get");
    methodGetIconFile << QString("org.freedesktop.Accounts.User") << QString("IconFile");
    QDBusMessage reply = QDBusConnection::systemBus().call(methodGetIconFile,
                                                           QDBus::Block, TIMEOUT_MS);
    if (reply.type() != QDBusMessage::ReplyMessage)
    {
        qWarning() << reply.errorMessage();
        return QString("");
    }
    QList<QVariant> argList = reply.arguments();
    if (argList.size() == 0)
    {
        qWarning() << "no arguments";
        return "";
    }
    QVariant firstArg = argList.takeFirst();
    QDBusVariant busVariant = firstArg.value<QDBusVariant>();
    QVariant iconFileVar = busVariant.variant();
    return iconFileVar.toString();
}
