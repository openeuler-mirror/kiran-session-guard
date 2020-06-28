#include "greetersetting.h"
#include "single/singleapplication.h"
#include "log.h"
#include "lightdmprefs.h"
#include <unistd.h>
#include <sys/types.h>

int main(int argc, char *argv[])
{
    ///初始化日志模块
    Log::instance()->init("/tmp/lightdm-kiran-greeter-setting.log");
    qInstallMessageHandler(Log::messageHandler);

    if(getuid()!=0||getgid()!=0){
        qWarning() << "need run with admin privilege.";
        return 0;
    }

    SingleApplication a(argc,argv);

    LightdmPrefs::instance();

    GreeterSetting w;
    w.show();

    return a.exec();
}
