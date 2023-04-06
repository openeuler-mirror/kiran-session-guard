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
#include <QSortFilterProxyModel>


namespace Kiran
{
namespace SessionGuard
{
namespace Greeter
{
class FilterUserProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit FilterUserProxyModel(QObject *parent = nullptr);
    ~FilterUserProxyModel();

    void setFilterUsers(QStringList users);

protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    QStringList m_filterUsers;
};
}  // namespace Greeter
}  // namespace SessionGuard
}  // namespace Kiran
