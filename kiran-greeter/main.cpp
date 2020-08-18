#include <QApplication>
#include <QDebug>
#include <QTranslator>
#include <QFile>
#include <signal.h>

#include "greeterloginwindow.h"
#include "log.h"
#include "greetersetting.h"
#include "greeterscreenmanager.h"
#include "greeterkeyboard.h"
#include "scalinghelper.h"
#include "cursorhelper.h"
#include "synclockstatus.h"

#define TRANSLATION_FILE_DIR "/usr/share/lightdm-kiran-greeter/translations"
#define DEFAULT_STYLE_FILE ":/themes/lightdm-kiran-greeter-normal.qss"

void termSignalHandler(int unused){
    qInfo() << "termSignalHandler";
    GreeterKeyboard::instance()->resetParentAndTermProcess();
}

void setup_unix_signal_handlers(){
    struct sigaction term;
    term.sa_handler = termSignalHandler;
    sigemptyset(&term.sa_mask);
    term.sa_flags = 0;
    term.sa_flags |= SA_RESETHAND;
    int iRet = sigaction(SIGTERM,&term,0);
    if(iRet!=0){
        qWarning() << "setup_unix_signal_handlers failed," << strerror(iRet);
    }
}

int main(int argc, char *argv[])
{
    ///初始化日志模块
    Log::instance()->init("/tmp/lightdm-kiran-greeter.log");
    qInstallMessageHandler(Log::messageHandler);

    setup_unix_signal_handlers();

    ///读取Greeter配置
    GreeterSetting::instance()->dumpGreeterSetting();

    ///设置缩放比
    double scaled_factor = 0.0;
    switch (GreeterSetting::instance()->getEnableScaling()) {
    case GreeterSetting::SCALING_AUTO:
    {
        ScalingHelper::auto_calculate_screen_scaling(scaled_factor);
        break;
    }
    case GreeterSetting::SCALING_ENABLE:
    {
        double scaleFcator = GreeterSetting::instance()->getScaleFactor();
        scaled_factor = scaleFcator;
        ScalingHelper::set_scale_factor(scaleFcator);
        break;
    }
    case GreeterSetting::SCALING_DISABLE:
        break;
    default:
        qWarning() << "enable-scaling: unsupported options";
        break;
    }

    QApplication a(argc, argv);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    if( !CursorHelper::setDefaultCursorSize(scaled_factor) ){
        qWarning() << "setDefaultCursorSize" << scaled_factor << "failed";
    }
    if(!CursorHelper::setRootWindowWatchCursor()){
        qWarning() << "setRootWindowWatchCursor failed";
    }

    //capslock numlock
    initLockStatus();

    ///翻译
    QTranslator tsor;
    //filename+prefix+language name+suffix
    qInfo() << "load translation file: " << tsor.load(QLocale(),
                                                      "lightdm-kiran-greeter"/*filename*/,
                                                      "."/*prefix*/,
                                                      TRANSLATION_FILE_DIR/*dir*/,
                                                      ".qm"/*suffix*/);
    qApp->installTranslator(&tsor);

    ///加载样式表
    QFile file(DEFAULT_STYLE_FILE);
    if(file.open(QIODevice::ReadOnly)){
        qApp->setStyleSheet(file.readAll());
    }else{
        qWarning() << "load style sheet failed";
    }

    ///初始键盘配置
    GreeterKeyboard::instance()->init();

    ///初始化屏幕管理,在屏幕管理中创建背景窗口和登录窗口，负责处理屏幕增加删除的情况
    GreeterScreenManager::instance()->init();

    return a.exec();
}
