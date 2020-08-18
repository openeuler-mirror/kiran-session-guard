#include "greeterkeyboard.h"
#include <QTimer>
#include <QDebug>
#include <QWindow>
#include <QWidget>
#include <QApplication>
#include <QDateTime>
#include <QMutex>
#include <QScopedPointer>

#define ONBOARD_LAYOUT "Phone"
#define ONBOARD_THEME  "Blackboard"

#define ONBOARD_FIXED_WIDTH 800
#define ONBOARD_FIXED_HEIGHT 300

GreeterKeyboard *GreeterKeyboard::instance()
{
    static QMutex mutex;
    static QScopedPointer<GreeterKeyboard> pInst;

    if(Q_UNLIKELY(!pInst)){
        QMutexLocker locker(&mutex);
        if(pInst.isNull()){
            pInst.reset(new GreeterKeyboard);
        }
    }

    return pInst.data();
}

GreeterKeyboard::~GreeterKeyboard()
{
    if(m_process->state()!=QProcess::NotRunning){
        m_process->terminate();
        m_process->waitForFinished();
    }
}

GreeterKeyboard::~GreeterKeyboard()
{
    if(m_process->state()!=QProcess::NotRunning){
        m_process->terminate();
        m_process->waitForFinished();
    }
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
        foreignWindow->setFlag(Qt::ForeignWindow);
        m_keyboardWidget = QWidget::createWindowContainer(foreignWindow,nullptr);
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

void GreeterKeyboard::keyboardProcessExit()
{
    if(m_process->state()!=QProcess::NotRunning){
        m_process->terminate();
        m_process->waitForFinished(300);
    }
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
