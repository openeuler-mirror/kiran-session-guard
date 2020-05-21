#include "greeterkeyboard.h"
#include <QTimer>
#include <QDebug>
#include <QWindow>
#include <QWidget>

#define ONBOARD_LAYOUT "Phone"
#define ONBOARD_THEME  "Blackboard"

#define ONBOARD_FIXED_WIDTH 800
#define ONBOARD_FIXED_HEIGHT 300

GreeterKeyboard &GreeterKeyboard::instance()
{
    static GreeterKeyboard keyboard;
    return keyboard;
}

void GreeterKeyboard::init(QWidget*parent)
{
    if(m_keyboardWidget!=nullptr){
        return;
    }
    qDebug() << "greeter keyboard init";
    QProcess* process = new QProcess(this);
    connect(process,&QProcess::readyReadStandardOutput,this,[this,process,parent]{
        QString stdoutput = process->readAllStandardOutput();
        stdoutput = stdoutput.trimmed();
        if(stdoutput.isEmpty()){
            qWarning() << "onboard output is empty,can't get onbaord xid.";
            return;
        }
        qulonglong xid = stdoutput.toULongLong();
        QWindow* window = QWindow::fromWinId(xid);
        m_keyboardWidget = QWidget::createWindowContainer(window,nullptr,Qt::FramelessWindowHint);
        m_keyboardWidget->setParent(parent);
        m_keyboardWidget->setFocusPolicy(Qt::NoFocus);
        m_keyboardWidget->raise();
        qDebug() << "greeter keyboard init finish";
    });
    process->start("onboard",QStringList()<<"--xid"<<"-t" ONBOARD_THEME<<"-l" ONBOARD_LAYOUT<<"-d"<<"all");
}

bool GreeterKeyboard::isVisible()
{
    if(m_keyboardWidget==nullptr){
        return false;
    }
    return m_keyboardWidget->isVisible();
}

void GreeterKeyboard::showAdjustSize(QWidget *parent)
{
    if(m_keyboardWidget==nullptr){
        qWarning() << "GreeterKeyboard::showAdjustSize must call after init";
        return;
    }
    m_keyboardWidget->hide();
    QRect parentRect = parent->geometry();
    m_keyboardWidget->setParent(parent);
    m_keyboardWidget->resize(ONBOARD_FIXED_WIDTH,ONBOARD_FIXED_HEIGHT);
    m_keyboardWidget->move((parentRect.width()-ONBOARD_FIXED_WIDTH)/2,parentRect.height()-ONBOARD_FIXED_HEIGHT);
    m_keyboardWidget->show();
}

void GreeterKeyboard::hide()
{
    if(m_keyboardWidget==nullptr){
        qWarning() << "GreeterKeyboard::hide must call after init";
        return;
    }
    m_keyboardWidget->hide();
}

QWidget *GreeterKeyboard::getKeyboard()
{
    if(m_keyboardWidget==nullptr){
        qWarning() << "GreeterKeyboard::getKeyboard must call after GreeterKeyboard::init";
        return nullptr;
    }
    return m_keyboardWidget;
}

GreeterKeyboard::GreeterKeyboard(QObject *parent)
    : QObject(parent)
    , m_keyboardWidget(nullptr)
{

}

void GreeterKeyboard::slotReadyReadStandardOutput()
{

}
