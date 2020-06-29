#include "screensaverdialog.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QTranslator>
#include <QDebug>
#include "log.h"
#include "greeterkeyboard.h"
#include "gsettingshelper.h"
#include "scalinghelper.h"

#define TRANSLATION_FILE_DIR "/usr/share/kiran-screensaver-dialog/translations/"

int main(int argc, char *argv[])
{
    ///初始化日志模块,需提供verbose启动参数日志才会写入文件
    Log::instance()->init("/tmp/kiran-screensaver-dialog.log");
    qInstallMessageHandler(Log::messageHandler);

    ///scaling
    int windowScalingFactor = GSettingsHelper::getMateScalingFactor();
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
    if(!GreeterKeyboard::instance().init()){
        qWarning() << "init keyboard failed";
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
