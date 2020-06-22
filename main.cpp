#include "screensaverdialog.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QTranslator>
#include <QDebug>
#include "log.h"
#include "greeterkeyboard.h"

#define TRANSLATION_FILE_DIR "/usr/share/kiran-screensaver-dialog/translations/"

#include "dbusapihelper.h"
int main(int argc, char *argv[])
{
    ///初始化日志模块
    Log::instance()->init("/tmp/kiran-screensaver-dialog.log");
    qInstallMessageHandler(Log::messageHandler);

    QApplication app(argc, argv);
    qInfo() << "arguments: " <<  app.arguments();

    ///翻译
    QTranslator tsor;
    //filename+prefix+language name+suffix
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
        //TODO
    }
    if( parser.isSet(logoutCommandOption) ){
        //TODO
    }
    if(parser.isSet(statusMsgOption)){
        //TODO
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
