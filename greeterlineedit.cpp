#include "greeterlineedit.h"
#include "ui_greeterlineedit.h"
#include <QDebug>
#include <QPainter>
#include <QStyleOption>

#define NORMAL_ICON             ":/images/arrow.png"
#define LOADING_ICON_FORMAT     ":/images/loading_%1.png"
#define LOADING_ICON_INDEX_MAX  17

#define NORMAL_EDIT_STYLE "QLineEdit{"\
                          "     border:opx;"\
                          "     background-color:transparent;"\
                          "     font-family:\"Noto Sans CJK SC\";"\
                          "     font-size:14px;"\
                          "     color:white;"\
                          "}"
#define PASSWD_EDIT_STYLE "QLineEdit{"\
                          "     border:opx;"\
                          "     background-color:transparent;"\
                          "     font-family:\"Noto Sans CJK SC\";"\
                          "     font-size:7px;"\
                          "     color:white;"\
                          "}"

GreeterLineEdit::GreeterLineEdit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GreeterLineEdit),
    m_animationTimerId(0)
{
    ui->setupUi(this);
    setDefaultIcon();
    initConnection();
}

GreeterLineEdit::~GreeterLineEdit()
{
    if(m_animationTimerId){
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
    connect(ui->edit,SIGNAL(returnPressed()),
            this,SLOT(slotEditReturnPressed()));
    connect(ui->button,SIGNAL(clicked()),
            this,SLOT(slotButtonPressed()));
    connect(ui->edit,SIGNAL(textChanged(const QString &)),
            this,SLOT(slotEditTextChanged(const QString &)));
}

void GreeterLineEdit::setDefaultIcon()
{
    ui->button->setIcon(QIcon(NORMAL_ICON));
}

void GreeterLineEdit::setEchoMode(QLineEdit::EchoMode echoMode)
{
    ui->edit->setEchoMode(echoMode);
    if(echoMode==QLineEdit::Normal){
        ui->edit->setStyleSheet(NORMAL_EDIT_STYLE);
    }
}

void GreeterLineEdit::setPlaceHolderText(const QString &text)
{
    ui->edit->setPlaceholderText(text);
}

void GreeterLineEdit::setFocus()
{
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

void GreeterLineEdit::slotEditReturnPressed()
{
    if( (ui->edit->echoMode()!=QLineEdit::Password) && (ui->edit->text().isEmpty()) ){
        return;
    }

    startMovieAndEmitSignal();
}

void GreeterLineEdit::slotButtonPressed()
{
    if( (ui->edit->echoMode()!=QLineEdit::Password) && (ui->edit->text().isEmpty()) ){
        return;
    }

    startMovieAndEmitSignal();
}

void GreeterLineEdit::slotEditTextChanged(const QString &text)
{
    ///密码框圆形字符缩小
    if(ui->edit->echoMode()==QLineEdit::Password && text.isEmpty()){
        ui->edit->setStyleSheet(NORMAL_EDIT_STYLE);
    }else if(ui->edit->echoMode()==QLineEdit::Password && !text.isEmpty() ){
        ui->edit->setStyleSheet(PASSWD_EDIT_STYLE);
    }
}

void GreeterLineEdit::timerEvent(QTimerEvent *e)
{
    static int iconIndex = 0;

    if(e->timerId()!=m_animationTimerId){
        return;
    }

    if(iconIndex>LOADING_ICON_INDEX_MAX){
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

void GreeterLineEdit::startMovieAndEmitSignal()
{
    ui->edit->setEnabled(false);
    if( m_animationTimerId == 0 ){
        m_animationTimerId = startTimer(80);
    }
    emit textConfirmed(ui->edit->text());
}

void GreeterLineEdit::reset()
{
    ui->edit->setEnabled(true);
    ui->edit->clear();
    ui->edit->setPlaceholderText("");
    setEchoMode(QLineEdit::Normal);
    if(m_animationTimerId!=0){
        killTimer(m_animationTimerId);
        m_animationTimerId=0;
    }
    setDefaultIcon();
}

GreeterLineEdit::InputMode GreeterLineEdit::inputMode() const
{
    return m_inputMode;
}
