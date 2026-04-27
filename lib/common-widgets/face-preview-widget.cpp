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
 */

#include "face-preview-widget.h"

#include <qt5-log-i.h>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusError>
#include <QDateTime>
#include <QHideEvent>
#include <QFile>
#include <QDir>
#include <QLabel>
#include <QLayout>
#include <QPixmap>
#include <QShowEvent>
#include <QVBoxLayout>

namespace
{
const QString kFaceService = QStringLiteral("com.czht.face.daemon");
const QString kFacePath = QStringLiteral("/com/czht/face/daemon");
const QString kFaceInterface = QStringLiteral("com.czht.face.daemon");
constexpr int kPreviewMaxW = 200;
constexpr int kPreviewMaxH = 150;

QDBusConnection selectBus()
{
    return QDBusConnection::systemBus();
}
}  // namespace

namespace Kiran
{
namespace SessionGuard
{

bool FacePreviewWidget::isFaceDaemonAvailable()
{
    auto bus = selectBus();
    auto* iface = bus.interface();
    if (!iface)
    {
        return false;
    }
    return iface->isServiceRegistered(kFaceService);
}

FacePreviewWidget::FacePreviewWidget(QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("FacePreviewWidget"));
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_label = new QLabel(this);
    m_label->setObjectName(QStringLiteral("FacePreviewLabel"));
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setScaledContents(false);
    m_label->setMinimumSize(kPreviewMaxW, kPreviewMaxH);
    m_label->setMaximumSize(kPreviewMaxW, kPreviewMaxH);
    m_label->setStyleSheet(QStringLiteral("border: 1px solid rgba(255,255,255,0.35); border-radius: 4px;"));
    layout->addWidget(m_label, 0, Qt::AlignCenter);
}

FacePreviewWidget::~FacePreviewWidget()
{
    disconnectVideoInfo();
}

void FacePreviewWidget::connectVideoInfo()
{
    if (m_connected)
    {
        return;
    }

    QDBusConnection bus = selectBus();
    const bool ok = bus.connect(kFaceService,
                                kFacePath,
                                kFaceInterface,
                                QStringLiteral("VideoInfo"),
                                this,
                                SLOT(onVideoInfo(QByteArray)));
    if (!ok)
    {
        if (!m_connectFailedLogged)
        {
            m_connectFailedLogged = true;
            KLOG_WARNING() << "FacePreview: connect VideoInfo failed, bus name=" << bus.name()
                           << "error=" << bus.lastError().message();
        }
        return;
    }
    m_signalBus = new QDBusConnection(bus);
    m_connected = true;
    KLOG_INFO() << "FacePreview: VideoInfo signal connected, bus name=" << bus.name();
}

void FacePreviewWidget::disconnectVideoInfo()
{
    if (!m_connected || !m_signalBus)
    {
        return;
    }
    m_signalBus->disconnect(kFaceService,
                            kFacePath,
                            kFaceInterface,
                            QStringLiteral("VideoInfo"),
                            this,
                            SLOT(onVideoInfo(QByteArray)));
    delete m_signalBus;
    m_signalBus = nullptr;
    m_connected = false;
    KLOG_INFO() << "FacePreview: VideoInfo disconnected";
}

void FacePreviewWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    KLOG_INFO() << "FacePreview: showEvent -> connect VideoInfo";
    connectVideoInfo();
}

void FacePreviewWidget::hideEvent(QHideEvent* event)
{
    KLOG_INFO() << "FacePreview: hideEvent -> disconnect VideoInfo";
    disconnectVideoInfo();
    QWidget::hideEvent(event);
}

void FacePreviewWidget::onVideoInfo(const QByteArray& jpegData)
{
    if (jpegData.isEmpty())
    {
        return;
    }
    if (!m_loggedFirstPayload)
    {
        m_loggedFirstPayload = true;
        KLOG_INFO() << "FacePreview: first VideoInfo payload, bytes=" << jpegData.size();
    }

    QPixmap pix;
    if (!pix.loadFromData(jpegData))
    {
        if (!m_loggedFirstDecodeFail)
        {
            m_loggedFirstDecodeFail = true;
            KLOG_INFO() << "FacePreview: loadFromData failed (payload not decodable as image), bytes="
                         << jpegData.size();
        }
        return;
    }
    m_label->setPixmap(
        pix.scaled(kPreviewMaxW, kPreviewMaxH, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}
}  // namespace SessionGuard
}  // namespace Kiran
