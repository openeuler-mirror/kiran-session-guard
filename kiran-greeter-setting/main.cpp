#include <QTranslator>
#include <unistd.h>
#include <sys/types.h>
#include <QCommandLineParser>

#include "greetersetting.h"
#include "single/singleapplication.h"
#include "log.h"
#include "lightdmprefs.h"
#include <QStringList>

#define TRANSLATION_FILE_DIR "/usr/share/lightdm-kiran-greeter/translations/"
#define ENV_XDG_CURRENT_DESKTOP "XDG_CURRENT_DESKTOP"


int main(int argc, char *argv[])
{
    ///初始化日志模块
    Log::instance()->init("/tmp/lightdm-kiran-greeter-setting.log");
    qInstallMessageHandler(Log::messageHandler);

    ///FIXME:为了解决Qt在Root权限下，启动的文件选择框和操作系统文件选择框样式不同
    ///      通过手动设置XDG_CURRENT_DESKTOP环境变量
    ///参数解析
    QStringList arguments;
    for(int i=0;i<argc;i++){
        arguments << argv[i];
    }
    QCommandLineParser parser;
    QCommandLineOption envOption("xdg-desktop","set environment to XDG_CURRENT_DESKTOP","env","");
    parser.addHelpOption();
    parser.addOption(envOption);
    parser.process(arguments);
    if(parser.isSet(envOption)){
        qputenv(ENV_XDG_CURRENT_DESKTOP,parser.value(envOption).toUtf8());
    }

    SingleApplication a(argc,argv);
    ///翻译
    QTranslator tsor;
    //filename+prefix+language name+suffix
    qInfo() << "load translation file: " << tsor.load(QLocale(),
                                                      "kiran-greeter-settings"/*filename*/,
                                                      "."/*prefix*/,
                                                      TRANSLATION_FILE_DIR/*dir*/,
                                                      ".qm"/*suffix*/);
    qApp->installTranslator(&tsor);

    LightdmPrefs::instance();
    GreeterSetting w;
    w.show();
    return a.exec();
}
