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
#include <signal.h>
#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QTranslator>
#include <qt5-log-i.h>

#include "../../lib/common-widgets/virtual-keyboard.h"
#include "cursor-helper.h"
#include "greeter-login-window.h"
#include "greeter-screen-manager.h"
#include "kiran-greeter-prefs.h"
#include "scaling-helper.h"
#include "sync-lock-status.h"

#define DEFAULT_STYLE_FILE ":/themes/lightdm-kiran-greeter-normal.qss"

void termSignalHandler(int unused)
{
#ifdef VIRTUAL_KEYBOARD
    VirtualKeyboard::instance()->keyboardProcessExit();
#endif
    qApp->quit();
}

void setup_unix_signal_handlers()
{
    struct sigaction term;
    term.sa_handler = termSignalHandler;
    sigemptyset(&term.sa_mask);
    term.sa_flags = 0;
    term.sa_flags |= SA_RESETHAND;
    int iRet = sigaction(SIGTERM, &term, 0);
    if (iRet != 0)
    {
        KLOG_WARNING("set sigaction failed,%s", strerror(iRet));
    }
}

int main(int argc, char *argv[])
{
    ///初始化日志模块
    int iRet = klog_qt5_init("/usr/share/lightdm-kiran-greeter/zlog.conf","kylinsec-nologin","lightdm-kiran-greeter","lightdm-kiran-greeter");
    if (iRet!=0)
    {
        qWarning() << "klog_qt5_init error:" << iRet;
    }

    ///安装信号处理
    setup_unix_signal_handlers();

    ///设置缩放比
    double scaled_factor = 0.0;
    switch (KiranGreeterPrefs::instance()->scale_mode())
    {
    case GREETER_SCALING_MODE_AUTO:
    {
        ScalingHelper::auto_calculate_screen_scaling();
        break;
    }
    case GREETER_SCALING_MODE_MANUAL:
    {
        double scaleFcator = KiranGreeterPrefs::instance()->scale_factor();
        scaled_factor = scaleFcator;
        ScalingHelper::set_scale_factor(scaleFcator);
        break;
    }
    case GREETER_SCALING_MODE_DISABLE:
        break;
    default:
        KLOG_ERROR("enable-scaling: unsupported options %d", KiranGreeterPrefs::instance()->scale_mode());
        break;
    }

    ///对静态库中包含的资源文件进行初始化
    Q_INIT_RESOURCE(commonWidgets);

    QApplication a(argc, argv);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    ///依据登陆器整体缩放比例，设置默认光标大小
    if (!CursorHelper::setDefaultCursorSize(scaled_factor))
    {
        KLOG_ERROR("set default cursor size for factor %lf failed!", scaled_factor);
    }

    ///登录成功和进入桌面的间隔会显示根窗口，为了避免显示根窗口时光标显示为"X",需设置ROOT窗口光标
    if (!CursorHelper::setRootWindowWatchCursor())
    {
        KLOG_ERROR("set root window watch cursor failed!");
    }

    KLOG_INFO() << "Num Lock Init State:" << KiranGreeterPrefs::instance()->numlockInitState();
    ///为了解决键盘NumLock灯亮起，但实际不可用的情况，手动同步键盘NumLock灯状态和实际状态
    initLockStatus(KiranGreeterPrefs::instance()->numlockInitState());

    ///翻译
    QTranslator translator;
    QString translationFileDir = QString("/usr/share/%1/translations/").arg(qAppName());
    translator.load(QLocale(), qAppName(), ".", translationFileDir, ".qm");
    QApplication::installTranslator(&translator);

    ///加载样式表
    QFile file(DEFAULT_STYLE_FILE);
    if (file.open(QIODevice::ReadOnly))
    {
        qApp->setStyleSheet(file.readAll());
    }
    else
    {
        KLOG_ERROR("load style sheet failed!");
    }

#ifdef VIRTUAL_KEYBOARD
    ///初始键盘配置
    VirtualKeyboard::instance()->init();
#endif

    ///初始化屏幕管理,在屏幕管理中创建背景窗口和登录窗口，负责处理屏幕增加删除的情况
    GreeterScreenManager screenManager;
    screenManager.init();

    return a.exec();
}
