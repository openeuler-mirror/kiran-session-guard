/**
 * Copyright (c) 2020 ~ 2023 KylinSec Co., Ltd.
 * kiran-session-guard is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     liuxinhao <liuxinhao@kylinsec.com.cn>
 */
#include "app-native-event-filter.h"

#include <QString>
#include <QX11Info>
#include <QDebug>
#include <QProcess>

#include <xcb/xcb.h>
#include <xcb/randr.h>
#include <memory>
#include <qt5-log-i.h>

#define XCB_REPLY_CONNECTION_ARG(connection, ...) connection
struct StdFreeDeleter {
    void operator()(void *p) const noexcept { return std::free(p); }
};
#define XCB_REPLY(call, ...) \
    std::unique_ptr<call##_reply_t, StdFreeDeleter>( \
        call##_reply(XCB_REPLY_CONNECTION_ARG(__VA_ARGS__), call(__VA_ARGS__), nullptr) \
    )
#define XCB_REPLY_UNCHECKED(call, ...) \
    std::unique_ptr<call##_reply_t, StdFreeDeleter>( \
        call##_reply(XCB_REPLY_CONNECTION_ARG(__VA_ARGS__), call##_unchecked(__VA_ARGS__), nullptr) \
    )

AppNativeEventFilter::AppNativeEventFilter()
    :QAbstractNativeEventFilter()
{
}

bool AppNativeEventFilter::init()
{
    xcb_connection_t* conn = QX11Info::connection();

    const xcb_query_extension_reply_t *reply = xcb_get_extension_data(conn, &xcb_randr_id);
    if (!reply || !reply->present)
    {
        KLOG_ERROR()  << "xcb_get_extension_data failed!";
        return false;
    }

    auto xrandrQuery = XCB_REPLY(xcb_randr_query_version, conn,
                                 XCB_RANDR_MAJOR_VERSION,
                                 XCB_RANDR_MINOR_VERSION);
    if (!xrandrQuery || (xrandrQuery->major_version < 1 ||
                         (xrandrQuery->major_version == 1 && xrandrQuery->minor_version < 2)))
    {
        KLOG_ERROR() << "xcb_randr_query_version failed!";
        return false;
    }

    m_randrFirstEvent = reply->first_event;

    m_reconfigureOutputTimer.setInterval(300);
    m_reconfigureOutputTimer.setSingleShot(true);
    connect(&m_reconfigureOutputTimer,&QTimer::timeout,this,&AppNativeEventFilter::reconfigureOutputs);
    return true;
}

bool AppNativeEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
    if( eventType != "xcb_generic_event_t" )
        return false;

    xcb_generic_event_t* event = static_cast<xcb_generic_event_t*>(message);
    if( event->response_type == (XCB_RANDR_NOTIFY+m_randrFirstEvent) )
    {
        xcb_randr_notify_event_t* rrEvent = reinterpret_cast<xcb_randr_notify_event_t*>(event);
        if( rrEvent->subCode ==  XCB_RANDR_NOTIFY_OUTPUT_CHANGE)
        {
            auto outputInfo = XCB_REPLY(xcb_randr_get_output_info,QX11Info::connection(),rrEvent->u.oc.output,QX11Info::appTime());
            uint8_t* outputName = xcb_randr_get_output_info_name(outputInfo.get());

            char* nameBuf = (char*)calloc(1,outputInfo->name_len+1);
            memcpy(nameBuf,outputName,outputInfo->name_len);
            QString name(nameBuf);
            free(nameBuf);

            qInfo() << "XCB_RANDR_NOTIFY - XCB_RANDR_NOTIFY_OUTPUT_CHANGE:";
            qInfo() << "id:        " << rrEvent->u.oc.output;
            qInfo() << "name:      " << name;
            qInfo() << "connection:" << rrEvent->u.oc.connection;

            bool oldConnected = false;
            if( m_outputConnectedMap.contains(rrEvent->u.oc.output) )
            {
                oldConnected = m_outputConnectedMap[rrEvent->u.oc.output];
            }

            bool connected = rrEvent->u.oc.connection == XCB_RANDR_CONNECTION_CONNECTED;
            if( oldConnected != connected )
            {
                m_outputConnectedMap[rrEvent->u.oc.output] = connected;
                ///重新配置屏幕
		m_reconfigureOutputTimer.start();
            }

        }
    }
    return false;
}

void AppNativeEventFilter::reconfigureOutputs()
{
    QProcess::execute("xrandr",QStringList()<<"--auto");
}
