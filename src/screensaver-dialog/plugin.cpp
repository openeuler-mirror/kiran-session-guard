/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd.
 * kiran-screensaver is licensed under Mulan PSL v2.
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
#include "plugin.h"
#include "frame.h"
#include "power.h"
#include "prefs.h"
#include "virtual-keyboard.h"

#include <qt5-log-i.h>
#include <QApplication>
#include <QLocale>

#define TRANSLATION_FILE_DIR "/usr/share/kiran-screensaver-dialog/translations/"

using namespace Kiran::ScreenSaver;

static void initResources()
{
    Q_INIT_RESOURCE(commonWidgets);
    Q_INIT_RESOURCE(loginFrame);
}

static void cleanupResouces()
{
    Q_CLEANUP_RESOURCE(commonWidgets);
    Q_CLEANUP_RESOURCE(loginFrame);
}

namespace Kiran
{
namespace SessionGuard
{
namespace Locker
{
int KSPlugin::init(Interface* ksInterface)
{
    Q_ASSERT(ksInterface != nullptr);
    m_ksInterface = ksInterface;

    initResources();

    Prefs::globalInit();
    Power::globalInit(Prefs::getInstance());

    m_translator = new QTranslator;
    if (m_translator->load(QLocale(),
                           "kiran-screensaver-dialog",
                           ".",
                           TRANSLATION_FILE_DIR,
                           ".qm"))
    {
        qApp->installTranslator(m_translator);
    }
    else
    {
        KLOG_WARNING() << "can't load kiran-screensaver-dialog translator";
    }

    return 0;
}

void KSPlugin::uninit()
{
    cleanupResouces();

    Power::globalDeinit();
    Prefs::globalDeinit();

    if (m_translator != nullptr)
    {
        qApp->removeTranslator(m_translator);
    }
}

LockerInterface* KSPlugin::createLocker()
{
    LockerInterface* lockerInterface = new Frame(m_ksInterface, Power::getInstance());
    return lockerInterface;
}

}  // namespace Locker
}  // namespace SessionGuard
}  // namespace Kiran