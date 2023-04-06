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
#include <kiran-system-daemon/greeter-i.h>
#include <qt5-log-i.h>

#include <QApplication>
#include <QTranslator>

#include "cursor-helper.h"
#include "keyboard-monitor.h"
#include "prefs.h"
#include "scaling-helper.h"
#include "screen-manager.h"
#include "term-signal-handler.h"
#include "virtual-keyboard.h"

#define DEFAULT_STYLE_FILE ":/greeter/stylesheets/lightdm-kiran-greeter-normal.qss"

using namespace ::Kiran::SessionGuard;
using namespace ::Kiran::SessionGuard::Greeter;

// 根据配置项,调整缩放率
void adjustScaleFactor(Prefs* prefs)
{
    /// 设置缩放比
    switch (prefs->scale_mode())
    {
    case GREETER_SCALING_MODE_AUTO:
    {
        ScalingHelper::auto_calculate_screen_scaling();
        break;
    }
    case GREETER_SCALING_MODE_MANUAL:
    {
        double scaleFcator = prefs->scale_factor();
        ScalingHelper::set_scale_factor(scaleFcator);
        break;
    }
    case GREETER_SCALING_MODE_DISABLE:
        break;
    default:
        KLOG_ERROR("enable-scaling: unsupported options %d", prefs->scale_mode());
        break;
    }
}

// 加载样式表
bool loadStyleSheet()
{
    bool bRes = false;
    QFile file(DEFAULT_STYLE_FILE);
    if (file.open(QIODevice::ReadOnly))
    {
        qApp->setStyleSheet(file.readAll());
        bRes = true;
    }
    else
    {
        KLOG_WARNING() << "load stylesheet failed!";
    }
    return bRes;
}

// 根据当前语言环境加载翻译
bool loadTranslator()
{
    bool bRes = false;
    auto translator = new QTranslator();
    QString translationFileDir = QString("/usr/share/%1/translations/").arg(qAppName());
    if (translator->load(QLocale(), qAppName(), ".", translationFileDir, ".qm"))
    {
        QApplication::installTranslator(translator);
        bRes = true;
        KLOG_INFO() << "loaded translator" << translator->filePath();
    }
    else
    {
        KLOG_WARNING() << "load translator failed!";
    }
    return bRes;
}

// 设置当前光标缩放,以及Root窗口光标避免开始会话到进入会话之中的空窗期光标错误显示
void setCursor(Prefs* prefs)
{
    // 光标放大
    if (!CursorHelper::setDefaultCursorSize(prefs->scale_factor()))
    {
        KLOG_ERROR("set default cursor size for factor %d failed!", prefs->scale_factor());
    }

    // 登录成功和进入桌面的间隔会显示根窗口，为了避免显示根窗口时光标显示为"X",需设置ROOT窗口光标
    if (!CursorHelper::setRootWindowWatchCursor())
    {
        KLOG_ERROR("set root window watch cursor failed!");
    }
}

int main(int argc, char* argv[])
{
    Q_INIT_RESOURCE(commonWidgets);
    Q_INIT_RESOURCE(loginFrame);

    if (klog_qt5_init("/usr/share/lightdm-kiran-greeter/zlog.conf",
                      "kylinsec-nologin",
                      "kiran-session-guard",
                      "lightdm-kiran-greeter"))
    {
        qWarning() << "init kiran-log failed";
    }

    QApplication app(argc, argv);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    TermSignalHandler signalHandler;
    signalHandler.init();

    Prefs::globalInit();
    auto prefs = Prefs::getInstance();

    adjustScaleFactor(prefs);
    setCursor(prefs);
    loadTranslator();
    loadStyleSheet();

    KeyboardMonitor::instance()->start();
    KeyboardMonitor::instance()->setNumlockStatus(prefs->numlockInitState());

    VirtualKeyboard::instance()->init();

    // 初始化屏幕管理,在屏幕管理中创建背景窗口和登录窗口，负责处理屏幕增加删除的情况
    ScreenManager screenManager;
    screenManager.init(prefs);

    auto ret = app.exec();

    Prefs::globalDeinit();

    return ret;
}