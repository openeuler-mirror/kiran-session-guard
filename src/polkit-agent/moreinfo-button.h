/**
 * Copyright (c) 2020 ~ 2023 KylinSec Co., Ltd.
 * kiran-session-guard is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     liuxinhao <liuxinhao@kylinsec.com.cn>
 */
#pragma once
#include <QIcon>
#include <QWidget>


namespace Kiran
{
namespace SessionGuard
{
namespace PolkitAgent
{
class MoreInfoButton : public QWidget
{
    Q_OBJECT
public:
    MoreInfoButton(QWidget* parent = nullptr);
    ~MoreInfoButton();

    void setText(const QString& text);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
signals:
    void expand();
    void shrink();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    bool m_isExpand = false;
    QPixmap m_expandPix;
    QPixmap m_shrinkPix;
    QSize m_iconSize;
    QString m_text;
};
}  // namespace PolkitAgent
}  // namespace SessionGuard
}  // namespace Kiran