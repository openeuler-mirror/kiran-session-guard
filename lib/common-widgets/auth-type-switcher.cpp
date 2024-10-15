/**
 * Copyright (c) 2020 ~ 2024 KylinSec Co., Ltd.
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
#include "auth-type-switcher.h"
#include <qt5-log-i.h>
#include <QDebug>
#include <QPainter>
#include "auth-type-drawer.h"

#include <QTimer>

#define SHRINK_ICON ":/common-widgets-images/arrow.svg"

namespace Kiran
{
namespace SessionGuard
{
AuthTypeSwitcher::AuthTypeSwitcher(AuthTypeDrawerExpandDirection direction, int radius, QWidget* parent)
    : QWidget(parent),
      m_radius(radius)
{
    setFixedSize(42, 42);

    m_drawer = new AuthTypeDrawer(direction, radius, parent, this);
    connect(m_drawer, &AuthTypeDrawer::authTypeClicked, this, &AuthTypeSwitcher::onAuthTypeChanged);
    // clang-format off
    connect(m_drawer, &AuthTypeDrawer::expandedStatusChanged, [this](bool expanded){
        m_isExpanded = expanded;
        update();
    });
    // clang-format on
}

AuthTypeSwitcher::~AuthTypeSwitcher()
{
}

bool AuthTypeSwitcher::getExpaned()
{
    return m_isExpanded;
}

void AuthTypeSwitcher::clear()
{
    m_authTypeMap.clear();
    m_currentAuthType = -1;
}

void AuthTypeSwitcher::setAdjustColorToTheme(bool enable)
{
    m_drawer->setAdjustColorToTheme(enable);
}

void AuthTypeSwitcher::setAuthTypes(QList<KADAuthType> authtypes)
{
    // clang-format off
        const QMap<KADAuthType, QPair<QString, QString> > AuthTypeIconMap {
            {KAD_AUTH_TYPE_FACE, {":/common-widgets-images/face-auth.svg",tr("face auth")}},
            {KAD_AUTH_TYPE_FINGERPRINT,{":/common-widgets-images/finger-auth.svg",tr("finger auth")}},
            {KAD_AUTH_TYPE_PASSWORD,{":/common-widgets-images/passwd-auth.svg",tr("password auth")}},
            {KAD_AUTH_TYPE_FINGERVEIN,{":/common-widgets-images/fingervein-auth.svg",tr("finger vein auth")}},
            {KAD_AUTH_TYPE_IRIS,{":/common-widgets-images/iris-auth.svg",tr("iris auth")}},
            {KAD_AUTH_TYPE_UKEY,{":/common-widgets-images/ukey-auth.svg",tr("ukey auth")}},
        };
    // clang-format on

    clear();

    QList<std::tuple<int, QString, QString>> authTypeInfoList;
    for (auto authType : authtypes)
    {
        if (m_authTypeMap.find(authType) != m_authTypeMap.end())
        {
            continue;
        }
        auto iter = AuthTypeIconMap.find(authType);
        if (iter == AuthTypeIconMap.end())
        {
            KLOG_WARNING() << "unsupported authentication type" << authType;
            return;
        }
        QString icon = iter->first;
        QString tooltip = iter->second;

        authTypeInfoList << std::make_tuple(authType, tooltip, icon);
        m_authTypeMap[authType] = icon;
    }
    m_drawer->setAuthTypes(authTypeInfoList);
}

int AuthTypeSwitcher::getCurrentAuthType()
{
    return m_currentAuthType;
}

void AuthTypeSwitcher::setCurrentAuthType(int authType)
{
    if (m_authTypeMap.find((KADAuthType)authType) == m_authTypeMap.end())
    {
        return;
    }
    onAuthTypeChanged(authType);
}

void AuthTypeSwitcher::onAuthTypeChanged(int authType)
{
    m_currentAuthType = authType;
    update();
    emit authTypeChanged((KADAuthType)m_currentAuthType);
}

void AuthTypeSwitcher::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QColor("#2eb3ff"));
    painter.setPen(QColor("#2eb3ff"));
    painter.drawRoundedRect(this->rect(), m_radius, m_radius);

    QPixmap pixmap;

    if (m_isExpanded)
    {
        QIcon icon(SHRINK_ICON);
        pixmap = icon.pixmap(14,14);

        QTransform transform;
        if( m_drawer->getDirection() == EXPAND_DIRECTION_BOTTOM )
        {
            transform.rotate(180);
        }
        else
        {
            transform.rotate(90);
        }
        pixmap = pixmap.transformed(transform,Qt::SmoothTransformation);
    }
    else
    {
        auto iter = m_authTypeMap.find((KADAuthType)m_currentAuthType);
        if (iter != m_authTypeMap.end())
        {
            QIcon icon(iter.value());
            pixmap = icon.pixmap(14, 14);
        }
    }

    if (!pixmap.isNull())
    {
        QPixmap icon = pixmap.scaled(QSize(14, 14), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QRect targetRect = icon.rect();
        targetRect.moveCenter(rect().center());
        painter.drawImage(targetRect, icon.toImage());
    }
}

void AuthTypeSwitcher::mousePressEvent(QMouseEvent* event)
{
    if (m_drawer->isExpanded())
    {
        m_drawer->shrink();
    }
    else
    {
        m_drawer->expand();
    }
    QWidget::mousePressEvent(event);
}
}  // namespace SessionGuard
}  // namespace Kiran