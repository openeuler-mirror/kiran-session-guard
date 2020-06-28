#ifndef USERLISTITEM_H
#define USERLISTITEM_H

#include <QWidget>
#include "userinfo.h"
namespace Ui {
class UserListItem;
}

class UserListItem : public QWidget
{
    Q_OBJECT

public:
    explicit UserListItem(QWidget *parent = nullptr);
    ~UserListItem();
public:
    void setUserInfo(const UserInfo& userInfo);
    UserInfo getUserInfo();
private:
    Ui::UserListItem *ui;
    UserInfo m_userInfo;
};

#endif // USERLISTITEM_H
