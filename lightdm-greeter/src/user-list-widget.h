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

#ifndef USERLISTWIDGET_H
#define USERLISTWIDGET_H

#include <QLightDM/UsersModel>
#include <QWidget>
#include "filter-user-proxy-model.h"
#include "user-info.h"

class QListWidgetItem;

namespace Ui
{
class UserListWidget;
}

class UserListWidget : public QWidget
{
    Q_OBJECT
public:
    explicit UserListWidget(QWidget *parent = nullptr);
    ~UserListWidget();
    void    loadUserList();
    bool    getCurrentSelected(UserInfo &userInfo);
    void    setRow0();
    void    JustForTest(int count);
    QString getIconByUserName(const QString &userName);
    bool    eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;
    int     userCount();

private:
    void initUI();
    bool getUserInfoFromModel(int row, UserInfo &userInfo);
    void appendItem(const UserInfo &userInfo);
    void insertItem(int row, const UserInfo &userInfo);
    void setCurrentRow(int idx);

private slots:
    void slotUserItemActivated();
    void slotRowsRemoved(const QModelIndex &parent, int first, int last);
    void slotRowsInserted(const QModelIndex &parent, int first, int last);

Q_SIGNALS:
    void userActivated(const UserInfo &userInfo);
    ///请求登录窗口重置窗口
    void sigRequestResetUI();

protected:
    virtual QSize sizeHint() const override;

private:
    Ui::UserListWidget * ui;
    QLightDM::UsersModel m_usersModel;
    FilterUserProxyModel m_filterModel;
};

#endif  // USERLISTWIDGET_H
