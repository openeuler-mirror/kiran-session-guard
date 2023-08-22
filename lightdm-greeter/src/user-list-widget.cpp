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

#include "user-list-widget.h"
#include <qt5-log-i.h>
#include <QDebug>
#include <QList>
#include <QMap>
#include <QModelIndex>
#include <QScrollBar>

#include "kiran-greeter-prefs.h"
#include "ui_user-list-widget.h"
#include "user-info.h"
#include "user-list-item.h"

using namespace QLightDM;

UserListWidget::UserListWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::UserListWidget)
{
    ui->setupUi(this);
    initUI();
}

UserListWidget::~UserListWidget()
{
    delete ui;
}

bool UserListWidget::eventFilter(QObject *obj, QEvent *event)
{
    /// 因QListWidget的上下键直接修改当前行，通过屏蔽上下键盘事件修改成切换焦点
    if (obj == ui->userList)
    {
        switch (event->type())
        {
        case QEvent::KeyPress:
        {
            QKeyEvent *keyEvent = dynamic_cast<QKeyEvent *>(event);
            if (qApp->focusWidget() == nullptr ||
                qApp->focusWidget()->objectName() != USERITEM_OBJ_NAME)
            {
                break;
            }
            UserListItem *userItem = dynamic_cast<UserListItem *>(qApp->focusWidget());
            const QListWidgetItem *listItem = userItem->getListItem();
            int rowIdx = ui->userList->row(listItem);
            int rowCount = ui->userList->count();
            if (keyEvent->key() == Qt::Key_Up)
            {
                if ((rowIdx != 0) && (rowCount > 1))
                {
                    ui->userList->itemWidget(ui->userList->item(rowIdx - 1))->setFocus(Qt::TabFocusReason);
                }
                return true;
            }
            else if (keyEvent->key() == Qt::Key_Down)
            {
                if ((rowIdx < (rowCount - 1)))
                {
                    ui->userList->itemWidget(ui->userList->item(rowIdx + 1))->setFocus(Qt::TabFocusReason);
                }
                return true;
            }
            break;
        }
        default:
            break;
        }
    }

    /// 聚焦在Item上按Tab直接跳出，不在用户列表上切换
    if (obj != nullptr && obj->objectName() == USERITEM_OBJ_NAME)
    {
        switch (event->type())
        {
        case QEvent::KeyPress:
        {
            QKeyEvent *keyEvent = dynamic_cast<QKeyEvent *>(event);
            if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return)
            {
                UserListItem *item = dynamic_cast<UserListItem *>(qApp->focusWidget());
                const QListWidgetItem *listItem = item->getListItem();
                int rowIdx = ui->userList->row(listItem);
                setCurrentRow(rowIdx);
                return true;
            }
            else if (keyEvent->key() == Qt::Key_Space)
            {
                return true;
            }
            break;
        }
        default:
            break;
        }
    }

    return false;
}

int UserListWidget::userCount()
{
    return ui->userList->count();
}

void UserListWidget::initUI()
{
    setAttribute(Qt::WA_Hover, true);
    ui->userList->setFocusPolicy(Qt::ClickFocus);
    ui->userList->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->userList->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(ui->userList, &DisableDeselectListWidget::itemSelectionChanged,
            this, &UserListWidget::slotUserItemActivated);

    ui->userList->setVisible(false);
    ui->userList->installEventFilter(this);

    /// 连接QApplication的焦点切换信号
    /// 处理ListWidget内部焦点切换或焦点切换出ListWidge，滑动条特殊处理
    /// 处理当焦点从外部到UserItem时，应默认到当前行
    connect(qApp, &QApplication::focusChanged, [this](QWidget *oldWidget, QWidget *newWidget)
            {
        bool oldFocusInList = oldWidget == nullptr ? false : oldWidget->objectName() == USERITEM_OBJ_NAME;
        bool newFocusInList = newWidget == nullptr ? false : newWidget->objectName() == USERITEM_OBJ_NAME;
        if (!oldFocusInList && !newFocusInList)
        {
            return;
        }
        else if (newFocusInList)
        {  ///UserItem->UserItem,滚动到焦点行
            UserListItem *userItem = dynamic_cast<UserListItem *>(newWidget);
            const QListWidgetItem *listItem = userItem->getListItem();
            ui->userList->scrollToItem(listItem);
        }
        else if (oldFocusInList)
        {  ///UserItem->外部，滚动到当前行
            ui->userList->scrollToItem(ui->userList->currentItem());
        } });
}

