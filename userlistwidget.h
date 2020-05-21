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
    virtual ~UserListWidget();
    void loadUserList();
    void justForTest(int count);
    void setRow0();
private slots:
    void slotUserItemActivated();
Q_SIGNALS:
    void userActivated(const UserInfo& userInfo);
protected:
    virtual bool eventFilter(QObject *obj, QEvent *event) override;
    virtual QSize sizeHint() const override;
private:
    Ui::UserListWidget *ui;
    QLightDM::UsersModel m_usersModel;
};

#endif // USERLISTWIDGET_H
