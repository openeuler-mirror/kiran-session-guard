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

#include "user-item.h"
#include <QPainter>
#include "ui_user-item.h"
#include "user-info.h"

namespace Kiran
{
namespace SessionGuard
{
namespace Greeter
{
UserItem::UserItem(QWidget *parent) : QWidget(parent),
                                      ui(new Ui::UserItem)
{
    ui->setupUi(this);
    setFocusPolicy(Qt::NoFocus);
    setObjectName(USERITEM_OBJ_NAME);
}

UserItem::~UserItem()
{
    delete ui;
}

void UserItem::setUserInfo(const UserInfo &userInfo)
{
    m_userInfo = userInfo;
    ui->avatar->setImage(userInfo.imagePath);
    ui->label_username->setText(userInfo.name);
    if (userInfo.loggedIn)
    {
        ui->label_logined->setPixmap(QPixmap(":/greeter/images/checked.png"));
    }
}

void UserItem::setListItem(const QListWidgetItem *item)
{
    m_listItem = item;
}

const QListWidgetItem *UserItem::getListItem()
{
    return m_listItem;
}

UserInfo UserItem::getUserInfo()
{
    return m_userInfo;
}

void UserItem::paintEvent(QPaintEvent *e)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    p.setRenderHint(QPainter::HighQualityAntialiasing);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    QWidget::paintEvent(e);
}
}  // namespace Greeter
}  // namespace SessionGuard
}  // namespace Kiran