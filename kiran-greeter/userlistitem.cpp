#include "userlistitem.h"
#include "ui_userlistitem.h"
#include "userinfo.h"

UserListItem::UserListItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UserListItem)
{
    ui->setupUi(this);
}

UserListItem::~UserListItem()
{
    delete ui;
}

void UserListItem::setUserInfo(const UserInfo &userInfo)
{
    m_userInfo = userInfo;
    ui->avatar->setImage(userInfo.imagePath);
    ui->label_username->setText(userInfo.name);
}

UserInfo UserListItem::getUserInfo()
{
    return m_userInfo;
}
