//
// Created by lxh on 2021/7/13.
//

#ifndef LIGHTDM_KIRAN_GREETER_SRC_FILTER_USER_PROXY_MODEL_H_
#define LIGHTDM_KIRAN_GREETER_SRC_FILTER_USER_PROXY_MODEL_H_

#include <QSortFilterProxyModel>

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

#endif  //LIGHTDM_KIRAN_GREETER_SRC_FILTER_USER_PROXY_MODEL_H_
