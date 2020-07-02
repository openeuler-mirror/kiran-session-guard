#include <QTranslator>
#include <unistd.h>
#include <sys/types.h>

#include "greetersetting.h"
#include "single/singleapplication.h"
#include "log.h"
#include "lightdmprefs.h"

#define TRANSLATION_FILE_DIR "/usr/share/lightdm-kiran-greeter/translations/"
int main(int argc, char *argv[])
{
    ///初始化日志模块
    Log::instance()->init("/tmp/lightdm-kiran-greeter-setting.log");
    qInstallMessageHandler(Log::messageHandler);

    if(getuid()!=0||getgid()!=0){
        qWarning() << "need run with admin privilege.";
        return 0;
    }

    ///翻译
    QTranslator tsor;
    //filename+prefix+language name+suffix
    qInfo() << "load translation file: " << tsor.load(QLocale(),
                                                      "kiran-greeter-settings"/*filename*/,
                                                      "."/*prefix*/,
                                                      TRANSLATION_FILE_DIR/*dir*/,
                                                      ".qm"/*suffix*/);
    SingleApplication a(argc,argv);
    qApp->installTranslator(&tsor);
    LightdmPrefs::instance();
    GreeterSetting w;
    w.show();
    return a.exec();
}
