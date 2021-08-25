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

#ifndef UNIXSIGNALMONITOR_H
#define UNIXSIGNALMONITOR_H

#include <QObject>

class QSocketNotifier;
class UnixSignalMonitor : public QObject
{
    Q_OBJECT
public:
    explicit UnixSignalMonitor(QObject *parent = nullptr);
    static bool setup_unix_signal_handlers();
    //unix signal handlers
    static void termSignalHandler(int unused);

public slots:
    void handlerSigTerm();

private:
    static int       sigTermFd[2];
    QSocketNotifier *m_snTerm;
};

#endif  // UNIXSIGNALMONITOR_H
