#include <QTranslator>
#include <unistd.h>
#include <sys/types.h>
#include <QCommandLineParser>
#include <QStringList>
#include <QFile>
#include <kiran-single-application.h>
#include <kiran-message-box.h>

#include "greeter-setting-window.h"
#include "single/singleapplication.h"
#include "log.h"
#include "kiran-greeter-prefs.h"

#define TRANSLATION_FILE_DIR "/usr/share/lightdm-kiran-greeter/translations/"
#define ENV_XDG_CURRENT_DESKTOP "XDG_CURRENT_DESKTOP"
#define DEFAULT_STYLE_FILE   ":/themes/kiran-greeter-settings-normal.qss"

int main(int argc, char *argv[])
{
    ///初始化日志模块
    Log::instance()->init("/tmp/lightdm-kiran-greeter-setting.log");
    qInstallMessageHandler(Log::messageHandler);

    KiranSingleApplication a(argc,argv);

    ///翻译
    QTranslator tsor;
    QString translationFileDir = QString("/usr/share/%1/translations/").arg(qAppName());
    bool loadTsor = tsor.load(QLocale(),qAppName(),".",translationFileDir,".qm");
    qInfo() << "load translation:" << loadTsor;
    qApp->installTranslator(&tsor);

    ///加载样式表
//    QFile styleFile(DEFAULT_STYLE_FILE);
//    if(styleFile.open(QIODevice::ReadOnly)){
//        qApp->setStyleSheet(styleFile.readAll());
//    }else{
//        qWarning() << "load style sheet failed";
//    }

    auto prefs = KiranGreeterPrefs::instance();
    if( !prefs->isValid() ){
        KiranMessageBox::message(nullptr,QObject::tr("Warning"),
                                 QObject::tr("failed to connect to the back end of the system,please try again"),
                                 KiranMessageBox::Yes);
        return -1;
    }

    GreeterSettingWindow w;
    w.show();

    return a.exec();
}
