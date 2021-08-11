#include "greeterlineedit.h"
#include <QKeyEvent>
#include <QPainter>
#include <QStyleOption>
#include <qt5-log-i.h>
#include "ui_greeterlineedit.h"

#define NORMAL_ICON ":/common-widgets-images/unlock.png"
#define LOADING_ICON_FORMAT ":/common-widgets-images/loading_%1.png"
#define LOADING_ICON_INDEX_MAX 17

GreeterLineEdit::GreeterLineEdit(QWidget *parent) : QWidget(parent),
                                                    ui(new Ui::GreeterLineEdit),
                                                    m_animationTimerId(0),
                                                    m_editFocus(false),
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

GreeterLineEdit::~GreeterLineEdit()
{
    if (m_animationTimerId)
    {
        killTimer(m_animationTimerId);
        m_animationTimerId = 0;
    }
    delete ui;
}

void GreeterLineEdit::initUI()
{
    setDefaultIcon();
}

void GreeterLineEdit::initConnection()
{
    connect(ui->edit, SIGNAL(returnPressed()),
            this, SLOT(slotEditReturnPressed()));
    connect(ui->button, SIGNAL(clicked()),
            this, SLOT(slotButtonPressed()));
    connect(ui->edit, SIGNAL(textChanged(const QString &)),
            this, SLOT(slotEditTextChanged(const QString &)));
}

void GreeterLineEdit::setDefaultIcon()
{
    KLOG_DEBUG() << "greeter lineedit default icon:" << NORMAL_ICON << QIcon(NORMAL_ICON).isNull();
    ui->button->setIcon(QIcon(NORMAL_ICON));
}

void GreeterLineEdit::setNormalLetterSpacing()
{
    QFont font = ui->edit->font();
    font.setLetterSpacing(QFont::PercentageSpacing, 0);
    ui->edit->setFont(font);
}

void GreeterLineEdit::setPasswdLetterSpacing()
{
    QFont font = ui->edit->font();
    font.setLetterSpacing(QFont::AbsoluteSpacing, 4);
    ui->edit->setFont(font);
}

void GreeterLineEdit::setEchoMode(QLineEdit::EchoMode echoMode)
{
    ui->edit->setEchoMode(echoMode);
    if (echoMode == QLineEdit::Normal)
    {
        setShowPasswordModeStyle(false);
        setNormalLetterSpacing();
    }
}

void GreeterLineEdit::setPlaceHolderText(const QString &text)
{
    ui->edit->setPlaceholderText(text);
}

void GreeterLineEdit::setFocus()
{
  if (!ui->edit->hasFocus())
    ui->edit->setFocus(Qt::OtherFocusReason);
}

QString GreeterLineEdit::getText()
{
    return ui->edit->text();
}

void GreeterLineEdit::setInputMode(GreeterLineEdit::InputMode inputMode)
{
    m_inputMode = inputMode;
}

void GreeterLineEdit::setEditFocus(bool editFocus)
{
    if (m_editFocus == editFocus)
        return;
    m_editFocus = editFocus;
    style()->polish(this);
    emit editFocusChanged(m_editFocus);
}

void GreeterLineEdit::setHasError(bool hasError)
{
    if (m_hasError == hasError)
        return;

    m_hasError = hasError;
    style()->polish(this);
    emit hasErrorChanged(m_hasError);
}

void GreeterLineEdit::setShowPasswordModeStyle(bool showPasswordModeStyle)
{
    if (m_showPasswordModeStyle == showPasswordModeStyle)
    {
        return;
    }
    m_showPasswordModeStyle = showPasswordModeStyle;
    style()->polish(this);
    style()->polish(ui->edit);
}

void GreeterLineEdit::slotEditReturnPressed()
{
    if ((ui->edit->echoMode() != QLineEdit::Password) && (ui->edit->text().isEmpty()))
    {
        return;
    }

    startMovieAndEmitSignal();
}

void GreeterLineEdit::slotButtonPressed()
{
    if ((ui->edit->echoMode() != QLineEdit::Password) && (ui->edit->text().isEmpty()))
    {
        return;
    }

    startMovieAndEmitSignal();
}

void GreeterLineEdit::slotEditTextChanged(const QString &text)
{
    ///密码框输入密码不为空的情况下调整字体和字间距
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

void GreeterLineEdit::timerEvent(QTimerEvent *e)
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

void GreeterLineEdit::paintEvent(QPaintEvent *e)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    QWidget::paintEvent(e);
}

///输入框聚焦改变，设置窗口的属性，便于QSS设置不同边框
bool GreeterLineEdit::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->edit)
    {
        if (event->type() == QEvent::FocusIn)
        {
            setEditFocus(true);
        }
        else if (event->type() == QEvent::FocusOut)
        {
            setEditFocus(false);
        }
        else if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = dynamic_cast<QKeyEvent *>(event);
            if (keyEvent->key() == Qt::Key_Escape && ui->edit->echoMode() == QLineEdit::Password)
            {
                ui->edit->clear();
            }
        }
    }
    return false;
}

void GreeterLineEdit::startMovieAndEmitSignal()
{
    ui->edit->setEnabled(false);
    if (m_animationTimerId == 0)
    {
        m_animationTimerId = startTimer(50);
    }
    emit textConfirmed(ui->edit->text());
}

void GreeterLineEdit::reset()
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

GreeterLineEdit::InputMode GreeterLineEdit::inputMode() const
{
    return m_inputMode;
}

bool GreeterLineEdit::editFocus() const
{
    return m_editFocus;
}

bool GreeterLineEdit::hasError() const
{
    return m_hasError;
}

bool GreeterLineEdit::showPasswordModeStyle() const
{
    return m_showPasswordModeStyle;
}
