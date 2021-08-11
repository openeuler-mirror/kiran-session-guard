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
