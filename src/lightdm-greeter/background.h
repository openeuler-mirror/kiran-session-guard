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

#include <QPixmap>
#include <QWidget>


class QScreen;

namespace Kiran
{
namespace SessionGuard
{
namespace Greeter
{
class Background : public QWidget
{
    Q_OBJECT
public:
    explicit Background(const QString &image, QScreen *screen, QWidget *parent = nullptr);
    ~Background();

    void setScreen(QScreen *screen);

signals:
    void mouseEnter(Background *window);

private slots:
    void onScreenGeometryChanged(const QRect &geometry);

protected:
    void enterEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    QScreen *m_screen;
    QPixmap m_background;
    QPixmap m_scaledBackground;
};
}  // namespace Greeter
}  // namespace SessionGuard
}  // namespace Kiran