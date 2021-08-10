#include "useravatarwidget.h"
#include <QDebug>
#include <QPainter>
#include <QFile>
#include <qt5-log-i.h>

#define DEFAULT_USER_AVATAR ":/common-widgets-images/user_180.png"


UserAvatarWidget::UserAvatarWidget (QWidget *parent) : QWidget(parent)
{
    setDefaultImage();
}

void UserAvatarWidget::setImage (const QString &path)
{
    QFile file(path);
    if (!file.exists())
    {
        KLOG_WARNING() << "UserAvatar: file path[" << path << "] is no't exist";
    }
    if (!file.isReadable())
    {
        KLOG_WARNING() << "UserAvatar: file path[" << path << "] can't read";
    }
    if (m_pixmap.load(path))
    {
        KLOG_WARNING() << "UserAvatar: load file " << path << "successed";
        int radius = this->width() < this->height() ? this->width() / 2 : this->height();
        m_scaledPixmap = m_pixmap.scaled(2 * radius, 2 * radius,
                                         Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    else
    {
        KLOG_WARNING() << "UserAvatar: file path[" << path << "] load failed.";
        setDefaultImage();
    }
    update();
}

void UserAvatarWidget::paintEvent (QPaintEvent *event)
{
    QPainter painter(this);
    QPen pen;
    if (!isVisible())
    {
        return;
    }
    painter.setRenderHints(
            QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing | QPainter::Antialiasing);
    double radius = (this->width() < this->height() ? this->width() : this->height()) / 2;
    if (!m_scaledPixmap.isNull())
    {
        QPainterPath painterPath;
        painterPath.addEllipse((this->width() - (radius * 2)) / 2, (this->height() - (radius * 2)) / 2, radius * 2,
                               radius * 2);
        painter.setClipPath(painterPath);
        painter.drawPixmap((this->width() - m_scaledPixmap.width()) / 2, (this->height() - m_scaledPixmap.height()) / 2,
                           m_scaledPixmap);
    }
    QWidget::paintEvent(event);
}

void UserAvatarWidget::resizeEvent (QResizeEvent *event)
{
    if ((!m_pixmap.isNull()) && (!m_scaledPixmap.isNull()) && (m_scaledPixmap.size() != this->size()))
    {
        m_scaledPixmap = scalePixmapAdjustSize(m_pixmap);
    }
    QWidget::resizeEvent(event);
}

QPixmap UserAvatarWidget::scalePixmapAdjustSize (const QPixmap &pixmap)
{
    double radius = (this->width() < this->height() ? this->width() : this->height()) / 2;
    //NOTE:拉升保持长宽比，尽可能放大，不留白
    QPixmap temp = pixmap.scaled(radius * 2, radius * 2, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    return temp;
}
#include <QApplication>
void UserAvatarWidget::setDefaultImage ()
{
    if (!m_pixmap.load(DEFAULT_USER_AVATAR))
    {
        KLOG_WARNING() << "UserAvatar: " << "load default avatar failed.";
        return;
    }
    m_scaledPixmap = scalePixmapAdjustSize(m_pixmap);
}
