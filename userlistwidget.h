#ifndef USERLISTWIDGET_H
#define USERLISTWIDGET_H

#include <QWidget>
#include <QLightDM/UsersModel>
#include "userinfo.h"

class QListWidgetItem;

namespace Ui {
class UserListWidget;
}

class UserListWidget : public QWidget
{
    Q_OBJECT
public:
    explicit UserListWidget(QWidget *parent = nullptr);
    ~UserListWidget();
    void loadUserList();
    void justForTest(int count);
    bool getCurrentSelected(UserInfo& userInfo);
    void setRow0();
private:
    bool getUserInfoFromModel(int row,UserInfo& userInfo);
    void appendItem(const UserInfo& userInfo);
    void insertItem(int row,const UserInfo& userInfo);
private slots:
    void slotUserItemActivated();
    void slotRowsRemoved(const QModelIndex&parent,int first,int last);
    void slotRowsInserted(const QModelIndex&parent,int first,int last);
Q_SIGNALS:
    void userActivated(const UserInfo& userInfo);
    ///请求登录窗口重置窗口
    void sigRequestResetUI();
protected:
    virtual bool eventFilter(QObject *obj, QEvent *event) override;
    virtual QSize sizeHint() const override;
private:
    Ui::UserListWidget *ui;
    QLightDM::UsersModel m_usersModel;
};

#endif // USERLISTWIDGET_H
