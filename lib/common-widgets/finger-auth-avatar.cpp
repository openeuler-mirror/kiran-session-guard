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


#include "finger-auth-avatar.h"
#include <QPainter>

FingerAuthAvatar::FingerAuthAvatar(QWidget *parent) : QWidget(parent)
{
    m_pixmap.load(":/common-widgets-images/finger_auth.png");
    m_scaledPixmap = scalePixmapAdjustSize(m_pixmap);
    m_animation.setTargetObject(this);
    m_animation.setPropertyName("progress");
    m_animation.setStartValue(0);
    m_animation.setEndValue(100);
    m_animation.setDuration(800);
    m_animation.setEasingCurve(QEasingCurve::InSine);
    connect(&m_animation, &QPropertyAnimation::finished, [this]() {
        QPropertyAnimation::Direction direction =
            m_animation.direction() == QAbstractAnimation::Forward ? QAbstractAnimation::Backward
                                                                   : QAbstractAnimation::Forward;
        m_animation.setDirection(direction);
        m_animation.start();
    });
}

FingerAuthAvatar::~FingerAuthAvatar()
{
}

void FingerAuthAvatar::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QPen     pen;
    if (!isVisible())
    {
        return;
    }
    painter.setRenderHints(
        QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing | QPainter::Antialiasing);
    painter.drawPixmap((this->width() - m_scaledPixmap.width()) / 2, (this->height() - m_scaledPixmap.height()) / 2,
                       m_scaledPixmap);
    if (m_animation.state() != QPropertyAnimation::Running)
    {
        return;
    }

    double       radius = (this->width() < this->height() ? this->width() : this->height()) / 2;
    QPainterPath painterPath;
    painterPath.addEllipse((this->width() - (radius * 2)) / 2, (this->height() - (radius * 2)) / 2, radius * 2,
                           radius * 2);
    painter.setClipPath(painterPath);

    int drawY = (height() / 100.0) * m_progress;
    pen.setWidthF(3);
    pen.setColor(QColor("#2eb3ff"));
    painter.setPen(pen);
    painter.drawLine(QLine(0, drawY, width(), drawY));
}

void FingerAuthAvatar::resizeEvent(QResizeEvent *event)
{
    if ((!m_pixmap.isNull()) && (!m_scaledPixmap.isNull()) && (m_scaledPixmap.size() != this->size()))
    {
        m_scaledPixmap = scalePixmapAdjustSize(m_pixmap);
    }
    QWidget::resizeEvent(event);
}

QPixmap FingerAuthAvatar::scalePixmapAdjustSize(const QPixmap &pixmap)
{
    double radius = (this->width() < this->height() ? this->width() : this->height()) / 2;
    //NOTE:拉升保持长宽比，尽可能放大，不留白
    QPixmap temp = pixmap.scaled(radius * 2, radius * 2, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    return temp;
}

uint FingerAuthAvatar::progress()
{
    return m_progress;
}

void FingerAuthAvatar::setProgress(uint progress)
{
    if (m_progress == progress)
    {
        return;
    }
    m_progress = progress;
    update();
}

void FingerAuthAvatar::startAnimation()
{
    m_animation.start();
}

void FingerAuthAvatar::stopAnimation()
{
    m_animation.stop();
}
