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
#pragma once

#include <QDBusConnection>
#include <QTimer>
#include <QWidget>

QT_BEGIN_NAMESPACE
class QLabel;
class QShowEvent;
class QHideEvent;
QT_END_NAMESPACE

namespace Kiran
{
namespace SessionGuard
{

/**
 * @brief 人脸预览控件
 *
 * 从 ks-auth-dbus-service 的 POSIX 共享内存 `/ks_auth_preview`
 * 读取 JPEG 帧，渲染为预览画面。
 *
 * 原实现通过 D-Bus VideoInfo 信号接收帧，现改为通过 SHM 轮询。
 */
class FacePreviewWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FacePreviewWidget(QWidget *parent = nullptr);
    ~FacePreviewWidget() override;

    /**
     * @brief 判断 kiran 人脸 D-Bus 服务是否可用
     * @return 若 com.ks.auth.dbus.service 已在 system bus 注册返回 true
     */
    static bool isFaceDaemonAvailable();

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private slots:
    /** SHM 定时刷新，读取共享内存中的 JPEG 帧并渲染 */
    void onRefreshTimer();
    /** 摄像头热插拔可用性变化 */
    void onCameraAvailabilityChanged(bool available);
    /** ks-auth-dbus-service 在 system bus 上注册/注销时重连 SHM */
    void onFaceServiceOwnerChanged(const QString &name,
                                   const QString &oldOwner,
                                   const QString &newOwner);

private:
    /** 订阅 face 服务 D-Bus 信号（服务重启后需重新 connect） */
    void connectFaceServiceSignals();
    /**
     * @brief 打开 POSIX 共享内存，通过 D-Bus ControlStreamNode 获取 SHM 信息
     * @return 成功返回 true
     */
    bool connectShm();
    /** 关闭共享内存映射并停止 SHM 流 */
    void disconnectShm();
    /** 仅释放本地 mmap/fd，不调用 ControlStreamNode stop */
    void unmapShmLocal();
    /**
     * @brief 以只读方式打开并映射 SHM（会先释放旧映射）
     * @param[in] shmSize SHM 总大小（字节）
     * @return 成功返回 true
     */
    bool mapShmReadOnly(size_t shmSize);
    /** 清空预览区，移除最后一帧画面 */
    void clearPreview();

    /**
     * @brief 通过 D-Bus 调用 ControlStreamNode 启动或停止 SHM 预览流
     * @param enable true 启动流，false 停止流
     * @return SHM 大小（字节），失败返回 0
     */
    size_t callControlStreamNode(bool enable);

    QLabel *m_label = nullptr;

    QTimer *m_refreshTimer = nullptr;

    /** 共享内存文件描述符 */
    int m_shmFd = -1;
    /** 共享内存映射地址 */
    void *m_shmAddr = nullptr;
    /** 共享内存大小（字节） */
    size_t m_shmSize = 0;

    /** 业务标识，用于 D-Bus 调用 ControlStreamNode */
    QString m_businessId = QStringLiteral("KylinsecOS");

    /** 是否已记录 SHM 保留流状态（code=7，流已在运行） */
    bool m_loggedShmAlreadyStreaming = false;
    /** 摄像头不可用时标记，待热插拔恢复后自动重连 */
    bool m_cameraUnavailable = false;
    /** face 服务重启后待重连 SHM（控件当前不可见时置位） */
    bool m_serviceReconnectPending = false;
};

}  // namespace SessionGuard
}  // namespace Kiran
