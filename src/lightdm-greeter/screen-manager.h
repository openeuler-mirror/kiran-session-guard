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
#pragma once
#include <QMap>
#include <QObject>
#include <QScreen>

QT_BEGIN_NAMESPACE
class QScreen;
QT_END_NAMESPACE

namespace Kiran
{
namespace SessionGuard
{
namespace Greeter
{
class Prefs;
class Frame;
class Background;
class ScreenManager : public QObject
{
    Q_OBJECT
public:
    ScreenManager(QObject *parent = nullptr);
    ~ScreenManager();
    void init(Prefs *prefs);

private slots:
    void slotScreenAdded(QScreen *screen);
    void slotScreenRemoved(QScreen *screen);
    void mouseEnterInWindow(Background *background);

private:
    void newScreenBackgroundWidget(QScreen *screen);
    void setGreeterOnBackground(Background *background);

private:
    Prefs *m_prefs;
    Frame *m_greeterFrame;
    // 保存屏幕和屏幕的映射
    QMap<QScreen *, Background *> m_BackgroundWidgetMap;
};
}  // namespace Greeter
}  // namespace SessionGuard
}  // namespace Kiran