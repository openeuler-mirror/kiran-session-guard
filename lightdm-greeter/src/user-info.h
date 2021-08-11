#ifndef USERINFO_H
#define USERINFO_H

#include <QObject>
#include <QVariant>

struct UserInfo
{
    QString    name;
    QString    realName;
    QString    session;
    qulonglong uid;
    QString    backgroundPath;
    bool       hasMessage;
    QString    imagePath;
    bool       loggedIn;
};

#endif  // USERINFO_H
