#include "useravatarwidget.h"
#include <QDebug>
#include <QFile>
#include <QPainter>
#include "log.h"

#define DEFAULT_USER_AVATAR ":/images/user_180.png"

UserAvatarWidget::UserAvatarWidget(QWidget *parent) : QWidget(parent)
{
    setDefaultImage();
}

void UserAvatarWidget::setImage(const QString &path)
{
    QFile file(path);
    if (!file.exists())
    {
        LOG_WARNING_S() << "UserAvatar: file path[" << path << "] is no't exist";
    }
    if (!file.isReadable())
    {
        LOG_WARNING_S() << "UserAvatar: file path[" << path << "] can't read";
    }
    if (m_pixmap.load(path))
    {
        LOG_WARNING_S() << "UserAvatar: load file " << path << "successed";
        m_scaledPixmap = scalePixmapAdjustSize(m_pixmap);
    }
    else
    {
        LOG_WARNING_S() << "UserAvatar: file path[" << path << "] load failed.";
        if (path != DEFAULT_USER_AVATAR)
        {
            setDefaultImage();
        }
    }
    update();
}

void UserAvatarWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QPen     pen;
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

void UserAvatarWidget::resizeEvent(QResizeEvent *event)
{
    if ((!m_pixmap.isNull()) && (!m_scaledPixmap.isNull()) && (m_scaledPixmap.size() != this->size()))
    {
        m_scaledPixmap = scalePixmapAdjustSize(m_pixmap);
    }
    QWidget::resizeEvent(event);
}

QPixmap UserAvatarWidget::scalePixmapAdjustSize(const QPixmap &pixmap)
{
    double radius = (this->width() < this->height() ? this->width() : this->height()) / 2;
    //NOTE:拉升保持长宽比，尽可能放大，不留白
    QPixmap temp = pixmap.scaled(radius * 2, radius * 2, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    return temp;
}

void UserAvatarWidget::setDefaultImage()
{
    setImage(DEFAULT_USER_AVATAR);
}
