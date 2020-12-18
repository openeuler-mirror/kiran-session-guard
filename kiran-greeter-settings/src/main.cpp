#include <QTranslator>
#include <unistd.h>
#include <sys/types.h>
#include <QCommandLineParser>
#include <QStringList>
#include <QFile>
#include <kiran-single-application.h>

#include "greetersetting.h"
#include "single/singleapplication.h"
#include "log.h"
#include "lightdmprefs.h"
#include "dbusapi.h"

#define ENV_XDG_CURRENT_DESKTOP "XDG_CURRENT_DESKTOP"
#define DEFAULT_STYLE_FILE   ":/themes/kiran-greeter-settings-normal.qss"

int main(int argc, char *argv[])
{
    ///初始化日志模块
    Log::instance()->init("/tmp/lightdm-kiran-greeter-settings.log");
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
    parser.addOption(envOption);
    parser.addHelpOption();

    parser.process(arguments);
    if(parser.isSet(envOption)){
        qputenv(ENV_XDG_CURRENT_DESKTOP,parser.value(envOption).toUtf8());
    }

    KiranSingleApplication a(argc,argv);

    ///翻译
    QTranslator tsor;
    QString translationFileDir = QString("/usr/share/%1/translations/").arg(qAppName());
    bool loadTsor = tsor.load(QLocale(),qAppName(),".",translationFileDir,".qm");
    qInfo() << "load translation:" << loadTsor;
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
