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
#ifndef LIGHTDM_KIRAN_GREETER_APP_NATIVE_EVENT_FILTER_H
#define LIGHTDM_KIRAN_GREETER_APP_NATIVE_EVENT_FILTER_H

#include <QAbstractNativeEventFilter>
#include <QMap>
#include <QObject>
#include <QTimer>

struct OutputInfo
{
    bool connected;
    QString outputName;
};

class AppNativeEventFilter : public QObject,public QAbstractNativeEventFilter
{
public:
    AppNativeEventFilter();
    ~AppNativeEventFilter(){};

    bool init();
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;

private slots:
    void reconfigureOutputs();
private:
    uint32_t  m_randrFirstEvent = 0;
    QMap<uint32_t,bool> m_outputConnectedMap;
    QTimer m_reconfigureOutputTimer;
};

#endif  // LIGHTDM_KIRAN_GREETER_APP_NATIVE_EVENT_FILTER_H
