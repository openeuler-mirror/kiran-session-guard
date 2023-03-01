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

#include "user-avatar.h"
#include <qt5-log-i.h>
#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QPainter>
#include <QPainterPath>

#define DEFAULT_USER_AVATAR ":/common-widgets-images/user_180.png"

GUARD_BEGIN_NAMESPACE
UserAvatar::UserAvatar(QWidget *parent) : QWidget(parent)
{
    setDefaultImage();
}

void UserAvatar::setImage(const QString &path)
{
    if (m_pixmap.load(path))
    {
        m_scaledPixmap = scalePixmapAdjustSize(m_pixmap);
    }
    else
    {
        KLOG_WARNING() << "user avatar: load avatar" << path << "failed";
        setDefaultImage();
    }
    update();
}

void UserAvatar::paintEvent(QPaintEvent *event)
{
    if (m_scaledPixmap.isNull())
    {
        return QWidget::paintEvent(event);
    }

    double radius = qMin(width(), height()) / 2;
    QRect circleRect(0, 0, radius * 2, radius * 2);
    circleRect.moveCenter(rect().center());

    QPainterPath clipedPath;
    clipedPath.addEllipse(circleRect);

    QPainter painter(this);
    painter.setRenderHints(QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing | QPainter::Antialiasing);
    painter.setClipPath(clipedPath);

    QRect imgTargetRect = m_scaledPixmap.rect();
    imgTargetRect.moveCenter(rect().center());
    painter.drawPixmap(imgTargetRect, m_scaledPixmap);

    QWidget::paintEvent(event);
}

void UserAvatar::resizeEvent(QResizeEvent *event)
{
    if ((!m_pixmap.isNull()) && (!m_scaledPixmap.isNull()) && (m_scaledPixmap.size() != this->size()))
    {
        m_scaledPixmap = scalePixmapAdjustSize(m_pixmap);
    }

    QWidget::resizeEvent(event);
}

QPixmap UserAvatar::scalePixmapAdjustSize(const QPixmap &pixmap)
{
    double radius = qMin(width(), height()) / 2;
    // 拉升保持长宽比，尽可能放大，不留白
    QPixmap temp = pixmap.scaled(radius * 2, radius * 2, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    return temp;
}

void UserAvatar::setDefaultImage()
{
    if (!m_pixmap.load(DEFAULT_USER_AVATAR))
    {
        KLOG_WARNING() << "user avatar load default avatar failed";
        return;
    }
    m_scaledPixmap = scalePixmapAdjustSize(m_pixmap);
    update();
}
GUARD_END_NAMESPACE