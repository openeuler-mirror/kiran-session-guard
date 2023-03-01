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
#include <QColor>
#include <QFont>
#include <QLabel>
#include <QWidget>
#include "guard-global.h"

namespace Ui
{
class LoginButton;
}

GUARD_GREETER_BEGIN_NAMESPACE
class LoginButton : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool pressed READ pressed WRITE setPressed)
    Q_PROPERTY(QPixmap icon READ icon WRITE setIcon)

public:
    explicit LoginButton(QWidget *parent = nullptr);
    ~LoginButton();
    bool    pressed() const;
    QPixmap icon() const;

public slots:
    void setPressed(bool pressed);
    void setIcon(QPixmap icon);

Q_SIGNALS:
    void sigPressed();
    void sigClicked();

protected:
    virtual void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    virtual void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

private:
    Ui::LoginButton *ui;
    bool             m_pressed;
    QPixmap          m_icon;
};
GUARD_GREETER_END_NAMESPACE