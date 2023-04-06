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

#include "gsettings-helper.h"
#include <gio/gio.h>
#include <glib-object.h>
#include <qt5-log-i.h>
#include <QDebug>

namespace Kiran
{
namespace SessionGuard
{
namespace Locker
{
GSettingsHelper::GSettingsHelper()
{
}

QString GSettingsHelper::getBackgrountPath()
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

int GSettingsHelper::getMateScalingFactor()
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
}  // namespace Locker
}  // namespace SessionGuard
}  // namespace Kiran