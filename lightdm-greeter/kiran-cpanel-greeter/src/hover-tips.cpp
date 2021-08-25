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

#include "hover-tips.h"

#include <QDebug>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QStyleOption>
#include <qt5-log-i.h>

//TODO: 需改成不使用样式表,可以加入到kiranwidgets-qt5自定义控件，调用KiranStyle进行绘制
#define HOVERTIPS_STYLESHEET "HoverTips {"                             \
                             "background-color:#222222;"               \
                             "border:1px solid rgba(255,255,255,0.1);" \
                             "border-radius:4px;"                      \
                             "}"                                       \
                             "HoverTips > QLabel#label_text{"          \
                             "color:white;"                            \
                             "}"

HoverTips::HoverTips(QWidget *parent)
    : QWidget(parent)
{
    initUI();
    setVisible(false);
    this->setFixedHeight(36);
}

HoverTips::~HoverTips()
{
}

void HoverTips::show(HoverTipsTypeEnum typeEnum, const QString &msg)
{
    auto iter = m_tipsTypeIconMap.find(typeEnum);
    if (iter == m_tipsTypeIconMap.end())
    {
        qWarning() << "invalid type enum";
        return;
    }

    if (isVisible())
    {
        hide();
    }

    QString pixmapPath = iter.value();
    m_iconLabel->setPixmap(pixmapPath);
    m_textLabel->setText(msg);

    QWidget::show();

    startHideTimer();
}

void HoverTips::hide()
{
    QWidget::hide();
}

void HoverTips::updatePostion()
{
    if (parentWidget() == nullptr)
    {
        KLOG_WARNING() << "hover tips parent widget is null";
        return;
    }
    this->move((parentWidget()->width() - width()) / 2,
               (parentWidget()->height() - height()) / 2);
}

bool HoverTips::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == parentWidget())
    {
        switch (event->type())
        {
        case QEvent::Resize:
            updatePostion();
            break;
        case QEvent::Move:
            updatePostion();
            break;
        default:
            break;
        }
    }
    return QObject::eventFilter(watched, event);
}

void HoverTips::setIcon(HoverTips::HoverTipsTypeEnum typeEnum, const QString &icon)
{
    QPixmap pixmap;
    if (!pixmap.load(icon) || pixmap.isNull())
    {
        KLOG_WARNING() << "load icon failed.";
        return;
    }
    m_tipsTypeIconMap[typeEnum] = icon;
}

void HoverTips::initUI()
{
    this->setStyleSheet(HOVERTIPS_STYLESHEET);
    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(6);
    mainLayout->setContentsMargins(12, -1, 12, -1);

    m_iconLabel = new QLabel(this);
    m_iconLabel->setObjectName("label_icon");
    m_iconLabel->setFixedSize(16, 16);
    mainLayout->addWidget(m_iconLabel);

    m_textLabel = new QLabel(this);
    m_textLabel->setObjectName("label_text");
    m_textLabel->setScaledContents(false);
    mainLayout->addWidget(m_textLabel);
}

void HoverTips::paintEvent(QPaintEvent *event)
{
    QStyleOption styleOption;
    QPainter     painter(this);

    styleOption.init(this);
    style()->drawPrimitive(QStyle::PE_Widget,
                           &styleOption,
                           &painter,
                           this);
}

bool HoverTips::event(QEvent *event)
{
    switch (event->type())
    {
    case QEvent::ShowToParent:
    {
        adjustSize();
        updatePostion();
        break;
    }
    case QEvent::Timer:
    {
        auto timerEvent = dynamic_cast<QTimerEvent *>(event);
        if (timerEvent->timerId() == m_hideTimerID)
        {
            QWidget::hide();
            stopHideTimer();
        }
        break;
    }
    case QEvent::MouseButtonPress:
    {
        QWidget::hide();
        stopHideTimer();
        break;
    }
    default:
        break;
    }
    return QWidget::event(event);
}

void HoverTips::setTimeout(quint32 ms)
{
    if (m_hideTimeout == ms)
    {
        return;
    }

    if (ms == 0)
    {
        stopHideTimer();
        return;
    }

    m_hideTimeout = ms;
}

void HoverTips::startHideTimer()
{
    if (m_hideTimeout == 0)
    {
        return;
    }
    stopHideTimer();
    m_hideTimerID = startTimer(m_hideTimeout);
}

void HoverTips::stopHideTimer()
{
    if (m_hideTimerID == -1)
    {
        return;
    }
    killTimer(m_hideTimerID);
    m_hideTimerID = -1;
}
