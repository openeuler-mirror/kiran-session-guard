#include "greeter-menu-item.h"
#include <QButtonGroup>
#include <QCheckBox>
#include <QDebug>
#include <QEvent>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QSpacerItem>
#include <QStyleOption>

GreeterMenuItem::GreeterMenuItem(const QString &text, bool checkable, QWidget *parent)
    : QWidget(parent), m_checkable(checkable), m_actionName(text), m_label(nullptr), m_checkbox(nullptr)
{
    setMouseTracking(true);
    initUI();
}

void GreeterMenuItem::setExclusiveGroup(QButtonGroup *group)
{
    group->addButton(m_checkbox);
}

QString GreeterMenuItem::getActionName()
{
    return m_actionName;
}

void GreeterMenuItem::setChecked(bool isChecked)
{
    if (!m_checkable)
    {
        return;
    }
    m_checkbox->setChecked(isChecked);
}

void GreeterMenuItem::initUI()
{
    QHBoxLayout *hboxLayout = new QHBoxLayout(this);
    hboxLayout->setSpacing(0);
    hboxLayout->setMargin(0);

    m_label = new QLabel(m_actionName);
    m_label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    m_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_label->setToolTip(m_actionName);
    hboxLayout->addWidget(m_label, Qt::AlignVCenter);
    if (m_checkable)
    {
        m_checkbox = new QCheckBox("");
        m_checkbox->setFixedSize(QSize(22, 14));
        m_checkbox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        ///鼠标事件由父窗口处理，不可直接取消勾选
        m_checkbox->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        connect(m_checkbox, &QCheckBox::stateChanged, [this]() {
            if (m_checkbox->isChecked())
            {
                emit sigChecked(m_actionName);
            }
        });
        hboxLayout->addWidget(m_checkbox, Qt::AlignVCenter | Qt::AlignRight);
    }
    this->setLayout(hboxLayout);
}

void GreeterMenuItem::paintEvent(QPaintEvent *event)
{
    QPainter     painter(this);
    QStyleOption option;
    option.init(this);
    style()->drawPrimitive(QStyle::PE_Widget, &option, &painter, this);
    QWidget::paintEvent(event);
}

/**
 * @brief 鼠标点击事件，如果可勾选并未勾选则设置勾选
 *        不允许取消勾选状态，需要给QCheckBox安装事件过滤，屏蔽掉鼠标点击事件
 *      　设置状态只能通过点击，取消勾选只能点击其他项
 *        调用父类的事件处理函数来产生QMenu的triggered信号
 */
void GreeterMenuItem::mousePressEvent(QMouseEvent *event)
{
    if (m_checkable && m_checkbox->isChecked() == false)
    {
        m_checkbox->setChecked(true);
    }
    return QWidget::mousePressEvent(event);
}
