#ifndef POWERAPI_H
#define POWERAPI_H

#include <QString>

namespace DBusApi
{
namespace SessionManager
{
bool suspend();
bool hibernate();
bool shutdown();
bool reboot();
}  // namespace SessionManager

namespace DisplayManager
{
bool switchToGreeter();
}

namespace AccountService
{
QString findUserObjectByName(const QString &user);
QString getUserObjectIconFileProperty(const QString &userObjPath);
QString getUserIconFilePath(const QString &user);
}  // namespace AccountService
};  // namespace DBusApi

#endif  // POWERAPI_H
