#include "greeterkeyboard.h"
#include <QApplication>
#include <QTimer>
#include <QWidget>
#include <QWindow>

#include <QMutex>
#include <QScopedPointer>
#include <QScreen>

#include "log.h"

#define ONBOARD_LAYOUT "Compact"
#define ONBOARD_THEME "Blackboard"

#define ONBOARD_WIDTH_FACTOR  0.6
#define ONBOARD_HEIGHT_FACTOR 0.3

GreeterKeyboard *GreeterKeyboard::instance()
{
    static QMutex                          mutex;
    static QScopedPointer<GreeterKeyboard> pInst;

    if (Q_UNLIKELY(!pInst))
    {
        QMutexLocker locker(&mutex);
        if (pInst.isNull())
        {
            pInst.reset(new GreeterKeyboard);
        }
    }

    return pInst.data();
}

GreeterKeyboard::~GreeterKeyboard()
{
    if (m_process->state() != QProcess::NotRunning)
    {
        m_process->terminate();
        m_process->waitForFinished(300);
    }
}

bool GreeterKeyboard::init(QWidget *parent)
{
    if (m_keyboardWidget != nullptr)
    {
        return false;
    }
    m_process = new QProcess(this);
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(slot_finished(int, QProcess::ExitStatus)));
    connect(m_process, &QProcess::readyReadStandardOutput, this, [this, parent] {
        QString stdoutput;
        qulonglong xid = 0;
        QWindow *foreignWindow = nullptr;

        stdoutput = m_process->readAllStandardOutput();
        stdoutput = stdoutput.trimmed();
        if (stdoutput.isEmpty())
        {
            LOG_WARNING("can't get onboard xid!");
            return;
        }

        xid = stdoutput.toULongLong();

        foreignWindow = QWindow::fromWinId(xid);
        foreignWindow->setFlag(Qt::ForeignWindow);
        m_keyboardWidget = QWidget::createWindowContainer(foreignWindow, nullptr);
        m_keyboardWidget->setParent(parent);
        m_keyboardWidget->setFocusPolicy(Qt::NoFocus);
        m_keyboardWidget->raise();
        LOG_INFO("greeter keyboard init finish.");
    });
    m_process->start("onboard", QStringList() << "--xid"
                                              << "-t" ONBOARD_THEME << "-l" ONBOARD_LAYOUT << "-d"
                                              << "all");
    return true;
}

bool GreeterKeyboard::isVisible()
{
    if (m_keyboardWidget == nullptr)
    {
        return false;
    }
    return m_keyboardWidget->isVisible();
}

void GreeterKeyboard::showAdjustSize(QWidget *parent)
{
    if (m_keyboardWidget == nullptr)
    {
        LOG_WARNING(" greeter keyboard show adjust size must call after init!");
        return;
    }

    if (parent == nullptr)
    {
        LOG_WARNING_S() << "GreeterKeyboard::showAdjustSize parent can't be nullptr";
    }

    LOG_DEBUG_S() << "GreeterKeyboard::showAdjustSize" << parent->objectName();
    m_keyboardWidget->hide();
    QRect parentRect = parent->geometry();
    m_keyboardWidget->setParent(parent);

    QSize screenSize = qApp->primaryScreen()->size();
    QSize keyboardSize(screenSize.width()*ONBOARD_WIDTH_FACTOR,screenSize.height()*ONBOARD_HEIGHT_FACTOR);

    m_keyboardWidget->resize(screenSize.width()*ONBOARD_WIDTH_FACTOR, screenSize.height()*ONBOARD_HEIGHT_FACTOR);
    m_keyboardWidget->move((parentRect.width() - keyboardSize.width()) / 2, parentRect.height() - keyboardSize.height());
    m_keyboardWidget->show();
}

void GreeterKeyboard::hide()
{
    if (m_keyboardWidget == nullptr)
    {
        LOG_WARNING_S() << "GreeterKeyboard::hide must call after init";
        return;
    }
    m_keyboardWidget->hide();
}

QWidget *GreeterKeyboard::getKeyboard()
{
    if (m_keyboardWidget == nullptr)
    {
        LOG_WARNING_S() << "GreeterKeyboard::getKeyboard must call after GreeterKeyboard::init";
        return nullptr;
    }
    return m_keyboardWidget;
}

void GreeterKeyboard::keyboardProcessExit()
{
    if (m_process->state() != QProcess::NotRunning)
    {
        LOG_DEBUG_S() << "terminate keyboard process and wait exit.";
        m_process->terminate();
        m_process->waitForFinished(300);
    }
}

void GreeterKeyboard::slot_finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    LOG_DEBUG_S() << "onboard process finished : "
                  << "exitCode" << exitCode << "exitStaus" << exitStatus;
}

GreeterKeyboard::GreeterKeyboard(QObject *parent)
    : QObject(parent), m_keyboardWidget(nullptr)
{
}

void GreeterKeyboard::slotReadyReadStandardOutput()
{
}
