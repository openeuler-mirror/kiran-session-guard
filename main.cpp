#include <QApplication>
#include "greeterloginwindow.h"
#include "log.h"
#include "greetersetting.h"
#include "greeterscreenmanager.h"
#include "greeterkeyboard.h"
#include <QDebug>
#include <QTranslator>
#include "scalinghelper.h"

#define TRANSLATION_FILE_DIR "/usr/share/lightdm-kiran-greeter/translations"

int main(int argc, char *argv[])
{
    ///初始化日志模块
    Log::instance()->init("/tmp/lightdm-kiran-greeter.log");
    qInstallMessageHandler(Log::messageHandler);

    ///读取Greeter配置
    GreeterSetting::instance()->dumpGreeterSetting();

    ///设置缩放比
    switch (GreeterSetting::instance()->getEnableScaling()) {
    case GreeterSetting::SCALING_AUTO:
        ScalingHelper::auto_calculate_screen_scaling();
        break;
    case GreeterSetting::SCALING_ENABLE:
        ScalingHelper::set_scale_factor(GreeterSetting::instance()->getScaleFactor());
        break;
    case GreeterSetting::SCALING_DISABLE:
        break;
    default:
        qWarning() << "enable-scaling: unsupported options";
        break;
    }

    QApplication a(argc, argv);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    ///翻译
    QTranslator tsor;
    //filename+prefix+language name+suffix
    qInfo() << "load translation file: " << tsor.load(QLocale(),
                                                      "lightdm-kiran-greeter"/*filename*/,
                                                      "."/*prefix*/,
                                                      TRANSLATION_FILE_DIR/*dir*/,
                                                      ".qm"/*suffix*/);
    qApp->installTranslator(&tsor);

    ///初始键盘配置
    GreeterKeyboard::instance().init();

    ///初始化屏幕管理,在屏幕管理中创建背景窗口和登录窗口，负责处理屏幕增加删除的情况
    GreeterScreenManager::instance()->init();

    return a.exec();
}
