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

#include "shadow-label.h"
#include <QDebug>
#include <QPaintEvent>

ShadowLabel::ShadowLabel(QWidget *parent, Qt::WindowFlags f)
    : QLabel(parent, f),
      m_shadowColor(Qt::black),
      m_shadowEnable(false),
      m_shadowBlurRadius(0),
      m_shadowOffsetX(0),
      m_shadowOffsetY(0)
{
    m_shadowEffect = new QGraphicsDropShadowEffect(this);
    m_shadowEffect->setEnabled(false);
    ///FIXME: screensaver plugin crash
//    this->setGraphicsEffect(m_shadowEffect);
    updateShadow();
}

ShadowLabel::ShadowLabel(const QString &text, QWidget *parent, Qt::WindowFlags f)
    : QLabel(text, parent, f)
{
    updateShadow();
}

QColor ShadowLabel::shadowColor() const
{
    return m_shadowColor;
}

bool ShadowLabel::shadowEnable() const
{
    return m_shadowEnable;
}

qreal ShadowLabel::shadowBlurRadius() const
{
    return m_shadowBlurRadius;
}

qreal ShadowLabel::shadowOffsetX() const
{
    return m_shadowOffsetX;
}

qreal ShadowLabel::shadowOffsetY() const
{
    return m_shadowOffsetY;
}

void ShadowLabel::setShadowColor(QColor shadowColor)
{
    if (m_shadowColor == shadowColor)
        return;
    m_shadowColor = shadowColor;
    updateShadow();
}

void ShadowLabel::setShadowEnable(bool shadowEnable)
{
    if (m_shadowEnable == shadowEnable)
        return;
    m_shadowEnable = shadowEnable;
    updateShadow();
}

void ShadowLabel::setShadowBlurRadius(qreal shadowBlurRadius)
{
    qWarning("Floating point comparison needs context sanity check");
    if (qFuzzyCompare(m_shadowBlurRadius, shadowBlurRadius))
        return;
    m_shadowBlurRadius = shadowBlurRadius;
    updateShadow();
}

void ShadowLabel::setShadowOffsetX(qreal shadowOffsetX)
{
    m_shadowOffsetX = shadowOffsetX;
    updateShadow();
}

void ShadowLabel::setShadowOffsetY(qreal shadowOffsetY)
{
    m_shadowOffsetY = shadowOffsetY;
    updateShadow();
}

void ShadowLabel::updateShadow()
{
    if (m_shadowEnable)
    {
        m_shadowEffect->setEnabled(true);
        m_shadowEffect->setOffset(m_shadowOffsetX, m_shadowOffsetY);
        m_shadowEffect->setColor(m_shadowColor);
        m_shadowEffect->setBlurRadius(m_shadowBlurRadius);
    }
    else
    {
        m_shadowEffect->setEnabled(false);
    }
}
