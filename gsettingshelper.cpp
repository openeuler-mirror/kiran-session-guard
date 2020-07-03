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

int GSettingsHelper::getMateScalingFactor()
{
    GSettings* settings = g_settings_new("org.mate.interface");
    if(!settings){
        return 0;
    }
    GSettingsSchema* mateSchema = g_settings_schema_source_lookup(g_settings_schema_source_get_default(),"org.mate.interface",FALSE);
    if(!g_settings_schema_has_key(mateSchema,"window-scaling-factor")){
        g_settings_schema_unref(mateSchema);
        g_object_unref(settings);
        return 0;
    }
    int res = g_settings_get_int(settings,"window-scaling-factor");
    g_settings_schema_unref(mateSchema);
    g_object_unref(settings);
    return res;
}
