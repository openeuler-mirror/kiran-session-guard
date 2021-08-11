#include "login-button.h"
#include <QPaintEvent>
#include <QPainter>
#include <QStyleOption>
#include "ui_login-button.h"

#include <iostream>

LoginButton::LoginButton(QWidget *parent) : QWidget(parent),
                                            ui(new Ui::LoginButton),
                                            m_pressed(false)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_Hover);
}

LoginButton::~LoginButton()
{
    delete ui;
}

bool LoginButton::pressed() const
{
    return m_pressed;
}

QPixmap LoginButton::icon() const
{
    return m_icon;
}

void LoginButton::setPressed(bool pressed)
{
    if (m_pressed == pressed)
        return;

    m_pressed = pressed;
    style()->polish(this);
    if (m_pressed)
        emit sigPressed();
}

void LoginButton::setIcon(QPixmap icon)
{
    m_icon = icon;
    ui->label_icon->setPixmap(m_icon);
}

void LoginButton::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    p.setRenderHint(QPainter::HighQualityAntialiasing);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    QWidget::paintEvent(event);
}

void LoginButton::mousePressEvent(QMouseEvent *event)
{
    setPressed(true);
    QWidget::mousePressEvent(event);
}

void LoginButton::mouseReleaseEvent(QMouseEvent *event)
{
    if (this->rect().contains(event->pos()))
    {
        emit sigClicked();
    }
    setPressed(false);
    QWidget::mouseReleaseEvent(event);
}
