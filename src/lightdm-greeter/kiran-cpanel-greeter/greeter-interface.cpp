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
#include "greeter-interface.h"
#include "greeter-subitem.h"

#include <QCoreApplication>
#include <QTranslator>
#include <QLocale>
#include <qt5-log-i.h>

namespace Kiran
{
namespace SessionGuard
{
GreeterInterface::GreeterInterface(QObject* parent)
    :QObject()
{

}

GreeterInterface::~GreeterInterface()
{

}

int GreeterInterface::init(KiranControlPanel::PanelInterface* interface)
{
    m_panelInterface = interface;

    if (m_translator != nullptr)
    {
        QCoreApplication::removeTranslator(m_translator);
        delete m_translator;
        m_translator = nullptr;
    }

    m_translator = new QTranslator;
    if (!m_translator->load(QLocale(),
                            "kiran-cpanel-greeter",
                            ".",
                            "/usr/share/lightdm-kiran-greeter/translations",
                            ".qm"))
    {
        KLOG_ERROR() << "greeter plugin load translator failed!";
        m_translator->deleteLater();
        m_translator = nullptr;
    }
    else
    {
        QCoreApplication::installTranslator(m_translator);
    }

    m_subitem.reset(new GreeterSubItem(interface, this));
    return 0;
}

void GreeterInterface::uninit()
{
    if (m_translator != nullptr)
    {
        qApp->removeTranslator(m_translator);
        m_translator->deleteLater();
        m_translator = nullptr;
    }
}

QVector<KiranControlPanel::SubItemPtr> GreeterInterface::getSubItems()
{
    return {m_subitem};
}
}
}
