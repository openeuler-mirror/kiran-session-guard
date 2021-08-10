#include "gsettingshelper.h"
#include <gio/gio.h>
#include <glib-object.h>
#include <QDebug>
#include <qt5-log-i.h>

GSettingsHelper::GSettingsHelper ()
{

}

QString GSettingsHelper::getBackgrountPath ()
{
    QString path;
    GSettings *settings = g_settings_new("org.mate.background");
    if (!settings)
    {
        return QString();
    }

    gchar *picture = g_settings_get_string(settings, "picture-filename");
    if (!picture)
    {
        return QString();
    }

    path = picture;
    g_free(picture);
    g_object_unref(settings);
    return path;
}

int GSettingsHelper::getMateScalingFactor ()
{
    GSettings *settings = g_settings_new("org.mate.interface");
    if (!settings)
    {
        KLOG_WARNING() << "g_settings_new org.mate.interface failed";
        return 0;
    }

    GSettingsSchemaSource *schemaSource = g_settings_schema_source_get_default();
    if (schemaSource == nullptr)
    {
        KLOG_WARNING() << "g_settings_schema_source_get_default failed";
        return 0;
    }

    GSettingsSchema *mateSchema = g_settings_schema_source_lookup(schemaSource, "org.mate.interface", TRUE);
    if (mateSchema == nullptr)
    {
        KLOG_WARNING() << "g_settings_schema_source_lookup org.mate.interface failed";
        return 0;
    }

    if (!g_settings_schema_has_key(mateSchema, "window-scaling-factor"))
    {
        g_settings_schema_unref(mateSchema);
        g_object_unref(settings);
        return 0;
    }

    int res = g_settings_get_int(settings, "window-scaling-factor");
    g_settings_schema_unref(mateSchema);
    g_object_unref(settings);
    return res;
}
