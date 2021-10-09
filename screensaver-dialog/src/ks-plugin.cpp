//
// Created by lxh on 2021/9/17.
//

#include "ks-plugin.h"
#include <QApplication>
#include <QTranslator>
#include <QFile>
#include <qt5-log-i.h>
#include "screensaver-dialog.h"

#define TRANSLATION_FILE_DIR "/usr/share/kiran-screensaver-dialog/translations/"

KSPlugin::KSPlugin()
{
}

KSPlugin::~KSPlugin()
{
}

int KSPlugin::init(KSInterface* ksInterface)
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

void KSPlugin::uninit()
{
    Q_CLEANUP_RESOURCE(commonWidgets);

    if(m_translator!= nullptr)
    {
        qApp->removeTranslator(m_translator);
    }
}

KSLockerInterface* KSPlugin::createLocker()
{
    KSLockerInterface* lockerInterface = new ScreenSaverDialog(m_ksInterface);
    return lockerInterface;
}
