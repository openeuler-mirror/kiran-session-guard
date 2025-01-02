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

#include "virtual-keyboard.h"
#include <QApplication>
#include <QTimer>
#include <QWidget>
#include <QWindow>

#include <qt5-log-i.h>
#include <QMutex>
#include <QScopedPointer>
#include <QScreen>
#include <QFileInfo>

#define ONBOARD_LAYOUT "Compact"
#define ONBOARD_THEME "Blackboard"

#define ONBOARD_WIDTH_FACTOR 0.6
#define ONBOARD_HEIGHT_FACTOR 0.3

VirtualKeyboard *VirtualKeyboard::instance()
{
    static QMutex mutex;
    static QScopedPointer<VirtualKeyboard> pInst;

    if (Q_UNLIKELY(!pInst))
    {
        QMutexLocker locker(&mutex);
        if (pInst.isNull())
        {
            pInst.reset(new VirtualKeyboard);
        }
    }

    return pInst.data();
}

VirtualKeyboard::~VirtualKeyboard()
{
    if (m_process->state() != QProcess::NotRunning)
    {
        m_process->terminate();
        m_process->waitForFinished();
    }
}

bool VirtualKeyboard::init(QWidget *parent)
{
    if( !QFileInfo::exists("/usr/bin/onboard") )
    {
        m_isSupported = false;
        return false;
    }

    if (m_keyboardWidget != nullptr)
    {
        return false;
    }

    m_isSupported = true;
    m_process = new QProcess(this);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &VirtualKeyboard::slotOnboardProcessfinished);
    connect(m_process, &QProcess::readyReadStandardOutput, this, [this, parent]
            {
                QString stdoutput;
                qulonglong xid = 0;
                QWindow *foreignWindow = nullptr;

                stdoutput = m_process->readAllStandardOutput();
                stdoutput = stdoutput.trimmed();
                if (stdoutput.isEmpty())
                {
                    KLOG_WARNING("can't get onboard xid!");
                    return;
                }

                xid = stdoutput.toULongLong();
                KLOG_DEBUG() << "foreign virtual keyboard window id:" << xid;
                foreignWindow = QWindow::fromWinId(xid);
                foreignWindow->setFlag(Qt::ForeignWindow);
                m_keyboardWidget = QWidget::createWindowContainer(foreignWindow, nullptr);
                m_keyboardWidget->setParent(parent);
                m_keyboardWidget->setFocusPolicy(Qt::NoFocus);
                m_keyboardWidget->raise();
                KLOG_INFO("greeter keyboard init finish.");
            });
    m_process->start("onboard", QStringList() << "--xid"
                                              << "-t" ONBOARD_THEME << "-l" ONBOARD_LAYOUT << "-d"
                                              << "all");
    return true;
}

bool VirtualKeyboard::isSupported()
{
    return m_isSupported;
}

bool VirtualKeyboard::isVisible()
{
    if (m_keyboardWidget == nullptr)
    {
        return false;
    }
    return m_keyboardWidget->isVisible();
}

void VirtualKeyboard::showAdjustSize(QWidget *parent)
{
    if (m_keyboardWidget == nullptr)
    {
        KLOG_WARNING(" greeter keyboard show adjust size must call after init!");
        return;
    }

    if (parent == nullptr)
    {
        KLOG_WARNING() << "VirtualKeyboard::showAdjustSize parent can't be nullptr";
        return;
    }

    KLOG_DEBUG() << "VirtualKeyboard::showAdjustSize" << parent->objectName();
    m_keyboardWidget->hide();
    m_keyboardWidget->setParent(parent);

    QRect parentRect = parent->geometry();
    QSize keyboardSize(parentRect.width() * ONBOARD_WIDTH_FACTOR, parentRect.height() * ONBOARD_HEIGHT_FACTOR);
    m_keyboardWidget->resize(parentRect.width() * ONBOARD_WIDTH_FACTOR, parentRect.height() * ONBOARD_HEIGHT_FACTOR);
    m_keyboardWidget->move((parentRect.width() - keyboardSize.width()) / 2, parentRect.height() - keyboardSize.height());
    m_keyboardWidget->show();
}

void VirtualKeyboard::hide()
{
    if (m_keyboardWidget == nullptr)
    {
        KLOG_WARNING() << "VirtualKeyboard::hide must call after init";
        return;
    }
    m_keyboardWidget->hide();
}

QWidget *VirtualKeyboard::getKeyboard()
{
    if (m_keyboardWidget == nullptr)
    {
        KLOG_WARNING() << "VirtualKeyboard::getKeyboard must call after VirtualKeyboard::init";
        return nullptr;
    }
    return m_keyboardWidget;
}

void VirtualKeyboard::keyboardProcessExit()
{
    if (m_process->state() != QProcess::NotRunning)
    {
        KLOG_DEBUG() << "terminate keyboard process and wait exit.";
        m_process->terminate();
        m_process->waitForFinished(300);
    }
}

void VirtualKeyboard::slotOnboardProcessfinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    KLOG_DEBUG() << "onboard process finished : "
                 << "exitCode" << exitCode << "exitStaus" << exitStatus;
}

VirtualKeyboard::VirtualKeyboard(QObject *parent)
    : QObject(parent), m_keyboardWidget(nullptr)
{
}

void VirtualKeyboard::slotReadyReadStandardOutput()
{
}