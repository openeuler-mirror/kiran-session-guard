#include "screensaverdialog.h"
#include "log.h"
#include "greeterkeyboard.h"
#include "gsettingshelper.h"
#include "scalinghelper.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QTranslator>
#include <QDebug>
#include <QFile>
#include <signal.h>

#define TRANSLATION_FILE_DIR "/usr/share/kiran-screensaver-dialog/translations/"
#define DEFAULT_STYLE_PATH ":/styles/kiran-screensaver-dialog-normal.qss"


void termSignalHandler(int unused){
    qApp->quit();
}

void setup_unix_signal_handlers(){
    struct sigaction term;
    term.sa_handler = termSignalHandler;
    sigemptyset(&term.sa_mask);
    term.sa_flags = 0;
    term.sa_flags |= SA_RESETHAND;
    int iRet = sigaction(SIGTERM,&term,nullptr);
    if(iRet!=0){
        qWarning() << "setup_unix_signal_handlers failed," << strerror(iRet);
    }
}

int main(int argc, char *argv[])
{
    ///初始化日志模块,需提供verbose启动参数日志才会写入文件
    Log::instance()->init("/tmp/kiran-screensaver-dialog.log");
    qInstallMessageHandler(Log::messageHandler);
#ifdef TEST
    Log::instance()->setAppend2File(true);
#endif

    setup_unix_signal_handlers();

    ///scaling
    int windowScalingFactor = GSettingsHelper::getMateScalingFactor();
    qInfo() << "org.mate.interface window-scaling-factor" << windowScalingFactor;
    switch (windowScalingFactor) {
    case 0:
        ScalingHelper::auto_calculate_screen_scaling();
        break;
    case 1:
        break;
    case 2:
        ScalingHelper::set_scale_factor(2);
        break;
    default:
        qWarning() << "Unsupported option" << "window-scaling-factor" << windowScalingFactor;
        break;
    }

    QCoreApplication::setAttribute(Qt::AA_DisableSessionManager);
    QApplication app(argc, argv);
    qInfo() << "arguments: " <<  app.arguments();

    ///翻译 filename+prefix+language name+suffix
    QTranslator tsor;
    qInfo() << "load translation file: " << tsor.load(QLocale(),
                                                      "kiran-screensaver-dialog"/*filename*/,
                                                      "."/*prefix*/,
                                                      TRANSLATION_FILE_DIR/*dir*/,
                                                      ".qm"/*suffix*/);
    qApp->installTranslator(&tsor);

    ///参数解析
    QCommandLineParser parser;
    QCommandLineOption logoutEnableOption("enable-logout","whether to allow logout");
    QCommandLineOption logoutCommandOption("logout-command","logout command");
    QCommandLineOption statusMsgOption("status-message","status message","status message","");
    QCommandLineOption enableSwitchOption("enable-switch","whether to allow switching users");
    QCommandLineOption verboseOption("verbose","debug mode output log");
    parser.addOptions({logoutEnableOption,logoutCommandOption,statusMsgOption,enableSwitchOption,verboseOption});
    parser.addHelpOption();
    parser.process(app);

    ///初始化虚拟键盘
    if(!GreeterKeyboard::instance()->init()){
        qWarning() << "init keyboard failed";
    }

    ///加载样式文件
    QFile file(DEFAULT_STYLE_PATH);
    if( file.open(QIODevice::ReadOnly) ){
        qApp->setStyleSheet(file.readAll());
    }else{
        qWarning() << "load style sheet failed";
    }

    ScreenSaverDialog w;
    if( parser.isSet(logoutEnableOption) ){
        //
    }
    if( parser.isSet(logoutCommandOption) ){
        //
    }
    if(parser.isSet(statusMsgOption)){
        //
    }
    if(parser.isSet(enableSwitchOption)){
        w.setSwitchUserEnabled(true);
    }
    if(parser.isSet(verboseOption)){
        Log::instance()->setAppend2File(true);
    }
    w.show();
    return app.exec();
}
