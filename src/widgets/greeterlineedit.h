#ifndef GREETERLINEEDIT_H
#define GREETERLINEEDIT_H

#include <QLineEdit>
#include <QTimerEvent>
#include <QWidget>

namespace Ui
{
class GreeterLineEdit;
}

class GreeterLineEdit : public QWidget
{
    Q_OBJECT
    Q_ENUMS(InputMode)
    Q_PROPERTY(InputMode inputMode READ inputMode WRITE setInputMode)
    ///提供给QSS状态值，通过该状态值设置聚焦边框
    Q_PROPERTY(bool editFocus READ editFocus WRITE setEditFocus NOTIFY editFocusChanged)
    ///提供给QSS状态值，通过该状态设置设置错误红色边框
    Q_PROPERTY(bool hasError READ hasError WRITE setHasError NOTIFY hasErrorChanged)
    ///因Passwd模式字体需比普通模式小,但是需要Passwd模式PlaceHoldText字体和普通模式一样,
    ///不能使用Qt提供的状态去判断,故添加该属性用作判断
    Q_PROPERTY(bool showPasswordModeStyle READ showPasswordModeStyle WRITE setShowPasswordModeStyle NOTIFY showPasswordModeStyleChanged)
public:
    enum InputMode
    {
        INPUT_PROMPT,
        INPUT_USERNAME
    };

public:
    explicit GreeterLineEdit(QWidget *parent = nullptr);
    ~GreeterLineEdit();
Q_SIGNALS:
    /**
     * @brief 当输入框中输入回车会或按钮被点击时发出该信号
     *        同时启动Loading动画，若需停止动画，需调用reset
     * @param 确认的文本内容
     */
    void textConfirmed(const QString &data);
    void editFocusChanged(bool editFocus);
    void hasErrorChanged(bool hasError);
    void showPasswordModeStyleChanged(bool showPasswordModeStyle);

public:
    void startMovieAndEmitSignal();
    void reset();
    void setEchoMode(QLineEdit::EchoMode echoMode);
    void setPlaceHolderText(const QString &text);
    void setFocus();
    QString getText();
    InputMode inputMode() const;
    bool editFocus() const;
    bool hasError() const;
    bool showPasswordModeStyle() const;
public slots:
    void setInputMode(InputMode inputMode);
    void setEditFocus(bool editFocus);
    void setHasError(bool hasError);
    void setShowPasswordModeStyle(bool showPasswordModeStyle);

private:
    void initUI();
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
    Ui::GreeterLineEdit *ui;
    int m_animationTimerId;
    InputMode m_inputMode;
    bool m_editFocus;
    bool m_hasError;
    bool m_showPasswordModeStyle;
};

#endif  // GREETERLINEEDIT_H
