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
#include "moreinfo-button.h"
#include <QDebug>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStyle>

#define ICON_TEXT_SPACE 2

namespace Kiran
{
namespace SessionGuard
{
namespace PolkitAgent
{
MoreInfoButton::MoreInfoButton(QWidget* parent)
    : QWidget(parent)
{
    m_shrinkPix = style()->standardPixmap(QStyle::SP_ArrowRight);
    m_expandPix = style()->standardPixmap(QStyle::SP_ArrowUp);

    m_iconSize = QSize(16, 16);

    // m_shrinkPix = m_shrinkPix.scaled(m_iconSize,Qt::KeepAspectRatio,Qt::SmoothTransformation);
    // m_expandPix = m_expandPix.scaled(m_iconSize,Qt::KeepAspectRatio,Qt::SmoothTransformation);

    setText("more info");
}

MoreInfoButton::~MoreInfoButton()
{
}

void MoreInfoButton::setText(const QString& text)
{
    m_text = text;
    update();
}

QSize MoreInfoButton::sizeHint() const
{
    return QWidget::sizeHint();
}

QSize MoreInfoButton::minimumSizeHint() const
{
    QFontMetrics fontMetrics(font());
    QSize textSize = fontMetrics.boundingRect(m_text).size();

    QSize totalSize = QSize(m_iconSize.width() + textSize.width() + ICON_TEXT_SPACE, qMax<int>(textSize.height(), m_iconSize.height()));
    return totalSize;
}

void MoreInfoButton::paintEvent(QPaintEvent* event)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QRect contentRect(event->rect());

    QRect imageRect(QPoint(0, 0), m_iconSize);
    imageRect.moveTop((contentRect.height() - imageRect.height()) / 2);
    QPixmap pixmap = m_isExpand ? m_expandPix : m_shrinkPix;
    p.drawImage(imageRect, pixmap.toImage(), pixmap.rect());

    contentRect.setLeft(imageRect.right() + ICON_TEXT_SPACE);

    QFontMetrics fontMetrics(font());
    QRect textRect = fontMetrics.boundingRect(contentRect, Qt::TextSingleLine | Qt::AlignVCenter | Qt::AlignLeft, m_text);
    p.drawText(textRect, m_text);
}

void MoreInfoButton::mousePressEvent(QMouseEvent* event)
{
    m_isExpand = !m_isExpand;
    m_isExpand ? emit expand() : emit shrink();
    update();
    return QWidget::mousePressEvent(event);
}
}  // namespace PolkitAgent
}  // namespace SessionGuard
}  // namespace Kiran