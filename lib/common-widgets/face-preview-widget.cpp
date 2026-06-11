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

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace
{
/** kiran-face-dbus-service 的 D-Bus 服务名 */
const QString kFaceService = QStringLiteral("com.kiran.face.service");
/** SHM key 文件路径 */
const QString kShmKeyFile = QStringLiteral("/tmp/kiran_face_preview_shm_key");
/** SHM 名称 */
const char *kShmName = "/kiran_face_preview";
/** 预览最大宽度 */
constexpr int kPreviewMaxW = 200;
/** 预览最大高度 */
constexpr int kPreviewMaxH = 150;
/** SHM 刷新间隔（ms），约 15 fps */
constexpr int kRefreshIntervalMs = 67;

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
    auto *iface = bus.interface();
    if (!iface)
    {
        return false;
    }
    return iface->isServiceRegistered(kFaceService);
}

FacePreviewWidget::FacePreviewWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("FacePreviewWidget"));
    auto *layout = new QVBoxLayout(this);
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

    m_refreshTimer = new QTimer(this);
    m_refreshTimer->setTimerType(Qt::PreciseTimer);
    connect(m_refreshTimer, &QTimer::timeout, this, &FacePreviewWidget::onRefreshTimer);
}

FacePreviewWidget::~FacePreviewWidget()
{
    disconnectShm();
}

bool FacePreviewWidget::connectShm()
{
    if (m_shmFd >= 0)
    {
        return true;
    }

    QFile keyFile(kShmKeyFile);
    if (!keyFile.open(QIODevice::ReadOnly))
    {
        KLOG_WARNING() << "FacePreview: cannot open SHM key file:" << kShmKeyFile;
        return false;
    }

    QByteArray keyData = keyFile.readAll().trimmed();
    keyFile.close();

    if (keyData.isEmpty())
    {
        KLOG_WARNING() << "FacePreview: SHM key file is empty:" << kShmKeyFile;
        return false;
    }

    size_t shmSize = keyData.toUInt();
    if (shmSize == 0)
    {
        KLOG_WARNING() << "FacePreview: SHM size parsed as 0 from key file";
        return false;
    }

    m_shmFd = ::shm_open(kShmName, O_RDONLY, 0666);
    if (m_shmFd < 0)
    {
        KLOG_WARNING() << "FacePreview: shm_open failed, errno:" << errno;
        return false;
    }

    m_shmAddr = ::mmap(nullptr, shmSize, PROT_READ, MAP_SHARED, m_shmFd, 0);
    if (m_shmAddr == MAP_FAILED)
    {
        KLOG_WARNING() << "FacePreview: mmap failed, errno:" << errno;
        ::close(m_shmFd);
        m_shmFd = -1;
        return false;
    }

    m_shmSize = shmSize;
    KLOG_INFO() << "FacePreview: SHM connected, size:" << m_shmSize;
    return true;
}

void FacePreviewWidget::disconnectShm()
{
    m_refreshTimer->stop();

    if (m_shmAddr && m_shmAddr != MAP_FAILED)
    {
        ::munmap(m_shmAddr, m_shmSize);
        m_shmAddr = nullptr;
    }

    if (m_shmFd >= 0)
    {
        ::close(m_shmFd);
        m_shmFd = -1;
    }

    m_shmSize = 0;
    KLOG_INFO() << "FacePreview: SHM disconnected";
}

void FacePreviewWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    KLOG_INFO() << "FacePreview: showEvent -> connect SHM";

    if (connectShm())
    {
        m_refreshTimer->start(kRefreshIntervalMs);
    }
}

void FacePreviewWidget::hideEvent(QHideEvent *event)
{
    KLOG_INFO() << "FacePreview: hideEvent -> disconnect SHM";
    disconnectShm();
    QWidget::hideEvent(event);
}

void FacePreviewWidget::onRefreshTimer()
{
    if (!m_shmAddr || m_shmAddr == MAP_FAILED || m_shmSize == 0)
    {
        return;
    }

    // 读取 SHM 头部的 4 字节帧长度
    if (m_shmSize < 4)
    {
        return;
    }

    uint32_t frameLen = 0;
    memcpy(&frameLen, m_shmAddr, sizeof(frameLen));

    if (frameLen == 0 || frameLen > m_shmSize - 4)
    {
        return;
    }

    QByteArray jpegData(static_cast<const char *>(m_shmAddr) + 4, static_cast<int>(frameLen));

    if (!m_loggedFirstPayload)
    {
        m_loggedFirstPayload = true;
        KLOG_INFO() << "FacePreview: first SHM payload, bytes=" << jpegData.size();
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
