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

#ifndef GREETERMENUITEM_H
#define GREETERMENUITEM_H

#include <QWidget>

class QButtonGroup;
class QLabel;
class QCheckBox;
class QEvent;

class GreeterMenuItem : public QWidget
{
    Q_OBJECT
public:
    explicit GreeterMenuItem(const QString &text, bool checkable, QWidget *parent = nullptr);
    void    setExclusiveGroup(QButtonGroup *group);
    QString getActionName();
    void    setChecked(bool isChecked);
signals:
    void sigChecked(QString action);
public slots:
private:
    void initUI();

protected:
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;

private:
    bool       m_checkable;
    QString    m_actionName;
    QLabel *   m_label;
    QCheckBox *m_checkbox;
};

#endif  // GREETERMENUITEM_H
