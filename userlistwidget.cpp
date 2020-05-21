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
        QVariant value;
        QListWidgetItem* newItem = nullptr;
        UserListItem*  customItem = nullptr;

        value = m_usersModel.data(m_usersModel.index(i,0),UsersModel::NameRole);
        userInfo.name = value.toString();

        value = m_usersModel.data(m_usersModel.index(i,0),UsersModel::RealNameRole);
        userInfo.realName = value.toString();

        value = m_usersModel.data(m_usersModel.index(i,0),UsersModel::LoggedInRole);
        userInfo.loggedIn = value.toBool();

        value = m_usersModel.data(m_usersModel.index(i,0),UsersModel::SessionRole);
        userInfo.session = value.toString();

        value = m_usersModel.data(m_usersModel.index(i,0),UsersModel::HasMessagesRole);
        userInfo.hasMessage = value.toBool();

        value = m_usersModel.data(m_usersModel.index(i,0),UsersModel::ImagePathRole);
        userInfo.imagePath = value.toString();

        value = m_usersModel.data(m_usersModel.index(i,0),UsersModel::BackgroundPathRole);
        userInfo.backgroundPath = value.toString();

        value = m_usersModel.data(m_usersModel.index(i,0),UsersModel::UidRole);
        userInfo.uid = value.toULongLong();

        qInfo() << userInfo.name << "  " << userInfo.imagePath;

        newItem = new QListWidgetItem;
        newItem->setSizeHint(QSize(0,60));

        customItem = new UserListItem;
        customItem->setUserInfo(userInfo);

        ui->listWidget->addItem(newItem);
        ui->listWidget->setItemWidget(newItem,customItem);
    }

#ifdef TEST
    justForTest(1);
#endif
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

void UserListWidget::setRow0()
{
    if( ui->listWidget->count() > 0 ){
        ui->listWidget->setCurrentItem(ui->listWidget->item(0));
        slotUserItemActivated();
    }
}

void UserListWidget::slotUserItemActivated()
{
    Q_ASSERT(ui->listWidget->selectedItems().size()==1);
    UserListItem* item = dynamic_cast<UserListItem*>(ui->listWidget->itemWidget(ui->listWidget->selectedItems().at(0)));
    UserInfo info = item->getUserInfo();
    emit userActivated(info);
}

bool UserListWidget::eventFilter(QObject *obj, QEvent *event)
{
    if( event->type()==QEvent::HoverEnter ){
        ui->listWidget->verticalScrollBar()->setVisible(true);
    }

    if( event->type()==QEvent::HoverLeave ){
        ui->listWidget->verticalScrollBar()->setVisible(false);
    }

    return QWidget::eventFilter(obj,event);
}

QSize UserListWidget::sizeHint() const
{

    return QSize(0,(ui->listWidget->count()*60)+2);
}
