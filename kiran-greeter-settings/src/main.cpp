#include <kiran-message-box.h>
#include <kiran-single-application.h>
#include <zlog_ex.h>
#include <QCommandLineParser>
#include <QTranslator>

#include "greeter-setting-window.h"
#include "kiran-greeter-prefs.h"
#include "log.h"

#define DEFAULT_STYLE_FILE ":/themes/kiran-greeter-settings-normal.qss"

int main(int argc, char *argv[])
{
    ///初始化日志模块
    dzlog_init_ex(NULL,
                  "kylinsec-session",
                  "lightdm-kiran-greeter",
                  "kiran-greeter-settings");
    Log::instance()->init();
    qInstallMessageHandler(Log::messageHandler);

    KiranSingleApplication a(argc, argv);

    ///翻译
    QTranslator translator;
    QString     translationFileDir = QString("/usr/share/lightdm-kiran-greeter/translations/");
    bool        loadRes            = translator.load(QLocale(), qAppName(), ".", translationFileDir, ".qm");
    if (!loadRes)
    {
        LOG_WARNING("load translation file failed!");
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
