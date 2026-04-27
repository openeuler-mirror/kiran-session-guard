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

// 仅连接 com.czht.face.daemon 的 VideoInfo(JPEG)，并固定走 system bus。
class FacePreviewWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FacePreviewWidget(QWidget* parent = nullptr);
    ~FacePreviewWidget() override;

    // 判断人脸 D-Bus 服务是否存在（system bus 上 com.czht.face.daemon）。
    static bool isFaceDaemonAvailable();

protected:
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;

private slots:
    void onVideoInfo(const QByteArray& jpegData);

private:
    void connectVideoInfo();
    void disconnectVideoInfo();

    QLabel* m_label = nullptr;
    QDBusConnection* m_signalBus = nullptr;
    bool m_connected = false;
    bool m_connectFailedLogged = false;
    bool m_loggedFirstPayload = false;
    bool m_loggedFirstDecodeFail = false;
};
}  // namespace SessionGuard
}  // namespace Kiran
