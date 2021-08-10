 /**
  * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd.
  *
  * Author:     liuxinhao <liuxinhao@kylinos.com.cn>
  *
  * This program is free software; you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation; either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program; If not, see <http: //www.gnu.org/licenses/>. 
  */
 
#pragma once

#include <QObject>
#include <QProcess>

class VirtualKeyboard : public QObject
{
    Q_OBJECT
public:
    static VirtualKeyboard *instance();
    ~VirtualKeyboard();

    bool     init(QWidget *parent = nullptr);
    void     hide();
    bool     isVisible();
    void     showAdjustSize(QWidget *parent = nullptr);
    QWidget *getKeyboard();
    void     keyboardProcessExit();

public slots:
    void slot_finished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    explicit VirtualKeyboard(QObject *parent = nullptr);

private slots:
    void slotReadyReadStandardOutput();

private:
    QWidget * m_keyboardWidget;
    QProcess *m_process;
};
