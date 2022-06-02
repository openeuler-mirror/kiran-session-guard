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

#ifndef __GREETER_SCREEN_MANAGER_H__
#define __GREETER_SCREEN_MANAGER_H__

#include <QMap>
#include <QObject>
#include <QScreen>

class QScreen;
class GreeterLoginWindow;
class GreeterBackground;

class GreeterScreenManager : public QObject
{
    Q_OBJECT
public:
    GreeterScreenManager(QObject *parent = nullptr);
    ~GreeterScreenManager();
    void init();

private slots:
    void slotScreenAdded(QScreen *screen);
    void slotScreenRemoved(QScreen *screen);
    void mouseEnterInWindow(GreeterBackground *background);

private:
    void newScreenBackgroundWidget(QScreen *screen);
    void setGreeterOnBackground(GreeterBackground *background);

private:
    //保存屏幕和屏幕的映射
    QMap<QScreen *, GreeterBackground *> m_BackgroundWidgetMap;
    //登录内容窗口
    GreeterLoginWindow *m_greeterWindow;
};

#endif  // __GREETER_SCREEN_MANAGER_H__
