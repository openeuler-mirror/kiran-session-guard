#pragma once

#include <QWidget>
#include "auth-type-switcher-define.h"
#include "guard-global.h"

QT_BEGIN_NAMESPACE
class QToolButton;
class QBoxLayout;
class QSpacerItem;
class QPropertyAnimation;
class QScrollArea;
class QColor;
QT_END_NAMESPACE

/**
 * 认证类型切换抽屉控件,需和AuthTypeSwitch同一个父控件
 * NOTE:若使用控件的进程不和桌面环境主题保持一致，需要手动调整Drawer颜色，以及QScrollArea中的widget背景色
*/
GUARD_BEGIN_NAMESPACE
class AuthTypeDrawer : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(double expandProgress READ expandProgress WRITE updateExpandProgress)
    Q_PROPERTY(QColor specifyBorderColor READ specifyBorderColor WRITE setSpecifyBorderColor)
    Q_PROPERTY(QColor specifyBackgroundColor READ specifyBackgroundColor WRITE setSpecifyBackgroundColor)

public:
    explicit AuthTypeDrawer(AuthTypeDrawerExpandDirection direction,int radius,QWidget* parent, QWidget* switcher);
    ~AuthTypeDrawer();

    void setAuthTypes(QList<std::tuple<int, QString, QString>> authTypes);

    bool isExpanded();
    void expand();
    void shrink();

    double expandProgress();
    void updateExpandProgress(double progress);

    QColor specifyBorderColor();
    void setSpecifyBorderColor(QColor border);
    QColor specifyBackgroundColor();
    void setSpecifyBackgroundColor(QColor background);
    
signals:
    void authTypeClicked(int authType);
    void expandedStatusChanged(bool isExpanded);

private:
    void init();
    void clear();
    void updateValidSizeHint();
    virtual void paintEvent(QPaintEvent* event) override;
    virtual bool eventFilter(QObject* watched, QEvent* event) override;

private:
    QWidget* m_switcher;
    QPropertyAnimation* m_animation;
    double m_expandProgress = 0.0;
    QSize m_validSizeHint;
    QSpacerItem* m_spacerItem;
    QBoxLayout* m_mainLayout;
    QScrollArea* m_scrollArea;
    QBoxLayout* m_containerLayout;
    AuthTypeDrawerExpandDirection m_direction = EXPAND_DIRECTION_RIGHT;
    quint64 m_fixedEdgeLength = 42;
    quint64 m_radius = 21;
    QMap<int, QToolButton*> m_buttonMap;
    QColor m_backgroundColor;
    QColor m_borderColor;
};
GUARD_END_NAMESPACE