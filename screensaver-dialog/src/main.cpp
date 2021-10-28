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

#if BUILD_SCREENSAVER_PLUGIN
#include <kiran-screensaver/ks-interface.h>
#include <kiran-screensaver/ks-plugin-interface.h>
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include "screensaver-dialog.h"

class KSPlugin : public QObject, public KSPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID KSPluginInterface_iid)
    Q_INTERFACES(KSPluginInterface)

public:
    KSPlugin() = default;
    ~KSPlugin() = default;

    int init(KSInterface* ksInterface) override
    {
        Q_ASSERT(ksInterface != nullptr);
        m_ksInterface = ksInterface;

        Q_INIT_RESOURCE(commonWidgets);

        m_translator = new QTranslator;
        m_translator->load(QLocale(),
                           "kiran-screensaver-dialog",
                           ".",
                           TRANSLATION_FILE_DIR,
                           ".qm");
        qApp->installTranslator(m_translator);

        return 0;
    }

    void uninit() override
    {
        Q_CLEANUP_RESOURCE(commonWidgets);

        if (m_translator != nullptr)
        {
            qApp->removeTranslator(m_translator);
        }
    }

    KSLockerInterface* createLocker() override
    {
        KSLockerInterface* lockerInterface = new ScreenSaverDialog(m_ksInterface);
        return lockerInterface;
    }

private:
    KSInterface* m_ksInterface = nullptr;
    QTranslator* m_translator = nullptr;
};
#include "main.moc"
#else
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