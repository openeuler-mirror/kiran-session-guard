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

UserListWidget::UserListWidget(QWidget *parent)
    :QWidget(parent)
    ,ui(new Ui::UserListWidget)
{
    ui->setupUi(this);
    ui->listWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->listWidget->verticalScrollBar()->setVisible(false);
    setAttribute(Qt::WA_Hover,true);
    installEventFilter(this);
    connect(ui->listWidget,SIGNAL(itemSelectionChanged()),
            this,SLOT(slotUserItemActivated()));
}

UserListWidget::~UserListWidget()
{
    delete ui;
}

//TODO:用户变更修改显示
void UserListWidget::loadUserList()
{
    for( int i=0;i<m_usersModel.rowCount(QModelIndex());i++){
        UserInfo userInfo;
        getUserInfoFromModel(i,userInfo);
        appendItem(userInfo);
    }
    qInfo() << "connect UserModel RowRemoved:  " << connect(&m_usersModel,SIGNAL(rowsRemoved(const QModelIndex&,int,int)),
                                                          this,SLOT(slotRowsRemoved(const QModelIndex&,int,int)));
    qInfo() << "connect UserModel RowInserted: " << connect(&m_usersModel,SIGNAL(rowsInserted(const QModelIndex&,int,int)),
                                                          this,SLOT(slotRowsInserted(const QModelIndex&,int,int)));
}

void UserListWidget::justForTest(int count)
{
    QListWidgetItem* listItem = nullptr;
    UserListItem* userItem = nullptr;
    for( int i=0;i<count;i++ ){
        QString testUserName = QString("TestUser%1").arg(i);
        UserInfo userInfo;

        userInfo.name = testUserName;

        listItem = new QListWidgetItem;
        listItem->setSizeHint(QSize(0,60));

        userItem = new UserListItem;
        userItem->setUserInfo(userInfo);

        ui->listWidget->addItem(listItem);
        ui->listWidget->setItemWidget(listItem,userItem);
    }
}

bool UserListWidget::getCurrentSelected(UserInfo &userInfo)
{
    QList<QListWidgetItem*> selectedItem = ui->listWidget->selectedItems();
    if(selectedItem.size() == 0){
        return false;
    }
    UserListItem* item = dynamic_cast<UserListItem*>(ui->listWidget->itemWidget(selectedItem.at(0)));
    UserInfo info = item->getUserInfo();
    userInfo = info;
    return true;
}

void UserListWidget::setRow0()
{
    if( ui->listWidget->count() > 0 ){
        ui->listWidget->setCurrentRow(0,QItemSelectionModel::ClearAndSelect);
    }
}

bool UserListWidget::getUserInfoFromModel(int row,UserInfo& userInfo)
{
    QVariant value;

    if( m_usersModel.rowCount(QModelIndex()) < row ){
        return false;
    }

    value = m_usersModel.data(m_usersModel.index(row,0),UsersModel::NameRole);
    userInfo.name = value.toString();

    value = m_usersModel.data(m_usersModel.index(row,0),UsersModel::RealNameRole);
    userInfo.realName = value.toString();

    value = m_usersModel.data(m_usersModel.index(row,0),UsersModel::LoggedInRole);
    userInfo.loggedIn = value.toBool();

    value = m_usersModel.data(m_usersModel.index(row,0),UsersModel::SessionRole);
    userInfo.session = value.toString();

    value = m_usersModel.data(m_usersModel.index(row,0),UsersModel::HasMessagesRole);
    userInfo.hasMessage = value.toBool();

    value = m_usersModel.data(m_usersModel.index(row,0),UsersModel::ImagePathRole);
    userInfo.imagePath = value.toString();

    value = m_usersModel.data(m_usersModel.index(row,0),UsersModel::BackgroundPathRole);
    userInfo.backgroundPath = value.toString();

    value = m_usersModel.data(m_usersModel.index(row,0),UsersModel::UidRole);
    userInfo.uid = value.toULongLong();
    qInfo() << "getUserInfoFromModel: " << userInfo.name;
    return true;
}

void UserListWidget::appendItem(const UserInfo &userInfo)
{
    QListWidgetItem* newItem = nullptr;
    UserListItem*  customItem = nullptr;

    newItem = new QListWidgetItem;
    newItem->setSizeHint(QSize(0,60));

    customItem = new UserListItem;
    customItem->setUserInfo(userInfo);

    ui->listWidget->addItem(newItem);
    ui->listWidget->setItemWidget(newItem,customItem);
}

void UserListWidget::insertItem(int row, const UserInfo &userInfo)
{
    QListWidgetItem* newItem = nullptr;
    UserListItem*  customItem = nullptr;

    newItem = new QListWidgetItem;
    newItem->setSizeHint(QSize(0,60));

    customItem = new UserListItem;
    customItem->setUserInfo(userInfo);

    ui->listWidget->insertItem(row,newItem);
    ui->listWidget->setItemWidget(newItem,customItem);
}

void UserListWidget::slotUserItemActivated()
{
    QList<QListWidgetItem*> selectedItems = ui->listWidget->selectedItems();

    if(selectedItems.size() == 0){
        qWarning() << "selected items: 0";
        return;
    }
    UserListItem* item = dynamic_cast<UserListItem*>(ui->listWidget->itemWidget(selectedItems.at(0)));
    UserInfo info = item->getUserInfo();
    emit userActivated(info);
}

void UserListWidget::slotRowsRemoved(const QModelIndex &parent, int first, int last)
{
    bool reSelect = false;

    for(int i=last;(i>=first);i--){
        QListWidgetItem* item = ui->listWidget->item(i);
        QList<QListWidgetItem*> selectedItems = ui->listWidget->selectedItems();
        if(isEnabled()&&(selectedItems.size()>0)&&(selectedItems.at(0)==item)){
            reSelect = true;
        }
        QListWidgetItem* removedItem = ui->listWidget->takeItem(i);
        delete removedItem;
    }
    if(reSelect){
        if(ui->listWidget->count()>0){
            setRow0();
        }else{
            emit sigRequestResetUI();
        }
    }
    updateGeometry();
}

void UserListWidget::slotRowsInserted(const QModelIndex &parent, int first, int last)
{
    for(int i=first;i<=last;i++){
        UserInfo userInfo;
        Q_ASSERT(getUserInfoFromModel(first,userInfo));
        insertItem(i,userInfo);
    }

    if( (isEnabled()) && (ui->listWidget->selectedItems().size()==0) && (ui->listWidget->count()>0) ){
        setRow0();
    }

    qInfo() << "row inserted: " << "cout[" << ui->listWidget->count() << "]";
    updateGeometry();
}

bool UserListWidget::eventFilter(QObject *obj, QEvent *event)
{
    if(obj==this){
        switch (event->type()) {
        case QEvent::HoverEnter:
            ui->listWidget->verticalScrollBar()->setVisible(true);
            break;
        case QEvent::HoverLeave:
            ui->listWidget->verticalScrollBar()->setVisible(false);
            break;
        default:
            break;
        }
    }
    return QWidget::eventFilter(obj,event);
}

QSize UserListWidget::sizeHint() const
{
    QSize size(0,(ui->listWidget->count()*62)+2);
    qInfo() << "count: " << ui->listWidget->count() << "size: " << size;
    return size;
}
