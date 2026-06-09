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
 * 从 kiran-face-dbus-service 的 POSIX 共享内存 `/kiran_face_preview`
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
     * @return 若 com.kiran.face.service 已在 system bus 注册返回 true
     */
    static bool isFaceDaemonAvailable();

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private slots:
    /** SHM 定时刷新，读取共享内存中的 JPEG 帧并渲染 */
    void onRefreshTimer();

private:
    /** 打开 POSIX 共享内存，key 来自 /tmp/kiran_face_preview_shm_key */
    bool connectShm();
    /** 关闭共享内存映射 */
    void disconnectShm();

    QLabel *m_label = nullptr;

    QTimer *m_refreshTimer = nullptr;

    /** 共享内存文件描述符 */
    int m_shmFd = -1;
    /** 共享内存映射地址 */
    void *m_shmAddr = nullptr;
    /** 共享内存大小（字节） */
    size_t m_shmSize = 0;

    bool m_loggedFirstPayload = false;
    bool m_loggedFirstDecodeFail = false;
};

}  // namespace SessionGuard
}  // namespace Kiran
