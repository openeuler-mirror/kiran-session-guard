/*
 * This file was generated by qdbusxml2cpp version 0.7
 * Command line was: qdbusxml2cpp -p AccountsInterface com.unikylin.Kiran.SystemDaemon.Accounts.xml
 *
 * qdbusxml2cpp is Copyright (C) 2015 The Qt Company Ltd.
 *
 * This is an auto-generated file.
 * This file may have been hand-edited. Look for HAND-EDIT comments
 * before re-generating it.
 */

#include "accounts-interface.h"

/*
 * Implementation of interface class ComUnikylinKiranSystemDaemonAccountsInterface
 */

using namespace com::kylinsec::Kiran::SystemDaemon;

AccountsInterface::AccountsInterface(const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(AccountsInterface::staticInterfaceName(), "/com/kylinsec/Kiran/SystemDaemon/Accounts",
                             staticInterfaceName(), connection, parent)
{
}

AccountsInterface::~AccountsInterface()
{
}
