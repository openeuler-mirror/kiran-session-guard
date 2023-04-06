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

#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include <QObject>
#include <QPointF>


namespace Kiran
{
namespace SessionGuard
{
class ShadowLabel : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(bool shadowEnable READ shadowEnable WRITE setShadowEnable)
    Q_PROPERTY(qreal shadowOffsetX READ shadowOffsetX WRITE setShadowOffsetX)
    Q_PROPERTY(qreal shadowOffsetY READ shadowOffsetY WRITE setShadowOffsetY)
    Q_PROPERTY(QColor shadowColor READ shadowColor WRITE setShadowColor)
    Q_PROPERTY(qreal shadowBlurRadius READ shadowBlurRadius WRITE setShadowBlurRadius)

public:
    explicit ShadowLabel(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    explicit ShadowLabel(const QString &text, QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    ~ShadowLabel() = default;
    QColor shadowColor() const;
    bool shadowEnable() const;
    qreal shadowBlurRadius() const;
    qreal shadowOffsetX() const;
    qreal shadowOffsetY() const;

public slots:
    void setShadowColor(QColor shadowColor);
    void setShadowEnable(bool shadowEnable);
    void setShadowBlurRadius(qreal shadowBlurRadius);
    void setShadowOffsetX(qreal shadowOffsetX);
    void setShadowOffsetY(qreal shadowOffsetY);

private:
    void updateShadow();

private:
    QGraphicsDropShadowEffect *m_shadowEffect = nullptr;
    QColor m_shadowColor;
    bool m_shadowEnable;
    qreal m_shadowBlurRadius;
    qreal m_shadowOffsetX;
    qreal m_shadowOffsetY;
};
}  // namespace SessionGuard
}  // namespace Kiran
