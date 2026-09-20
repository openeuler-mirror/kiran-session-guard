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
#include "greeter-define.h"
#include <qt5-log-i.h>
#include <locale.h>

#include <QApplication>
#include <QTranslator>
#include <QProcess>

#include "cursor-helper.h"
#include "keyboard-monitor.h"
#include "prefs.h"
#include "scaling-helper.h"
#include "screen-manager.h"
#include "term-signal-handler.h"
#include "virtual-keyboard.h"
#include "app-native-event-filter.h"

#define DEFAULT_STYLE_FILE ":/greeter/stylesheets/lightdm-kiran-greeter-normal.qss"

using namespace ::Kiran::SessionGuard;
using namespace ::Kiran::SessionGuard::Greeter;

// 根据配置项,调整缩放率
qreal adjustScaleFactor(Prefs* prefs)
{
    qreal factor = 1.0;

    /// 设置缩放比
    switch (prefs->scale_mode())
    {
    case GREETER_SCALING_MODE_AUTO:
    {
        factor = ScalingHelper::auto_calculate_screen_scaling();
        break;
    }
    case GREETER_SCALING_MODE_MANUAL:
    {
        factor = prefs->scale_factor();
        ScalingHelper::set_scale_factor(factor);
        break;
    }
    case GREETER_SCALING_MODE_DISABLE:
        break;
    default:
        KLOG_ERROR("enable-scaling: unsupported options %d", prefs->scale_mode());
        break;
    }

    return factor;
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
    }
    else
    {
        KLOG_WARNING() << "load translator failed!";
    }
    return bRes;
}

// 设置当前光标缩放,以及Root窗口光标避免开始会话到进入会话之中的空窗期光标错误显示
void setCursor(qreal factor)
{
    // 光标放大
    if (!CursorHelper::setDefaultCursorSize(factor))
    {
        KLOG_ERROR("set default cursor size for factor %f failed!", factor);
    }

    // 登录成功和进入桌面的间隔会显示根窗口，为了避免显示根窗口时光标显示为"X",需设置ROOT窗口光标
    if (!CursorHelper::setRootWindowWatchCursor())
    {
        KLOG_ERROR("set root window watch cursor failed!");
    }
}

// 把登录界面的字符集固定为 UTF-8（保留语言部分）
//
// 背景：greeter 的界面文案与翻译（.qm）都是 UTF-8；业务系统可能把会话/系统字符集设成
// GB18030（如 D5000 类老业务），直接沿用会话环境变量会导致登录界面乱码/翻译加载失败。
// 这里只把“语言.字符集”中的字符集替换为 UTF-8（zh_CN.GB18030 -> zh_CN.UTF-8），语言不变；
// 且 LANG 与 LC_ALL 一起设置——按优先级 LC_ALL > LC_CTYPE > LANG，只改 LANG 会被继承来的
// LC_CTYPE/LC_ALL 压掉（本仓库 session-guard-checkpass 就有这个问题）。
static void forceUtf8Locale()
{
    QByteArray source = qgetenv("LC_ALL");
    if (source.isEmpty())
        source = qgetenv("LC_CTYPE");
    if (source.isEmpty())
        source = qgetenv("LANG");

    QString language = QString::fromLatin1(source).section('.', 0, 0);
    if (language.isEmpty())
        language = QStringLiteral("zh_CN");  // 兜底：界面默认中文

    const QByteArray utf8Locale = QStringLiteral("%1.UTF-8").arg(language).toUtf8();
    qputenv("LANG", utf8Locale);
    qputenv("LC_ALL", utf8Locale);
    setlocale(LC_ALL, "");

    KLOG_INFO() << "greeter locale:" << source << "->" << utf8Locale.constData();
}

int main(int argc, char* argv[])
{
    Q_INIT_RESOURCE(commonWidgets);
    Q_INIT_RESOURCE(loginFrame);

    if (klog_qt5_init("/usr/share/lightdm-kiran-greeter/zlog.conf",
                      "kylinsec-greeter",
                      "kiran-session-guard",
                      "lightdm-kiran-greeter"))
    {
        qWarning() << "init kiran-log failed";
    }

    // 登录界面固定 UTF-8 字符集（必须在 QApplication 构造之前，Qt 会缓存 locale）
    forceUtf8Locale();

    Prefs::globalInit();
    auto prefs = Prefs::getInstance();
    qreal factor = adjustScaleFactor(prefs);

    QApplication app(argc, argv);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    // 监听x11事件，处理屏幕从无到有，并配置屏幕
    AppNativeEventFilter appNativeEventFilter;
    if (appNativeEventFilter.init())
    {
        app.installNativeEventFilter(&appNativeEventFilter);
    }
    else
    {
        KLOG_ERROR() << "can't install app native event filter!";
    }

    if (prefs->monitorAlwaysOn())
    {
        // 登录界面阶段不熄灭屏幕
        KLOG_INFO() << "set screen saver off!";
        QProcess::startDetached("xset", {"s", "0", "0"});

        KLOG_INFO() << "set dpms off!";
        QProcess::startDetached("xset", {"dpms", "0", "0", "0"});
    }

    setCursor(factor);
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