void UserListWidget::loadUserList()
{
    QStringList hiddenUsers = KiranGreeterPrefs::instance()->hiddenUsers();
    m_filterModel.setSourceModel(&m_usersModel);
    m_filterModel.setFilterRole(UsersModel::NameRole);
    m_filterModel.setFilterUsers(hiddenUsers);

    for (int i = 0; i < m_filterModel.rowCount(QModelIndex()); i++)
    {
        UserInfo userInfo;
        getUserInfoFromModel(i, userInfo);
        if (hiddenUsers.contains(userInfo.name))
        {
            continue;
        }
        appendItem(userInfo);
    }

    KLOG_INFO() << "connect UserModel RowRemoved:  "
                << connect(&m_filterModel, &QLightDM::UsersModel::rowsRemoved, this, &UserListWidget::slotRowsRemoved);
    KLOG_INFO() << "connect UserModel RowInserted: "
                << connect(&m_filterModel, &QLightDM::UsersModel::rowsInserted, this, &UserListWidget::slotRowsInserted);
    connect(&m_filterModel, &QLightDM::UsersModel::dataChanged, this, &UserListWidget::slotDataChanged);
}

bool UserListWidget::getCurrentSelected(UserInfo &userInfo)
{
    QList<QListWidgetItem *> selectedItem = ui->userList->selectedItems();
    if (selectedItem.size() == 0)
    {
        return false;
    }
    UserListItem *item = dynamic_cast<UserListItem *>(ui->userList->itemWidget(selectedItem.at(0)));
    UserInfo info = item->getUserInfo();
    userInfo = info;
    return true;
}

void UserListWidget::setRow0()
{
    setCurrentRow(0);
}

bool UserListWidget::getUserInfoFromModel(int row, UserInfo &userInfo)
{
    QVariant value;

    if (m_filterModel.rowCount(QModelIndex()) < row)
    {
        return false;
    }

    value = m_filterModel.data(m_filterModel.index(row, 0), UsersModel::NameRole);
    userInfo.name = value.toString();

    value = m_filterModel.data(m_filterModel.index(row, 0), UsersModel::RealNameRole);
    userInfo.realName = value.toString();

    value = m_filterModel.data(m_filterModel.index(row, 0), UsersModel::LoggedInRole);
    userInfo.loggedIn = value.toBool();

    value = m_filterModel.data(m_filterModel.index(row, 0), UsersModel::SessionRole);
    userInfo.session = value.toString();

    value = m_filterModel.data(m_filterModel.index(row, 0), UsersModel::HasMessagesRole);
    userInfo.hasMessage = value.toBool();

    value = m_filterModel.data(m_filterModel.index(row, 0), UsersModel::ImagePathRole);
    userInfo.imagePath = value.toString();

    value = m_filterModel.data(m_filterModel.index(row, 0), UsersModel::BackgroundPathRole);
    userInfo.backgroundPath = value.toString();

    value = m_filterModel.data(m_filterModel.index(row, 0), UsersModel::UidRole);
    userInfo.uid = value.toULongLong();
    KLOG_INFO() << "getUserInfoFromModel: " << userInfo.name;
    return true;
}

void UserListWidget::appendItem(const UserInfo &userInfo)
{
    QListWidgetItem *newItem = nullptr;
    UserListItem *customItem = nullptr;

    newItem = new QListWidgetItem;

    customItem = new UserListItem;
    customItem->setUserInfo(userInfo);
    customItem->setListItem(newItem);
    customItem->installEventFilter(this);

    ui->userList->addItem(newItem);
    ui->userList->setItemWidget(newItem, customItem);
    if ((!ui->userList->isVisible()) && (ui->userList->count() >= 2))
    {
        ui->userList->setVisible(true);
    }
}

void UserListWidget::insertItem(int row, const UserInfo &userInfo)
{
    QListWidgetItem *newItem = nullptr;
    UserListItem *customItem = nullptr;

    newItem = new QListWidgetItem;

    customItem = new UserListItem;
    customItem->setUserInfo(userInfo);
    customItem->setListItem(newItem);
    customItem->installEventFilter(this);

    ui->userList->insertItem(row, newItem);
    ui->userList->setItemWidget(newItem, customItem);
    if ((!ui->userList->isVisible()) && (ui->userList->count() >= 2))
    {
        ui->userList->setVisible(true);
    }
}

void UserListWidget::JustForTest(int count)
{
    for (int i = 0; i < count; i++)
    {
        QString testUserName = QString("TestUser%1").arg(i);
        UserInfo userInfo;
        userInfo.name = testUserName;
        appendItem(userInfo);
    }
}

QString UserListWidget::getIconByUserName(const QString &userName)
{
    QString iconPath = "";

    for (int i = 0; i < m_filterModel.rowCount(QModelIndex()); i++)
    {
        UserInfo userInfo;
        getUserInfoFromModel(i, userInfo);
        if (userInfo.name == userName)
        {
            iconPath = userInfo.imagePath;
            break;
        }
    }

    return iconPath;
}

