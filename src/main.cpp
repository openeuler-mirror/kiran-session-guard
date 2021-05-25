#include <signal.h>
#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QTranslator>
#include <zlog_ex.h>
#include <kiran-system-daemon/greeter_i.h>

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
#ifdef VIRTUAL_KEYBOARD
    GreeterKeyboard::instance()->keyboardProcessExit();
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
    int iRet      = sigaction(SIGTERM, &term, 0);
    if (iRet != 0)
    {
        LOG_WARNING_S() << "setup_unix_signal_handlers failed," << strerror(iRet);
    }
}

int main(int argc, char *argv[])
{
    ///初始化日志模块
    dzlog_init_ex(NULL,
                  "kylinsec-nologin",
                  "lightdm-kiran-greeter",
                  "lightdm-kiran-greeter");
    Log::instance()->init();
    qInstallMessageHandler(Log::messageHandler);

    setup_unix_signal_handlers();

    ///设置缩放比
    double scaled_factor = 0.0;
    switch (KiranGreeterPrefs::instance()->scale_mode())
    {
    case GREETER_SCALING_MODE_AUTO:
    {
        ScalingHelper::auto_calculate_screen_scaling(scaled_factor);
        break;
    }
    case GREETER_SCALING_MODE_MANUAL:
    {
        double scaleFcator = KiranGreeterPrefs::instance()->scale_factor();
        scaled_factor      = scaleFcator;
        ScalingHelper::set_scale_factor(scaleFcator);
        break;
    }
    case GREETER_SCALING_MODE_DISABLE:
        break;
    default:
        LOG_ERROR("enable-scaling: unsupported options %d",KiranGreeterPrefs::instance()->scale_mode());
        break;
    }

    QApplication a(argc, argv);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    if (!CursorHelper::setDefaultCursorSize(scaled_factor))
    {
        LOG_ERROR("set default cursor size for factor %lf failed!",scaled_factor);
    }
    if (!CursorHelper::setRootWindowWatchCursor())
    {
        LOG_ERROR("set root window watch cursor failed!");
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
        LOG_ERROR("load style sheet failed!");
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
