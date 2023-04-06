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

#include <QObject>
#include <QProcess>

namespace Kiran
{
namespace SessionGuard
{
class VirtualKeyboard : public QObject
{
    Q_OBJECT
public:
    explicit VirtualKeyboard(QObject *parent = nullptr);
    ~VirtualKeyboard();

    static VirtualKeyboard *instance();

    bool init(QWidget *parent = nullptr);
    void hide();
    bool isVisible();
    void showAdjustSize(QWidget *parent = nullptr);
    QWidget *getKeyboard();
    void keyboardProcessExit();

public slots:
    void slot_finished(int exitCode, QProcess::ExitStatus exitStatus);

private slots:
    void slotReadyReadStandardOutput();

private:
    QWidget *m_keyboardWidget;
    QProcess *m_process;
    QWidget *m_keyboardEmbed = nullptr;
};
}  // namespace SessionGuard
}  // namespace Kiran