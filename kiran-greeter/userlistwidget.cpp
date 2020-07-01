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
    initUI();
}

UserListWidget::~UserListWidget()
{
    delete ui;
}

bool UserListWidget::eventFilter(QObject *obj, QEvent *event)
{
    ///因QListWidget的上下键直接修改当前行，通过屏蔽上下键盘事件修改成切换焦点
    if(obj==ui->listWidget){
        switch (event->type()) {
        case QEvent::KeyPress:
        {
            QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>(event);
            if(qApp->focusWidget()==nullptr||
                    qApp->focusWidget()->objectName()!=USERITEM_OBJ_NAME){
                break;
            }
            UserListItem* userItem = dynamic_cast<UserListItem*>(qApp->focusWidget());
            const QListWidgetItem* listItem = userItem->getListItem();
            int rowIdx = ui->listWidget->row(listItem);
            int rowCount = ui->listWidget->count();
            if(keyEvent->key() == Qt::Key_Up){
                if((rowIdx!=0)&&(rowCount>1)){
                    ui->listWidget->itemWidget(ui->listWidget->item(rowIdx-1))->setFocus(Qt::OtherFocusReason);
                }
                return true;
            }else if(keyEvent->key() == Qt::Key_Down){
                if((rowIdx<(rowCount-1))){
                    ui->listWidget->itemWidget(ui->listWidget->item(rowIdx+1))->setFocus(Qt::OtherFocusReason);
                }
                return true;
            }
            break;
        }
        default:
            break;
        }
    }

    ///聚焦在Item上按Tab直接跳出，不在用户列表上切换
    if( obj!=nullptr && obj->objectName()==USERITEM_OBJ_NAME ){
        switch (event->type()) {
        case QEvent::KeyPress:
        {
            QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>(event);
            if(keyEvent->key()==Qt::Key_Enter||keyEvent->key()==Qt::Key_Return){
                UserListItem* item = dynamic_cast<UserListItem*>(qApp->focusWidget());
                const QListWidgetItem* listItem = item->getListItem();
                int rowIdx = ui->listWidget->row(listItem);
                setCurrentRow(rowIdx);
                return true;
            }else if(keyEvent->key()==Qt::Key_Space){
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

void UserListWidget::initUI()
{
    setAttribute(Qt::WA_Hover,true);
    ui->listWidget->setFocusPolicy(Qt::ClickFocus);
    ui->listWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(ui->listWidget,SIGNAL(itemSelectionChanged()),
            this,SLOT(slotUserItemActivated()));

    ui->listWidget->setVisible(false);
    ui->listWidget->installEventFilter(this);

    /// 连接QApplication的焦点切换信号
    /// 处理ListWidget内部焦点切换或焦点切换出ListWidge，滑动条特殊处理
    /// 处理当焦点从外部到UserItem时，应默认到当前行
    connect(qApp,static_cast<void (QApplication::*)(QWidget*,QWidget*)>(&QApplication::focusChanged),
            this,[this](QWidget*oldWidget,QWidget*newWidget){
        bool oldFocusInList = oldWidget==nullptr?false:oldWidget->objectName()==USERITEM_OBJ_NAME;
        bool newFocusInList = newWidget==nullptr?false:newWidget->objectName()==USERITEM_OBJ_NAME;
        qInfo() << "focus changed:" << (oldWidget?oldWidget->objectName():"null")
                << " -> "  << (newWidget?newWidget->objectName():"null");
        if( !oldFocusInList && !newFocusInList ){
            return;
        }   else if(newFocusInList){///UserItem->UserItem,滚动到焦点行
            UserListItem* userItem = dynamic_cast<UserListItem*>(newWidget);
            const QListWidgetItem* listItem = userItem->getListItem();
            ui->listWidget->scrollToItem(listItem);
        }else if(oldFocusInList){///UserItem->外部，滚动到当前行
            ui->listWidget->scrollToItem(ui->listWidget->currentItem());
        }
    });
}

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
    setCurrentRow(0);
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
    customItem->setListItem(newItem);
    customItem->installEventFilter(this);

    ui->listWidget->addItem(newItem);
    ui->listWidget->setItemWidget(newItem,customItem);
    if( (!ui->listWidget->isVisible()) && (ui->listWidget->count()>=2) ){
        ui->listWidget->setVisible(true);
    }
}

void UserListWidget::insertItem(int row, const UserInfo &userInfo)
{
    QListWidgetItem* newItem = nullptr;
    UserListItem*  customItem = nullptr;

    newItem = new QListWidgetItem;
    newItem->setSizeHint(QSize(0,60));

    customItem = new UserListItem;
    customItem->setUserInfo(userInfo);
    customItem->setListItem(newItem);
    customItem->installEventFilter(this);

    ui->listWidget->insertItem(row,newItem);
    ui->listWidget->setItemWidget(newItem,customItem);
    if( (!ui->listWidget->isVisible()) && (ui->listWidget->count()>=2) ){
        ui->listWidget->setVisible(true);
    }
}

void UserListWidget::JustForTest(int count)
{
    for( int i=0;i<count;i++ ){
        QString testUserName = QString("TestUser%1").arg(i);
        UserInfo userInfo;
        userInfo.name = testUserName;
        appendItem(userInfo);
    }
}

QString UserListWidget::getIconByAccount(const QString &account)
{
    QString iconPath = "";

    for( int i=0;i<m_usersModel.rowCount(QModelIndex());i++){
        UserInfo userInfo;
        getUserInfoFromModel(i,userInfo);
        if(userInfo.name == account){
            iconPath = userInfo.imagePath;
            break;
        }
    }

    return iconPath;
}

void UserListWidget::setCurrentRow(int idx)
{
    if( ui->listWidget->count() > idx ){
        ui->listWidget->setCurrentRow(idx,QItemSelectionModel::ClearAndSelect);
    }
}

void UserListWidget::slotUserItemActivated()
{
    QList<QListWidgetItem*> selectedItems = ui->listWidget->selectedItems();
    if(selectedItems.size() == 0){
        qWarning() << "selected items: 0";
        return;
    }
    QListWidgetItem* activatedItem = selectedItems.at(0);
    UserListItem* userItem = dynamic_cast<UserListItem*>(ui->listWidget->itemWidget(activatedItem));

    userItem->setFocusPolicy(Qt::TabFocus);
    for(int i=0;i<ui->listWidget->count();i++){
        QListWidgetItem* item = ui->listWidget->item(i);
        if(item!=activatedItem){
            UserListItem* uItem = dynamic_cast<UserListItem*>(ui->listWidget->itemWidget(item));
            uItem->setFocusPolicy(Qt::NoFocus);
        }
    }
    qInfo() << userItem->getUserInfo().name << "activate";
    UserInfo info = userItem->getUserInfo();
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
    if( (ui->listWidget->isVisible()) && (ui->listWidget->count()<2) ){
        ui->listWidget->setVisible(false);
    }
    updateGeometry();
}

void UserListWidget::slotRowsInserted(const QModelIndex &parent, int first, int last)
{
    for(int i=first;i<=last;i++){
        UserInfo userInfo;
        getUserInfoFromModel(first,userInfo);
        insertItem(i,userInfo);
    }

    if( (isEnabled()) && (ui->listWidget->selectedItems().size()==0) && (ui->listWidget->count()>0) ){
        setRow0();
    }

    qInfo() << "row inserted: " << "cout[" << ui->listWidget->count() << "]";
    updateGeometry();
}

QSize UserListWidget::sizeHint() const
{
    QSize size(0,(ui->listWidget->count()*62)+2);
    qInfo() << "count: " << ui->listWidget->count() << "size: " << size;
    return size;
}
