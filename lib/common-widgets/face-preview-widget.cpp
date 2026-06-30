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
#include <QDBusMessage>
#include <QDateTime>
#include <QHideEvent>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
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
/** ks-auth-dbus-service 的 D-Bus 服务名 */
const QString kFaceService = QStringLiteral("com.ks.auth.dbus.service");
/** ks-auth-dbus-service 的 D-Bus 对象路径 */
const QString kFacePath = QStringLiteral("/com/ks/auth/dbus/service");
/** SHM 名称 */
const char *kShmName = "/ks_auth_preview";
/** D-Bus 调用超时（毫秒） */
constexpr int kDbusTimeoutMs = 1000;
/** 预览最大宽度 */
constexpr int kPreviewMaxW = 200;
/** 预览最大高度 */
constexpr int kPreviewMaxH = 150;
/** SHM 刷新间隔（ms），约 15 fps */
constexpr int kRefreshIntervalMs = 67;
/** SHM 帧头部大小（字节），对应 KiranShmPreviewHeader（magic+version+flags+sequence+payload_size+reserved0+reserved1=24B） */
constexpr size_t kShmHeaderSize = 24;
/** SHM 魔数 "KIRN" */
constexpr uint32_t kShmMagic = 0x4E52494B;  // "KIRN" 小端序
/** SHM 有效帧标志位 */
constexpr uint16_t kShmFlagValid = 0x0001;
/** SHM 头部中 payload_size 字段的偏移量（字节） */
constexpr size_t kShmPayloadSizeOffset = 12;
/** SHM 头部中 flags 字段的偏移量（字节） */
constexpr size_t kShmFlagsOffset = 6;

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

    /* 监听摄像头热插拔信号，摄像头恢复后自动重连 SHM 预览 */
    auto bus = selectBus();
    bus.connect(kFaceService, kFacePath, kFaceService,
                QStringLiteral("CameraAvailabilityChanged"),
                this,
                SLOT(onCameraAvailabilityChanged(bool)));
}

FacePreviewWidget::~FacePreviewWidget()
{
    disconnectShm();
}

