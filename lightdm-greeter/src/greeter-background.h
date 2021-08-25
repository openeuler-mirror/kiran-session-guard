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

#ifndef __GREETER_BACKGROUND_H__
#define __GREETER_BACKGROUND_H__

#include <QPixmap>
#include <QWidget>

class QScreen;

class GreeterBackground : public QWidget
{
    Q_OBJECT
public:
    explicit GreeterBackground(QScreen *screen, QWidget *parent = nullptr);
    ~GreeterBackground();

public:
    ///设置背景窗口绑定的屏幕
    void setScreen(QScreen *screen);
Q_SIGNALS:
    ///鼠标移动到背景窗口传出的信号
    void mouseEnter(GreeterBackground *window);
private slots:
    ///屏幕的大小改变槽函数
    void slotScreenGeometryChanged(const QRect &geometry);

protected:
    ///鼠标移入事件
    virtual void enterEvent(QEvent *event) override;
    ///大小改变事件
    virtual void resizeEvent(QResizeEvent *event) override;
    ///窗口绘制事件
    virtual void paintEvent(QPaintEvent *event) override;

private:
    ///背景窗口绑定的屏幕
    QScreen *m_screen;
    ///绘制的背景图片
    QPixmap m_background;
    ///根据大小拉升之后的背景图片
    QPixmap m_scaledBackground;
};

#endif  // __GREETER_BACKGROUND_H__
