#include "greeterbackground.h"
#include <QDebug>
#include <QPainter>
#include <QScreen>

#include "greeterloginwindow.h"
#include "kiran-greeter-prefs.h"

#define DEFAULT_BACKGROUND "/usr/share/backgrounds/default.jpg"

QT_BEGIN_NAMESPACE
Q_WIDGETS_EXPORT void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed = 0);
QT_END_NAMESPACE

GreeterBackground::GreeterBackground(QScreen *screen, QWidget *parent)
    : QWidget(parent), m_screen(nullptr)
{
#ifndef TEST
    setWindowFlags(windowFlags() | Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnBottomHint);
#endif
    if (!m_background.load(KiranGreeterPrefs::instance()->backgroundFile()))
    {
        qWarning() << "background load pixmap <" << KiranGreeterPrefs::instance()->backgroundFile() << "> failed.";
        m_background.load(DEFAULT_BACKGROUND);
    }
    setScreen(screen);
}

GreeterBackground::~GreeterBackground()
{
}

void GreeterBackground::setScreen(QScreen *screen)
{
    if (m_screen != nullptr)
    {
        disconnect(screen, &QScreen::geometryChanged,
                   this, &GreeterBackground::slotScreenGeometryChanged);
    }

    if (screen != nullptr)
    {
        connect(screen, &QScreen::geometryChanged,
                this, &GreeterBackground::slotScreenGeometryChanged);
    }

    m_screen = screen;
    if (m_screen)
    {
        slotScreenGeometryChanged(m_screen->geometry());
    }
}

void GreeterBackground::slotScreenGeometryChanged(const QRect &geometry)
{
    qInfo() << "background screen geometry changed: " << objectName() << " " << geometry;
    this->resize(geometry.size());
    this->move(geometry.x(), geometry.y());
}

void GreeterBackground::enterEvent(QEvent *event)
{
    qInfo() << "background mouse enter in: " << objectName();
    emit mouseEnter(this);
    QWidget::enterEvent(event);
}

void GreeterBackground::resizeEvent(QResizeEvent *event)
{
    qInfo() << "background resize: " << objectName() << ":" << this->size();
    if (!m_background.isNull())
    {
        m_scaledBackground = m_background.scaled(this->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        QImage tmp         = m_scaledBackground.toImage();
        qt_blurImage(tmp, 10, true);
        m_scaledBackground = QPixmap::fromImage(tmp);
    }
    //NOTE:子窗体因未加入布局，需要手动Resize
    GreeterLoginWindow *greeterWindow = findChild<GreeterLoginWindow *>("GreeterLoginWindow");
    if (greeterWindow != nullptr)
    {
        greeterWindow->resize(this->size());
    }
    QWidget::resizeEvent(event);
}

void GreeterBackground::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    if (!m_scaledBackground.isNull())
    {
        painter.drawPixmap(this->rect(), m_scaledBackground, m_scaledBackground.rect());
    }
    else
    {
        painter.fillRect(this->rect(), QColor(0, 0, 0));
    }
    QWidget::paintEvent(event);
}
