#include "userlistitem.h"
#include "ui_userlistitem.h"
#include "userinfo.h"
#include <QPainter>

UserListItem::UserListItem (QWidget *parent) :
        QWidget(parent),
        ui(new Ui::UserListItem)
{
    ui->setupUi(this);
    setFocusPolicy(Qt::NoFocus);
    setObjectName(USERITEM_OBJ_NAME);
}

UserListItem::~UserListItem ()
{
    delete ui;
}

void UserListItem::setUserInfo (const UserInfo &userInfo)
{
    m_userInfo = userInfo;
    ui->avatar->setImage(userInfo.imagePath);
    ui->label_username->setText(userInfo.name);
    if (userInfo.loggedIn)
    {
        ui->label_logined->setPixmap(QPixmap(":/images/checked.png"));
    }
}

void UserListItem::setListItem (const QListWidgetItem *item)
{
    m_listItem = item;
}

const QListWidgetItem *UserListItem::getListItem ()
{
    return m_listItem;
}

UserInfo UserListItem::getUserInfo ()
{
    return m_userInfo;
}

void UserListItem::paintEvent (QPaintEvent *e)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    p.setRenderHint(QPainter::HighQualityAntialiasing);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    QWidget::paintEvent(e);
}
