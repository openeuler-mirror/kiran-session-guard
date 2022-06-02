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
