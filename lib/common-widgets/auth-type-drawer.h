#pragma once

#include <QWidget>
#include "auth-type-switcher-define.h"

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
namespace Kiran
{
namespace SessionGuard
{
class AuthTypeDrawer : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(double expandProgress READ expandProgress WRITE updateExpandProgress)
    Q_PROPERTY(QColor specifyBorderColor READ specifyBorderColor WRITE setSpecifyBorderColor)
    Q_PROPERTY(QColor specifyBackgroundColor READ specifyBackgroundColor WRITE setSpecifyBackgroundColor)

public:
    explicit AuthTypeDrawer(AuthTypeDrawerExpandDirection direction, int radius, QWidget* parent, QWidget* switcher);
    ~AuthTypeDrawer();

    /// @brief 设置是否根据主题调整图标颜色
    /// 默认图标应传入白色图标(即深色默认下的图标)，若打开该功能，则会监听Kiran主题变化
    //  根据主题颜色变化，将白色图标进行RGB翻转
    /// @param enable 是否启用
    void setAdjustColorToTheme(bool enable);

    /// @brief 重新设置认证类型，将会清空之前的认证类型
    /// @param authTypes 认证类型列表 list<KADAuthType,tooltip,icon>
    void setAuthTypes(QList<std::tuple<int, QString, QString>> authTypes);

    bool isExpanded();
    void expand();
    void shrink();

    double expandProgress();
    void updateExpandProgress(double progress);

    /// @brief 特殊指定边框颜色，不从Kiran主题颜色表中取
    /// 主要用于不随主题变化的场景下，例如登录/解锁界面
    /// @param border 边框颜色
    void setSpecifyBorderColor(QColor border);
    QColor specifyBorderColor();

    /// @brief 特殊指定背景颜色，不从Kiran主题颜色表中取
    /// 主要用于不随主题变化的场景下，例如登录/解锁界面
    /// @param border 背景颜色
    void setSpecifyBackgroundColor(QColor background);
    QColor specifyBackgroundColor();

signals:
    void authTypeClicked(int authType);
    void expandedStatusChanged(bool isExpanded);

private:
    void init();
    void clear();
    void updateValidSizeHint();
    void updateButtonIconColor();
    void onThemeChanged();

    virtual void paintEvent(QPaintEvent* event) override;
    virtual bool eventFilter(QObject* watched, QEvent* event) override;

private:
    QWidget* m_switcher;
    struct AuthTypeButtonInfo
    {
        AuthTypeButtonInfo()
            : m_button(nullptr), m_icon("") {}
        QToolButton* m_button;
        QString m_icon;
    };
    QMap<int, AuthTypeButtonInfo> m_buttonMap;
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
    QColor m_backgroundColor;
    QColor m_borderColor;
    bool m_adjustColorToTheme = false;
};
}  // namespace SessionGuard
}  // namespace Kiran