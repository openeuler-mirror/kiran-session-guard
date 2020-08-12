#include <QTranslator>
#include <unistd.h>
#include <sys/types.h>
#include <QCommandLineParser>
#include <QStringList>
#include <QFile>

#include "greetersetting.h"
#include "single/singleapplication.h"
#include "log.h"
#include "lightdmprefs.h"
#include "scalinghelper.h"

#define TRANSLATION_FILE_DIR "/usr/share/lightdm-kiran-greeter/translations/"
#define ENV_XDG_CURRENT_DESKTOP "XDG_CURRENT_DESKTOP"
#define DEFAULT_STYLE_FILE   ":/themes/kiran-greeter-settings-normal.qss"

#include "dbusapi.h"
int main(int argc, char *argv[])
{
    ///初始化日志模块
    Log::instance()->init("/tmp/lightdm-kiran-greeter-setting.log");
    qInstallMessageHandler(Log::messageHandler);

#ifndef TEST
    ///限制普通用户启动
    if(getuid()!=0||getgid()!=0){
        qWarning() << "need run with admin privilege.";
        return 0;
    }
#endif

    ///FIXME:为了解决Qt在Root权限下，启动的文件选择框和操作系统文件选择框样式不同
    ///      通过手动设置XDG_CURRENT_DESKTOP环境变量
    ///参数解析
    QStringList arguments;
    for(int i=0;i<argc;i++){
        arguments << argv[i];
    }

    QCommandLineParser parser;
    QCommandLineOption envOption("xdg-desktop","set environment to XDG_CURRENT_DESKTOP","env","");
    QCommandLineOption windowScalingFactor("window-scaling-factor","set window scaling factor","scaling-factor","1");
    parser.addOption(envOption);
    parser.addOption(windowScalingFactor);
    parser.addHelpOption();

    parser.process(arguments);
    if(parser.isSet(envOption)){
        qputenv(ENV_XDG_CURRENT_DESKTOP,parser.value(envOption).toUtf8());
    }

    int scalingFactor = 0;
    if(parser.isSet(windowScalingFactor)){
        bool toIntOk = false;
        int tmp =  parser.value(windowScalingFactor).toInt(&toIntOk);
        if(toIntOk){
            scalingFactor = tmp;
        }else{
            qWarning() << "toInt failed";
        }
    }
    switch (scalingFactor) {
    case 0:
        ScalingHelper::auto_calculate_screen_scaling();
        break;
    case 1:
        break;
    case 2:
        ScalingHelper::set_scale_factor(2);
        break;
    default:
        qWarning() << "Unsupported option" << "window-scaling-factor" << scalingFactor;
        break;
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

    ///加载样式表
    QFile styleFile(DEFAULT_STYLE_FILE);
    if(styleFile.open(QIODevice::ReadOnly)){
        qApp->setStyleSheet(styleFile.readAll());
    }else{
        qWarning() << "load style sheet failed";
    }

    LightdmPrefs::instance();
    GreeterSetting w;
    w.show();

    return a.exec();
}
