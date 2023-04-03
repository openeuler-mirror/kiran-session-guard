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

#include <qt5-log-i.h>
#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>

#include "background.h"
#include "frame.h"
#include "prefs.h"
#include "screen-manager.h"

namespace Kiran
{
namespace SessionGuard
{
namespace Greeter
{
/// 析构:
///      清理登录窗口和背景窗口
ScreenManager::~ScreenManager()
{
    if (m_greeterFrame != nullptr)
    {
        delete m_greeterFrame;
    }
    for (auto iter = m_BackgroundWidgetMap.begin(); iter != m_BackgroundWidgetMap.end();)
    {
        Background *background = iter.value();
        delete background;
        background = nullptr;
        m_BackgroundWidgetMap.erase(iter++);
    }
}

/// 初始化窗口：
///      获取显示器数量，根据主显示器创建背景窗口，保存显示器和背景窗口的映射关系
///      创建GreeterWindow与主显示器绑定.若不存在主显示器，则不显示
void ScreenManager::init(Prefs *prefs)
{
    m_prefs = prefs;
    for (QScreen *screen : qApp->screens())
    {
        // fix #36459,避免错误的数据,导致显示的问题
        if (screen->physicalSize().isEmpty())
        {
            KLOG_WARNING() << screen->name() << "physical size is invalid,ignore it!";
            continue;
        }
        KLOG_DEBUG() << "create background window for" << screen;
        newScreenBackgroundWidget(screen);
    }

    m_greeterFrame = new Frame(prefs);
    auto backgroundIter = m_BackgroundWidgetMap.find(qApp->primaryScreen());
    if (backgroundIter != m_BackgroundWidgetMap.end())
    {
        KLOG_DEBUG() << "move login window on background" << backgroundIter.value()->objectName();
        setGreeterOnBackground(backgroundIter.value());
    }
    else
    {
        KLOG_WARNING() << "can't find primary screen!";
    }
}

/// 屏幕添加：
///      创建新的背景窗口
///      如果登录窗口还未显示，则显示到新背景窗口上
void ScreenManager::slotScreenAdded(QScreen *screen)
{
    KLOG_DEBUG() << "screen added:" << screen;

    newScreenBackgroundWidget(screen);
    if (m_greeterFrame->parent() == nullptr)
    {
        auto backgroundIter = m_BackgroundWidgetMap.find(qApp->primaryScreen());
        if (backgroundIter != m_BackgroundWidgetMap.end())
        {
            setGreeterOnBackground(backgroundIter.value());
        }
    }
}

/// 屏幕被移除:
///      删除和屏幕映射的背景窗口
///      更新窗口和屏幕映射关系
///      如果GreeterWindow显示在该窗口上移动到新的主显示器　调用GreeterWindow中的setScreen重新绑定屏幕
///      若主屏幕不存在不显示
void ScreenManager::slotScreenRemoved(QScreen *screen)
{
    KLOG_DEBUG() << "screen removed:" << screen->objectName();

    auto iter = m_BackgroundWidgetMap.find(screen);
    if (iter != m_BackgroundWidgetMap.end())
    {
        // 隐藏窗口并延迟删除
        QObject *backgroundObject = dynamic_cast<QObject *>(iter.value());
        if (backgroundObject == m_greeterFrame->parent())
        {
            m_greeterFrame->setParent(nullptr);
        }
        iter.value()->hide();
        iter.value()->deleteLater();
        m_BackgroundWidgetMap.erase(iter);
    }

    if (m_greeterFrame->parent() == nullptr)
    {
        auto backgroundIter = m_BackgroundWidgetMap.find(qApp->primaryScreen());
        if (backgroundIter != m_BackgroundWidgetMap.end())
        {
            setGreeterOnBackground(backgroundIter.value());
        }
    }
}

/// 鼠标移入背景窗口槽函数:
///      ]动GreeterWindow到背景窗口
void ScreenManager::mouseEnterInWindow(Background *background)
{
    if (m_greeterFrame->parent() != background)
    {
        KLOG_DEBUG() << "move login content to" << background->objectName();
        setGreeterOnBackground(background);
    }
}

void ScreenManager::newScreenBackgroundWidget(QScreen *screen)
{
    auto background = new Background(m_prefs->background(), screen);
    background->setObjectName(QString("BackgroundWindow_%1").arg(screen->name()));
    m_BackgroundWidgetMap.insert(screen, background);
    connect(background, &Background::mouseEnter,
            this, &ScreenManager::mouseEnterInWindow);
    background->show();
}

void ScreenManager::setGreeterOnBackground(Background *background)
{
    m_greeterFrame->hide();
    m_greeterFrame->resize(background->width(), background->height());
    m_greeterFrame->setParent(background, Qt::X11BypassWindowManagerHint);
    m_greeterFrame->show();
    // m_greeterFrame->setEditPromptFocus();
    m_greeterFrame->activateWindow();
}

ScreenManager::ScreenManager(QObject *parent)
    : QObject(parent), m_greeterFrame(nullptr)
{
    connect(qApp, &QApplication::screenAdded,
            this, &ScreenManager::slotScreenAdded);
    connect(qApp, &QApplication::screenRemoved,
            this, &ScreenManager::slotScreenRemoved);
}
}  // namespace Greeter
}  // namespace SessionGuard
}  // namespace Kiran