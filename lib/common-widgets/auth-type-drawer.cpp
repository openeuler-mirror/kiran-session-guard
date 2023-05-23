#include "auth-type-drawer.h"
#include "style-palette.h"

#include <kiran-style/style-palette.h>
#include <QApplication>
#include <QBoxLayout>
#include <QDebug>
#include <QEvent>
#include <QIcon>
#include <QMoveEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QResizeEvent>
#include <QScrollArea>
#include <QSpacerItem>
#include <QTimer>
#include <QToolButton>

namespace Kiran
{
namespace SessionGuard
{
AuthTypeDrawer::AuthTypeDrawer(AuthTypeDrawerExpandDirection direction, int radius, QWidget* parent, QWidget* switcher)
    : QWidget(parent),
      m_switcher(switcher),
      m_direction(direction),
      m_radius(radius)
{
    init();

    auto kiranPalette = Kiran::StylePalette::instance();
    connect(kiranPalette, &Kiran::StylePalette::themeChanged, this, &AuthTypeDrawer::onThemeChanged);
}

AuthTypeDrawer::~AuthTypeDrawer()
{
}

void AuthTypeDrawer::setAdjustColorToTheme(bool enable)
{
    if (m_adjustColorToTheme == enable)
    {
        return;
    }

    m_adjustColorToTheme = enable;

    if (m_adjustColorToTheme)
    {
        updateButtonIconColor();
    }
}

void AuthTypeDrawer::setAuthTypes(QList<std::tuple<int, QString, QString>> authTypes)
{
    clear();

    for (auto authTypeTuple : authTypes)
    {
        int authType = std::get<0>(authTypeTuple);
        const QString tooltip = std::get<1>(authTypeTuple);
        const QString icon = std::get<2>(authTypeTuple);

        if (m_buttonMap.find(authType) != m_buttonMap.end())
        {
            continue;
        }

        auto btn = new QToolButton(this);
        btn->setToolTip(tooltip);
        btn->setIcon(QIcon(icon));
        btn->setAutoFillBackground(false);
        btn->setIconSize(QSize(14, 14));
        btn->setFixedSize(QSize(16, 16));
        connect(btn, &QToolButton::clicked, this, [this, authType]()
                {
                    emit authTypeClicked(authType);
                    shrink(); });
        m_containerLayout->addWidget(btn, 0, Qt::AlignCenter);

        AuthTypeButtonInfo authButtonInfo;
        authButtonInfo.m_button = btn;
        authButtonInfo.m_icon = icon;
        m_buttonMap[authType] = authButtonInfo;
    }
    
    updateValidSizeHint();

    if (m_adjustColorToTheme)
    {
        updateButtonIconColor();
    }
}

void AuthTypeDrawer::clear()
{
    m_buttonMap.clear();
    auto childs = findChildren<QToolButton*>();
    for (auto child : childs)
    {
        delete child;
    }
}

bool AuthTypeDrawer::isExpanded()
{
    return m_expandProgress > 0;
}

void AuthTypeDrawer::expand()
{
    if (m_expandProgress > 0)
    {
        return;
    }
    setVisible(true);
    m_animation->setDirection(QPropertyAnimation::Forward);
    m_animation->start();
    emit expandedStatusChanged(true);
}

void AuthTypeDrawer::shrink()
{
    if (m_expandProgress == 0)
    {
        return;
    }
    m_animation->setDirection(QPropertyAnimation::Backward);
    m_animation->start();
    emit expandedStatusChanged(false);
}

double AuthTypeDrawer::expandProgress()
{
    return m_expandProgress;
}

void AuthTypeDrawer::updateExpandProgress(double progress)
{
    bool isHori = (m_direction == EXPAND_DIRECTION_RIGHT);
    double expandLength = isHori ? m_validSizeHint.width() : m_validSizeHint.height();
    int currentExpandLength = qRound(expandLength * progress);

    bool showScrollBar = progress == 1;

    QRect drawRect;
    switch (m_direction)
    {
    case EXPAND_DIRECTION_RIGHT:
    {
        drawRect = QRect(this->geometry().topLeft(), QSize(currentExpandLength, m_fixedEdgeLength));
        // 若控件大小超出父控件范围限制,压缩至限制内
        if (!parentWidget()->rect().contains(drawRect, true))
        {
            drawRect.setRight(parentWidget()->rect().right());
        }
        m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_scrollArea->setHorizontalScrollBarPolicy(showScrollBar ? Qt::ScrollBarAsNeeded : Qt::ScrollBarAlwaysOff);
        break;
    }
    case EXPAND_DIRECTION_BOTTOM:
    {
        drawRect = QRect(this->geometry().topLeft(), QSize(m_fixedEdgeLength, currentExpandLength));
        // 若控件大小超出父控件范围限制,压缩至限制内
        if (!parentWidget()->rect().contains(drawRect, true))
        {
            drawRect.setBottom(parentWidget()->rect().bottom());
        }
        m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_scrollArea->setVerticalScrollBarPolicy(showScrollBar ? Qt::ScrollBarAsNeeded : Qt::ScrollBarAlwaysOff);
    }
    break;
    default:
        break;
    }

    setFixedSize(drawRect.size());
    m_expandProgress = progress;
}

AuthTypeDrawerExpandDirection AuthTypeDrawer::getDirection() const
{
    return m_direction;
}

QColor AuthTypeDrawer::specifyBorderColor()
{
    return m_borderColor;
}

void AuthTypeDrawer::setSpecifyBorderColor(QColor border)
{
    m_borderColor = border;
    update();
}

QColor AuthTypeDrawer::specifyBackgroundColor()
{
    return m_backgroundColor;
}

void AuthTypeDrawer::setSpecifyBackgroundColor(QColor background)
{
    m_backgroundColor = background;
    update();
}

void AuthTypeDrawer::init()
{
    // 按钮背景透明
    setStyleSheet("QToolButton{background:transparent;border:0px;}");

    // 窗口背景透明
    setAttribute(Qt::WA_TranslucentBackground);

    // 设置z轴属性,位于切换按钮之下
    stackUnder(m_switcher);

    // 初始化动画
    m_animation = new QPropertyAnimation(this, "expandProgress");
    m_animation->setDuration(300);
    m_animation->setDirection(QPropertyAnimation::Forward);
    m_animation->setEasingCurve(QEasingCurve::OutQuad);
    m_animation->setStartValue(0);
    m_animation->setEndValue(1);
    // clang-format off
    connect(m_animation, &QPropertyAnimation::finished, [this](){
        // emit expandedStatusChanged(m_animation->direction() == QPropertyAnimation::Forward);
        if( m_expandProgress == 0 )
        {
            setVisible(false);
        }
    });
    // clang-format on

    bool isHori = (m_direction == EXPAND_DIRECTION_RIGHT);
    auto boxLayoutDirection = isHori ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom;
    m_fixedEdgeLength = isHori ? m_switcher->width() : m_switcher->height();

    m_mainLayout = new QBoxLayout(boxLayoutDirection, this);
    m_mainLayout->setSpacing(0);

    m_spacerItem = new QSpacerItem(42, 10, QSizePolicy::Fixed, QSizePolicy::Minimum);
    m_mainLayout->insertItem(0, m_spacerItem);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_scrollArea->setAutoFillBackground(false);
    m_scrollArea->setFrameStyle(QFrame::NoFrame);
    m_scrollArea->setWidgetResizable(true);
    m_mainLayout->insertWidget(1, m_scrollArea);

    auto containerWidget = new QWidget();
    containerWidget->setVisible(true);
    m_containerLayout = new QBoxLayout(boxLayoutDirection, containerWidget);
    m_containerLayout->setContentsMargins(0, 0, 0, 0);
    m_containerLayout->setSpacing(16);
    m_scrollArea->setWidget(containerWidget);

    if (isHori)
    {
        m_mainLayout->setContentsMargins(m_radius + 1, 1, m_radius + 1, 1);
        m_spacerItem->changeSize(m_fixedEdgeLength, 10, QSizePolicy::Fixed, QSizePolicy::Minimum);
    }
    else
    {
        m_mainLayout->setContentsMargins(1, m_radius + 1, 1, m_radius + 1);
        m_spacerItem->changeSize(10, m_fixedEdgeLength, QSizePolicy::Minimum, QSizePolicy::Fixed);
    }
    m_mainLayout->invalidate();

    // 安装事件监控器,用于监听切换按钮大小改变,移动事件
    // 监听全局鼠标点击时间,用于收回认证类型按钮
    m_switcher->installEventFilter(this);
    qApp->installEventFilter(this);

    // 设置默认状态
    setFixedSize(0, 0);
    setVisible(false);
}

// NOTE:
//  此处不能用sizeHint,由于ScrollArea::sizeHint只会再第一次读取容器sizeHint,后续不会变化
//  现采取特殊处理
void AuthTypeDrawer::updateValidSizeHint()
{
    QSize totalSize(0, 0);
    bool isHori = (m_direction == EXPAND_DIRECTION_RIGHT);

    auto mainMargin = m_mainLayout->contentsMargins();
    auto spacingItemSize = m_spacerItem->sizeHint();
    auto scrollFrameWidth = m_scrollArea->frameWidth();
    auto contentSpacing = m_containerLayout->spacing();
    auto buttonSize = QSize(16, 16);
    auto buttonCount = m_buttonMap.count();
    if (isHori)
    {
        totalSize.setHeight(m_fixedEdgeLength);
        totalSize.rwidth() += mainMargin.left() + mainMargin.right();
        totalSize.rwidth() += spacingItemSize.width();
        totalSize.rwidth() += 2 * scrollFrameWidth;
        totalSize.rwidth() += (buttonSize.width() * buttonCount) + (contentSpacing * (buttonCount - 1));
    }
    else
    {
        totalSize.setWidth(m_fixedEdgeLength);

        totalSize.rheight() += mainMargin.top() + mainMargin.bottom();
        totalSize.rheight() += spacingItemSize.height();
        totalSize.rheight() += 2 * scrollFrameWidth;
        totalSize.rheight() += (buttonSize.height() * buttonCount) + (contentSpacing * (buttonCount - 1));
    }

    m_validSizeHint = totalSize;
}

void AuthTypeDrawer::updateButtonIconColor()
{
    auto kiranPalette = Kiran::StylePalette::instance();

    for (auto buttonInfo : m_buttonMap)
    {
        QIcon icon(buttonInfo.m_icon);
        auto pixmap = icon.pixmap(QSize(16, 16));
        if (kiranPalette->paletteType() == Kiran::PALETTE_LIGHT)
        {
            auto image = pixmap.toImage();
            image.invertPixels(QImage::InvertRgb);
            pixmap = QPixmap::fromImage(image);
        }
        buttonInfo.m_button->setIcon(pixmap);
    }
}

void AuthTypeDrawer::onThemeChanged()
{
    if (m_adjustColorToTheme)
    {
        updateButtonIconColor();
    }
}

void AuthTypeDrawer::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (m_direction == EXPAND_DIRECTION_RIGHT && width() < m_switcher->width())
    {
        return;
    }

