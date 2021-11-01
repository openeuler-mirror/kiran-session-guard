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

#include "config.h"

#define TRANSLATION_FILE_DIR "/usr/share/kiran-screensaver-dialog/translations/"

#if !BUILD_SCREENSAVER_PLUGIN
#include <QApplication>
#include "screensaver-dialog.h"
#include <kiran-screensaver/ks-interface.h>
#include <kiran-log/qt5-log-i.h>
class FakeKsInterface:public KSInterface
{
public:
    void authenticationPassed() override
    {
        qApp->exit();
    }
};
int main(int argc, char* argv[])
{
    QApplication app(argc,argv);
    klog_qt5_init("","kylinsec-session","kiran-screensaver-dialog","kiran-screensaver-dialog");
    Q_INIT_RESOURCE(commonWidgets);
    FakeKsInterface fakeInterface;
    ScreenSaverDialog dialog(&fakeInterface);
    dialog.show();
    dialog.fadeIn();
    return app.exec();
}
#endif