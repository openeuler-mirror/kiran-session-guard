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

#include <QLineEdit>
#include <QTimerEvent>
#include <QWidget>
#include "guard-global.h"

namespace Ui
{
class PromptEdit;
}
GUARD_BEGIN_NAMESPACE
class PromptEdit : public QWidget
{
    Q_OBJECT
    Q_ENUMS(InputMode)
    Q_PROPERTY(InputMode inputMode READ inputMode WRITE setInputMode)
    /// 提供给QSS状态值，通过该状态值设置聚焦边框
    Q_PROPERTY(bool editFocused READ editFocused WRITE setEditFocused NOTIFY editFocusedChanged)
    /// 提供给QSS状态值，通过该状态设置设置错误红色边框
    Q_PROPERTY(bool hasError READ hasError WRITE setHasError NOTIFY hasErrorChanged)
    /// 因Passwd模式字体需比普通模式小,但是需要Passwd模式PlaceHoldText字体和普通模式一样,
    /// 不能使用Qt提供的状态去判断,故添加该属性用作判断
    Q_PROPERTY(bool showPasswordModeStyle READ showPasswordModeStyle WRITE setShowPasswordModeStyle NOTIFY showPasswordModeStyleChanged)
public:
    enum InputMode
    {
        INPUT_PROMPT,
        INPUT_USERNAME
    };

public:
    explicit PromptEdit(QWidget *parent = nullptr);
    ~PromptEdit();

signals:
    /**
     * @brief 当输入框中输入回车会或按钮被点击时发出该信号
     *        同时启动Loading动画，若需停止动画，需调用reset
     * @param 确认的文本内容
     */
    void textConfirmed(const QString &data);
    void editFocusedChanged(bool editFocus);
    void hasErrorChanged(bool hasError);
    void showPasswordModeStyleChanged(bool showPasswordModeStyle);

public:
    void startMovieAndEmitSignal();
    void reset();
    void setEchoMode(QLineEdit::EchoMode echoMode);
    void setPlaceHolderText(const QString &text);
    QString getText();
    InputMode inputMode() const;
    bool editFocused() const;
    bool hasError() const;
    bool showPasswordModeStyle() const;

public slots:
    void setFocus();
    void setInputMode(InputMode inputMode);
    void setEditFocused(bool editFocused);
    void setHasError(bool hasError);
    void setShowPasswordModeStyle(bool showPasswordModeStyle);

private:
    void initConnection();
    void setDefaultIcon();
    void setNormalLetterSpacing();
    void setPasswdLetterSpacing();

private slots:
    void slotEditReturnPressed();
    void slotButtonPressed();
    void slotEditTextChanged(const QString &text);

protected:
    void timerEvent(QTimerEvent *e) override;
    void paintEvent(QPaintEvent *e) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::PromptEdit *ui;
    InputMode m_inputMode;
    int m_animationTimerId;
    bool m_editFocused;
    bool m_hasError;
    bool m_showPasswordModeStyle;
};
GUARD_END_NAMESPACE
