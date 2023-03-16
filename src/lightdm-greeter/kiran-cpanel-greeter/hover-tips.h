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
#include <QMap>
#include <QWidget>

QT_BEGIN_NAMESPACE
class QLabel;
QT_END_NAMESPACE

namespace Kiran
{
namespace SessionGuard
{
namespace Greeter
{
class HoverTips : private QWidget
{
    Q_OBJECT
public:
    enum HoverTipsTypeEnum
    {
        HOVE_TIPS_SUC,
        HOVE_TIPS_INFO,
        HOVE_TIPS_WARNING,
        HOVE_TIPS_ERR
    };
    Q_ENUMS(HoverTipsEnum);

public:
    explicit HoverTips(QWidget *parent = nullptr);
    ~HoverTips();

    void setTimeout(quint32 ms);

    void setIcon(HoverTipsTypeEnum typeEnum, const QString &icon);

    void show(HoverTipsTypeEnum typeEnum, const QString &msg);
    void hide();

private:
    void initUI();
    void updatePostion();
    void startHideTimer();
    void stopHideTimer();

protected:
    bool event(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event);
    void paintEvent(QPaintEvent *event) override;

private:
    QMap<HoverTipsTypeEnum, QString> m_tipsTypeIconMap = {
        {HOVE_TIPS_SUC, ":/kcp-greeter-images/suc.svg"},
        {HOVE_TIPS_INFO, ":/kcp-greeter-images/info.svg"},
        {HOVE_TIPS_WARNING, ":/kcp-greeter-images/warning.svg"},
        {HOVE_TIPS_ERR, ":/kcp-greeter-images/err.svg"}};
    QLabel *m_iconLabel;
    QLabel *m_textLabel;
    quint32 m_hideTimeout = 3000;
    int m_hideTimerID = -1;
};
}  // namespace Greeter
}  // namespace SessionGuard
}  // namespace Kiran