    if (m_direction == EXPAND_DIRECTION_BOTTOM && height() < m_switcher->height())
    {
        return;
    }

    auto stylePalette = Kiran::StylePalette::instance();
    QColor background = m_backgroundColor;
    QColor border = m_borderColor;

    if (!m_backgroundColor.isValid())
    {
        background = stylePalette->color(Kiran::StylePalette::Normal, Kiran::StylePalette::Window, Kiran::StylePalette::Background);
    }
    if (!m_borderColor.isValid())
    {
        border = stylePalette->color(Kiran::StylePalette::Normal, Kiran::StylePalette::Window, Kiran::StylePalette::Border);
    }

    QPainterPath painterPath;
    QRectF rectf = this->rect();
    rectf.adjust(0.5, 0.5, -0.5, -0.5);
    painterPath.addRoundedRect(rectf, m_radius, m_radius);

    painter.setPen(border);
    painter.setBrush(background);

    painter.drawPath(painterPath);
    QWidget::paintEvent(event);
}

bool AuthTypeDrawer::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonPress)
    {
        auto mouseEvent = dynamic_cast<QMouseEvent*>(event);
        QPoint pos = mapFromGlobal(mouseEvent->globalPos());
        if (isExpanded() && !rect().contains(pos))
        {
            shrink();
        }
    }

    if (watched == m_switcher)
    {
        if (event->type() == QEvent::Move)
        {
            QPoint movePoint;
            auto moveEvent = dynamic_cast<QMoveEvent*>(event);

            // 不同父控件,需将坐标转换
            // auto globalPos = m_switcher->parentWidget()->mapToGlobal(moveEvent->pos());
            // movePoint = this->parentWidget()->mapFromGlobal(globalPos);

            // 同一个父控件可直接移动
            movePoint = moveEvent->pos();
            move(movePoint);
        }
        else if (event->type() == QEvent::Resize)
        {
            auto resizeEvent = dynamic_cast<QResizeEvent*>(event);
            auto newSize = resizeEvent->size();

            switch (m_direction)
            {
            case EXPAND_DIRECTION_RIGHT:
                m_spacerItem->changeSize(newSize.width(), 10, QSizePolicy::Fixed, QSizePolicy::Minimum);
                m_fixedEdgeLength = newSize.height();
                break;
            case EXPAND_DIRECTION_BOTTOM:
                m_spacerItem->changeSize(10, newSize.height(), QSizePolicy::Minimum, QSizePolicy::Fixed);
                m_fixedEdgeLength = newSize.width();
                break;
            default:
                break;
            }

            updateValidSizeHint();
            updateExpandProgress(m_expandProgress);
        }
        else if (event->type() == QEvent::ParentChange)
        {
            // 需要保持drawer和switch同一个父控件,stackUnder才会生效
            setParent(m_switcher->parentWidget());
            stackUnder(m_switcher);
        }
    }

    return false;
}
}  // namespace SessionGuard
}  // namespace Kiran