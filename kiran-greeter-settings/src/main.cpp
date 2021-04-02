#include <kiran-message-box.h>
#include <kiran-single-application.h>
#include <QCommandLineParser>
#include <QTranslator>

#include "greeter-setting-window.h"
#include "kiran-greeter-prefs.h"
#include "log.h"

#define DEFAULT_STYLE_FILE ":/themes/kiran-greeter-settings-normal.qss"

int main(int argc, char *argv[])
{
    ///初始化日志模块
    Log::instance()->init("/tmp/lightdm-kiran-greeter-setting.log");
    qInstallMessageHandler(Log::messageHandler);

    KiranSingleApplication a(argc, argv);

    ///翻译
    QTranslator translator;
    QString     translationFileDir = QString("/usr/share/%1/translations/").arg(qAppName());
    bool        loadRes           = translator.load(QLocale(), qAppName(), ".", translationFileDir, ".qm");
    if(!loadRes){
        qWarning() << "load translation file faield";
    }
    qApp->installTranslator(&translator);
    
    auto prefs = KiranGreeterPrefs::instance();
    if (!prefs->isValid())
    {
        KiranMessageBox::message(nullptr, QObject::tr("Warning"),
                                 QObject::tr("failed to connect to the back end of the system,please try again"),
                                 KiranMessageBox::Yes);
        return -1;
    }

    GreeterSettingWindow w;
    w.show();

    return a.exec();
}
