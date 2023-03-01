#include <QApplication>
#include <PolkitQt1/Subject>
#include <kiran-single-application.h>
#include <qt5-log-i.h>
#include <unistd.h>

#include "guard-global.h"
#include "listener.h"
#include "dialog.h"

GUARD_POLKIT_AGENT_USING_NAMESPACE

int main(int argc,char* argv[])
{
    Q_INIT_RESOURCE(commonWidgets);
    
    KiranSingleApplication app(argc,argv);
    app.setQuitOnLastWindowClosed(false);

    if( klog_qt5_init("","kylinsec-session","kiran-polkit-agent","kiran-polkit-agent") != 0 )
    {
        qWarning() << "kiran-log initialization failed!";
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