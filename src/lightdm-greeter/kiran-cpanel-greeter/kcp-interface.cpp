/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd.
 * kiran-session-guard is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     liuxinhao <liuxinhao@kylinos.com.cn>
 */

#include "kcp-interface.h"
#include "setting-window.h"

#include <qt5-log-i.h>
#include <QApplication>
#include <QTranslator>

#define KCP_SUBITEM_GREETER_ID "Greeter"

namespace Kiran
{
namespace SessionGuard
{
namespace Greeter
{
KcpInterface::KcpInterface()
{
}

KcpInterface::~KcpInterface()
{
}

int KcpInterface::init()
{
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
        KLOG_ERROR() << "load translator failed!";
        m_translator->deleteLater();
        m_translator = nullptr;
    }
    else
    {
        QCoreApplication::installTranslator(m_translator);
    }

    return 0;
}

void KcpInterface::uninit()
{
}
QWidget* KcpInterface::getSubItemWidget(QString subItemName)
{
    QWidget* widget = nullptr;
    if (subItemName == KCP_SUBITEM_GREETER_ID)
    {
        widget = new SettingWindow;
    }
    m_currentWidget = widget;
    return m_currentWidget;
}

bool KcpInterface::haveUnsavedOptions()
{
    return false;
}

QStringList KcpInterface::visibleSubItems()
{
    return QStringList() << KCP_SUBITEM_GREETER_ID;
}
}  // namespace Greeter
}  // namespace SessionGuard
}  // namespace Kiran