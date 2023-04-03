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

#include "prompt-edit.h"
#include "ui_prompt-edit.h"

#include <qt5-log-i.h>
#include <QKeyEvent>
#include <QPainter>
#include <QStyleOption>

#define NORMAL_ICON ":/login-frame/images/unlock.png"
#define LOADING_ICON_FORMAT ":/login-frame/images/loading_%1.png"
#define LOADING_ICON_INDEX_MAX 17

namespace Kiran
{
namespace SessionGuard
{
PromptEdit::PromptEdit(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::PromptEdit),
      m_animationTimerId(0),
      m_editFocused(false),
      m_hasError(false),
      m_showPasswordModeStyle(false)
{
    ui->setupUi(this);
    ui->edit->setContextMenuPolicy(Qt::NoContextMenu);
    ui->edit->setAttribute(Qt::WA_InputMethodEnabled, false);

    setDefaultIcon();
    initConnection();

    ui->edit->installEventFilter(this);
}

PromptEdit::~PromptEdit()
{
    if (m_animationTimerId)
    {
        killTimer(m_animationTimerId);
        m_animationTimerId = 0;
    }
    delete ui;
}

void PromptEdit::initConnection()
{
    connect(ui->edit, SIGNAL(returnPressed()), this, SLOT(slotEditReturnPressed()));
    connect(ui->button, SIGNAL(clicked()), this, SLOT(slotButtonPressed()));
    connect(ui->edit, SIGNAL(textChanged(const QString &)), this, SLOT(slotEditTextChanged(const QString &)));
}

void PromptEdit::setDefaultIcon()
{
    ui->button->setIcon(QIcon(NORMAL_ICON));
}

void PromptEdit::setNormalLetterSpacing()
{
    QFont font = ui->edit->font();
    font.setLetterSpacing(QFont::PercentageSpacing, 0);
    ui->edit->setFont(font);
}

void PromptEdit::setPasswdLetterSpacing()
{
    QFont font = ui->edit->font();
    font.setLetterSpacing(QFont::AbsoluteSpacing, 4);
    ui->edit->setFont(font);
}

void PromptEdit::setEchoMode(QLineEdit::EchoMode echoMode)
{
    ui->edit->setEchoMode(echoMode);
    if (echoMode == QLineEdit::Normal)
    {
        setShowPasswordModeStyle(false);
        setNormalLetterSpacing();
    }
}

void PromptEdit::setPlaceHolderText(const QString &text)
{
    ui->edit->setPlaceholderText(text);
}

void PromptEdit::setFocus()
{
    if (!ui->edit->hasFocus())
        ui->edit->setFocus(Qt::OtherFocusReason);
}

QString PromptEdit::getText()
{
    return ui->edit->text();
}

void PromptEdit::setInputMode(PromptEdit::InputMode inputMode)
{
    m_inputMode = inputMode;
}

void PromptEdit::setEditFocused(bool editFocused)
{
    if (m_editFocused == editFocused)
        return;
    m_editFocused = editFocused;
    style()->polish(this);
    emit editFocusedChanged(m_editFocused);
}

void PromptEdit::setHasError(bool hasError)
{
    if (m_hasError == hasError)
        return;

    m_hasError = hasError;
    style()->polish(this);
    emit hasErrorChanged(m_hasError);
}

void PromptEdit::setShowPasswordModeStyle(bool showPasswordModeStyle)
{
    if (m_showPasswordModeStyle == showPasswordModeStyle)
    {
        return;
    }
    m_showPasswordModeStyle = showPasswordModeStyle;
    style()->polish(this);
    style()->polish(ui->edit);
}

void PromptEdit::slotEditReturnPressed()
{
    if ((ui->edit->echoMode() != QLineEdit::Password) && (ui->edit->text().isEmpty()))
    {
        return;
    }

    startMovieAndEmitSignal();
}

void PromptEdit::slotButtonPressed()
{
    if ((ui->edit->echoMode() != QLineEdit::Password) && (ui->edit->text().isEmpty()))
    {
        return;
    }

    startMovieAndEmitSignal();
}

void PromptEdit::slotEditTextChanged(const QString &text)
{
    /// 密码框输入密码不为空的情况下调整字体和字间距
    if (ui->edit->echoMode() == QLineEdit::Password && text.isEmpty())
    {
        setShowPasswordModeStyle(false);
        setNormalLetterSpacing();
    }
    else if (ui->edit->echoMode() == QLineEdit::Password && !text.isEmpty())
    {
        setShowPasswordModeStyle(true);
        setPasswdLetterSpacing();
    }
}

void PromptEdit::timerEvent(QTimerEvent *e)
{
    static int iconIndex = 0;

    if (e->timerId() != m_animationTimerId)
    {
        return;
    }

    if (iconIndex > LOADING_ICON_INDEX_MAX)
    {
        iconIndex = 0;
    }

    QString iconPath = QString(LOADING_ICON_FORMAT).arg(iconIndex++);
    ui->button->setIcon(QIcon(iconPath));
}

void PromptEdit::paintEvent(QPaintEvent *e)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    QWidget::paintEvent(e);
}

/// 输入框聚焦改变，设置窗口的属性，便于QSS设置不同边框
bool PromptEdit::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->edit)
    {
        if (event->type() == QEvent::FocusIn)
        {
            setEditFocused(true);
        }
        else if (event->type() == QEvent::FocusOut)
        {
            setEditFocused(false);
        }
        else if (event->type() == QEvent::KeyPress)
        {
            auto keyEvent = dynamic_cast<QKeyEvent *>(event);
            if (keyEvent->key() == Qt::Key_Escape && ui->edit->echoMode() == QLineEdit::Password)
            {
                ui->edit->clear();
            }
        }
    }
    return false;
}

void PromptEdit::startMovieAndEmitSignal()
{
    ui->edit->setEnabled(false);
    if (m_animationTimerId == 0)
    {
        m_animationTimerId = startTimer(50);
    }
    emit textConfirmed(ui->edit->text());
}

void PromptEdit::reset()
{
    this->setEnabled(true);
    ui->edit->setEnabled(true);
    ui->edit->clear();
    ui->edit->setPlaceholderText("");
    setEchoMode(QLineEdit::Normal);
    if (m_animationTimerId != 0)
    {
        killTimer(m_animationTimerId);
        m_animationTimerId = 0;
    }
    setDefaultIcon();
}

PromptEdit::InputMode PromptEdit::inputMode() const
{
    return m_inputMode;
}

bool PromptEdit::editFocused() const
{
    return m_editFocused;
}

bool PromptEdit::hasError() const
{
    return m_hasError;
}

bool PromptEdit::showPasswordModeStyle() const
{
    return m_showPasswordModeStyle;
}
}  // namespace SessionGuard
}  // namespace Kiran