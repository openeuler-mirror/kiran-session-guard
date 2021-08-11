#ifndef USERAVATARWIDGET_H
#define USERAVATARWIDGET_H

#include <QWidget>

class UserAvatar : public QWidget
{
    Q_OBJECT
public:
    explicit UserAvatar(QWidget *parent = nullptr);
    void setImage(const QString &path);
    void setDefaultImage();

protected:
    virtual void paintEvent(QPaintEvent *event) override final;
    virtual void resizeEvent(QResizeEvent *event) override final;

private:
    QPixmap scalePixmapAdjustSize(const QPixmap &pixmap);

private:
    QPixmap m_scaledPixmap;
    QPixmap m_pixmap;
};

#endif  // USERAVATARWIDGET_H
