#include "screensaverdialog.h"
#include "log.h"
#include "greeterkeyboard.h"
#include "gsettingshelper.h"
#include "scalinghelper.h"
#include "singleapplication.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QTranslator>
#include <QDebug>
#include <QFile>
#include <signal.h>
#include <zlog_ex.h>

#define TRANSLATION_FILE_DIR "/usr/share/kiran-screensaver-dialog/translations/"
#define DEFAULT_STYLE_PATH ":/styles/kiran-screensaver-dialog-normal.qss"

void termSignalHandler (int unused)
{
#ifdef VIRTUAL_KEYBOARD
    ///初始化虚拟键盘
    GreeterKeyboard::instance()->keyboardProcessExit();
#endif
    qApp->quit();
}

void setupUnixSignalHandlers ()
{
    struct sigaction term;
    term.sa_handler = termSignalHandler;
    sigemptyset(&term.sa_mask);
    term.sa_flags = 0;
    term.sa_flags |= SA_RESETHAND;
    int iRet = sigaction(SIGTERM, &term, nullptr);
    if (iRet != 0)
    {
        LOG_WARNING_S() << "setupUnixSignalHandlers failed," << strerror(iRet);
    }
}

void handleWindowScaleFactor ()
{
    ///scaling
    int windowScalingFactor = GSettingsHelper::getMateScalingFactor();
    LOG_INFO_S() << "screensaver-dialog scale-factor: " << windowScalingFactor;
    switch (windowScalingFactor)
    {
        case 0:ScalingHelper::auto_calculate_screen_scaling();
            break;
        case 1:break;
        case 2:ScalingHelper::set_scale_factor(2);
            break;
        default:
            LOG_WARNING_S() << "Unsupported option" << "window-scaling-factor" << windowScalingFactor;
            break;
    }
}


int main (int argc, char *argv[])
{
    ///初始化日志模块,需提供verbose启动参数日志才会写入文件
    dzlog_init_ex(NULL,
                  "kylinsec-session",
                  "kiran-screensaver-dialog",
                  "kiran-screensaver-dialog");
    Log::instance()->init();
    qInstallMessageHandler(Log::messageHandler);
#ifdef TEST
    Log::instance()->setAppend2File(true);
#endif

    setupUnixSignalHandlers();

    handleWindowScaleFactor();

    QCoreApplication::setAttribute(Qt::AA_DisableSessionManager);
    SingleApplication app(argc, argv);

    ///安装翻译
    QTranslator tsor;
    LOG_INFO_S() << "load translation file: "
                 << tsor.load(QLocale(),
                              "kiran-screensaver-dialog" /*filename*/,
                              "." /*prefix*/, TRANSLATION_FILE_DIR /*dir*/,
                              ".qm" /*suffix*/);
    qApp->installTranslator(&tsor);

    ///参数解析
    //部分没使用到的,只是为了添加兼容mate-screensaver启动dialog的参数
    QCommandLineParser parser;
    QCommandLineOption logoutEnableOption("enable-logout", "whether to allow logout");
    QCommandLineOption logoutCommandOption("logout-command", "logout command");
    QCommandLineOption statusMsgOption("status-message", "status message", "status message", "");
    QCommandLineOption enableSwitchOption("enable-switch", "whether to allow switching users");
    QCommandLineOption verboseOption("verbose","verbose");
    parser.addOptions({logoutEnableOption, logoutCommandOption, statusMsgOption, enableSwitchOption});
    parser.addHelpOption();
    parser.process(app);

#ifdef VIRTUAL_KEYBOARD
    ///初始化虚拟键盘
    if(!GreeterKeyboard::instance()->init()){
        qWarning() << "init keyboard failed";
    }
#endif

    ///加载样式文件
    QFile file(DEFAULT_STYLE_PATH);
    if (file.open(QIODevice::ReadOnly))
    {
        qApp->setStyleSheet(file.readAll());
    }
    else
    {
        LOG_WARNING_S() << "load style sheet failed";
    }

    ScreenSaverDialog w;
    if (parser.isSet(enableSwitchOption))
    {
        w.setSwitchUserEnabled(true);
    }
    w.show();
    return app.exec();
}
