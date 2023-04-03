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

#include "login-button.h"
#include <QPaintEvent>
#include <QPainter>
#include <QStyleOption>
#include "ui_login-button.h"

namespace Kiran
{
namespace SessionGuard
{
namespace Greeter
{
LoginButton::LoginButton(QWidget *parent) : QWidget(parent),
                                            ui(new Ui::LoginButton),
                                            m_pressed(false)
{
    ui->setupUi(this);
    ui->label_text->setText(tr("login"));
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
}  // namespace Greeter
}  // namespace SessionGuard
}  // namespace Kiran