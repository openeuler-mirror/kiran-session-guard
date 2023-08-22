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
#include <QDebug>
#include <QFile>
#include <QPainter>
#include <QPainterPath>
#include <QApplication>

#define DEFAULT_USER_AVATAR ":/common-widgets-images/user_180.png"

UserAvatar::UserAvatar(QWidget *parent) : QWidget(parent)
{
    setDefaultImage();
}

QString UserAvatar::getImagePath()
{
    return m_pixmapPath;
}

void UserAvatar::setImage (const QString &path)
{
    QFile file(path);
    if (m_pixmap.load(path))
    {
        KLOG_DEBUG() << "UserAvatar: load file " << path << "successed";
        m_pixmapPath = path;
        m_scaledPixmap = scalePixmapAdjustSize(m_pixmap);
    }
    else
    {
        KLOG_WARNING() << "UserAvatar: file path[" << path << "] load failed.";
        setDefaultImage();
    }
    update();
}

void UserAvatar::paintEvent (QPaintEvent *event)
{
    QPainter painter(this);
    QPen pen;
    if (!isVisible())
    {
        return;
    }
    painter.setRenderHints(
            QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing | QPainter::Antialiasing);
    double radius = (this->width() < this->height() ? this->width() : this->height()) / 2;
    if (!m_scaledPixmap.isNull())
    {
        QPainterPath painterPath;
        painterPath.addEllipse((this->width() - (radius * 2)) / 2, (this->height() - (radius * 2)) / 2, radius * 2,
                               radius * 2);
        painter.setClipPath(painterPath);
        painter.drawPixmap((this->width() - m_scaledPixmap.width()) / 2, (this->height() - m_scaledPixmap.height()) / 2,
                           m_scaledPixmap);
    }
    QWidget::paintEvent(event);
}

void UserAvatar::resizeEvent (QResizeEvent *event)
{
    if ((!m_pixmap.isNull()) && (!m_scaledPixmap.isNull()) && (m_scaledPixmap.size() != this->size()))
    {
        m_scaledPixmap = scalePixmapAdjustSize(m_pixmap);
    }
    QWidget::resizeEvent(event);
}

QPixmap UserAvatar::scalePixmapAdjustSize (const QPixmap &pixmap)
{
    double radius = (this->width() < this->height() ? this->width() : this->height()) / 2;
    //NOTE:拉升保持长宽比，尽可能放大，不留白
    QPixmap temp = pixmap.scaled(radius * 2, radius * 2, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    return temp;
}

void UserAvatar::setDefaultImage ()
{
    if (!m_pixmap.load(DEFAULT_USER_AVATAR))
    {
        KLOG_WARNING() << "UserAvatar: " << "load default avatar failed.";
        return;
    }
    m_pixmapPath = DEFAULT_USER_AVATAR;
    m_scaledPixmap = scalePixmapAdjustSize(m_pixmap);
    update();
}
