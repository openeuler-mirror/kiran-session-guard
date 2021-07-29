#ifndef SHADOWLABEL_H
#define SHADOWLABEL_H

#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include <QObject>
#include <QPointF>

///FIXME:设置阴影会对所有子控件和边框生效
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
    bool   shadowEnable() const;
    qreal  shadowBlurRadius() const;
    qreal  shadowOffsetX() const;
    qreal  shadowOffsetY() const;

public slots:
    void setShadowColor(QColor shadowColor);
    void setShadowEnable(bool shadowEnable);
    void setShadowBlurRadius(qreal shadowBlurRadius);
    void setShadowOffsetX(qreal shadowOffsetX);
    void setShadowOffsetY(qreal shadowOffsetY);

private:
    void updateShadow();

private:
    QGraphicsDropShadowEffect m_shadowEffect;
    QColor                    m_shadowColor;
    bool                      m_shadowEnable;
    qreal                     m_shadowBlurRadius;
    qreal                     m_shadowOffsetX;
    qreal                     m_shadowOffsetY;
};

#endif  // SHADOWLABEL_H
