#ifndef KIRAN_SCREENSAVER_DIALOG_FACEAUTHAVATAR_H
#define KIRAN_SCREENSAVER_DIALOG_FACEAUTHAVATAR_H

#include <QPixmap>
#include <QPropertyAnimation>
#include <QWidget>

class FaceAuthAvatar : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(uint progress READ progress WRITE setProgress)
public:
    explicit FaceAuthAvatar(QWidget *parent = nullptr);
    ~FaceAuthAvatar();

    void startAnimation();
    void stopAnimation();

private:
    void    init();
    uint    progress();
    void    setProgress(uint progress);
    QPixmap scalePixmapAdjustSize(const QPixmap &pixmap);

protected:
    virtual void paintEvent(QPaintEvent *event) override final;
    virtual void resizeEvent(QResizeEvent *event) override final;

private:
    QPixmap            m_pixmap;
    QPixmap            m_scaledPixmap;
    uint               m_progress;
    QPropertyAnimation m_animation;
};

#endif  //KIRAN_SCREENSAVER_DIALOG_FACEAUTHAVATAR_H
