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

#include "filter-user-proxy-model.h"

namespace Kiran
{
namespace SessionGuard
{
namespace Greeter
{
FilterUserProxyModel::FilterUserProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

FilterUserProxyModel::~FilterUserProxyModel()
{
}

void FilterUserProxyModel::setFilterUsers(QStringList users)
{
    m_filterUsers = users;
    invalidateFilter();
}

bool FilterUserProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    int role = filterRole();
    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
    QVariant var = sourceModel()->data(index, role);
    QString str = var.toString();
    if (m_filterUsers.contains(str))
    {
        return false;
    }
    else
    {
        return true;
    }
}
}  // namespace Greeter
}  // namespace SessionGuard
}  // namespace Kiran