/**
 * Copyright (c) 2020 ~ 2024 KylinSec Co., Ltd.
 * kiran-session-guard is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     liuxinhao <liuxinhao@kylinsec.com.cn>
 */
#include "greeter-subitem.h"
#include "setting-window.h"

using namespace Kiran::SessionGuard::Greeter;

GreeterSubItem::GreeterSubItem(KiranControlPanel::PanelInterface* interface, QObject* parent)
    : QObject(parent),
      m_interface(interface)
{
}

GreeterSubItem ::~GreeterSubItem()
{
}

QString GreeterSubItem::getID()
{
    return "Greeter";
}

QString GreeterSubItem::getName()
{
    return tr("Greeter Settings");
}

QString GreeterSubItem::getCategory()
{
    return "login-settings";
}

QString GreeterSubItem::getDesc()
{
    return "";
}

QString GreeterSubItem::getIcon()
{
    return "kcp-greeter";
}

int GreeterSubItem::getWeight()
{
    return 999;
}

QVector<QPair<QString, QString>> GreeterSubItem::getSearchKeys()
{
    auto res = SettingWindow::getSearchKeys();
    return res;
}

QWidget* GreeterSubItem::createWidget()
{
    m_subitemWidget = new SettingWindow();
    m_subitemWidget->installEventFilter(this);
    return m_subitemWidget;
}

bool GreeterSubItem::jumpToSearchEntry(const QString& key)
{
    if (!m_subitemWidget)
        return false;

    m_subitemWidget->jumpToSearchKey(key);
    return true;
}

bool GreeterSubItem::haveUnSavedOptions()
{
    return false;
}