/**
 *@file  FingerAuthAvatar.h
 *@brief 
 *@auth  liuxinhao <liuxinhao@kylinos.com.cn>
 *@copyright (c) 2021 KylinSec. All rights reserved.
 */
#ifndef __FINGERAUTHAVATAR_H__
#define __FINGERAUTHAVATAR_H__

#include <QPropertyAnimation>
#include <QWidget>

class FingerAuthAvatar : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(uint progress READ progress WRITE setProgress)
public:
    FingerAuthAvatar(QWidget *parent = nullptr);
    ~FingerAuthAvatar();

    void startAnimation();
    void stopAnimation();

private:
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

#endif  //__FINGERAUTHAVATAR_H__
