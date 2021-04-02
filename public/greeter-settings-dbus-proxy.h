#ifndef GREETERSETTINGSDBUSPROXY_H
#define GREETERSETTINGSDBUSPROXY_H

#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

class ComKylinsecKiranSystemDaemonGreeterSettingsInterface : public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    {
        return "com.kylinsec.Kiran.SystemDaemon.GreeterSettings";
    }

public:
    ComKylinsecKiranSystemDaemonGreeterSettingsInterface(const QString &service,
                                                         const QString &path,
                                                         const QDBusConnection &connection,
                                                         QObject *parent = nullptr);

    ~ComKylinsecKiranSystemDaemonGreeterSettingsInterface();

    Q_PROPERTY(bool allowManualLogin READ allowManualLogin)
    inline bool allowManualLogin() const
    {
        return qvariant_cast<bool>(property("allowManualLogin"));
    }

    Q_PROPERTY(qulonglong autologinTimeout READ autologinTimeout)
    inline qulonglong autologinTimeout() const
    {
        return qvariant_cast<qulonglong>(property("autologinTimeout"));
    }

    Q_PROPERTY(QString autologinUser READ autologinUser)
    inline QString autologinUser() const
    {
        return qvariant_cast<QString>(property("autologinUser"));
    }

    Q_PROPERTY(QString backgroundFile READ backgroundFile)
    inline QString backgroundFile() const
    {
        return qvariant_cast<QString>(property("backgroundFile"));
    }

    Q_PROPERTY(bool hideUserList READ hideUserList)
    inline bool hideUserList() const
    {
        return qvariant_cast<bool>(property("hideUserList"));
    }

    Q_PROPERTY(ushort scaleFactor READ scaleFactor)
    inline ushort scaleFactor() const
    {
        return qvariant_cast<ushort>(property("scaleFactor"));
    }

    Q_PROPERTY(ushort scaleMode READ scaleMode)
    inline ushort scaleMode() const
    {
        return qvariant_cast<ushort>(property("scaleMode"));
    }

public Q_SLOTS:  // METHODS
    inline QDBusPendingReply<> SetAllowManualLogin(bool allow)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(allow);
        return asyncCallWithArgumentList(QStringLiteral("SetAllowManualLogin"), argumentList);
    }

    inline QDBusPendingReply<> SetAutologinTimeout(qulonglong seconds)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(seconds);
        return asyncCallWithArgumentList(QStringLiteral("SetAutologinTimeout"), argumentList);
    }

    inline QDBusPendingReply<> SetAutologinUser(const QString &autologin_user)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(autologin_user);
        return asyncCallWithArgumentList(QStringLiteral("SetAutologinUser"), argumentList);
    }

    inline QDBusPendingReply<> SetBackgroundFile(const QString &file_path)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(file_path);
        return asyncCallWithArgumentList(QStringLiteral("SetBackgroundFile"), argumentList);
    }

    inline QDBusPendingReply<> SetHideUserList(bool hide)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(hide);
        return asyncCallWithArgumentList(QStringLiteral("SetHideUserList"), argumentList);
    }

    inline QDBusPendingReply<> SetScaleMode(ushort mode, ushort factor)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(mode) << QVariant::fromValue(factor);
        return asyncCallWithArgumentList(QStringLiteral("SetScaleMode"), argumentList);
    }

Q_SIGNALS:  // SIGNALS
};

namespace com
{
namespace kylinsec
{
namespace Kiran
{
namespace SystemDaemon
{
typedef ::ComKylinsecKiranSystemDaemonGreeterSettingsInterface GreeterSettings;
}
}  // namespace Kiran
}  // namespace kylinsec
}  // namespace com
#endif
