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
#pragma once

#include <QWidget>


namespace Kiran
{
namespace SessionGuard
{
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
}  // namespace SessionGuard
}  // namespace Kiran
