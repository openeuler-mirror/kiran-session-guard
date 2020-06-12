#include "gsettingshelper.h"
#include <gio/gio.h>
#include <glib-object.h>

GSettingsHelper::GSettingsHelper()
{

}

QString GSettingsHelper::getBackgrountPath()
{
    QString path;
    GSettings* settings = g_settings_new("org.mate.background");
    if(!settings){
        return QString();
    }

    gchar* picture = g_settings_get_string(settings,"picture-filename");
    if(!picture){
        return QString();
    }

    path = picture;
    g_free(picture);
    g_object_unref(settings);
    return path;
}
