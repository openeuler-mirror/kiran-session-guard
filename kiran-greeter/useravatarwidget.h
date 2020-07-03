#ifndef USERAVATARWIDGET_H
#define USERAVATARWIDGET_H

#include <QWidget>

class UserAvatarWidget : public QWidget
{
    Q_OBJECT
public:
    explicit UserAvatarWidget(QWidget *parent = nullptr);
    void setImage(const QString& path);
    void setDefaultImage();
signals:
public slots:
protected:
    virtual void paintEvent(QPaintEvent *event) override final;
    virtual void resizeEvent(QResizeEvent *event) override final;
protected:
    QPixmap generateUserIconFitWidgetSize(const QPixmap& pixmap);
private:
    QPixmap scalePixmapAdjustSize(const QPixmap& pixmap);
private:
    QPixmap m_scaledPixmap;
    QPixmap m_pixmap;
};

#endif // USERAVATARWIDGET_H
