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

#include "disabledeselectlistwidget.h"
#include <QDebug>
#include <QEvent>
#include <QMouseEvent>

DisableDeselectListWidget::DisableDeselectListWidget(QWidget *parent)
    : QListWidget(parent)
{
}

QItemSelectionModel::SelectionFlags
DisableDeselectListWidget::selectionCommand(const QModelIndex &index, const QEvent *event) const
{
    //FIXME:为了避免一些环境下会出现ItemFocus直接设置选中的情况
    if (event == nullptr)
    {
        return QItemSelectionModel::NoUpdate;
    }
    //禁用用户按住鼠标切换用户情况
    if (event->type() == QEvent::MouseMove)
    {
        return QItemSelectionModel::NoUpdate;
    }
    if ((event != nullptr) && (event->type() == QEvent::MouseButtonPress))
    {
        const QMouseEvent *mouseEvent = (QMouseEvent *)event;
        if ((mouseEvent->modifiers() & Qt::ControlModifier) != 0)
        {
            return QItemSelectionModel::NoUpdate;
        }
    }
    return QListWidget::selectionCommand(index, event);
}