bool FacePreviewWidget::connectShm()
{
    /* 始终调用 ControlStreamNode 确保服务端推流存活；
     * 仅在 SHM fd 无效时才打开/映射，避免每次 showEvent 都重复 shm_open+mmap。 */
    size_t shmSize = callControlStreamNode(true);
    if (shmSize == 0)
    {
        KLOG_WARNING() << "FacePreview: failed to start SHM preview via D-Bus";
        return false;
    }

    if (m_shmFd >= 0 && m_shmAddr && m_shmAddr != MAP_FAILED)
    {
        /* 已有有效映射：只确认服务端已恢复推流，不重复打开 SHM */
        KLOG_INFO() << "FacePreview: SHM already mapped, server stream ensured";
        return true;
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
    m_loggedShmAlreadyStreaming = false;
    m_cameraUnavailable = false;
    KLOG_INFO() << "FacePreview: SHM disconnected";

    callControlStreamNode(false);
}

size_t FacePreviewWidget::callControlStreamNode(bool enable)
{
    QJsonObject params;
    params[QStringLiteral("business_id")] = m_businessId;
    params[QStringLiteral("enable")] = enable;

    QJsonDocument doc(params);
    QString json = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));

    QDBusMessage msg = QDBusMessage::createMethodCall(
        kFaceService, kFacePath, kFaceService,
        QStringLiteral("ControlStreamNode"));
    msg << json;

    auto bus = selectBus();
    QDBusMessage reply = bus.call(msg, QDBus::Block, kDbusTimeoutMs);

    if (reply.type() != QDBusMessage::ReplyMessage)
    {
        KLOG_WARNING() << "FacePreview: ControlStreamNode D-Bus call failed, error="
                       << reply.errorMessage();
        return 0;
    }

    if (reply.arguments().isEmpty())
    {
        KLOG_WARNING() << "FacePreview: ControlStreamNode returned empty reply";
        return 0;
    }

    QString replyJson = reply.arguments().first().toString();
    QJsonParseError parseError;
    QJsonDocument replyDoc = QJsonDocument::fromJson(replyJson.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError)
    {
        KLOG_WARNING() << "FacePreview: ControlStreamNode reply JSON parse error:"
                       << parseError.errorString();
        return 0;
    }

    QJsonObject replyObj = replyDoc.object();
    int code = replyObj[QStringLiteral("code")].toInt(-1);

    if (!enable)
    {
        if (code != 0)
        {
            KLOG_WARNING() << "FacePreview: ControlStreamNode stop failed, code=" << code
                           << "error=" << replyObj[QStringLiteral("error_msg")].toString();
        }
        else
        {
            KLOG_INFO() << "FacePreview: ControlStreamNode SHM streaming stopped";
        }
        return 0;
    }

    if (code == 0)
    {
        // 正常启动成功，从返回值获取 SHM 大小
        m_cameraUnavailable = false;
        size_t shmSize = static_cast<size_t>(
            replyObj[QStringLiteral("shm_size")].toDouble(0));
        QString shmName = replyObj[QStringLiteral("shm_name")].toString();
        if (shmSize == 0)
        {
            KLOG_WARNING() << "FacePreview: ControlStreamNode returned zero shm_size";
            return 0;
        }
        KLOG_INFO() << "FacePreview: ControlStreamNode SHM streaming started, name="
                    << shmName << "size=" << shmSize;
        return shmSize;
    }

    if (code == 9)
    {
        // 摄像头不可用：标记待恢复，收到 CameraAvailabilityChanged(true) 后自动重连
        m_cameraUnavailable = true;
        KLOG_WARNING() << "FacePreview: ControlStreamNode camera unavailable, will retry on hotplug";
        return 0;
    }

    if (code == 7)
    {
        // SHM 已在流式传输中，尝试直接打开
        if (!m_loggedShmAlreadyStreaming)
        {
            m_loggedShmAlreadyStreaming = true;
            KLOG_INFO() << "FacePreview: SHM already streaming, try to use existing SHM";
        }

        int fd = ::shm_open(kShmName, O_RDONLY, 0666);
        if (fd < 0)
        {
            KLOG_WARNING() << "FacePreview: existing SHM shm_open failed, errno=" << errno;
            return 0;
        }

        struct stat st;
        if (::fstat(fd, &st) != 0)
        {
            KLOG_WARNING() << "FacePreview: existing SHM fstat failed, errno=" << errno;
            ::close(fd);
            return 0;
        }

        ::close(fd);
        KLOG_INFO() << "FacePreview: using existing SHM, size=" << st.st_size;
        return static_cast<size_t>(st.st_size);
    }

    KLOG_WARNING() << "FacePreview: ControlStreamNode unexpected code=" << code
                   << "error=" << replyObj[QStringLiteral("error_msg")].toString();
    return 0;
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

    if (m_shmSize < kShmHeaderSize)
    {
        return;
    }

    auto *data = static_cast<const char *>(m_shmAddr);

    // 校验魔数
    uint32_t magic = 0;
    memcpy(&magic, data, sizeof(magic));
    if (magic != kShmMagic)
    {
        return;
    }

    // 校验有效帧标志
    uint16_t flags = 0;
    memcpy(&flags, data + kShmFlagsOffset, sizeof(flags));
    if (!(flags & kShmFlagValid))
    {
        return;
    }

    // 读取 payload_size（offset 12，uint32_t 小端）
    uint32_t frameLen = 0;
    memcpy(&frameLen, data + kShmPayloadSizeOffset, sizeof(frameLen));

    if (frameLen == 0 || frameLen > m_shmSize - kShmHeaderSize)
    {
        return;
    }

    QByteArray jpegData(data + kShmHeaderSize, static_cast<int>(frameLen));

    QPixmap pix;
    if (!pix.loadFromData(jpegData))
    {
        return;
    }
    m_label->setPixmap(
        pix.scaled(kPreviewMaxW, kPreviewMaxH, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void FacePreviewWidget::onCameraAvailabilityChanged(bool available)
{
    KLOG_INFO() << "FacePreview: CameraAvailabilityChanged available=" << available;

    if (!available)
    {
        m_cameraUnavailable = true;
        return;
    }

    /* 摄像头已恢复：若有挂起的 SHM 请求且控件可见，自动重连 */
    if (!m_cameraUnavailable)
    {
        return;
    }

    if (!isVisible())
    {
        KLOG_INFO() << "FacePreview: camera recovered but widget hidden, skip retry";
        return;
    }

    KLOG_INFO() << "FacePreview: camera recovered, retrying SHM connect";
    if (connectShm())
    {
        m_cameraUnavailable = false;
        m_refreshTimer->start(kRefreshIntervalMs);
        KLOG_INFO() << "FacePreview: SHM reconnected after camera hotplug";
    }
}

}  // namespace SessionGuard
}  // namespace Kiran
