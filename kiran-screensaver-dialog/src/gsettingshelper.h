#ifndef GSETTINGSHELPER_H
#define GSETTINGSHELPER_H

#include <QString>

class GSettingsHelper
{
private:
    GSettingsHelper();

public:
    static QString getBackgrountPath();
    static int getMateScalingFactor();
};

#endif  // GSETTINGSHELPER_H
