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

#include <QLightDM/UsersModel>
#include <QWidget>
#include "filter-user-proxy-model.h"
#include "user-info.h"

QT_BEGIN_NAMESPACE
class QListWidgetItem;
QT_END_NAMESPACE

namespace Ui
{
class UserList;
}

namespace Kiran
{
namespace SessionGuard
{
namespace Greeter
{
class UserList : public QWidget
{
    Q_OBJECT
public:
    explicit UserList(QWidget *parent = nullptr);
    ~UserList();

    void loadUserList();
    bool getCurrentSelected(UserInfo &userInfo);

    void setRow0();
    QString getIconByUserName(const QString &userName);
    UserInfo getUserInfoByUserName(const QString &userName);
    int userCount();

    bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;

private:
    void initUI();
    bool getUserInfoFromModel(int row, UserInfo &userInfo);
    void appendItem(const UserInfo &userInfo);
    void insertItem(int row, const UserInfo &userInfo);
    void setCurrentRow(int idx);

private slots:
    void onUserItemActivated();
    void onModelRowsRemoved(const QModelIndex &parent, int first, int last);
    void onModelRowsInserted(const QModelIndex &parent, int first, int last);
    void onAppFocusChanged(QWidget* oldFocus,QWidget* newFocus);
    void onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> roles);

signals:
    void userActivated(const UserInfo &userInfo);
    void userCountChanged(int oldCount, int newCount);
    void userRemoved(const QString &userName);

protected:
    virtual QSize sizeHint() const override;

private:
    Ui::UserList *ui;
    QLightDM::UsersModel m_usersModel;
    FilterUserProxyModel m_filterModel;
};
}  // namespace Greeter
}  // namespace SessionGuard
}  // namespace Kiran