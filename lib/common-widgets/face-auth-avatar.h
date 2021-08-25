/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd.
 * kiran-session-guard is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     liuxinhao <liuxinhao@kylinos.com.cn>
 */

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
