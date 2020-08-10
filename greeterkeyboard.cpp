#include "greeterkeyboard.h"
#include <QTimer>
#include <QDebug>
#include <QWindow>
#include <QWidget>
#include <QApplication>
#include <QDateTime>

#define ONBOARD_LAYOUT "Phone"
#define ONBOARD_THEME  "Blackboard"

#define ONBOARD_FIXED_WIDTH 800
#define ONBOARD_FIXED_HEIGHT 300

GreeterKeyboard &GreeterKeyboard::instance()
{
    static GreeterKeyboard keyboard;
    return keyboard;
}

bool GreeterKeyboard::init(QWidget*parent)
{
    if(m_keyboardWidget!=nullptr){
        return false;
    }
    m_process = new QProcess(this);
    connect(m_process,SIGNAL(finished(int,QProcess::ExitStatus)),
            this,SLOT(slot_finished(int,QProcess::ExitStatus)));
    connect(m_process,&QProcess::readyReadStandardOutput,this,[this,parent]{
        QString stdoutput;
        qulonglong xid = 0;
        QWindow* foreignWindow=nullptr;

        stdoutput = m_process->readAllStandardOutput();
        stdoutput = stdoutput.trimmed();
        if(stdoutput.isEmpty()){
            qWarning() << "onboard output is empty,can't get onbaord xid.";
            return;
        }

        xid = stdoutput.toULongLong();

        foreignWindow = QWindow::fromWinId(xid);

        m_keyboardWidget = QWidget::createWindowContainer(foreignWindow,nullptr,Qt::ForeignWindow);
        m_keyboardWidget->setParent(parent);
        m_keyboardWidget->setFocusPolicy(Qt::NoFocus);
        m_keyboardWidget->raise();
        qDebug() << "greeter keyboard init finish";
    });
    m_process->start("onboard",QStringList()<<"--xid"<<"-t" ONBOARD_THEME<<"-l" ONBOARD_LAYOUT<<"-d"<<"all");
    return true;
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

    if(parent==nullptr){
        qWarning() << "GreeterKeyboard::showAdjustSize parent can't be nullptr";
    }

    qDebug() << "GreeterKeyboard::showAdjustSize" << parent->objectName();

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

///FIXME: 不复位parent，当parent析构时，onboard会crash
void GreeterKeyboard::resetParentAndTermProcess()
{
    qInfo() << "resetParentAndTermProcess 1:" << QDateTime::currentDateTime();
    if(m_keyboardWidget!=nullptr){
        m_keyboardWidget->setParent(nullptr);
        m_keyboardWidget->deleteLater();
        if(m_process->state()!=QProcess::NotRunning){
            m_process->terminate();
            m_process->waitForFinished(1000);
        }
    }
    qInfo() << "resetParentAndTermProcess 2:" << QDateTime::currentDateTime();
}

void GreeterKeyboard::slot_finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qInfo() << "onboard process finished : " << "exitCode" << exitCode << "exitStaus" << exitStatus;
}

GreeterKeyboard::GreeterKeyboard(QObject *parent)
    : QObject(parent)
    , m_keyboardWidget(nullptr)
{

}

void GreeterKeyboard::slotReadyReadStandardOutput()
{

}
