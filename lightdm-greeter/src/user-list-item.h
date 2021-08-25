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

#ifndef USERLISTITEM_H
#define USERLISTITEM_H

#include <QListWidgetItem>
#include <QPaintEvent>
#include <QWidget>
#include "user-info.h"

#define USERITEM_OBJ_NAME "UserItem"
namespace Ui
{
class UserListItem;
}

class UserListItem : public QWidget
{
    Q_OBJECT
public:
    explicit UserListItem(QWidget *parent = nullptr);
    ~UserListItem();

public:
    void                   setUserInfo(const UserInfo &userInfo);
    void                   setListItem(const QListWidgetItem *item);
    const QListWidgetItem *getListItem();
    UserInfo               getUserInfo();

protected:
    void paintEvent(QPaintEvent *e);

private:
    Ui::UserListItem *     ui;
    UserInfo               m_userInfo;
    const QListWidgetItem *m_listItem;
};

#endif  // USERLISTITEM_H
