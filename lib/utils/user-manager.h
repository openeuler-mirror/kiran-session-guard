#pragma once

#include <QString>
#include <QStringList>
#include "guard-global.h"

GUARD_BEGIN_NAMESPACE

namespace UserManager
{
QString getUserIcon(const QString& name);
QString getUserLastSession(const QString& name);
QStringList getCachedUsers();
QString getCurrentUser();
bool switchToGreeter();
};  // namespace UserManager

GUARD_END_NAMESPACE