//
// Created by lxh on 2021/6/1.
//

#include "kcp-interface.h"
#include "greeter-setting-window.h"

#include <QApplication>
#include <QTranslator>
#include <qt5-log-i.h>

#define KCP_SUBITEM_GREETER_ID "Greeter"

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
        return -1;
    }
    QCoreApplication::installTranslator(m_translator);

    return 0;
}

void KcpInterface::uninit()
{
}
QWidget* KcpInterface::getSubItemWidget(QString subItemName)
{
    QWidget* widget = nullptr;
    if(subItemName == KCP_SUBITEM_GREETER_ID)
    {
        widget = new GreeterSettingWindow;
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
