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

#include <qt5-log-i.h>
#include <QPainter>
#include <QResizeEvent>
#include <QScreen>

#include "background.h"
#include "greeter-define.h"
#include "frame.h"

QT_BEGIN_NAMESPACE
Q_WIDGETS_EXPORT void qt_blurImage(QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0);
QT_END_NAMESPACE

GUARD_GREETER_BEGIN_NAMESPACE
Background::Background(const QString& image,QScreen *screen, QWidget *parent)
    : QWidget(parent),
      m_screen(nullptr)
{
    setWindowFlags(windowFlags() | Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnBottomHint);
    QStringList backgrounds = {image, DEFAULT_BACKGROUND};
    foreach (const QString background, backgrounds)
    {
        if (!m_background.load(background))
        {
            KLOG_WARNING() << "load background <" << background << "> failed!";
            continue;
        }
        else
        {
            KLOG_DEBUG() << "load background <" << background << "> successes!";
            break;
        }
    }
    if (m_background.isNull())
    {
        KLOG_ERROR() << "load greeter background failed!";
    }
    setScreen(screen);
}

Background::~Background()
{
}

void Background::setScreen(QScreen *screen)
{
    if (m_screen != nullptr)
    {
        disconnect(m_screen, &QScreen::geometryChanged,
                   this, &Background::onScreenGeometryChanged);
    }

    if (screen != nullptr)
    {
        connect(screen, &QScreen::geometryChanged,
                this, &Background::onScreenGeometryChanged);
    }

    m_screen = screen;
    if (m_screen)
    {
        onScreenGeometryChanged(m_screen->geometry());
    }
}

void Background::onScreenGeometryChanged(const QRect &geometry)
{
    KLOG_DEBUG() << m_screen->name() << "screen geometry changed:" << geometry << ",update background geometry.";
    this->resize(geometry.size());
    this->move(geometry.x(), geometry.y());
}

void Background::enterEvent(QEvent *event)
{
    emit mouseEnter(this);
    QWidget::enterEvent(event);
}

void Background::resizeEvent(QResizeEvent *event)
{
    KLOG_DEBUG() << objectName() << "resize " << event->oldSize() << "->" << event->size();
    if (!m_background.isNull())
    {
        m_scaledBackground = m_background.scaled(this->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        QImage tmp = m_scaledBackground.toImage();

        QImage blurImage(tmp.size(), QImage::Format_ARGB32_Premultiplied);
        QPainter painter(&blurImage);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        qt_blurImage(&painter, tmp, 10, true, false);
        m_scaledBackground = QPixmap::fromImage(blurImage);
    }

    Frame *greeterWindow = findChild<Frame *>("Frame");
    if (greeterWindow != nullptr)
    {
        KLOG_DEBUG() << "greeter login window resize:" << this->size();
        greeterWindow->resize(this->size());
    }
    QWidget::resizeEvent(event);
}

void Background::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    if (!m_scaledBackground.isNull())
    {
        painter.drawPixmap(this->rect(), m_scaledBackground, m_scaledBackground.rect());
    }
    else
    {
        painter.fillRect(this->rect(), QColor(0, 0, 0));
    }
    QWidget::paintEvent(event);
}
GUARD_GREETER_END_NAMESPACE