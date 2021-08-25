/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd.
 * kiran-session-guard is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     liuxinhao <liuxinhao@kylinos.com.cn>
 */

#include "gsettings-helper.h"
#include "scaling-helper.h"
#include "screensaver-dialog.h"
#include "virtual-keyboard.h"

#include <kiran-single-application.h>
#include <qt5-log-i.h>
#include <signal.h>
#include <QCommandLineParser>
#include <QDebug>
#include <QFile>
#include <QTranslator>

#define TRANSLATION_FILE_DIR "/usr/share/kiran-screensaver-dialog/translations/"
#define DEFAULT_STYLE_PATH ":/styles/kiran-screensaver-dialog-normal.qss"

void termSignalHandler(int unused)
{
    ///初始化虚拟键盘
    VirtualKeyboard::instance()->keyboardProcessExit();
    qApp->quit();
}

void setupUnixSignalHandlers()
{
    struct sigaction term;
    term.sa_handler = termSignalHandler;
    sigemptyset(&term.sa_mask);
    term.sa_flags = 0;
    term.sa_flags |= SA_RESETHAND;
    int iRet = sigaction(SIGTERM, &term, nullptr);
    if (iRet != 0)
    {
        KLOG_WARNING() << "setup signal handle failed," << strerror(iRet);
    }
}

void handleWindowScaleFactor()
{
    int windowScalingFactor = GSettingsHelper::getMateScalingFactor();

    KLOG_DEBUG() << "scale factor: " << windowScalingFactor;

    switch (windowScalingFactor)
    {
    case 0:
        ScalingHelper::auto_calculate_screen_scaling();
        break;
    case 1:
        break;
    case 2:
        ScalingHelper::set_scale_factor(2);
        break;
    default:
        KLOG_WARNING() << "unsupported option"
                       << "window-scaling-factor" << windowScalingFactor;
        break;
    }
}

int main(int argc, char *argv[])
{
    klog_qt5_init("",
                  "kylinsec-session",
                  "kiran-screensaver-dialog",
                  "kiran-screensaver-dialog");

    setupUnixSignalHandlers();

    handleWindowScaleFactor();

    QCoreApplication::setAttribute(Qt::AA_DisableSessionManager);
    KiranSingleApplication app(argc, argv);

    //对静态库中包含的资源文件进行初始化
    Q_INIT_RESOURCE(commonWidgets);

    QTranslator tsor;
    tsor.load(QLocale(),
              "kiran-screensaver-dialog",
              ".", TRANSLATION_FILE_DIR,
              ".qm");
    if (tsor.isEmpty())
    {
        KLOG_ERROR() << "can't load translation!";
    }
    qApp->installTranslator(&tsor);

    QCommandLineParser parser;
    QCommandLineOption logoutEnableOption("enable-logout", "whether to allow logout");
    QCommandLineOption logoutCommandOption("logout-command", "logout command");
    QCommandLineOption statusMsgOption("status-message", "status message", "status message", "");
    QCommandLineOption enableSwitchOption("enable-switch", "whether to allow switching users");
    QCommandLineOption verboseOption("verbose", "verbose");
    parser.addOptions({logoutEnableOption, logoutCommandOption, statusMsgOption, enableSwitchOption});
    parser.addHelpOption();
    parser.process(app);

#ifdef VIRTUAL_KEYBOARD
    ///初始化虚拟键盘
    if (!VirtualKeyboard::instance()->init())
    {
        qWarning() << "init keyboard failed";
    }
#endif

    QFile file(DEFAULT_STYLE_PATH);
    if (file.open(QIODevice::ReadOnly))
    {
        qApp->setStyleSheet(file.readAll());
    }
    else
    {
        KLOG_WARNING() << "load style sheet failed";
    }

    ScreenSaverDialog w;
    if (parser.isSet(enableSwitchOption))
    {
        w.setSwitchUserEnabled(true);
    }
    w.show();

    return app.exec();
}