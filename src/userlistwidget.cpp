#include "userlistwidget.h"
#include "ui_userlistwidget.h"
#include "userlistitem.h"
#include <QDebug>
#include <QModelIndex>
#include <QMap>
#include <QList>
#include <QScrollBar>
#include "userinfo.h"

using namespace QLightDM;

UserListWidget::UserListWidget (QWidget *parent)
        : QWidget(parent), ui(new Ui::UserListWidget)
{
    ui->setupUi(this);
    initUI();
}

UserListWidget::~UserListWidget ()
{
    delete ui;
}

bool UserListWidget::eventFilter (QObject *obj, QEvent *event)
{
    ///因QListWidget的上下键直接修改当前行，通过屏蔽上下键盘事件修改成切换焦点
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
            default:break;
        }
    }

    ///聚焦在Item上按Tab直接跳出，不在用户列表上切换
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
            default:break;
        }
    }

    return false;
}

int UserListWidget::userCount ()
{
    return ui->userList->count();
}

void UserListWidget::initUI ()
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
    connect(qApp, &QApplication::focusChanged, [this] (QWidget *oldWidget, QWidget *newWidget) {
        bool oldFocusInList = oldWidget == nullptr ? false : oldWidget->objectName() == USERITEM_OBJ_NAME;
        bool newFocusInList = newWidget == nullptr ? false : newWidget->objectName() == USERITEM_OBJ_NAME;
        if (!oldFocusInList && !newFocusInList)
        {
            return;
        }
        else if (newFocusInList)
        {///UserItem->UserItem,滚动到焦点行
            UserListItem *userItem = dynamic_cast<UserListItem *>(newWidget);
            const QListWidgetItem *listItem = userItem->getListItem();
            ui->userList->scrollToItem(listItem);
        }
        else if (oldFocusInList)
        {///UserItem->外部，滚动到当前行
            ui->userList->scrollToItem(ui->userList->currentItem());
        }
    });
}

void UserListWidget::loadUserList ()
{
    for (int i = 0; i < m_usersModel.rowCount(QModelIndex()); i++)
    {
        UserInfo userInfo;
        getUserInfoFromModel(i, userInfo);
        appendItem(userInfo);
    }
    qInfo() << "connect UserModel RowRemoved:  " << connect(&m_usersModel, &QLightDM::UsersModel::rowsRemoved,
                                                            this, &UserListWidget::slotRowsRemoved);
    qInfo() << "connect UserModel RowInserted: " << connect(&m_usersModel, &QLightDM::UsersModel::rowsInserted,
                                                            this, &UserListWidget::slotRowsInserted);
}

bool UserListWidget::getCurrentSelected (UserInfo &userInfo)
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

void UserListWidget::setRow0 ()
{
    setCurrentRow(0);
}

bool UserListWidget::getUserInfoFromModel (int row, UserInfo &userInfo)
{
    QVariant value;

    if (m_usersModel.rowCount(QModelIndex()) < row)
    {
        return false;
    }

    value = m_usersModel.data(m_usersModel.index(row, 0), UsersModel::NameRole);
    userInfo.name = value.toString();

    value = m_usersModel.data(m_usersModel.index(row, 0), UsersModel::RealNameRole);
    userInfo.realName = value.toString();

    value = m_usersModel.data(m_usersModel.index(row, 0), UsersModel::LoggedInRole);
    userInfo.loggedIn = value.toBool();

    value = m_usersModel.data(m_usersModel.index(row, 0), UsersModel::SessionRole);
    userInfo.session = value.toString();

    value = m_usersModel.data(m_usersModel.index(row, 0), UsersModel::HasMessagesRole);
    userInfo.hasMessage = value.toBool();

    value = m_usersModel.data(m_usersModel.index(row, 0), UsersModel::ImagePathRole);
    userInfo.imagePath = value.toString();

    value = m_usersModel.data(m_usersModel.index(row, 0), UsersModel::BackgroundPathRole);
    userInfo.backgroundPath = value.toString();

    value = m_usersModel.data(m_usersModel.index(row, 0), UsersModel::UidRole);
    userInfo.uid = value.toULongLong();
    qInfo() << "getUserInfoFromModel: " << userInfo.name;
    return true;
}

void UserListWidget::appendItem (const UserInfo &userInfo)
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

void UserListWidget::insertItem (int row, const UserInfo &userInfo)
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

void UserListWidget::JustForTest (int count)
{
    for (int i = 0; i < count; i++)
    {
        QString testUserName = QString("TestUser%1").arg(i);
        UserInfo userInfo;
        userInfo.name = testUserName;
        appendItem(userInfo);
    }
}

QString UserListWidget::getIconByAccount (const QString &account)
{
    QString iconPath = "";

    for (int i = 0; i < m_usersModel.rowCount(QModelIndex()); i++)
    {
        UserInfo userInfo;
        getUserInfoFromModel(i, userInfo);
        if (userInfo.name == account)
        {
            iconPath = userInfo.imagePath;
            break;
        }
    }

    return iconPath;
}

void UserListWidget::setCurrentRow (int idx)
{
    if (ui->userList->count() > idx)
    {
        ui->userList->setCurrentRow(idx, QItemSelectionModel::ClearAndSelect);
    }
}

void UserListWidget::slotUserItemActivated ()
{
    QList<QListWidgetItem *> selectedItems = ui->userList->selectedItems();
    if (selectedItems.size() == 0)
    {
        qWarning() << "selected items: 0";
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
    qInfo() << userItem->getUserInfo().name << "activate";
    UserInfo info = userItem->getUserInfo();
    emit userActivated(info);
}

void UserListWidget::slotRowsRemoved (const QModelIndex &parent, int first, int last)
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

void UserListWidget::slotRowsInserted (const QModelIndex &parent, int first, int last)
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

    qInfo() << "row inserted: " << "cout[" << ui->userList->count() << "]";
    updateGeometry();
}

QSize UserListWidget::sizeHint () const
{
    QSize size(0, (ui->userList->count() * 62) + 2);
    qInfo() << "count: " << ui->userList->count() << "size: " << size;
    return size;
}
