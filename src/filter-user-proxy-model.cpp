//
// Created by lxh on 2021/7/13.
//

#include "filter-user-proxy-model.h"

FilterUserProxyModel::FilterUserProxyModel(QObject *parent)
    :QSortFilterProxyModel(parent)
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
    QVariant var = sourceModel()->data(index,role);
    QString str = var.toString();
    if( m_filterUsers.contains(str) ){
        return false;
    }else{
        return true;
    }
}
