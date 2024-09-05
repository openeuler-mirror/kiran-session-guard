/**
 * Copyright (c) 2020 ~ 2023 KylinSec Co., Ltd.
 * kiran-session-guard is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     liuxinhao <liuxinhao@kylinsec.com.cn>
 */
#include <kiran-single-application.h>
#include <qt5-log-i.h>
#include <unistd.h>
#include <PolkitQt1/Subject>
#include <QApplication>
#include <QTranslator>

#include "dialog.h"
#include "listener.h"

using namespace ::Kiran::SessionGuard::PolkitAgent;

int main(int argc, char* argv[])
{
    Q_INIT_RESOURCE(commonWidgets);
    // 用户内单例可能存在同一用户登录多个图形会话（本地+远程）
    // 加入DISPLAY信息一起做进程单例的判断，确保一个会话内只存在一个。
    // XDG_SESSION_ID不可用，部分图形会话可能不会新创建。
    // 后续新版本，应将该方法内置到KiranSingleApplication。
    KiranSingleApplication::addApplicationIDUserData(qgetenv("DISPLAY"));
    KiranSingleApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    if (klog_qt5_init("", "kylinsec-session", "kiran-polkit-agent", "kiran-polkit-agent") != 0)
    {
        qWarning() << "kiran-log initialization failed!";
    }

    QScopedPointer<QTranslator> translatorPointer(new QTranslator);
    QString translationFileDir = QString("/usr/share/%1/translations/").arg(qAppName());
    if( !translatorPointer->load(QLocale(), qAppName(), ".", translationFileDir, ".qm") )
    {
        KLOG_WARNING() << "can't load translator";
    }
    else
    {
        app.installTranslator(translatorPointer.data());
    }

    PolkitQt1::UnixSessionSubject session(app.applicationPid());
    Listener listener;
    if (!listener.registerListener(session, "/com/kylinsec/Kiran/PolkitAgent"))
    {
        KLOG_WARNING() << "register listener failed!";
        return EXIT_FAILURE;
    }

    return app.exec();
}
