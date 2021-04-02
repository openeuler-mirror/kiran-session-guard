#include <signal.h>
#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QTranslator>

#include "cursorhelper.h"
#include "greeterkeyboard.h"
#include "greeterloginwindow.h"
#include "greeterscreenmanager.h"
#include "kiran-greeter-prefs.h"
#include "log.h"
#include "scalinghelper.h"
#include "synclockstatus.h"

#define DEFAULT_STYLE_FILE ":/themes/lightdm-kiran-greeter-normal.qss"

void termSignalHandler(int unused)
{
    qApp->quit();
}

void setup_unix_signal_handlers()
{
    struct sigaction term;
    term.sa_handler = termSignalHandler;
    sigemptyset(&term.sa_mask);
    term.sa_flags = 0;
    int iRet      = sigaction(SIGTERM, &term, 0);
    if (iRet != 0)
    {
        qWarning() << "setup_unix_signal_handlers failed," << strerror(iRet);
    }
}

int main(int argc, char *argv[])
{
    ///初始化日志模块
    Log::instance()->init("/tmp/lightdm-kiran-greeter.log");
    qInstallMessageHandler(Log::messageHandler);

    setup_unix_signal_handlers();

    ///设置缩放比
    double scaled_factor = 0.0;
    switch (KiranGreeterPrefs::instance()->scaleMode())
    {
    case KiranGreeterPrefs::ScaleMode_Auto:
    {
        ScalingHelper::auto_calculate_screen_scaling(scaled_factor);
        break;
    }
    case KiranGreeterPrefs::ScaleMode_Manual:
    {
        double scaleFcator = KiranGreeterPrefs::instance()->scaleFactor();
        scaled_factor      = scaleFcator;
        ScalingHelper::set_scale_factor(scaleFcator);
        break;
    }
    case KiranGreeterPrefs::ScaleMode_Disable:
        break;
    default:
        qWarning() << "enable-scaling: unsupported options";
        break;
    }

    QApplication a(argc, argv);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    if (!CursorHelper::setDefaultCursorSize(scaled_factor))
    {
        qWarning() << "setDefaultCursorSize" << scaled_factor << "failed";
    }
    if (!CursorHelper::setRootWindowWatchCursor())
    {
        qWarning() << "setRootWindowWatchCursor failed";
    }

    //capslock numlock
    initLockStatus();

    ///翻译
    QTranslator tsor;
    QString     translationFileDir = QString("/usr/share/%1/translations/").arg(qAppName());
    tsor.load(QLocale(), qAppName(), ".", translationFileDir, ".qm");
    qApp->installTranslator(&tsor);

    ///加载样式表
    QFile file(DEFAULT_STYLE_FILE);
    if (file.open(QIODevice::ReadOnly))
    {
        qApp->setStyleSheet(file.readAll());
    }
    else
    {
        qWarning() << "load style sheet failed";
    }

#ifdef VIRTUAL_KEYBOARD
    ///初始键盘配置
    GreeterKeyboard::instance()->init();
#endif

    ///初始化屏幕管理,在屏幕管理中创建背景窗口和登录窗口，负责处理屏幕增加删除的情况
    GreeterScreenManager screenManager;
    screenManager.init();

    return a.exec();
}
