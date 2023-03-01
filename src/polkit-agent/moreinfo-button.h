#pragma once
#include <QWidget>
#include <QIcon>
#include "guard-global.h"


GUARD_POLKIT_AGENT_BEGIN_NAMESPACE
class MoreInfoButton: public QWidget
{
    Q_OBJECT
public:
    MoreInfoButton(QWidget* parent = nullptr);
    ~MoreInfoButton();

    void setText(const QString& text);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
signals:
    void expand();
    void shrink();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent *event) override;
    
private:
    bool m_isExpand = false;
    QPixmap m_expandPix;
    QPixmap m_shrinkPix;
    QSize m_iconSize;
    QString m_text;
};
GUARD_POLKIT_AGENT_END_NAMESPACE