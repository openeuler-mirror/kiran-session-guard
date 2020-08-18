#include "greeterkeyboard.h"
#include <QTimer>
#include <QDebug>
#include <QWindow>
#include <QWidget>
#include <QMutex>
#include <QApplication>

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

void GreeterKeyboard::init(QWidget*parent)
{
    if(m_keyboardWidget!=nullptr){
        return;
    }
    qDebug() << "greeter keyboard init";
    m_process = new QProcess(this);
    connect(m_process,QOverload<int,QProcess::ExitStatus>::of(&QProcess::finished),
            this,&GreeterKeyboard::slot_finished);
    connect(m_process,&QProcess::readyReadStandardOutput,this,[this,parent]{
        QString stdoutput = m_process->readAllStandardOutput();
        stdoutput = stdoutput.trimmed();
        if(stdoutput.isEmpty()){
            qWarning() << "onboard output is empty,can't get onbaord xid.";
            return;
        }
        qulonglong xid = stdoutput.toULongLong();
        QWindow* window = QWindow::fromWinId(xid);
        m_keyboardWidget = QWidget::createWindowContainer(window,nullptr,Qt::ForeignWindow);

        m_keyboardWidget->setParent(parent);
        m_keyboardWidget->setFocusPolicy(Qt::NoFocus);
        m_keyboardWidget->raise();
        qDebug() << "greeter keyboard init finish";
    });
    m_process->start("onboard",QStringList()<<"--xid"<<"-t" ONBOARD_THEME<<"-l" ONBOARD_LAYOUT<<"-d"<<"all");
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

///FIXME: 不复位parent，当parent析构时，onboard会crash
void GreeterKeyboard::resetParentAndTermProcess()
{
    if(m_keyboardWidget!=nullptr){
        m_keyboardWidget->setParent(nullptr);
        m_keyboardWidget->deleteLater();
        m_process->terminate();
        m_process->waitForFinished();
    }
}

void GreeterKeyboard::slot_finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qInfo() << "onboard: " << "exitCode" << exitCode << "exitStaus" << exitStatus;
}

GreeterKeyboard::GreeterKeyboard(QObject *parent)
    : QObject(parent)
    , m_keyboardWidget(nullptr)
{
    qApp->installEventFilter(this);
}

bool GreeterKeyboard::eventFilter(QObject *obj, QEvent *event)
{
    if(m_keyboardWidget!=nullptr&&m_keyboardWidget->parentWidget()!=nullptr&&
            obj==qApp&&event->type()==QEvent::Quit){
        GreeterKeyboard::resetParentAndTermProcess();
    }
    return false;
}

void GreeterKeyboard::slotReadyReadStandardOutput()
{

}