UserInfo UserListWidget::getUserInfoByUserName(const QString &userName)
{
    UserInfo info;

    for (int i = 0; i < m_filterModel.rowCount(QModelIndex()); i++)
    {
        UserInfo userInfo;
        getUserInfoFromModel(i, userInfo);
        if (userInfo.name == userName)
        {
            info = userInfo;
            break;
        }
    }

    return info;
}

void UserListWidget::setCurrentRow(int idx)
{
    if (ui->userList->count() > idx)
    {
        ui->userList->setCurrentRow(idx, QItemSelectionModel::ClearAndSelect);
    }
}

void UserListWidget::slotUserItemActivated()
{
    QList<QListWidgetItem *> selectedItems = ui->userList->selectedItems();
    if (selectedItems.size() == 0)
    {
        KLOG_WARNING() << "selected items: 0";
        return;
    }
    QListWidgetItem *activatedItem = selectedItems.at(0);
    UserListItem *userItem = dynamic_cast<UserListItem *>(ui->userList->itemWidget(activatedItem));

    userItem->setFocusPolicy(Qt::TabFocus);
    for (int i = 0; i < ui->userList->count(); i++)
    {
        QListWidgetItem *item = ui->userList->item(i);
        if (item != activatedItem)
        {
            UserListItem *uItem = dynamic_cast<UserListItem *>(ui->userList->itemWidget(item));
            uItem->setFocusPolicy(Qt::NoFocus);
        }
    }
    KLOG_INFO() << userItem->getUserInfo().name << "activate";
    UserInfo info = userItem->getUserInfo();
    emit userActivated(info);
}

void UserListWidget::slotRowsRemoved(const QModelIndex &parent, int first, int last)
{
    bool reSelect = false;

    for (int i = last; (i >= first); i--)
    {
        QListWidgetItem *item = ui->userList->item(i);
        QList<QListWidgetItem *> selectedItems = ui->userList->selectedItems();
        if (isEnabled() && (selectedItems.size() > 0) && (selectedItems.at(0) == item))
        {
            reSelect = true;
        }
        QListWidgetItem *removedItem = ui->userList->takeItem(i);
        delete removedItem;
    }
    if (reSelect)
    {
        if (ui->userList->count() > 0)
        {
            setRow0();
        }
        else
        {
            emit sigRequestResetUI();
        }
    }
    if ((ui->userList->isVisible()) && (ui->userList->count() < 2))
    {
        ui->userList->setVisible(false);
    }
    updateGeometry();
}

void UserListWidget::slotDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,const QVector<int> roles)
{
    auto userInfoUpdateFunc = [this](const UserInfo &newInfo) -> void
    {
        for (int i = 0; i < ui->userList->count(); i++)
        {
            auto item = ui->userList->item(i);
            auto userItem = dynamic_cast<UserListItem *>(ui->userList->itemWidget(item));
            auto itemUserInfo = userItem->getUserInfo();
            if (newInfo.name != itemUserInfo.name)
            {
                continue;
            }

            if (itemUserInfo.imagePath != newInfo.imagePath ||
                itemUserInfo.loggedIn != newInfo.loggedIn)
            {
                itemUserInfo.imagePath = newInfo.imagePath;
                itemUserInfo.loggedIn = newInfo.loggedIn;
                userItem->setUserInfo(itemUserInfo);
            }
            break;
        }
    };

    // FIXME: QLightdDM此处信号发出时roles为默认参数,无法判断数据变化范围,只能通过遍历检查数据是否变更
    // 检查图片更新,用户列表中的顺序可能和用户不一致
    int startRow = topLeft.row();
    int endRow = topLeft.row();
    for (int i = startRow; i <= endRow; i++)
    {
        UserInfo userInfo;
        getUserInfoFromModel(i, userInfo);
        userInfoUpdateFunc(userInfo);
    }
}

void UserListWidget::slotRowsInserted(const QModelIndex &parent, int first, int last)
{
    for (int i = first; i <= last; i++)
    {
        UserInfo userInfo;
        getUserInfoFromModel(first, userInfo);
        insertItem(i, userInfo);
    }

    if ((isEnabled()) && (ui->userList->selectedItems().size() == 0) && (ui->userList->count() > 0))
    {
        setRow0();
    }

    KLOG_DEBUG() << "row inserted: "
                 << "cout[" << ui->userList->count() << "]";
    updateGeometry();
}

QSize UserListWidget::sizeHint() const
{
    QSize size(0, (ui->userList->count() * 62) + 2);
    KLOG_DEBUG() << "count: " << ui->userList->count() << "size: " << size;
    return size;
}