#ifndef DBUSAPI_H
#define DBUSAPI_H

#include <QDBusObjectPath>
#include <QVector>

typedef QVector<QDBusObjectPath> QDBusObjectPathVector;
namespace DBusApi
{
    template<typename T>
    bool getProperty (const QString &service, const QString &obj, const QString &propertyName, T &retValue);
    namespace AccountsService
    {
        bool listCachedUsers (QDBusObjectPathVector &userObjects);

        bool findUserByName (const QString &name, QDBusObjectPath &obj);

        bool getUserObjectUserNameProperty (const QDBusObjectPath &obj, QString &userName);
        bool getUserObjectUserNameProperty (const QString &obj, QString &userName);

        bool getUserObjectIconFileProperty (const QDBusObjectPath &userObj, QString &iconFile);
        bool getUserObjectIconFileProperty (const QString &obj, QString &iconFile);

        bool getRootIconFileProperty (QString &iconFile);
    }
}
#endif // DBUSAPI_H
